/* Focused CRC32 equivalence, bounds, first-use and timing checks.
 * This file is part of dvdisaster, distributed under GPL-3.0-or-later.
 * CRC32_ARM_STANDALONE tests the exact ARM kernel without target GLib;
 * the normal make target also tests Crc32() dispatch and software fallback.
 */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

static uint32_t disk_order(uint32_t crc)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
   crc = __builtin_bswap32(crc);
#endif
   return crc;
}

#ifdef CRC32_ARM_STANDALONE
#include "../src/crc32-arm.h"
#define HAVE_ARM_CRC32 1
static uint32_t Crc32(unsigned char *data, int len)
{
   return disk_order(crc32_arm(data, len));
}
#else
#include "../src/dvdisaster.h"
/* Only this endian helper is needed from endian.c; its other functions
 * require application logging. Use GLib's equivalent for this unit test.
 */
guint32 SwapBytes32(guint32 crc) { return GUINT32_SWAP_LE_BE(crc); }
#if defined(HAVE_ARM_CRC32) && defined(CRC32_TEST_NO_ARM_CRC32)
/* Exercise production dispatch on a CPU reporting no CRC extension, even
 * when this test runs on CRC-capable hardware. Keep this override in tests. */
#include <sys/auxv.h>
#define getauxval(type) 0UL
#endif
#include "../src/crc32.c"
#if defined(HAVE_ARM_CRC32) && defined(CRC32_TEST_NO_ARM_CRC32)
#undef getauxval
#endif
#endif

static int have_arm;
static unsigned long cases;

static uint32_t reference(const unsigned char *data, int len)
{  uint32_t crc = UINT32_C(0xffffffff);
   int i;

   while(len--)
   {  crc ^= *data++;
      for(i=0; i<8; i++)
         crc = (crc >> 1) ^ (UINT32_C(0xedb88320) & (0u - (crc & 1)));
   }
   return crc;
}

static void check(unsigned char *data, int len)
{  uint32_t expected = reference(data, len);

   if(Crc32(data, len) != disk_order(expected))
   {  fprintf(stderr, "Crc32 mismatch at length %d\n", len);
      exit(1);
   }
#ifndef CRC32_ARM_STANDALONE
   if(crc32_software(data, len) != expected)
   {  fprintf(stderr, "Software CRC mismatch at length %d\n", len);
      exit(1);
   }
#endif
#ifdef HAVE_ARM_CRC32
   if(have_arm && crc32_arm(data, len) != expected)
   {  fprintf(stderr, "ARM CRC mismatch at length %d\n", len);
      exit(1);
   }
#endif
   cases++;
}

enum { NTHREADS = 8 };
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int ready, go;

static void *first_call(void *arg)
{  unsigned char data[2048];
   int id = *(int *)arg, i;
   uint32_t expected;

   for(i=0; i<(int)sizeof(data); i++) data[i] = (unsigned char)(197*i + id);
   expected = disk_order(reference(data, sizeof(data)));
   pthread_mutex_lock(&lock);
   ready++;
   pthread_cond_broadcast(&cond);
   while(!go) pthread_cond_wait(&cond, &lock);
   pthread_mutex_unlock(&lock);
   for(i=0; i<1000; i++)
      if(Crc32(data, sizeof(data)) != expected) abort();
   return NULL;
}

static void concurrent_first_use(void)
{  pthread_t thread[NTHREADS];
   int id[NTHREADS], i;

   for(i=0; i<NTHREADS; i++)
   {  id[i] = i;
      if(pthread_create(&thread[i], NULL, first_call, &id[i])) abort();
   }
   pthread_mutex_lock(&lock);
   while(ready != NTHREADS) pthread_cond_wait(&cond, &lock);
   go = 1;
   pthread_cond_broadcast(&cond);
   pthread_mutex_unlock(&lock);
   for(i=0; i<NTHREADS; i++)
      if(pthread_join(thread[i], NULL)) abort();
   puts("PASS: 8 concurrent first callers, 8000 checks");
}

static uint32_t random_state = UINT32_C(0x12345678);
static uint32_t random32(void)
{
   random_state ^= random_state << 13;
   random_state ^= random_state >> 17;
   random_state ^= random_state << 5;
   return random_state;
}

