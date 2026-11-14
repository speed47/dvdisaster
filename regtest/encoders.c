/* Encoder backends against an independent polynomial-form shift register.
   Run with make check-encoders. No production log or encoder lookup tables
   are used to calculate the expected parity. */
#include "dvdisaster.h"
#include <assert.h>

static GlobalClosure closure;
GlobalClosure *Closure = &closure;

void Stop(char *format, ...)
{
   va_list args;
   va_start(args, format);
   vfprintf(stderr, format, args);
   va_end(args);
   exit(1);
}

void Verbose(char *format, ...)
{  (void)format;
}

static guint8 multiply(unsigned a, unsigned b)
{
   unsigned result = 0;
   for(int i=0; i<8; i++)
   {  if(b&1) result ^= a;
      b >>= 1;
      a <<= 1;
      if(a&256) a ^= 0x187;
   }
   return result;
}

static guint8 *make_products(int roots)
{
   guint8 generator[256] = {1}, alpha[255];
   guint8 *products = g_malloc(256*roots);
   alpha[0] = 1;
   for(int i=1; i<255; i++) alpha[i] = multiply(alpha[i-1], 2);

   for(int i=0; i<roots; i++)
   {  guint8 c = alpha[((112+i)*11)%255];
      generator[i+1] = generator[i];
      for(int j=i; j>0; j--)
         generator[j] = generator[j-1] ^ multiply(generator[j], c);
      generator[0] = multiply(generator[0], c);
   }
   for(int x=0; x<256; x++)
      for(int j=0; j<roots; j++)
         products[x*roots+j] = multiply(x, generator[j]);
   return products;
}

static unsigned rng = 1234567;
static guint8 random_byte(void)
{
   rng ^= rng<<13; rng ^= rng>>17; rng ^= rng<<5;
   return rng;
}

static unsigned cases;

static void check(ReedSolomonTables *rt, const guint8 *products, int len,
                  int shift, int skew, int pattern, int rounds)
{
   int roots = rt->nroots, stride = (roots+15)&~15;
   const char *name = GetRSEncoderName(Closure->encodingAlgorithm);
   /* Portable integer and AltiVec encoders require aligned parity. */
   int unaligned = !strcmp(name, "NEON") || !strcmp(name, "SSE2") || !strcmp(name, "AVX2");
   size_t prefix = 32 + (unaligned ? skew : 0), bytes = (size_t)len*stride;
   guint8 *raw = g_malloc(prefix+bytes+32), *parity = raw+prefix;
   guint8 *input = g_malloc(len+32), *data = input+skew;
   guint8 *saved = g_malloc(len+32), *expected = g_malloc((size_t)len*roots+1);

   memset(raw, 0xa5, prefix+bytes+32);
   memset(input, 0x5a, len+32);
   for(size_t i=0; i<bytes; i++) parity[i] = pattern==3 ? 0 : random_byte();
   for(int i=0; i<len; i++)
      for(int j=0; j<roots; j++)
         expected[i*roots+j] = parity[i*stride+(shift+j)%roots];

   for(int round=0; round<rounds; round++)
   {
      for(int i=0; i<len; i++)
      {  if(pattern==0) data[i] = parity[i*stride+shift]; /* zero feedback */
         else if(pattern==2) data[i] = parity[i*stride+shift] ^ (guint8)i;
         else data[i] = random_byte();
      }
      memcpy(saved, input, len+32);

      for(int i=0; i<len; i++)
      {  guint8 *p = expected+i*roots;
         const guint8 *product = products+(data[i]^p[0])*roots;
         for(int j=0; j<roots-1; j++) p[j] = p[j+1] ^ product[roots-1-j];
         p[roots-1] = product[0];
      }

      EncodeNextLayer(rt, data, parity, len, shift);
      shift = (shift+1)%roots;
      assert(!memcmp(input, saved, len+32));
      for(size_t i=0; i<prefix; i++) assert(raw[i]==0xa5);
      for(size_t i=0; i<32; i++) assert(parity[bytes+i]==0xa5);
      /* Padding is scratch space and can differ between implementations. */
      for(int i=0; i<len; i++)
         for(int j=0; j<roots; j++)
            if(expected[i*roots+j] != parity[i*stride+(shift+j)%roots])
               Stop("%s mismatch: roots=%d len=%d shift=%d skew=%d pattern=%d round=%d byte=%d root=%d\n",
                    name, roots, len, shift, skew, pattern, round, i, j);
   }
   g_free(raw); g_free(input); g_free(saved); g_free(expected);
   cases++;
}

