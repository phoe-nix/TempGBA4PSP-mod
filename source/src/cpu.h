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

#ifndef CPU_H
#define CPU_H

// System mode and user mode are represented as the same here

typedef enum
{
  MODE_USER,
  MODE_IRQ,
  MODE_FIQ,
  MODE_SUPERVISOR,
  MODE_ABORT,
  MODE_UNDEFINED,
  MODE_INVALID
} CPU_MODE_TYPE;

typedef enum
{
  CPU_ALERT_NONE    = 0,
  CPU_ALERT_SMC     = 1,  // bit0 - SMC
  CPU_ALERT_IRQ     = 2,  // bit1 - IRQ
  CPU_ALERT_SMC_IRQ = 3,
  CPU_ALERT_HALT    = 4,  // bit2 - HALT
  CPU_ALERT_TIMER   = 8,  // bit3 - timer counter change.
  CPU_ALERT_DMA     = 16  // bit4 - DMA transfers
} CPU_ALERT_TYPE;

typedef enum
{
  CPU_ACTIVE,
  CPU_HALT,
  CPU_STOP
} CPU_HALT_TYPE;

typedef enum
{
  IRQ_NONE    = 0x0000,
  IRQ_VBLANK  = 0x0001,
  IRQ_HBLANK  = 0x0002,
  IRQ_VCOUNT  = 0x0004,
  IRQ_TIMER0  = 0x0008,
  IRQ_TIMER1  = 0x0010,
  IRQ_TIMER2  = 0x0020,
  IRQ_TIMER3  = 0x0040,
  IRQ_SERIAL  = 0x0080,
  IRQ_DMA0    = 0x0100,
  IRQ_DMA1    = 0x0200,
  IRQ_DMA2    = 0x0400,
  IRQ_DMA3    = 0x0800,
  IRQ_KEYPAD  = 0x1000,
  IRQ_GAMEPAK = 0x2000,
} IRQ_TYPE;

typedef enum
{
  REG_SP            = 13,
  REG_LR            = 14,
  REG_PC            = 15,
  REG_N_FLAG        = 16,
  REG_Z_FLAG        = 17,
  REG_C_FLAG        = 18,
  REG_V_FLAG        = 19,
  REG_CPSR          = 20,
  REG_SAVE          = 21,
  REG_SAVE2         = 22,
  REG_SAVE3         = 23,

  CPU_MODE          = 29,
  CPU_HALT_STATE    = 30,
  CHANGED_PC_STATUS = 31,

  EXECUTE_CYCLES    = 60,

  CPU_DMA_HACK      = 61,
  CPU_DMA_LAST      = 62,

  REG_TMP           = 63

} EXT_REG_NUMBERS;

typedef enum
{
  TRANSLATION_REGION_READONLY,
  TRANSLATION_REGION_WRITABLE
} TRANSLATION_REGION_TYPE;

#define TRANSLATION_REGION_COUNT 2

typedef enum {
  METADATA_AREA_BIOS,
  METADATA_AREA_EWRAM,
  METADATA_AREA_IWRAM,
  METADATA_AREA_VRAM,
  METADATA_AREA_ROM
} METADATA_AREA_TYPE;

#define METADATA_AREA_COUNT 5

typedef enum {
  /* Both caches are being thoroughly flushed during initialisation. */
  FLUSH_REASON_INITIALIZING,
  /* The read-only code cache is being cleared after a new ROM has been
   * loaded. */
  FLUSH_REASON_LOADING_ROM,
  /* The writable code cache is being cleared in response to the read-only one
   * being flushed. The recompiler will have patched the branch addresses
   * towards the read-only area, and they have become invalid. */
  FLUSH_REASON_NATIVE_BRANCHING,
  /* One of the caches is being flushed because it's full of code. */
  FLUSH_REASON_FULL_CACHE
} CACHE_FLUSH_REASON_TYPE;

#define CACHE_FLUSH_REASON_COUNT 4

typedef enum {
  /* All metadata areas are being thoroughly cleared during initialisation. */
  CLEAR_REASON_INITIALIZING,
  /* All metadata areas are being cleared after a new ROM has been loaded. */
  CLEAR_REASON_LOADING_ROM,
  /* Writable metadata areas are being cleared in response to a read-only
   * code cache being flushed. The recompiler will have patched the branch
   * addresses towards the read-only area, and they have become invalid. */
  CLEAR_REASON_NATIVE_BRANCHING,
  /* All read-only metadata areas or all writable metadata areas are being
   * cleared due to its code cache being full of code. (If that cache
   * is for the read-only area, the writable one will follow suit.) */
  CLEAR_REASON_FULL_CACHE,
  /* One metadata area is being cleared before assigning a tag in it because
   * the tag that would be assigned to the code exceeds MAX_TAG. */
  CLEAR_REASON_LAST_TAG,
  /* Writable metadata areas are being flushed because a saved state has been
   * loaded. */
  CLEAR_REASON_LOADING_STATE
} METADATA_CLEAR_REASON_TYPE;

#define METADATA_CLEAR_REASON_COUNT 6


void set_cpu_mode(CPU_MODE_TYPE new_mode);

u32 execute_load_u8(u32 address);
u32 execute_load_u16(u32 address);
u32 execute_load_u32(u32 address);
u32 execute_load_s8(u32 address);
u32 execute_load_s16(u32 address);
void execute_store_u8(u32 address, u32 source);
void execute_store_u16(u32 address, u32 source);
void execute_store_u32(u32 address, u32 source);
void execute_arm_translate(u32 cycles);

void invalidate_all_cache(void);
void invalidate_icache_region(u8 *addr, u32 length);

u8 *block_lookup_address_arm(u32 pc);
u8 *block_lookup_address_thumb(u32 pc);
u8 *block_lookup_address_dual(u32 pc);


#define MAX_IDLE_LOOPS 8

extern s32 idle_loop_targets;
extern u32 idle_loop_target_pc[MAX_IDLE_LOOPS];

extern u32 iwram_stack_optimize;

void partial_clear_metadata(u32 offset, u8 region);
void flush_translation_cache(TRANSLATION_REGION_TYPE translation_region, CACHE_FLUSH_REASON_TYPE flush_reason);
void clear_metadata_area(METADATA_AREA_TYPE metadata_area, METADATA_CLEAR_REASON_TYPE clear_reason);
void dump_translation_cache(void);

extern u32 reg[64];

extern u32 reg_mode[7][7];
extern u32 spsr[7];

void init_cpu(void);

void cpu_write_mem_savestate(SceUID savestate_file);
void cpu_read_savestate(SceUID savestate_file);


#define ARM_IRQ_STATE  ((reg[REG_CPSR] & 0x80) == 0)

#endif /* CPU_H */