static double seconds(void)
{  struct timespec t;
   if(clock_gettime(CLOCK_MONOTONIC, &t)) abort();
   return t.tv_sec + t.tv_nsec / 1e9;
}

static void benchmark(void)
{
#if defined(HAVE_ARM_CRC32) && !defined(CRC32_ARM_STANDALONE)
   unsigned char data[4096];
   double elapsed[2], begin;
   uint32_t sum[2];
   int len, implementation, i;

   if(!have_arm) return;
   for(len=2048; len<=4096; len*=2)
   {  for(implementation=0; implementation<2; implementation++)
      {  memset(data, 0x5a, sizeof(data));
         sum[implementation] = 0;
         begin = seconds();
         for(i=0; i<100000; i++)
         {  data[i % len] ^= (unsigned char)(i | 1);
            sum[implementation] += implementation ? crc32_arm(data, len)
                                                 : crc32_software(data, len);
         }
         elapsed[implementation] = seconds() - begin;
      }
      if(sum[0] != sum[1]) abort();
      printf("%d bytes: software %.3fs, ARM %.3fs, %.2fx speedup\n",
             len, elapsed[0], elapsed[1], elapsed[0] / elapsed[1]);
   }
#endif
}

int main(int argc, char **argv)
{  unsigned char *data, *mapping;
   size_t accessible, page;
   int n, offset, i, pattern;
   double begin = seconds();
   int require_arm = argc == 2 && !strcmp(argv[1], "--require-arm");

   if(argc > 2 || (argc == 2 && !require_arm))
   {  fprintf(stderr, "Usage: %s [--require-arm]\n", argv[0]);
      return 1;
   }
#ifdef HAVE_ARM_CRC32
   have_arm = crc32_arm_available();
#endif
#ifdef CRC32_ARM_STANDALONE
   require_arm = 1;
   puts("Standalone ARM kernel test (application dispatch not included)");
#endif
   if(require_arm && !have_arm)
   {  fprintf(stderr, "ARM CRC32 unavailable: compiler/backend or CPU capability missing\n");
      return 1;
   }

   /* No Crc32() calls before these threads: exercise lazy initialization. */
   concurrent_first_use();
#if defined(HAVE_ARM_CRC32) && !defined(CRC32_ARM_STANDALONE)
   if(crc32_use_arm() != have_arm) abort();
#endif
   printf("Selected backend: %s\n", have_arm ? "ARM CRC32" : "software slicing-by-8");

   check(NULL, 0);
   check((unsigned char *)"123456789", 9);
   if(Crc32((unsigned char *)"123456789", 9) != disk_order(UINT32_C(0x340bc6d9)))
      abort();
   data = malloc(65536 + 64);
   if(!data) abort();
   for(i=0; i<65536+64; i++) data[i] = (unsigned char)random32();
   for(offset=0; offset<16; offset++)
   {  for(n=0; n<=1024; n++) check(data + offset, n);
      for(n=2040; n<=2056; n++) check(data + offset, n);
      for(n=4088; n<=4104; n++) check(data + offset, n);
      for(n=65528; n<=65536; n++) check(data + offset, n);
   }
   for(i=0; i<10000; i++)
   {  n = random32() % 8193;
      offset = random32() % 64;
      for(int j=0; j<n; j++) data[offset+j] = (unsigned char)random32();
      check(data + offset, n);
   }
   for(pattern=0; pattern<3; pattern++)
   {  for(i=0; i<65536+64; i++)
         data[i] = pattern == 0 ? 0 : pattern == 1 ? 255 : (unsigned char)i;
      for(n=0; n<=32; n++) check(data, n);
      check(data+1, 2048);
      check(data+3, 4096);
      check(data+7, 65536);
   }
   free(data);

   page = (size_t)sysconf(_SC_PAGESIZE);
   accessible = ((8192 + page - 1) / page) * page;
   mapping = mmap(NULL, accessible + page, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if(mapping == MAP_FAILED || mprotect(mapping + accessible, page, PROT_NONE))
      abort();
   for(size_t j=0; j<accessible; j++) mapping[j] = (unsigned char)random32();
   for(n=0; n<=8192; n++) check(mapping + accessible - n, n);
   munmap(mapping, accessible + page);

   printf("PASS: %lu equivalence/bounds cases in %.2fs\n", cases, seconds() - begin);
   benchmark();
   return 0;
}