int main(int argc, char **argv)
{
   assert(argc==2);
   Closure->useSSE2 = ProbeSSE2();
   Closure->useAVX2 = ProbeAVX2();
   Closure->useAltiVec = ProbeAltiVec();
   Closure->useNEON = ProbeNEON();

   if(!strcmp(argv[1], "auto"))
   {  const char *name = Closure->useAVX2 ? "AVX2" : Closure->useSSE2 ? "SSE2" :
                        Closure->useAltiVec ? "AltiVec" : Closure->useNEON ? "NEON" : "64bit";
      Closure->encodingAlgorithm = ENCODING_ALG_DEFAULT;
      assert(!strcmp(GetRSEncoderName(ENCODING_ALG_DEFAULT), name));
   }
   else if(!strcmp(argv[1], "32bit")) Closure->encodingAlgorithm = ENCODING_ALG_32BIT;
   else if(!strcmp(argv[1], "64bit")) Closure->encodingAlgorithm = ENCODING_ALG_64BIT;
   else if(!strcmp(argv[1], "SSE2"))
   {  if(!Closure->useSSE2) return 77;
      Closure->encodingAlgorithm = ENCODING_ALG_SSE2;
   }
   else if(!strcmp(argv[1], "AVX2"))
   {  if(!Closure->useAVX2) return 77;
      Closure->encodingAlgorithm = ENCODING_ALG_AVX2;
   }
   else if(!strcmp(argv[1], "AltiVec"))
   {  if(!Closure->useAltiVec) return 77;
      Closure->encodingAlgorithm = ENCODING_ALG_ALTIVEC;
   }
   else if(!strcmp(argv[1], "NEON"))
   {  if(!Closure->useNEON) return 77;
      Closure->encodingAlgorithm = ENCODING_ALG_NEON;
   }
   else return 1;

   GaloisTables *gt = CreateGaloisTables(RS_GENERATOR_POLY);
   int roots[] = {8,15,16,17,31,32,33,63,64,65,100,127,128,129,169,170};
   for(unsigned r=0; r<G_N_ELEMENTS(roots); r++)
   {  int n = roots[r];
      ReedSolomonTables *rt = CreateReedSolomonTables(gt, RS_FIRST_ROOT, RS_PRIM_ELEM, n);
      guint8 *products = make_products(n);
      /* Every circular shift, including every unaligned table offset. */
      for(int shift=0; shift<n; shift++) check(rt, products, 17, shift, shift%16, shift%3, 1);
      /* All feedback bytes at every input/parity alignment, including tails. */
      for(int skew=0; skew<16; skew++)
      {  check(rt, products, 256, 0, skew, 2, 1);
         check(rt, products, 257, n-1, skew, 2, 1);
      }
      int lengths[] = {0,1,15,16,31,32,33,63,64,65,2048,4097};
      for(unsigned k=0; k<G_N_ELEMENTS(lengths); k++)
         check(rt, products, lengths[k], rt->shiftInit, k%16, k%3, 1);
      /* Complete codewords from zero parity, and more than one shift wrap. */
      check(rt, products, 33, rt->shiftInit, 1, 3, rt->ndata+n+1);
      if(n==8 || n==32 || n==170)
         check(rt, products, 2048, rt->shiftInit, 15, 3, rt->ndata);
      g_free(products);
      FreeReedSolomonTables(rt);
   }
   FreeGaloisTables(gt);
   printf("%s (%s): %u encoder cases passed (independent parity, shifts, feedback bytes, alignment, bounds and complete codewords).\n",
          argv[1], GetRSEncoderName(Closure->encodingAlgorithm), cases);
   return 0;
}
