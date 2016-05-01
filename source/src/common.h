/* unofficial gameplaySP kai
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 * Copyright (C) 2007 takka <takka@tfact.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef COMMON_H
#define COMMON_H

#include <kubridge.h>

#include <pspgu.h>
#include <pspdisplay.h>
#include <pspaudio.h>
#include <psprtc.h>

#include <pspimpose_driver.h>
#include <psputility_sysparam.h>

#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#define MAX_PATH (512)
#define MAX_FILE (256)

#define ALIGN_PSPDATA __attribute__((aligned(16)))
#define ALIGN_DATA    __attribute__((aligned(4)))
#define MEM_ALIGN     (4)

#define ROR(dest, value, shift)                                               \
  dest = __builtin_allegrex_rotr(value, shift)                                \

#define LIMIT_MAX(input, maxout)                                              \
  __builtin_allegrex_min(input, maxout)                                       \

#define LIMIT_MIN(input, minout)                                              \
  __builtin_allegrex_max(input, minout)                                       \


#define PSP_FILE_OPEN_READ  "r"
#define PSP_FILE_OPEN_WRITE "w"

#define FILE_OPEN(filename_tag, filename, mode)                               \
  filename_tag = psp_fopen(filename, PSP_FILE_OPEN_##mode)                    \

#define FILE_CHECK_VALID(filename_tag)                                        \
  ((filename_tag) >= 0)                                                       \

#define FILE_CLOSE(filename_tag)                                              \
  psp_fclose(filename_tag)                                                    \

#define FILE_READ(filename_tag, buffer, size)                                 \
  sceIoRead(filename_tag, buffer, size)                                       \

#define FILE_WRITE(filename_tag, buffer, size)                                \
  sceIoWrite(filename_tag, buffer, size)                                      \

// type = FILE_OPEN_READ / FILE_OPEN_WRITE
#define FILE_SEEK(filename_tag, offset, type)                                 \
  sceIoLseek(filename_tag, offset, PSP_##type)                                \

#define FILE_WRITE_MEM(filename_tag, buffer, size)                            \
{                                                                             \
  memcpy(write_mem_ptr, buffer, size);                                        \
  write_mem_ptr += (size);                                                    \
}                                                                             \

// These must be variables, not constants.

#define FILE_READ_VARIABLE(filename_tag, variable)                            \
  FILE_READ(filename_tag, &variable, sizeof(variable))                        \

#define FILE_WRITE_VARIABLE(filename_tag, variable)                           \
  FILE_WRITE(filename_tag, &variable, sizeof(variable))                       \

#define FILE_WRITE_MEM_VARIABLE(filename_tag, variable)                       \
  FILE_WRITE_MEM(filename_tag, &variable, sizeof(variable))                   \

// These must be statically declared arrays (ie, global or on the stack,
// not dynamically allocated on the heap)

#define FILE_READ_ARRAY(filename_tag, array)                                  \
  FILE_READ(filename_tag, array, sizeof(array))                               \

#define FILE_WRITE_ARRAY(filename_tag, array)                                 \
  FILE_WRITE(filename_tag, array, sizeof(array))                              \

#define FILE_WRITE_MEM_ARRAY(filename_tag, array)                             \
  FILE_WRITE_MEM(filename_tag, array, sizeof(array))                          \


typedef u32 FIXED08_24;

#define FLOAT_TO_FP08_24(value)                                               \
  (FIXED08_24)((value) * 16777216.0)                                          \

#define FP08_24_TO_FLOAT(value)                                               \
  (float)((value) / 16777216.0)                                               \

#define U32_TO_FP08_24(value)                                                 \
  ((value) << 24)                                                             \

#define FP08_24_TO_U32(value)                                                 \
  ((value) >> 24)                                                             \

#define FP08_24_FRACTIONAL_PART(value)                                        \
  ((value) & 0x00FFFFFF)                                                      \


#define FIXED_DIV(numerator, denominator, bits)                               \
  (((numerator * (1 << bits)) + (denominator / 2)) / denominator)             \


#define ADDRESS8(base, offset)                                                \
  *((u8 *)((u8 *)(base) + (offset)))                                          \

#define ADDRESS16(base, offset)                                               \
  *((u16 *)((u8 *)(base) + (offset)))                                         \

#define ADDRESS32(base, offset)                                               \
  *((u32 *)((u8 *)(base) + (offset)))                                         \


#include "zlib/zlib.h"
#include "libpower/psppower.h"

#include "zip.h"
#include "png.h"

#include "message.h"

#include "cheats.h"

#include "mh_print.h"
#include "ch_print.h"
#include "gui.h"

#include "cpu.h"
#include "memory.h"
#include "video.h"
#include "input.h"
#include "sound.h"
#include "main.h"

#include "ku_bridge.h"
#include "exception.h"

#endif /* COMMON_H */
