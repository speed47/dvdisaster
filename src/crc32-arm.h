/* Linux AArch64 CRC-32 accelerator (IEEE polynomial, not CRC-32C).
 * This file is part of dvdisaster, distributed under GPL-3.0-or-later.
 * Kept separate so configure can compile the exact accelerator without
 * executing optional instructions on the build machine.
 */

#ifndef DVDISASTER_CRC32_ARM_H
#define DVDISASTER_CRC32_ARM_H

#if !defined(__linux__) || !defined(__aarch64__)
#error "The ARM CRC32 backend requires Linux AArch64"
#endif

#include <stdint.h>
#include <string.h>
#include <sys/auxv.h>
#include <asm/hwcap.h>
#include <arm_acle.h>

static int crc32_arm_available(void)
{
   return (getauxval(AT_HWCAP) & HWCAP_CRC32) != 0;
}

/* Limit optional instructions to this function, including with LTO. The
 * caller must first check crc32_arm_available(). Do not build the whole
 * application with -march=armv8-a+crc for a portable distribution binary.
 */
#ifdef __clang__
__attribute__((target("crc"), noinline))
#else
__attribute__((target("+crc"), noinline))
#endif
static uint32_t crc32_arm(const unsigned char *data, int len)
{  uint32_t crc = UINT32_C(0xffffffff);

   for(; len >= 8; data += 8, len -= 8)
   {  uint64_t word;

      /* memcpy permits unaligned buffers without violating aliasing rules.
       * CRC32X consumes the least significant byte first on either endian.
       */
      memcpy(&word, data, sizeof(word));
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      word = __builtin_bswap64(word);
#endif
      crc = __crc32d(crc, word);
   }

   while(len--)
      crc = __crc32b(crc, *data++);

   /* Crc32() applies the on-disk byte order. There is no final complement. */
   return crc;
}

#endif
