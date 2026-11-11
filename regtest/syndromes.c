/* Syndrome backend regression test. Run with make check-syndromes.
   Includes the decoder to exercise runtime dispatch with restricted CPU
   features, without adding test switches to the application. */
#include "dvdisaster.h"
#include <assert.h>

static int cpu_features;
static int test_cpu_supports(const char *feature) G_GNUC_UNUSED;
static int test_cpu_supports(const char *feature)
{
   if(!strcmp(feature, "avx2")) return cpu_features & 1;
   if(!strcmp(feature, "gfni")) return cpu_features & 2;
   return 0;
}
#define __builtin_cpu_supports(feature) test_cpu_supports(feature)
#include "../src/rs-decoder.c"
#undef __builtin_cpu_supports

void Stop(char *format, ...)
{
   va_list args;
   va_start(args, format);
   vfprintf(stderr, format, args);
   va_end(args);
   exit(1);
}

/* Independent polynomial multiplication, without the production log tables. */
static guint8 multiply(unsigned a, unsigned b)
{
   unsigned result = 0;
   for(int i=0; i<8; i++)
   {
      if(b&1) result ^= a;
      b >>= 1;
      a <<= 1;
      if(a&256) a ^= 0x187;
   }
   return result;
}

static guint8 coefficient(int root)
{
   guint8 c = 1;
   for(int i=0; i<((RS_FIRST_ROOT+root)*RS_PRIM_ELEM)%255; i++)
      c = multiply(c, 2);
   return c;
}

#ifdef HAVE_GFNI
__attribute__((target("avx2,gfni")))
static void check_matrices(ReedSolomonTables *rt)
{
   guint8 input[32], output[32];
   for(int root=0; root<255; root++)
   {
      guint8 c = coefficient(root);
      __m256i matrix = _mm256_set1_epi64x(rt->synMatrix[root]);
      for(int base=0; base<256; base+=32)
      {
         for(int k=0; k<32; k++) input[k] = base+k;
         _mm256_storeu_si256((__m256i*)output,
            _mm256_gf2p8affine_epi64_epi8(_mm256_loadu_si256((const __m256i*)input), matrix, 0));
         for(int k=0; k<32; k++)
            assert(output[k] == multiply(c, input[k]));
      }
   }
   puts("All 255 nonzero coefficients x 256 input bytes passed.");
}
#endif

static unsigned rng = 1234567;
static guint8 random_byte(void)
{
   rng ^= rng<<13; rng ^= rng>>17; rng ^= rng<<5;
   return rng;
}

static void check(GaloisTables *gt, int roots, int offset, int pattern)
{
   unsigned char *storage[255], *layer[255];
   ReedSolomonTables *rt = CreateReedSolomonTables(gt, RS_FIRST_ROOT, RS_PRIM_ELEM, roots);
   size_t size = roots*2048;
   guint8 *raw = g_malloc(size+31), *syn = raw+31;
   guint8 expected[2048], products[256];

   memset(raw, 0xa5, size+31);
   for(int j=0; j<255; j++)
   {
      int skew = j%32;
      storage[j] = g_malloc(offset+2048+skew);
      layer[j] = storage[j]+skew;
      for(int k=0; k<offset+2048; k++)
         layer[j][k] = pattern==0 ? 0 : pattern==1 ? 255 : pattern==2 ? (guint8)(j*17+k) : random_byte();
   }
   CalcSyndromes(rt, layer, offset, syn);
   for(int i=0; i<roots; i++)
   {
      guint8 c = coefficient(i);
      for(int x=0; x<256; x++) products[x] = multiply(c, x);
      memcpy(expected, layer[0]+offset, 2048);
      for(int j=1; j<255; j++)
         for(int k=0; k<2048; k++)
            expected[k] = layer[j][offset+k] ^ products[expected[k]];
      assert(!memcmp(syn+i*2048, expected, 2048));
   }
   for(int i=0; i<31; i++) assert(raw[i] == 0xa5);
   for(int j=0; j<255; j++) g_free(storage[j]);
   g_free(raw);
   FreeReedSolomonTables(rt);
}

int main(int argc, char **argv)
{
   int available = 0;
   assert(argc == 2);
#if defined(HAVE_AVX2) || defined(HAVE_GFNI)
   if(__builtin_cpu_supports("avx2")) available |= 1;
#endif
#ifdef HAVE_GFNI
   if(__builtin_cpu_supports("gfni")) available |= 2;
#endif
   if(!strcmp(argv[1], "gfni"))
   {
#ifdef HAVE_GFNI
      if((available & 3) != 3) return 77;
      cpu_features = 3;
#else
      return 77;
#endif
   }
   else if(!strcmp(argv[1], "avx2"))
   {
#ifdef HAVE_AVX2
      if(!(available & 1)) return 77;
      cpu_features = 1;
#else
      return 77;
#endif
   }
   else if(strcmp(argv[1], "fallback")) return 1;
   (void)available;

   GaloisTables *gt = CreateGaloisTables(RS_GENERATOR_POLY);
#ifdef HAVE_GFNI
   if(cpu_features == 3)
   {
      ReedSolomonTables *rt = CreateReedSolomonTables(gt, RS_FIRST_ROOT, RS_PRIM_ELEM, 255);
      check_matrices(rt);
      FreeReedSolomonTables(rt);
   }
#endif
   int roots[] = {8,16,17,32,33,64,170};
   int offsets[] = {0,1,15,31,63,2048,2049};
   for(int pattern=0; pattern<4; pattern++)
      for(unsigned i=0; i<G_N_ELEMENTS(roots); i++) check(gt, roots[i], offsets[i], pattern);
   FreeGaloisTables(gt);
   printf("%s: 28 syndrome cases passed (independent reference, unaligned buffers, nonzero offsets).\n", argv[1]);
   return 0;
}
