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

#include "common.h"


#define CONFIG_FILENAME  "game_config.txt"


u8 ALIGN_DATA memory_waitstate_n[2][16] =
{
  { 0, 0, 2, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0 }, // 8,16bit accesses
  { 0, 0, 5, 0, 0, 1, 1, 1, 7, 7, 9, 9,13,13, 4, 0 }  // 32bit accesses
};

u8 ALIGN_DATA memory_waitstate_s[2][16] =
{
  { 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 4, 4, 8, 8, 4, 0 },
  { 0, 0, 5, 0, 0, 1, 1, 1, 5, 5, 9, 9,17,17, 4, 0 }
};

u8 ALIGN_DATA fetch_waitstate_n[2][16] =
{
  { 0, 0, 2, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0 },
  { 0, 0, 5, 0, 0, 1, 1, 1, 7, 7, 9, 9,13,13, 4, 0 }
};

u8 ALIGN_DATA fetch_waitstate_s[2][16] =
{
  { 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 4, 4, 8, 8, 4, 0 },
  { 0, 0, 5, 0, 0, 1, 1, 1, 5, 5, 9, 9,17,17, 4, 0 }
};

// GBA memory areas.

u16 palette_ram   [  0x200]; // Palette RAM             (05000000h)      1 KiB
u16 oam_ram       [  0x200]; // Object Attribute Memory (07000000h)      1 KiB
u16 io_registers  [  0x400]; // I/O Registers           (04000000h)      1 KiB + 1KiB (io_readable)
u8  ewram         [0x40000]; // External Working RAM    (02000000h)    256 KiB
u8  iwram         [ 0x8000]; // Internal Working RAM    (03000000h)     32 KiB
u8  vram          [0x18000]; // Video RAM               (06000000h)     96 KiB
struct BiosData bios;        // BIOS ROM and code tags  (00000000h)     48 KiB
                             // ----------------------------------------------
                             // Total                                  594 KiB
/*
 * These are Metadata Areas corresponding to the Data Areas above. They
 * contain information about the native code compilation status of each
 * Data Word in that Data Area. For more information about these, see
 * "doc/partial flushing of RAM code.txt" in your source tree.
 */
u16 ewram_metadata[0x40000]; // External Working RAM code metadata     512 KiB
u16 iwram_metadata[ 0x8000]; // Internal Working RAM code metadata      64 KiB
u16 vram_metadata [0x18000]; // Video RAM code metadata                192 KiB
                             // ----------------------------------------------
                             // Total                                  768 KiB

u32 bios_read_protect;

u32 iwram_control;
u8 *io_readable = (u8 *)io_registers + 0x400;

u32 obj_address = 0x10000; // OBJ Tiles Address

DmaTransferType ALIGN_DATA dma[4];
const ALIGN_DATA s32 dma_addr_control[4] = { 2, -2, 0, 2 };


// Keeps us knowing how much we have left.
u8 *gamepak_rom;
u32 gamepak_size;

// Picks a page to evict
u32 page_time = 0;

typedef struct
{
  u32 page_timestamp;
  u32 physical_index;
} GamepakSwapEntryType;

u32 gamepak_ram_buffer_size;
u32 gamepak_ram_pages;

// Enough to map the gamepak RAM space.
GamepakSwapEntryType *gamepak_memory_map;

// This is global so that it can be kept open for large ROMs to swap
// pages from, so there's no slowdown with opening and closing the file
// a lot.

SceUID gamepak_file_large = -1;

// Writes to these respective locations should trigger an update
// so the related subsystem may react to it.

// If OAM is written to:
u32 oam_update = 1;


// RTC
typedef enum
{
  RTC_DISABLED,
  RTC_IDLE,
  RTC_COMMAND,
  RTC_OUTPUT_DATA,
  RTC_INPUT_DATA
} RTC_STATE_TYPE;

typedef enum
{
  RTC_COMMAND_RESET            = 0x60,
  RTC_COMMAND_WRITE_STATUS     = 0x62,
  RTC_COMMAND_READ_STATUS      = 0x63,
  RTC_COMMAND_OUTPUT_TIME_FULL = 0x65,
  RTC_COMMAND_OUTPUT_TIME      = 0x67
} RTC_COMMAND_TYPE;

typedef enum
{
  RTC_WRITE_TIME,
  RTC_WRITE_TIME_FULL,
  RTC_WRITE_STATUS
} RTC_WRITE_MODE_TYPE;

RTC_STATE_TYPE rtc_state = RTC_DISABLED;
RTC_WRITE_MODE_TYPE rtc_write_mode;

u16 ALIGN_DATA rtc_registers[3];
u32 ALIGN_DATA rtc_data[12];
u32 rtc_command;
u32 rtc_status = 0x40;
u32 rtc_data_bytes;
s32 rtc_bit_count;


// Up to 128kb, store SRAM, flash ROM, or EEPROM here.
static u8 ALIGN_DATA gamepak_backup[1024 * 128];

// Write out backup file this many cycles after the most recent
// backup write.
#define WRITE_BACKUP_DELAY  (10)

// If the backup space is written (only update once this hits 0)
s32 backup_update = WRITE_BACKUP_DELAY + 1;

typedef enum
{
  BACKUP_SRAM,
  BACKUP_FLASH,
  BACKUP_EEPROM,
  BACKUP_NONE
} BACKUP_TYPE_TYPE;

typedef enum
{
  SRAM_SIZE_32KB = 0x08000,
  SRAM_SIZE_64KB = 0x10000
} SRAM_SIZE_TYPE;

// Keep it 32KB until the upper 64KB is accessed, then make it 64KB.

BACKUP_TYPE_TYPE backup_type = BACKUP_NONE;
SRAM_SIZE_TYPE sram_size = SRAM_SIZE_32KB;

typedef enum
{
  FLASH_BASE_MODE,
  FLASH_ERASE_MODE,
  FLASH_ID_MODE,
  FLASH_WRITE_MODE,
  FLASH_BANKSWITCH_MODE
} FLASH_MODE_TYPE;

typedef enum
{
  FLASH_SIZE_64KB  = 0x10000,
  FLASH_SIZE_128KB = 0x20000
} FLASH_SIZE_TYPE;

typedef enum
{
  FLASH_DEVICE_MACRONIX_64KB   = 0x1C,
  FLASH_DEVICE_ATMEL_64KB      = 0x3D,
  FLASH_DEVICE_SST_64K         = 0xD4,
  FLASH_DEVICE_PANASONIC_64KB  = 0x1B,
  FLASH_DEVICE_SANYO_128KB     = 0x13,
  FLASH_DEVICE_MACRONIX_128KB  = 0x09
} FLASH_DEVICE_ID_TYPE;

typedef enum
{
  FLASH_MANUFACTURER_MACRONIX  = 0xC2,
  FLASH_MANUFACTURER_ATMEL     = 0x1F,
  FLASH_MANUFACTURER_PANASONIC = 0x32,
  FLASH_MANUFACTURER_SST       = 0xBF, // sanyo or sst
  FLASH_MANUFACTURER_SANYO     = 0x62
} FLASH_MANUFACTURER_ID_TYPE;

FLASH_MODE_TYPE flash_mode = FLASH_BASE_MODE;
u32 flash_command_position = 0;
u32 flash_bank = 0;

FLASH_DEVICE_ID_TYPE flash_device_id = FLASH_DEVICE_PANASONIC_64KB;
FLASH_MANUFACTURER_ID_TYPE flash_manufacturer_id = FLASH_MANUFACTURER_PANASONIC;
FLASH_SIZE_TYPE flash_size = FLASH_SIZE_64KB;

typedef enum
{
  EEPROM_512_BYTE = 0x0200,
  EEPROM_8_KBYTE  = 0x2000
} EEPROM_SIZE_TYPE;

typedef enum
{
  EEPROM_BASE_MODE,
  EEPROM_READ_MODE,
  EEPROM_READ_HEADER_MODE,
  EEPROM_ADDRESS_MODE,
  EEPROM_WRITE_MODE,
  EEPROM_WRITE_ADDRESS_MODE,
  EEPROM_ADDRESS_FOOTER_MODE,
  EEPROM_WRITE_FOOTER_MODE
} EEPROM_MODE_TYPE;

EEPROM_SIZE_TYPE eeprom_size = EEPROM_512_BYTE;
EEPROM_MODE_TYPE eeprom_mode = EEPROM_BASE_MODE;
u32 eeprom_address_length;
u32 eeprom_address = 0;
u32 eeprom_counter = 0;
u8 ALIGN_DATA eeprom_buffer[8];


// SIO
typedef enum
{
  NORMAL8,
  NORMAL32,
  MULTIPLAYER,
  UART,
  GP,
  JOYBUS
} SIO_MODE_TYPE;

static SIO_MODE_TYPE sio_mode(u16 reg_sio_cnt, u16 reg_rcnt);
static CPU_ALERT_TYPE sio_control(u32 value);

static void waitstate_control(u32 value);

static char *skip_spaces(char *line_ptr);
static s32 load_game_config(char *gamepak_title, char *gamepak_code, char *gamepak_maker);

char gamepak_filename[MAX_FILE];
char gamepak_filename_raw[MAX_FILE];

static void init_memory_gamepak(void);

static s32 load_gamepak_raw(char *name);
static u32 evict_gamepak_page(void);

char backup_id[16];
static void load_backup_id(void);

char backup_filename[MAX_FILE];
static u32 save_backup(char *name);

static u32 encode_bcd(u8 value);

#define SAVESTATE_SIZE  0x80000 // 512K Byte (524288 Byte)
u8 *write_mem_ptr;
static void memory_write_mem_savestate(SceUID savestate_file);
static void memory_read_savestate(SceUID savestate_file);


u8 *read_rom_block = NULL;
u8 *read_ram_block = NULL;

u32 read_rom_region = 0xFFFFFFFF;
u32 read_ram_region = 0xFFFFFFFF;

inline static CPU_ALERT_TYPE check_smc_write(u16 *metadata, u32 offset, u8 region);

static u32 read_null(u32 address);

static u32 read8_ram(u32 address);
static u32 read16_ram(u32 address);
static u32 read32_ram(u32 address);

static u32 read8_bios(u32 address);
// static u32 read8_ewram(u32 address);
// static u32 read8_iwram(u32 address);
static u32 read8_io_registers(u32 address);
static u32 read8_palette_ram(u32 address);
// static u32 read8_vram(u32 address);
static u32 read8_oam_ram(u32 address);
static u32 read8_gamepak(u32 address);
static u32 read8_backup(u32 address);
static u32 read8_open(u32 address);

static u32 read16_bios(u32 address);
// static u32 read16_ewram(u32 address);
// static u32 read16_iwram(u32 address);
static u32 read16_io_registers(u32 address);
static u32 read16_palette_ram(u32 address);
// static u32 read16_vram(u32 address);
static u32 read16_oam_ram(u32 address);
static u32 read16_eeprom(u32 address);
static u32 read16_gamepak(u32 address);
static u32 read16_backup(u32 address);
static u32 read16_open(u32 address);

static u32 read32_bios(u32 address);
// static u32 read32_ewram(u32 address);
// static u32 read32_iwram(u32 address);
static u32 read32_io_registers(u32 address);
static u32 read32_palette_ram(u32 address);
// static u32 read32_vram(u32 address);
static u32 read32_oam_ram(u32 address);
static u32 read32_gamepak(u32 address);
static u32 read32_backup(u32 address);
static u32 read32_open(u32 address);

static u32 (*mem_read8[16])(u32) =
{
  read8_bios,          // 0
  read8_open,          // 1
  read8_ram,           // 2
  read8_ram,           // 3
  read8_io_registers,  // 4
  read8_palette_ram,   // 5
  read8_ram,           // 6
  read8_oam_ram,       // 7
  read8_gamepak,       // 8
  read8_gamepak,       // 9
  read8_gamepak,       // a
  read8_gamepak,       // b
  read8_gamepak,       // c
  read8_gamepak,       // d
  read8_backup,        // e
  read8_open           // f
};

static u32 (*mem_read16[16])(u32) =
{
  read16_bios,         // 0
  read16_open,         // 1
  read16_ram,          // 2
  read16_ram,          // 3
  read16_io_registers, // 4
  read16_palette_ram,  // 5
  read16_ram,          // 6
  read16_oam_ram,      // 7
  read16_gamepak,      // 8
  read16_gamepak,      // 9
  read16_gamepak,      // a
  read16_gamepak,      // b
  read16_gamepak,      // c
  read16_eeprom,       // d
  read16_backup,       // e
  read16_open          // f
};

static u32 (*mem_read32[16])(u32) =
{
  read32_bios,         // 0
  read32_open,         // 1
  read32_ram,          // 2
  read32_ram,          // 3
  read32_io_registers, // 4
  read32_palette_ram,  // 5
  read32_ram,          // 6
  read32_oam_ram,      // 7
  read32_gamepak,      // 8
  read32_gamepak,      // 9
  read32_gamepak,      // a
  read32_gamepak,      // b
  read32_gamepak,      // c
  read32_gamepak,      // d
  read32_backup,       // e
  read32_open          // f
};

static u32 (*dma_read16[16])(u32) =
{
  read_null,           // 0
  read_null,           // 1
  read16_ram,          // 2
  read16_ram,          // 3
  read16_io_registers, // 4
  read16_palette_ram,  // 5
  read16_ram,          // 6
  read16_oam_ram,      // 7
  read16_gamepak,      // 8
  read16_gamepak,      // 9
  read16_gamepak,      // a
  read16_gamepak,      // b
  read16_gamepak,      // c
  read16_eeprom,       // d
  read_null,           // e
  read_null            // f
};

static u32 (*dma_read32[16])(u32) =
{
  read_null,           // 0
  read_null,           // 1
  read32_ram,          // 2
  read32_ram,          // 3
  read32_io_registers, // 4
  read32_palette_ram,  // 5
  read32_ram,          // 6
  read32_oam_ram,      // 7
  read32_gamepak,      // 8
  read32_gamepak,      // 9
  read32_gamepak,      // a
  read32_gamepak,      // b
  read32_gamepak,      // c
  read32_gamepak,      // d
  read_null,           // e
  read_null            // f
};

static u32 (*open_read8[16])(u32) =
{
  read8_bios,          // 0
  read_null,           // 1
  read8_ram,           // 2
  read8_ram,           // 3
  read_null,           // 4
  read_null,           // 5
  read8_ram,           // 6
  read_null,           // 7
  read8_gamepak,       // 8
  read8_gamepak,       // 9
  read8_gamepak,       // a
  read8_gamepak,       // b
  read8_gamepak,       // c
  read8_gamepak,       // d
  read_null,           // e
  read_null            // f
};

static u32 (*open_read16[16])(u32) =
{
  read16_bios,         // 0
  read_null,           // 1
  read16_ram,          // 2
  read16_ram,          // 3
  read_null,           // 4
  read_null,           // 5
  read16_ram,          // 6
  read_null,           // 7
  read16_gamepak,      // 8
  read16_gamepak,      // 9
  read16_gamepak,      // a
  read16_gamepak,      // b
  read16_gamepak,      // c
  read16_gamepak,      // d
  read_null,           // e
  read_null            // f
};

static u32 (*open_read32[16])(u32) =
{
  read32_bios,         // 0
  read_null,           // 1
  read32_ram,          // 2
  read32_ram,          // 3
  read_null,           // 4
  read_null,           // 5
  read32_ram,          // 6
  read_null,           // 7
  read32_gamepak,      // 8
  read32_gamepak,      // 9
  read32_gamepak,      // a
  read32_gamepak,      // b
  read32_gamepak,      // c
  read32_gamepak,      // d
  read_null,           // e
  read_null            // f
};


static CPU_ALERT_TYPE write_null(u32 address, u32 value);

static CPU_ALERT_TYPE write8_ewram(u32 address, u32 value);
static CPU_ALERT_TYPE write8_iwram(u32 address, u32 value);
static CPU_ALERT_TYPE write8_io_registers(u32 address, u32 value);
static CPU_ALERT_TYPE write8_palette_ram(u32 address, u32 value);
static CPU_ALERT_TYPE write8_vram(u32 address, u32 value);

static CPU_ALERT_TYPE write16_ewram(u32 address, u32 value);
static CPU_ALERT_TYPE write16_iwram(u32 address, u32 value);
static CPU_ALERT_TYPE write16_io_registers(u32 address, u32 value);
static CPU_ALERT_TYPE write16_palette_ram(u32 address, u32 value);
static CPU_ALERT_TYPE write16_vram(u32 address, u32 value);
static CPU_ALERT_TYPE write16_oam_ram(u32 address, u32 value);

static CPU_ALERT_TYPE write32_ewram(u32 address, u32 value);
static CPU_ALERT_TYPE write32_iwram(u32 address, u32 value);
static CPU_ALERT_TYPE write32_io_registers(u32 address, u32 value);
static CPU_ALERT_TYPE write32_palette_ram(u32 address, u32 value);
static CPU_ALERT_TYPE write32_vram(u32 address, u32 value);
static CPU_ALERT_TYPE write32_oam_ram(u32 address, u32 value);

static CPU_ALERT_TYPE (*mem_write8[16])(u32, u32) =
{
  write_null,           // 0
  write_null,           // 1
  write8_ewram,         // 2
  write8_iwram,         // 3
  write8_io_registers,  // 4
  write8_palette_ram,   // 5
  write8_vram,          // 6
  write_null,           // 7
  write_null,           // 8
  write_null,           // 9
  write_null,           // a
  write_null,           // b
  write_null,           // c
  write_null,           // d
  write_backup,         // e
  write_null            // f
};

static CPU_ALERT_TYPE (*mem_write16[16])(u32, u32) =
{
  write_null,           // 0
  write_null,           // 1
  write16_ewram,        // 2
  write16_iwram,        // 3
  write16_io_registers, // 4
  write16_palette_ram,  // 5
  write16_vram,         // 6
  write16_oam_ram,      // 7
  write_rtc,            // 8
  write_null,           // 9
  write_null,           // a
  write_null,           // b
  write_null,           // c
  write_null,           // d
  write_null,           // e
  write_null            // f
};

static CPU_ALERT_TYPE (*mem_write32[16])(u32, u32) =
{
  write_null,           // 0
  write_null,           // 1
  write32_ewram,        // 2
  write32_iwram,        // 3
  write32_io_registers, // 4
  write32_palette_ram,  // 5
  write32_vram,         // 6
  write32_oam_ram,      // 7
  write_null,           // 8
  write_null,           // 9
  write_null,           // a
  write_null,           // b
  write_null,           // c
  write_null,           // d
  write_null,           // e
  write_null            // f
};

static CPU_ALERT_TYPE (*dma_write16[16])(u32, u32) =
{
  write_null,           // 0
  write_null,           // 1
  write16_ewram,        // 2
  write16_iwram,        // 3
  write16_io_registers, // 4
  write16_palette_ram,  // 5
  write16_vram,         // 6
  write16_oam_ram,      // 7
  write_null,           // 8
  write_null,           // 9
  write_null,           // a
  write_null,           // b
  write_null,           // c
  write_eeprom,         // d
  write_null,           // e
  write_null            // f
};

static CPU_ALERT_TYPE (*dma_write32[16])(u32, u32) =
{
  write_null,           // 0
  write_null,           // 1
  write32_ewram,        // 2
  write32_iwram,        // 3
  write32_io_registers, // 4
  write32_palette_ram,  // 5
  write32_vram,         // 6
  write32_oam_ram,      // 7
  write_null,           // 8
  write_null,           // 9
  write_null,           // a
  write_null,           // b
  write_null,           // c
  write_null,           // d
  write_null,           // e
  write_null            // f
};


#define READ_BIOS(type)                                                       \
  if ((address >> 14) != 0)                                                   \
    return read##type##_open(address);                                        \
                                                                              \
  if ((reg[REG_PC] >> 14) != 0)                                               \
    return ADDRESS##type(&bios_read_protect, address & 0x03);                 \
                                                                              \
  return ADDRESS##type(bios.rom, address & 0x3FFF);                           \

static u32 read8_bios(u32 address)
{
  READ_BIOS(8);
}

static u32 read16_bios(u32 address)
{
  READ_BIOS(16);
}

static u32 read32_bios(u32 address)
{
  READ_BIOS(32);
}

/*
#define READ_EWRAM(type)                                                      \
  return ADDRESS##type(ewram, address & 0x3FFFF)                              \

static u32 read8_ewram(u32 address)
{
  READ_EWRAM(8);
}

static u32 read16_ewram(u32 address)
{
  READ_EWRAM(16);
}

static u32 read32_ewram(u32 address)
{
  READ_EWRAM(32);
}

#define READ_IWRAM(type)                                                      \
  return ADDRESS##type(iwram, address & 0x7FFF)                               \

static u32 read8_iwram(u32 address)
{
  READ_IWRAM(8);
}

static u32 read16_iwram(u32 address)
{
  READ_IWRAM(16);
}

static u32 read32_iwram(u32 address)
{
  READ_IWRAM(32);
}
*/

static u32 read8_io_registers(u32 address)
{
  if ((address & 0xFFFC) == 0x0800) // repeated each 64K
    return ADDRESS8(&iwram_control, address & 0x03);

  if ((((address >> 10) & 0x3FFF) == 0) && (io_readable[address & 0x3FF] != 0))
    return ADDRESS8(io_registers, address & 0x3FF);

  return read8_open(address);
}

static u32 read16_io_registers(u32 address)
{
  if ((address & 0xFFFC) == 0x0800)
    return ADDRESS16(&iwram_control, address & 0x02);

  if (((address >> 10) & 0x3FFF) == 0)
  {
    if (io_readable[address & 0x3Fe] != 0)
      return ADDRESS16(io_registers, address & 0x3Fe);
    else if (io_readable[address & 0x3FC] != 0)
      return 0;
  }

  return read16_open(address);
}

static u32 read32_io_registers(u32 address)
{
  if ((address & 0xFFFC) == 0x0800)
    return iwram_control;

  if ((((address >> 10) & 0x3FFF) == 0) && (io_readable[address & 0x3FC] != 0))
  {
    if (io_readable[(address & 0x3FC) + 2] != 0)
      return ADDRESS32(io_registers, address & 0x3FC);
    else
      return ADDRESS16(io_registers, address & 0x3FC);
  }

  return read32_open(address);
}

#define READ_PALETTE_RAM(type)                                                \
  return ADDRESS##type(palette_ram, address & 0x3FF);                         \

static u32 read8_palette_ram(u32 address)
{
  READ_PALETTE_RAM(8);
}

static u32 read16_palette_ram(u32 address)
{
  READ_PALETTE_RAM(16);
}

static u32 read32_palette_ram(u32 address)
{
  READ_PALETTE_RAM(32);
}

/*
#define READ_VRAM(type)                                                       \
  if (((address >> 16) & 0x01) != 0)                                          \
    address &= 0x17FFF;                                                       \
  else                                                                        \
    address &= 0x0FFFF;                                                       \
                                                                              \
  return ADDRESS##type(vram, address)                                         \

static u32 read8_vram(u32 address)
{
  READ_VRAM(8);
}

static u32 read16_vram(u32 address)
{
  READ_VRAM(16);
}

static u32 read32_vram(u32 address)
{
  READ_VRAM(32);
}
*/

#define READ_OAM_RAM(type)                                                    \
  return ADDRESS##type(oam_ram, address & 0x3FF);                             \

static u32 read8_oam_ram(u32 address)
{
  READ_OAM_RAM(8);
}

static u32 read16_oam_ram(u32 address)
{
  READ_OAM_RAM(16);
}

static u32 read32_oam_ram(u32 address)
{
  READ_OAM_RAM(32);
}

#define READ_GAMEPAK(type)                                                    \
  if ((address & 0x1FFFFFF) < gamepak_size)                                   \
  {                                                                           \
    u32 new_region = address >> 15;                                           \
                                                                              \
    if (new_region != read_rom_region)                                        \
    {                                                                         \
      read_rom_region = new_region;                                           \
      read_rom_block = memory_map_read[read_rom_region];                      \
                                                                              \
      if (read_rom_block == NULL)                                             \
        read_rom_block = load_gamepak_page(read_rom_region & 0x3FF);          \
    }                                                                         \
                                                                              \
    return ADDRESS##type(read_rom_block, address & 0x7FFF);                   \
  }                                                                           \

static u32 read8_gamepak(u32 address)
{
  READ_GAMEPAK(8);

  return read8_open(address);
}

static u32 read16_gamepak(u32 address)
{
  READ_GAMEPAK(16);

  return read16_open(address);
}

static u32 read32_gamepak(u32 address)
{
  READ_GAMEPAK(32);

  return read32_open(address);
}

static u32 read16_eeprom(u32 address)
{
  READ_GAMEPAK(16);

  return read_eeprom();
}

static u32 read8_backup(u32 address)
{
  return read_backup(address);
}

static u32 read16_backup(u32 address)
{
  u32 value = read_backup(address);
  return value | (value << 8);
}

static u32 read32_backup(u32 address)
{
  u32 value = read_backup(address);
  return value | (value << 8) | (value << 16) | (value << 24);
}

// ewram, iwram, vram
#define READ_RAM(type)                                                        \
  u32 new_region = address >> 15;                                             \
                                                                              \
  if (new_region != read_ram_region)                                          \
  {                                                                           \
    read_ram_region = new_region;                                             \
    read_ram_block = memory_map_read[read_ram_region];                        \
  }                                                                           \
                                                                              \
  return ADDRESS##type(read_ram_block, address & 0x7FFF);                     \

static u32 read8_ram(u32 address)
{
  READ_RAM(8);
}

static u32 read16_ram(u32 address)
{
  READ_RAM(16);
}

static u32 read32_ram(u32 address)
{
  READ_RAM(32);
}

static u32 read_null(u32 address)
{
  return 0;
}

#define THUMB_STATE (reg[REG_CPSR] & 0x20)

static u32 read8_open(u32 address)
{
  u32 offset = 0;

  if (reg[CPU_DMA_HACK] != 0)
    return reg[CPU_DMA_LAST] & 0xFF;

  if (THUMB_STATE != 0)
    offset = reg[REG_PC] + 2 + (address & 0x01);
  else
    offset = reg[REG_PC] + 4 + (address & 0x03);

  return (*open_read8[offset >> 24])(offset);
}

static u32 read16_open(u32 address)
{
  u32 offset = 0;

  if (reg[CPU_DMA_HACK] != 0)
    return reg[CPU_DMA_LAST] & 0xFFFF;

  if (THUMB_STATE != 0)
    offset = reg[REG_PC] + 2;
  else
    offset = reg[REG_PC] + 4 + (address & 0x02);

  return (*open_read16[offset >> 24])(offset);
}

static u32 read32_open(u32 address)
{
  if (reg[CPU_DMA_HACK] != 0)
    return reg[CPU_DMA_LAST];

  if (THUMB_STATE != 0)
  {
    u32 current_instruction = (*open_read16[reg[REG_PC] >> 24])(reg[REG_PC] + 2);
    return current_instruction | (current_instruction << 16);
  }

  return (*open_read32[reg[REG_PC] >> 24])(reg[REG_PC] + 4);
}


inline static CPU_ALERT_TYPE check_smc_write(u16 *metadata, u32 offset, u8 region)
{
  /* Get the Metadata Entry's [3], bits 0-1, to see if there's code at this
   * location. See "doc/partial flushing of RAM code.txt" for more info. */
  u16 smc = metadata[offset | 3] & 0x03;
  if (smc != 0) {
    partial_clear_metadata(offset, region);
    return CPU_ALERT_SMC;
  }
  return CPU_ALERT_NONE;
}

#define WRITE_EWRAM(type)                                                     \
  address &= 0x3FFFF;                                                         \
  ADDRESS##type(ewram, address) = value;                                      \
  return check_smc_write(ewram_metadata, address, 0x02);                      \

static CPU_ALERT_TYPE write8_ewram(u32 address, u32 value)
{
  WRITE_EWRAM(8);
}

static CPU_ALERT_TYPE write16_ewram(u32 address, u32 value)
{
  WRITE_EWRAM(16);
}

static CPU_ALERT_TYPE write32_ewram(u32 address, u32 value)
{
  WRITE_EWRAM(32);
}

#define WRITE_IWRAM(type)                                                     \
  address &= 0x7FFF;                                                          \
  ADDRESS##type(iwram, address) = value;                                      \
  return check_smc_write(iwram_metadata, address, 0x03);                      \

static CPU_ALERT_TYPE write8_iwram(u32 address, u32 value)
{
  WRITE_IWRAM(8);
}

static CPU_ALERT_TYPE write16_iwram(u32 address, u32 value)
{
  WRITE_IWRAM(16);
}

static CPU_ALERT_TYPE write32_iwram(u32 address, u32 value)
{
  WRITE_IWRAM(32);
}

#define WRITE_IO_REGISTERS(type)                                              \
  if (((address >> 10) & 0x3FFF) != 0)                                        \
    return CPU_ALERT_NONE;                                                    \
                                                                              \
  return write_io_register##type(address & 0x3FF, value);                     \

static CPU_ALERT_TYPE write8_io_registers(u32 address, u32 value)
{
  WRITE_IO_REGISTERS(8);
}

static CPU_ALERT_TYPE write16_io_registers(u32 address, u32 value)
{
  WRITE_IO_REGISTERS(16);
}

static CPU_ALERT_TYPE write32_io_registers(u32 address, u32 value)
{
  WRITE_IO_REGISTERS(32);
}

static CPU_ALERT_TYPE write8_palette_ram(u32 address, u32 value)
{
  ADDRESS16(palette_ram, address & 0x3Fe) = value | (value << 8);

  return CPU_ALERT_NONE;
}

static CPU_ALERT_TYPE write16_palette_ram(u32 address, u32 value)
{
  ADDRESS16(palette_ram, address & 0x3Fe) = value;

  return CPU_ALERT_NONE;
}

static CPU_ALERT_TYPE write32_palette_ram(u32 address, u32 value)
{
  ADDRESS32(palette_ram, address & 0x3FC) = value;

  return CPU_ALERT_NONE;
}

#define WRITE_VRAM(type)                                                      \
  if (((address >> 16) & 0x01) != 0)                                          \
    address &= 0x17FFF;                                                       \
  else                                                                        \
    address &= 0x0FFFF;                                                       \
                                                                              \
  ADDRESS##type(vram, address) = value;                                       \
  return check_smc_write(vram_metadata, address, 0x06);                       \

static CPU_ALERT_TYPE write8_vram(u32 address, u32 value)
{
  if (((address >> 16) & 0x01) != 0)
    address &= 0x17FFe;
  else
    address &= 0x0FFFe;

  if (address >= obj_address)
    return  CPU_ALERT_NONE;

  ADDRESS16(vram, address) = value | (value << 8);
  return check_smc_write(vram_metadata, address, 0x06);
}

static CPU_ALERT_TYPE write16_vram(u32 address, u32 value)
{
  WRITE_VRAM(16);
}

static CPU_ALERT_TYPE write32_vram(u32 address, u32 value)
{
  WRITE_VRAM(32);
}

#define WRITE_OAM_RAM(type)                                                   \
  oam_update = 1;                                                             \
  ADDRESS##type(oam_ram, address & 0x3FF) = value;                            \
                                                                              \
  return  CPU_ALERT_NONE;                                                     \

static CPU_ALERT_TYPE write16_oam_ram(u32 address, u32 value)
{
  WRITE_OAM_RAM(16);
}

static CPU_ALERT_TYPE write32_oam_ram(u32 address, u32 value)
{
  WRITE_OAM_RAM(32);
}

static CPU_ALERT_TYPE write_null(u32 address, u32 value)
{
  return CPU_ALERT_NONE;
}


// write io registers

static SIO_MODE_TYPE sio_mode(u16 reg_sio_cnt, u16 reg_rcnt)
{
  if ((reg_rcnt & 0x8000) == 0x0000)
  {
    switch (reg_sio_cnt & 0x3000)
    {
      case 0x0000:
        return NORMAL8;

      case 0x1000:
        return NORMAL32;

      case 0x2000:
        return MULTIPLAYER;

      case 0x3000:
        return UART;
    }
  }

  if ((reg_rcnt & 0x4000) != 0)
    return JOYBUS;

  return GP;
}

static CPU_ALERT_TYPE sio_control(u32 value)
{
  SIO_MODE_TYPE mode = sio_mode(value, pIO_REG(REG_RCNT));
  CPU_ALERT_TYPE alert = CPU_ALERT_NONE;

  switch (mode)
  {
    case NORMAL8:
    case NORMAL32:
      if ((value & 0x80) != 0)
      {
        value &= 0xFF7F;

        if ((value & 0x4001) == 0x4001)
        {
          pIO_REG(REG_IF) |= IRQ_SERIAL;
          alert = CPU_ALERT_IRQ;
        }
      }
      break;

    case MULTIPLAYER:
      value &= 0xFF83;
      value |= 0x0C;
      break;

    case UART:
    case JOYBUS:
    case GP:
      break;
  }

  pIO_REG(REG_SIOCNT) = value;

  return alert;
}

static void waitstate_control(u32 value)
{
  u32 i;
  const u8 waitstate_table[4] = { 4, 3, 2, 8 };
  const u8 gamepak_ws0_seq[2] = { 2, 1 };
  const u8 gamepak_ws1_seq[2] = { 4, 1 };
  const u8 gamepak_ws2_seq[2] = { 8, 1 };

  // Wait State First Access (8/16bit)
  pMEMORY_WS16N(0x08) = pMEMORY_WS16N(0x09) = waitstate_table[(value >> 2) & 0x03];
  pMEMORY_WS16N(0x0A) = pMEMORY_WS16N(0x0B) = waitstate_table[(value >> 5) & 0x03];
  pMEMORY_WS16N(0x0C) = pMEMORY_WS16N(0x0D) = waitstate_table[(value >> 8) & 0x03];

  // Wait State Second Access (8/16bit)
  pMEMORY_WS16S(0x08) = pMEMORY_WS16S(0x09) = gamepak_ws0_seq[(value >>  4) & 0x01];
  pMEMORY_WS16S(0x0A) = pMEMORY_WS16S(0x0B) = gamepak_ws1_seq[(value >>  7) & 0x01];
  pMEMORY_WS16S(0x0C) = pMEMORY_WS16S(0x0D) = gamepak_ws2_seq[(value >> 10) & 0x01];

  // SRAM Wait Control (8bit)
  pMEMORY_WS16N(0x0e) = pMEMORY_WS16S(0x0e) =
  pMEMORY_WS32N(0x0e) = pMEMORY_WS32S(0x0e) = waitstate_table[value & 0x03];

  for (i = 0x08; i <= 0x0D; i++)
  {
    // Wait State First Access (32bit)
    pMEMORY_WS32N(i) = pMEMORY_WS16N(i) + pMEMORY_WS16S(i) + 1;

    // Wait State Second Access (32bit)
    pMEMORY_WS32S(i) = (pMEMORY_WS16S(i) << 1) + 1;
  }

  // gamepak prefetch
  if (((value >> 14) & 0x01) != 0)
  {
    for (i = 0x08; i <= 0x0D; i++)
    {
      pFETCH_WS16N(i) = 2;
      pFETCH_WS16S(i) = 2;
      pFETCH_WS32N(i) = 5;
      pFETCH_WS32S(i) = 5;
    }
  }
  else
  {
    for (i = 0x08; i <= 0x0D; i++)
    {
      // Prefetch Disable Bug
      // the opcode fetch time from 1S to 1N.
      pFETCH_WS16N(i) = pMEMORY_WS16N(i);
      pFETCH_WS16S(i) = pMEMORY_WS16N(i);
      pFETCH_WS32N(i) = pMEMORY_WS32N(i);
      pFETCH_WS32S(i) = pMEMORY_WS32N(i);
    }
  }

  pIO_REG(REG_WAITCNT) = (pIO_REG(REG_WAITCNT) & 0x8000) | (value & 0x7FFF);
}


#define TRIGGER_DMA0()                                                        \
{                                                                             \
  DmaTransferType *dma0 = dma + 0;                                            \
                                                                              \
  if ((value & 0x8000) != 0)                                                  \
  {                                                                           \
    dma0->dma_channel = 0;                                                    \
    dma0->source_direction = (value >> 7) & 0x03;                             \
    dma0->repeat_type = (value >> 9) & 0x01;                                  \
    dma0->irq = (value >> 14) & 0x01;                                         \
                                                                              \
    if (dma0->start_type == DMA_INACTIVE)                                     \
    {                                                                         \
      dma0->start_type = (value >> 12) & 0x03;                                \
                                                                              \
      dma0->source_address = ADDRESS32(io_registers, 0xB0);                   \
      dma0->dest_address   = ADDRESS32(io_registers, 0xB4);                   \
                                                                              \
      u32 length = ADDRESS16(io_registers, 0xB8);                             \
                                                                              \
      dma0->dest_direction = (value >> 5) & 0x03;                             \
      dma0->length_type = (value >> 10) & 0x01;                               \
      dma0->length = (length == 0) ? 0x4000 : length;                         \
                                                                              \
      if (dma0->start_type == DMA_START_IMMEDIATELY)                          \
        return dma_transfer(dma + 0);                                         \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      dma0->start_type = (value >> 12) & 0x03;                                \
                                                                              \
      dma0->dest_direction = (value >> 5) & 0x03;                             \
      dma0->length_type = (value >> 10) & 0x01;                               \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    dma0->start_type = DMA_INACTIVE;                                          \
  }                                                                           \
}                                                                             \

#define TRIGGER_DMA1()                                                        \
{                                                                             \
  DmaTransferType *dma1 = dma + 1;                                            \
                                                                              \
  if ((value & 0x8000) != 0)                                                  \
  {                                                                           \
    dma1->dma_channel = 1;                                                    \
    dma1->source_direction = (value >> 7) & 0x03;                             \
    dma1->repeat_type = (value >> 9) & 0x01;                                  \
    dma1->irq = (value >> 14) & 0x01;                                         \
                                                                              \
    if (dma1->start_type == DMA_INACTIVE)                                     \
    {                                                                         \
      dma1->start_type = (value >> 12) & 0x03;                                \
                                                                              \
      dma1->source_address = ADDRESS32(io_registers, 0xBC);                   \
      dma1->dest_address   = ADDRESS32(io_registers, 0xC0);                   \
                                                                              \
      if (dma1->start_type == DMA_START_SPECIAL)                              \
      {                                                                       \
        dma1->dest_direction = DMA_FIXED;                                     \
        dma1->length_type = DMA_32BIT;                                        \
        dma1->length = 4;                                                     \
                                                                              \
        if (dma1->dest_address == 0x40000A4)                                  \
          dma1->direct_sound_channel = DMA_DIRECT_SOUND_B;                    \
        else                                                                  \
          dma1->direct_sound_channel = DMA_DIRECT_SOUND_A;                    \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        u32 length = ADDRESS16(io_registers, 0xC4);                           \
                                                                              \
        dma1->dest_direction = (value >> 5) & 0x03;                           \
        dma1->length_type = (value >> 10) & 0x01;                             \
        dma1->length = (length == 0) ? 0x4000 : length;                       \
                                                                              \
        if (dma1->start_type == DMA_START_IMMEDIATELY)                        \
          return dma_transfer(dma + 1);                                       \
      }                                                                       \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      dma1->start_type = (value >> 12) & 0x03;                                \
                                                                              \
      if (dma1->start_type == DMA_START_SPECIAL)                              \
      {                                                                       \
        dma1->dest_direction = DMA_FIXED;                                     \
        dma1->length_type = DMA_32BIT;                                        \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        dma1->dest_direction = (value >> 5) & 0x03;                           \
        dma1->length_type = (value >> 10) & 0x01;                             \
      }                                                                       \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    dma1->start_type = DMA_INACTIVE;                                          \
    dma1->direct_sound_channel = DMA_NO_DIRECT_SOUND;                         \
  }                                                                           \
}                                                                             \

#define TRIGGER_DMA2()                                                        \
{                                                                             \
  DmaTransferType *dma2 = dma + 2;                                            \
                                                                              \
  if ((value & 0x8000) != 0)                                                  \
  {                                                                           \
    dma2->dma_channel = 2;                                                    \
    dma2->source_direction = (value >> 7) & 0x03;                             \
    dma2->repeat_type = (value >> 9) & 0x01;                                  \
    dma2->irq = (value >> 14) & 0x01;                                         \
                                                                              \
    if (dma2->start_type == DMA_INACTIVE)                                     \
    {                                                                         \
      dma2->start_type = (value >> 12) & 0x03;                                \
                                                                              \
      dma2->source_address = ADDRESS32(io_registers, 0xC8);                   \
      dma2->dest_address   = ADDRESS32(io_registers, 0xCC);                   \
                                                                              \
      if (dma2->start_type == DMA_START_SPECIAL)                              \
      {                                                                       \
        dma2->dest_direction = DMA_FIXED;                                     \
        dma2->length_type = DMA_32BIT;                                        \
        dma2->length = 4;                                                     \
                                                                              \
        if (dma2->dest_address == 0x40000A4)                                  \
          dma2->direct_sound_channel = DMA_DIRECT_SOUND_B;                    \
        else                                                                  \
          dma2->direct_sound_channel = DMA_DIRECT_SOUND_A;                    \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        u32 length = ADDRESS16(io_registers, 0xD0);                           \
                                                                              \
        dma2->dest_direction = (value >> 5) & 0x03;                           \
        dma2->length_type = (value >> 10) & 0x01;                             \
        dma2->length = (length == 0) ? 0x4000 : length;                       \
                                                                              \
        if (dma2->start_type == DMA_START_IMMEDIATELY)                        \
          return dma_transfer(dma + 2);                                       \
      }                                                                       \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      dma2->start_type = (value >> 12) & 0x03;                                \
                                                                              \
      if (dma2->start_type == DMA_START_SPECIAL)                              \
      {                                                                       \
        dma2->dest_direction = DMA_FIXED;                                     \
        dma2->length_type = DMA_32BIT;                                        \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        dma2->dest_direction = (value >> 5) & 0x03;                           \
        dma2->length_type = (value >> 10) & 0x01;                             \
      }                                                                       \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    dma2->start_type = DMA_INACTIVE;                                          \
    dma2->direct_sound_channel = DMA_NO_DIRECT_SOUND;                         \
  }                                                                           \
}                                                                             \

#define TRIGGER_DMA3()                                                        \
{                                                                             \
  DmaTransferType *dma3 = dma + 3;                                            \
                                                                              \
  if ((value & 0x8000) != 0)                                                  \
  {                                                                           \
    dma3->dma_channel = 3;                                                    \
    dma3->source_direction = (value >> 7) & 0x03;                             \
    dma3->repeat_type = (value >> 9) & 0x01;                                  \
    dma3->irq = (value >> 14) & 0x01;                                         \
                                                                              \
    if (dma3->start_type == DMA_INACTIVE)                                     \
    {                                                                         \
      dma3->start_type = (value >> 12) & 0x03;                                \
                                                                              \
      dma3->source_address = ADDRESS32(io_registers, 0xD4);                   \
      dma3->dest_address   = ADDRESS32(io_registers, 0xD8);                   \
                                                                              \
      u32 length = ADDRESS16(io_registers, 0xDC);                             \
                                                                              \
      dma3->dest_direction = (value >> 5) & 0x03;                             \
      dma3->length_type = (value >> 10) & 0x01;                               \
      dma3->length = (length == 0) ? 0x10000 : length;                        \
                                                                              \
      if (((dma3->dest_address >> 24) == 0x0D) && (length == 17))             \
        eeprom_size = EEPROM_8_KBYTE;                                         \
                                                                              \
      if (dma3->start_type == DMA_START_IMMEDIATELY)                          \
        return dma_transfer(dma + 3);                                         \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      dma3->start_type = (value >> 12) & 0x03;                                \
                                                                              \
      dma3->dest_direction = (value >> 5) & 0x03;                             \
      dma3->length_type = (value >> 10) & 0x01;                               \
    }                                                                         \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    dma3->start_type = DMA_INACTIVE;                                          \
  }                                                                           \
}                                                                             \


#define ACCESS_REGISTER8_HIGH(address)                                        \
  value = ADDRESS8(io_registers, address) | (value << 8);                     \

#define ACCESS_REGISTER8_LOW(address)                                         \
  value = value | (ADDRESS8(io_registers, address + 1) << 8);                 \

#define ACCESS_REGISTER16_HIGH(address)                                       \
  value = ADDRESS16(io_registers, address) | (value << 16);                   \

#define ACCESS_REGISTER16_LOW(address)                                        \
  value = value | (ADDRESS16(io_registers, address + 2) << 16);               \


CPU_ALERT_TYPE write_io_register8(u32 address, u32 value)
{
  switch (address)
  {
    // Post Boot
    case 0x300:
      ADDRESS8(io_registers, 0x300) = value;
      break;

    // Halt
    case 0x301:
      if ((value & 0x80) != 0)
        reg[CPU_HALT_STATE] = CPU_STOP;
      else
        reg[CPU_HALT_STATE] = CPU_HALT;

      ADDRESS8(io_registers, 0x301) = value;
      return CPU_ALERT_HALT;

    default:
      if ((address & 0x01) != 0)
      {
        address &= ~0x01;
        ACCESS_REGISTER8_HIGH(address);
      }
      else
      {
        ACCESS_REGISTER8_LOW(address);
      }
      return write_io_register16(address, value);
  }

  return CPU_ALERT_NONE;
}


CPU_ALERT_TYPE write_io_register16(u32 address, u32 value)
{
  switch (address)
  {
    // DISPCNT
    case 0x00:
    {
      u16 bg_mode = value & 0x07;
      u16 dispcnt = pIO_REG(REG_DISPCNT);

      if (bg_mode > 5)
        value &= 0x07;

      if (bg_mode < 3)
        obj_address = 0x10000;
      else
        obj_address = 0x14000;

      if (bg_mode != (dispcnt & 0x07))
        oam_update = 1;

      if ((((dispcnt ^ value) & 0x80) != 0) && ((value & 0x80) == 0))
      {
        if ((pIO_REG(REG_DISPSTAT) & 0x01) == 0)
          pIO_REG(REG_DISPSTAT) &= 0xFFFC;
      }

      ADDRESS16(io_registers, 0x00) = value;
      break;
    }

    // DISPSTAT
    case 0x04:
      ADDRESS16(io_registers, 0x04) = (ADDRESS16(io_registers, 0x04) & 0x07) | (value & ~0x07);
      break;

    // VCOUNT
    case 0x06:
      /* Read only */
      break;

    // BG2 reference X
    case 0x28:
      ACCESS_REGISTER16_LOW(0x28);
      affine_reference_x[0] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x28) = value;
      break;

    case 0x2A:
      ACCESS_REGISTER16_HIGH(0x28);
      affine_reference_x[0] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x28) = value;
      break;

    // BG2 reference Y
    case 0x2C:
      ACCESS_REGISTER16_LOW(0x2C);
      affine_reference_y[0] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x2C) = value;
      break;

    case 0x2E:
      ACCESS_REGISTER16_HIGH(0x2C);
      affine_reference_y[0] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x2C) = value;
      break;

    // BG3 reference X
    case 0x38:
      ACCESS_REGISTER16_LOW(0x38);
      affine_reference_x[1] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x38) = value;
      break;

    case 0x3A:
      ACCESS_REGISTER16_HIGH(0x38);
      affine_reference_x[1] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x38) = value;
      break;

    // BG3 reference Y
    case 0x3C:
      ACCESS_REGISTER16_LOW(0x3C);
      affine_reference_y[1] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x3C) = value;
      break;

    case 0x3E:
      ACCESS_REGISTER16_HIGH(0x3C);
      affine_reference_y[1] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x3C) = value;
      break;

    // Sound 1 control sweep
    case 0x60:
      gbc_sound_tone_control_sweep(value);
      ADDRESS16(io_registers, 0x60) = value;
      break;

    // Sound 1 control duty/length/envelope
    case 0x62:
      gbc_sound_tone_control_low(0, value);
      ADDRESS16(io_registers, 0x62) = value;
      break;

    // Sound 1 control frequency
    case 0x64:
      gbc_sound_tone_control_high(0, value);
      ADDRESS16(io_registers, 0x64) = value;
      break;

    // Sound 2 control duty/length/envelope
    case 0x68:
      gbc_sound_tone_control_low(1, value);
      ADDRESS16(io_registers, 0x68) = value;
      break;

    // Sound 2 control frequency
    case 0x6C:
      gbc_sound_tone_control_high(1, value);
      ADDRESS16(io_registers, 0x6C) = value;
      break;

    // Sound 3 control wave
    case 0x70:
      gbc_sound_wave_control(value);
      ADDRESS16(io_registers, 0x70) = value;
      break;

    // Sound 3 control length/volume
    case 0x72:
      gbc_sound_tone_control_low_wave(value);
      ADDRESS16(io_registers, 0x72) = value;
      break;

    // Sound 3 control frequency
    case 0x74:
      gbc_sound_tone_control_high_wave(value);
      ADDRESS16(io_registers, 0x74) = value;
      break;

    // Sound 4 control length/envelope
    case 0x78:
      gbc_sound_tone_control_low(3, value);
      ADDRESS16(io_registers, 0x78) = value;
      break;

    // Sound 4 control frequency
    case 0x7C:
      gbc_sound_noise_control(value);
      ADDRESS16(io_registers, 0x7C) = value;
      break;

    // Sound control L
    case 0x80:
      sound_control_low(value);
      ADDRESS16(io_registers, 0x80) = value;
      break;

    // Sound control H
    case 0x82:
      sound_control_high(value);
      ADDRESS16(io_registers, 0x82) = value;
      break;

    // Sound control X
    case 0x84:
      sound_control_x(value);
      ADDRESS16(io_registers, 0x84) = (ADDRESS16(io_registers, 0x84) & 0x000F) | (value & 0xFFF0);
      break;

    // Sound wave RAM
    case 0x90: case 0x92: case 0x94: case 0x96:
    case 0x98: case 0x9A: case 0x9C: case 0x9E:
      gbc_sound_wave_pattern_ram16(address, value);
      ADDRESS16(io_registers, address) = value;
      break;

    // Sound FIFO A
    case 0xA0:
    case 0xA2:
      ADDRESS16(io_registers, address) = value;
      sound_timer_queue(0);
      break;

    // Sound FIFO B
    case 0xA4:
    case 0xA6:
      ADDRESS16(io_registers, address) = value;
      sound_timer_queue(1);
      break;

    // DMA Source Address High (internal memory)
    case 0xB2:
    // DMA Destination Address High (internal memory)
    case 0xB6: case 0xC2: case 0xCe:
      ADDRESS16(io_registers, address) = value & 0x07FF;
      break;

    // DMA Source Address High (any memory)
    case 0xBe: case 0xCA: case 0xD6:
    // DMA Destination Address High (any memory)
    case 0xDA:
      ADDRESS16(io_registers, address) = value & 0x0FFF;
      break;

    // DMA Word Count (14 bit)
    case 0xB8: case 0xC4: case 0xD0:
      ADDRESS16(io_registers, address) = value & 0x3FFF;
      break;

    // DMA control
    case 0xBA:
      ADDRESS16(io_registers, 0xBA) = value;
      TRIGGER_DMA0();
      break;

    case 0xC6:
      ADDRESS16(io_registers, 0xC6) = value;
      TRIGGER_DMA1();
      break;

    case 0xD2:
      ADDRESS16(io_registers, 0xD2) = value;
      TRIGGER_DMA2();
      break;

    case 0xDE:
      ADDRESS16(io_registers, 0xDE) = value;
      TRIGGER_DMA3();
      break;

    // Timer counts
    case 0x100:
      timer_control_low(0, value);
      break;

    case 0x104:
      timer_control_low(1, value);
      break;

    case 0x108:
      timer_control_low(2, value);
      break;

    case 0x10C:
      timer_control_low(3, value);
      break;

    // Timer control
    case 0x102:
      ADDRESS16(io_registers, 0x102) = value;
      return timer_control_high(0, value);

    case 0x106:
      ADDRESS16(io_registers, 0x106) = value;
      return timer_control_high(1, value);

    case 0x10A:
      ADDRESS16(io_registers, 0x10A) = value;
      return timer_control_high(2, value);

    case 0x10E:
      ADDRESS16(io_registers, 0x10e) = value;
      return timer_control_high(3, value);

    // SIOCNT
    case 0x128:
      return sio_control(value);

    // P1
    case 0x130:
      /* Read only */
      break;

    // IE - Interrupt Enable Register
    case 0x200:
      value &= 0x3FFF;
      ADDRESS16(io_registers, 0x200) = value;
      if (((value & pIO_REG(REG_IF)) != 0) && GBA_IME_STATE && ARM_IRQ_STATE)
        return CPU_ALERT_IRQ;
      break;

    // IF - Interrupt Request flags
    case 0x202:
      value = ~value & 0x3FFF;
      ADDRESS16(io_registers, 0x202) &= value;
      break;

    // WAITCNT
    case 0x204:
      waitstate_control(value);
      break;

    // IME - Interrupt Master Enable Register
    case 0x208:
      value &= 0x0001;
      ADDRESS16(io_registers, 0x208) = value;
      if (((pIO_REG(REG_IE) & pIO_REG(REG_IF)) != 0) && (value != 0) && ARM_IRQ_STATE)
        return CPU_ALERT_IRQ;
      break;

    // Halt
    case 0x300:
      if ((value & 0x8000) != 0)
        reg[CPU_HALT_STATE] = CPU_STOP;
      else
        reg[CPU_HALT_STATE] = CPU_HALT;

      ADDRESS16(io_registers, 0x300) = value;
      return CPU_ALERT_HALT;

    default:
      ADDRESS16(io_registers, address) = value;
      break;
  }

  return CPU_ALERT_NONE;
}


CPU_ALERT_TYPE write_io_register32(u32 address, u32 value)
{
  switch (address)
  {
    // BG2 reference X
    case 0x28:
      affine_reference_x[0] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x28) = value;
      break;

    // BG2 reference Y
    case 0x2C:
      affine_reference_y[0] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x2C) = value;
      break;

    // BG3 reference X
    case 0x38:
      affine_reference_x[1] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x38) = value;
      break;

    // BG3 reference Y
    case 0x3C:
      affine_reference_y[1] = ((s32)value << 4) >> 4;
      ADDRESS32(io_registers, 0x3C) = value;
      break;

    // Sound FIFO A
    case 0xA0:
      ADDRESS32(io_registers, 0xA0) = value;
      sound_timer_queue(0);
      break;

    // Sound FIFO B
    case 0xA4:
      ADDRESS32(io_registers, 0xA4) = value;
      sound_timer_queue(1);
      break;

    // DMA Source Address (internal memory)
    case 0xB0:
    // DMA Destination Address (internal memory)
    case 0xB4: case 0xC0: case 0xCC:
      ADDRESS32(io_registers, address) = value & 0x07FFFFFF;
      break;

    // DMA Source Address (any memory)
    case 0xBC: case 0xC8: case 0xD4:
    // DMA Destination Address (any memory)
    case 0xD8:
      ADDRESS32(io_registers, address) = value & 0x0FFFFFFF;
      break;

    // SIO Data (Normal-32bit Mode)
    case 0x120:
    // SIO JOY Bus
    case 0x150: case 0x154:
      ADDRESS32(io_registers, address) = value;
      break;

    default:
    {
      CPU_ALERT_TYPE alert_low  = write_io_register16(address, value & 0xFFFF);
      CPU_ALERT_TYPE alert_high = write_io_register16(address + 2, value >> 16);

      return alert_high | alert_low;
    }
  }

  return CPU_ALERT_NONE;
}


// EEPROM is 512 bytes by default; it is autodetecte as 8KB if
// 14bit address DMAs are made (this is done in the DMA handler).

CPU_ALERT_TYPE write_eeprom(u32 address, u32 value)
{
  // ROM is restricted to 8000000h-9FFFeFFh
  // (max.1FFFF00h bytes = 32MB minus 256 bytes)
  if (gamepak_size > 0x1FFFF00)
  {
    gamepak_size = 0x1FFFF00;
  }

  switch (eeprom_mode)
  {
    case EEPROM_BASE_MODE:
      backup_type = BACKUP_EEPROM;
      eeprom_buffer[0] |= ((value & 0x01) << (1 - eeprom_counter));
      eeprom_counter++;
      if (eeprom_counter == 2)
      {
        if (eeprom_size == EEPROM_512_BYTE)
          eeprom_address_length = 6;
        else
          eeprom_address_length = 14;

        eeprom_counter = 0;

        switch (eeprom_buffer[0] & 0x03)
        {
          case 0x02:
            eeprom_mode = EEPROM_WRITE_ADDRESS_MODE;
            break;

          case 0x03:
            eeprom_mode = EEPROM_ADDRESS_MODE;
            break;
        }
        eeprom_buffer[0] = 0;
        eeprom_buffer[1] = 0;
      }
      break;

    case EEPROM_ADDRESS_MODE:
    case EEPROM_WRITE_ADDRESS_MODE:
      eeprom_buffer[eeprom_counter / 8] |= ((value & 0x01) << (7 - (eeprom_counter % 8)));
      eeprom_counter++;
      if (eeprom_counter == eeprom_address_length)
      {
        if (eeprom_size == EEPROM_512_BYTE)
          eeprom_address = (((u32)eeprom_buffer[0] >> 2) | ((u32)eeprom_buffer[1] << 6)) * 8;
        else
          eeprom_address = (((u32)eeprom_buffer[1] >> 2) | ((u32)eeprom_buffer[0] << 6)) * 8;

        eeprom_buffer[0] = 0;
        eeprom_buffer[1] = 0;

        eeprom_counter = 0;

        if (eeprom_mode == EEPROM_ADDRESS_MODE)
        {
          eeprom_mode = EEPROM_ADDRESS_FOOTER_MODE;
        }
        else
        {
          eeprom_mode = EEPROM_WRITE_MODE;
          memset(gamepak_backup + eeprom_address, 0, 8);
        }
      }
      break;

    case EEPROM_WRITE_MODE:
      gamepak_backup[eeprom_address + (eeprom_counter / 8)] |= ((value & 0x01) << (7 - (eeprom_counter % 8)));
      eeprom_counter++;
      if (eeprom_counter == 64)
      {
        backup_update = WRITE_BACKUP_DELAY;
        eeprom_mode = EEPROM_WRITE_FOOTER_MODE;
        eeprom_counter = 0;
      }
      break;

    case EEPROM_ADDRESS_FOOTER_MODE:
    case EEPROM_WRITE_FOOTER_MODE:
      eeprom_counter = 0;
      if (eeprom_mode == EEPROM_ADDRESS_FOOTER_MODE)
        eeprom_mode = EEPROM_READ_HEADER_MODE;
      else
        eeprom_mode = EEPROM_BASE_MODE;
      break;

    case EEPROM_READ_MODE:
    case EEPROM_READ_HEADER_MODE:
      break;
  }

  return CPU_ALERT_NONE;
}

u32 read_eeprom(void)
{
  u32 value;

  switch (eeprom_mode)
  {
    case EEPROM_BASE_MODE:
      value = 1;
      break;

    case EEPROM_READ_MODE:
      value = (gamepak_backup[eeprom_address + (eeprom_counter / 8)] >> (7 - (eeprom_counter % 8))) & 0x01;
      eeprom_counter++;
      if (eeprom_counter == 64)
      {
        eeprom_mode = EEPROM_BASE_MODE;
        eeprom_counter = 0;
      }
      break;

    case EEPROM_READ_HEADER_MODE:
      value = 0;
      eeprom_counter++;
      if (eeprom_counter == 4)
      {
        eeprom_mode = EEPROM_READ_MODE;
        eeprom_counter = 0;
      }
      break;

    default:
      value = 0;
      break;
  }

  return value;
}


u32 read_backup(u32 address)
{
  address &= 0xFFFF;
  u32 value = 0;

  if (backup_type == BACKUP_NONE)
    backup_type = BACKUP_SRAM;

  switch (backup_type)
  {
    case BACKUP_SRAM:
      value = gamepak_backup[address];
      break;

    case BACKUP_FLASH:
      if (flash_mode == FLASH_ID_MODE)
      {
        /* ID manufacturer type */
        if (address == 0x0000)
          value = flash_manufacturer_id;
        else
        /* ID device type */
        if (address == 0x0001)
          value = flash_device_id;
      }
      else
      {
        value = gamepak_backup[(flash_bank << 16) + address];
      }
      break;

    case BACKUP_EEPROM:
      // Tilt Sensor
      if (enable_tilt_sensor != 0)
      {
        switch (address)
        {
          case 0x8200:
            // Lower 8 bits of X axis
            value = tilt_sensorX & 0xFF;
            break;

          case 0x8300:
            // Upper 4 bits of X axis,
            // and Bit7: ADC Status (0=Busy, 1=Ready)
            value = (tilt_sensorX >> 8) | 0x80;
            break;

          case 0x8400:
            // Lower 8 bits of Y axis
            value = tilt_sensorY & 0xFF;
            break;

          case 0x8500:
            // Upper 4 bits of Y axis
            value = tilt_sensorY >> 8;
            break;
        }
      }
      break;

    case BACKUP_NONE:
      break;
  }

  return value & 0xFF;
}

CPU_ALERT_TYPE write_backup(u32 address, u32 value)
{
  address &= 0xFFFF;
  value &= 0xFF;

  // Tilt Sensor
  if (backup_type == BACKUP_EEPROM)
  {
    // start sampling
    if (((address == 0x8000) && (value == 0x55)) ||
        ((address == 0x8100) && (value == 0xAA)))
    {
      enable_tilt_sensor = 1;
    }

    return CPU_ALERT_NONE;
  }

  if (backup_type == BACKUP_NONE)
  {
    backup_type = BACKUP_SRAM;
  }

  // gamepak SRAM or Flash ROM
  if ((address == 0x5555) && (flash_mode != FLASH_WRITE_MODE))
  {
    if ((flash_command_position == 0) && (value == 0xAA))
    {
      backup_type = BACKUP_FLASH;
      flash_command_position = 1;
    }

    if (flash_command_position == 2)
    {
      switch (value)
      {
        case 0x90:
          // Enter ID mode, this also tells the emulator that we're using
          // flash, not SRAM
          if (flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_ID_MODE;
          break;

        case 0x80:
          // Enter erase mode
          if (flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_ERASE_MODE;
          break;

        case 0xF0:
          // Terminate ID mode
          if (flash_mode == FLASH_ID_MODE)
            flash_mode = FLASH_BASE_MODE;
          break;

        case 0xA0:
          // Write mode
          if (flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_WRITE_MODE;
          break;

        case 0xB0:
          // Bank switch
          // Here the chip is now officially 128KB.
          flash_size = FLASH_SIZE_128KB;

          if (flash_mode == FLASH_BASE_MODE)
            flash_mode = FLASH_BANKSWITCH_MODE;
          break;

        case 0x10:
          // Erase chip
          if (flash_mode == FLASH_ERASE_MODE)
          {
            if (flash_size == FLASH_SIZE_64KB)
              memset(gamepak_backup, 0xFF, 1024 * 64);
            else
              memset(gamepak_backup, 0xFF, 1024 * 128);

            backup_update = WRITE_BACKUP_DELAY;
            flash_mode = FLASH_BASE_MODE;
          }
          break;
      }

      flash_command_position = 0;
    }

    if (backup_type == BACKUP_SRAM)
    {
      backup_update = WRITE_BACKUP_DELAY;
      gamepak_backup[0x5555] = value;
    }
  }
  else
  {
    if ((flash_command_position == 1) && (address == 0x2AAA) && (value == 0x55))
    {
      flash_command_position = 2;
    }
    else
    {
      if ((flash_command_position == 2) && (flash_mode == FLASH_ERASE_MODE) && (value == 0x30))
      {
        // Erase sector
        memset(&gamepak_backup[(flash_bank << 16) + (address & 0xF000)], 0xFF, 1024 * 4);
        backup_update = WRITE_BACKUP_DELAY;
        flash_mode = FLASH_BASE_MODE;
        flash_command_position = 0;
      }
      else
      {
        if ((flash_command_position == 0) && (address == 0x0000) && (flash_mode == FLASH_BANKSWITCH_MODE) && (flash_size == FLASH_SIZE_128KB))
        {
          flash_bank = value & 0x01;
          flash_mode = FLASH_BASE_MODE;
        }
        else
        {
          if ((flash_command_position == 0) && (flash_mode == FLASH_WRITE_MODE))
          {
            // Write value to flash ROM
            backup_update = WRITE_BACKUP_DELAY;

            gamepak_backup[(flash_bank << 16) + address] = value;
            flash_mode = FLASH_BASE_MODE;
          }
        }
      }
    }

    if (backup_type == BACKUP_SRAM)
    {
      // Write value to SRAM
      backup_update = WRITE_BACKUP_DELAY;

      // Hit 64KB territory?
      if (address >= 0x8000)
        sram_size = SRAM_SIZE_64KB;

      gamepak_backup[address] = value;
    }

  }

  return CPU_ALERT_NONE;
}


// RTC code derived from VBA's (due to lack of any real publically available
// documentation...)

static u32 encode_bcd(u8 value)
{
  return ((value / 10) << 4) | (value % 10);
}

#define WRITE_RTC_REGISTER(index, _value)                                     \
  update_address = 0x80000C4 + (index << 1);                                  \
  rtc_registers[index] = _value;                                              \
  rtc_page_index = update_address >> 15;                                      \
  map = memory_map_read[rtc_page_index];                                      \
                                                                              \
  if (map == NULL)                                                            \
    map = load_gamepak_page(rtc_page_index & 0x3FF);                          \
                                                                              \
  ADDRESS16(map, update_address & 0x7FFF) = _value;                           \

CPU_ALERT_TYPE write_rtc(u32 address, u32 value)
{
  u32 rtc_page_index;
  u32 update_address;
  u8 *map = NULL;

  address &= 0xFF;
  value &= 0xFFFF;

  switch (address)
  {
    // RTC command
    // Bit 0: SCHK, perform action
    // Bit 1: IO, input/output command data
    // Bit 2: CS, select input/output? If high make I/O write only
    case 0xC4:
      if (rtc_state == RTC_DISABLED)
        rtc_state = RTC_IDLE;

      if ((rtc_registers[0] & 0x04) == 0)
        value = (rtc_registers[0] & 0x02) | (value & ~0x02);

      if ((rtc_registers[2] & 0x01) != 0)
      {
        // To begin writing a command 1, 5 must be written to the command
        // registers.
        if ((rtc_state == RTC_IDLE) && (rtc_registers[0] == 0x01) && (value == 0x05))
        {
          // We're now ready to begin receiving a command.
          WRITE_RTC_REGISTER(0, value);
          rtc_state = RTC_COMMAND;
          rtc_command = 0;
          rtc_bit_count = 7;
        }
        else
        {
          WRITE_RTC_REGISTER(0, value);
          switch (rtc_state)
          {
            // Accumulate RTC command by receiving the next bit, and if we
            // have accumulated enough bits to form a complete command
            // execute it.
            case RTC_COMMAND:
              if ((rtc_registers[0] & 0x01) != 0)
              {
                rtc_command |= (((value & 0x02) >> 1) << rtc_bit_count);
                rtc_bit_count--;
              }

              // Have we received a full RTC command? If so execute it.
              if (rtc_bit_count < 0)
              {
                switch (rtc_command)
                {
                  // Resets RTC
                  case RTC_COMMAND_RESET:
                    rtc_state = RTC_IDLE;
                    memset(rtc_registers, 0, sizeof(rtc_registers));
                    break;

                  // Sets status of RTC
                  case RTC_COMMAND_WRITE_STATUS:
                    rtc_state = RTC_INPUT_DATA;
                    rtc_data_bytes = 1;
                    rtc_write_mode = RTC_WRITE_STATUS;
                    break;

                  // Outputs current status of RTC
                  case RTC_COMMAND_READ_STATUS:
                    rtc_state = RTC_OUTPUT_DATA;
                    rtc_data_bytes = 1;
                    rtc_data[0] = rtc_status;
                    break;

                  // Actually outputs the time, all of it
                  case RTC_COMMAND_OUTPUT_TIME_FULL:
                  {
                    pspTime current_time;
                    int day_of_week;

                    sceRtcGetCurrentClockLocalTime(&current_time);
                    day_of_week = sceRtcGetDayOfWeek(current_time.year, current_time.month, current_time.day);

                    if (day_of_week == 0)
                      day_of_week = 6;
                    else
                      day_of_week--;

                    rtc_state = RTC_OUTPUT_DATA;
                    rtc_data_bytes = 7;
                    rtc_data[0] = encode_bcd(current_time.year % 100);
                    rtc_data[1] = encode_bcd(current_time.month);
                    rtc_data[2] = encode_bcd(current_time.day);
                    rtc_data[3] = encode_bcd(day_of_week);
                    rtc_data[4] = encode_bcd(current_time.hour);
                    rtc_data[5] = encode_bcd(current_time.minutes);
                    rtc_data[6] = encode_bcd(current_time.seconds);
                    break;
                  }

                  // Only outputs the current time of day.
                  case RTC_COMMAND_OUTPUT_TIME:
                  {
                    pspTime current_time;
                    sceRtcGetCurrentClockLocalTime(&current_time);

                    rtc_state = RTC_OUTPUT_DATA;
                    rtc_data_bytes = 3;
                    rtc_data[0] = encode_bcd(current_time.hour);
                    rtc_data[1] = encode_bcd(current_time.minutes);
                    rtc_data[2] = encode_bcd(current_time.seconds);
                    break;
                  }
                }
                rtc_bit_count = 0;
              }
              break;

            // Receive parameters from the game as input to the RTC
            // for a given command. Read one bit at a time.
            case RTC_INPUT_DATA:
              // Bit 1 of parameter A must be high for input
              if ((rtc_registers[1] & 0x02) != 0)
              {
                // Read next bit for input
                if ((value & 0x01) == 0)
                {
                  rtc_data[rtc_bit_count >> 3] |= ((value & 0x01) << (7 - (rtc_bit_count & 0x07)));
                }
                else
                {
                  rtc_bit_count++;

                  if ((u32)rtc_bit_count == (rtc_data_bytes << 3))
                  {
                    rtc_state = RTC_IDLE;
                    switch (rtc_write_mode)
                    {
                      case RTC_WRITE_STATUS:
                        rtc_status = rtc_data[0];
                        break;

                      case RTC_WRITE_TIME:
                      case RTC_WRITE_TIME_FULL:
                        break;
                    }
                  }
                }
              }
              break;

            case RTC_OUTPUT_DATA:
              // Bit 1 of parameter A must be low for output
              if ((rtc_registers[1] & 0x02) == 0)
              {
                // Write next bit to output, on bit 1 of parameter B
                if ((value & 0x01) == 0)
                {
                  u8 current_output_byte = rtc_registers[2];
                  current_output_byte = (current_output_byte & ~0x02) | (((rtc_data[rtc_bit_count >> 3] >> (rtc_bit_count & 0x07)) & 0x01) << 1);

                  WRITE_RTC_REGISTER(0, current_output_byte);
                }
                else
                {
                  rtc_bit_count++;

                  if ((u32)rtc_bit_count == (rtc_data_bytes << 3))
                  {
                    rtc_state = RTC_IDLE;
                    memset(rtc_registers, 0, sizeof(rtc_registers));
                  }
                }
              }
              break;

            case RTC_DISABLED:
            case RTC_IDLE:
              break;
          }
        }
      }
      else
      {
        WRITE_RTC_REGISTER(2, value);
      }
      break;

    // Write parameter A
    case 0xC6:
      WRITE_RTC_REGISTER(1, value);
      break;

    // Write parameter B
    case 0xC8:
      WRITE_RTC_REGISTER(2, value);
      break;
  }

  return CPU_ALERT_NONE;
}


u8 read_memory8(u32 address)
{
  u32 region = address >> 24;

  if ((region & 0xF0) != 0)
    return read8_open(address);

  return (u8)(*mem_read8[region])(address);
}

s16 read_memory16_signed(u32 address)
{
  if ((address & 0x01) != 0)
    return (s8)read_memory8(address);

  return (s16)read_memory16(address);
}

// unaligned reads are actually 32bit

u32 read_memory16(u32 address)
{
  u32 value;
  u32 region = address >> 24;
  u32 rotate = address & 0x01;

  if ((region & 0xF0) != 0)
    value = read16_open(address);
  else
    value = (*mem_read16[region])(address & ~0x01);

  if (rotate != 0)
  {
    ROR(value, value, 8);
  }

  return value;
}

u32 read_memory32(u32 address)
{
  u32 value;
  u32 region = address >> 24;
  u32 rotate = (address & 0x03) << 3;

  if ((region & 0xF0) != 0)
    value = read32_open(address);
  else
    value = (*mem_read32[region])(address & ~0x03);

  if (rotate != 0)
  {
    ROR(value, value, rotate);
  }

  return value;
}


u8 read_open_memory8(u32 address)
{
  return (u8)(*open_read8[address >> 24])(address);
}

u16 read_open_memory16(u32 address)
{
  return (u16)(*open_read16[address >> 24])(address & ~0x01);
}

u32 read_open_memory32(u32 address)
{
  return (*open_read32[address >> 24])(address & ~0x03);
}


CPU_ALERT_TYPE write_memory8(u32 address, u8 value)
{
  u32 region = address >> 24;

  if ((region & 0xF0) != 0)
    return CPU_ALERT_NONE;

  return (*mem_write8[region])(address, value);
}

CPU_ALERT_TYPE write_memory16(u32 address, u16 value)
{
  u32 region = address >> 24;

  if ((region & 0xF0) != 0)
    return CPU_ALERT_NONE;

  return (*mem_write16[region])(address & ~0x01, value);
}

CPU_ALERT_TYPE write_memory32(u32 address, u32 value)
{
  u32 region = address >> 24;

  if ((region & 0xF0) != 0)
    return CPU_ALERT_NONE;

  return (*mem_write32[region])(address & ~0x03, value);
}


// 2N + 2(n-1)S + xI
// 2I (normally), or 4I (if both source and destination are in gamepak memory area)

#define COUNT_DMA_CYCLES()                                                    \
{                                                                             \
  u8 *ws_n = memory_waitstate_n[dma->length_type];                            \
  u8 *ws_s = memory_waitstate_s[dma->length_type];                            \
  u8 src_region = src_ptr >> 24;                                              \
  u8 dest_region = dest_ptr >> 24;                                            \
                                                                              \
  dma_cycle_count += ws_n[src_region] + ws_n[dest_region] + 2 + ((ws_s[src_region] + ws_s[dest_region] + 2) * (length - 1)) + 2; \
}                                                                             \

#define DMA_TRANSFER_LOOP(type)                                               \
  while (length--)                                                            \
  {                                                                           \
    read_value = (*dma_read##type[src_ptr >> 24])(src_ptr);                   \
    src_ptr += src_increment;                                                 \
                                                                              \
    return_value |= (*dma_write##type[dest_ptr >> 24])(dest_ptr, read_value); \
    dest_ptr += dest_increment;                                               \
  }                                                                           \

CPU_ALERT_TYPE dma_transfer(DmaTransferType *dma)
{
  u32 read_value = 0;
  CPU_ALERT_TYPE return_value = CPU_ALERT_DMA;

  u32 length = dma->length;

  u32 src_ptr  = dma->source_address;
  u32 dest_ptr = dma->dest_address;

  s32 src_increment  = dma_addr_control[dma->source_direction];
  s32 dest_increment = dma_addr_control[dma->dest_direction];

  COUNT_DMA_CYCLES();

  if (dma->length_type == DMA_16BIT)
  {
    src_ptr  &= ~0x01;
    dest_ptr &= ~0x01;

    DMA_TRANSFER_LOOP(16);

    reg[CPU_DMA_LAST] = read_value | (read_value << 16);
  }
  else
  {
    src_ptr  &= ~0x03;
    dest_ptr &= ~0x03;

    src_increment  <<= 1;
    dest_increment <<= 1;

    DMA_TRANSFER_LOOP(32);

    reg[CPU_DMA_LAST] = read_value;
  }


  reg[CPU_DMA_HACK] = 1;

  dma->source_address = src_ptr;

  if (dma->dest_direction == DMA_RELOAD)
    dma->dest_address = ADDRESS32(io_registers, 0xB4 + (dma->dma_channel * 12));
  else
    dma->dest_address = dest_ptr;

  if ((dma->repeat_type == DMA_REPEAT) && (dma->start_type != DMA_START_SPECIAL))
  {
    u32 length_max;
    length = ADDRESS16(io_registers, 0xB8 + (dma->dma_channel * 12));
    length_max = (dma->dma_channel == 3) ? 0x10000 : 0x4000;

    dma->length = (length == 0) ? length_max : length;
  }

  if ((dma->repeat_type == DMA_NO_REPEAT) || (dma->start_type == DMA_START_IMMEDIATELY))
  {
    dma->start_type = DMA_INACTIVE;
    dma->direct_sound_channel = DMA_NO_DIRECT_SOUND;

    ADDRESS16(io_registers, 0xBA + (dma->dma_channel * 12)) &= 0x7FFF;
  }

  if (dma->irq != 0)
  {
    pIO_REG(REG_IF) |= (IRQ_DMA0 << dma->dma_channel);
    return_value |= CPU_ALERT_IRQ;
  }

  return return_value;
}


// Be sure to do this after loading ROMs.

#define MAP_REGION(type, start, end, mirror_blocks, region)                   \
  for (i = ((start) / 0x8000); i < ((end) / 0x8000); i++)                     \
  {                                                                           \
    memory_map_##type[i] = ((u8 *)region) + ((i % (mirror_blocks)) * 0x8000); \
  }                                                                           \

#define MAP_NULL(type, start, end)                                            \
  for (i = ((start) / 0x8000); i < ((end) / 0x8000); i++)                     \
  {                                                                           \
    memory_map_##type[i] = NULL;                                              \
  }                                                                           \

#define MAP_VRAM(type)                                                        \
  for (i = (0x6000000 / 0x8000); i < (0x7000000 / 0x8000); i += 4)            \
  {                                                                           \
    memory_map_##type[i] = vram;                                              \
    memory_map_##type[i + 1] = vram + 0x8000;                                 \
    memory_map_##type[i + 2] = vram + (0x8000 * 2);                           \
    memory_map_##type[i + 3] = vram + (0x8000 * 2);                           \
  }                                                                           \


static u32 evict_gamepak_page(void)
{
  // Find the one with the smallest frame timestamp
  u32 page_index = 0;
  u32 physical_index;
  u32 smallest = gamepak_memory_map[0].page_timestamp;
  u32 i;

  for (i = 1; i < gamepak_ram_pages; i++)
  {
    if (gamepak_memory_map[i].page_timestamp <= smallest)
    {
      smallest = gamepak_memory_map[i].page_timestamp;
      page_index = i;
    }
  }

  physical_index = gamepak_memory_map[page_index].physical_index;

  memory_map_read[(0x8000000 / (32 * 1024)) + physical_index] = NULL;
  memory_map_read[(0xA000000 / (32 * 1024)) + physical_index] = NULL;
  memory_map_read[(0xC000000 / (32 * 1024)) + physical_index] = NULL;

  return page_index;
}

u8 *load_gamepak_page(u32 physical_index)
{
  if (physical_index >= (gamepak_size >> 15))
    return gamepak_rom;

  u32 page_index = evict_gamepak_page();
  u32 page_offset = page_index * (32 * 1024);
  u8 *swap_location = gamepak_rom + page_offset;

  gamepak_memory_map[page_index].page_timestamp = page_time;
  gamepak_memory_map[page_index].physical_index = physical_index;
  page_time++;

  FILE_SEEK(gamepak_file_large, physical_index * (32 * 1024), SEEK_SET);
  FILE_READ(gamepak_file_large, swap_location, (32 * 1024));

  memory_map_read[(0x8000000 / (32 * 1024)) + physical_index] = swap_location;
  memory_map_read[(0xA000000 / (32 * 1024)) + physical_index] = swap_location;
  memory_map_read[(0xC000000 / (32 * 1024)) + physical_index] = swap_location;

  // If RTC is active page the RTC register bytes so they can be read
  if ((rtc_state != RTC_DISABLED) && (physical_index == 0))
    memcpy(swap_location + 0xC4, rtc_registers, sizeof(rtc_registers));

  return swap_location;
}

static void init_memory_gamepak(void)
{
  u32 i = 0;

  if (gamepak_size > gamepak_ram_buffer_size)
  {
    // Large ROMs get special treatment because they
    // can't fit into the 16MB ROM buffer.
    u32 i;
    for (i = 0; i < gamepak_ram_pages; i++)
    {
      gamepak_memory_map[i].page_timestamp = 0;
      gamepak_memory_map[i].physical_index = 0;
    }

    MAP_NULL(read, 0x8000000, 0xE000000);
  }
  else
  {
    MAP_REGION(read, 0x8000000, 0x8000000 + gamepak_size, 1024, gamepak_rom);
    MAP_NULL(read, 0x8000000 + gamepak_size, 0xA000000);
    MAP_REGION(read, 0xA000000, 0xA000000 + gamepak_size, 1024, gamepak_rom);
    MAP_NULL(read, 0xA000000 + gamepak_size, 0xC000000);
    MAP_REGION(read, 0xC000000, 0xC000000 + gamepak_size, 1024, gamepak_rom);
    MAP_NULL(read, 0xC000000 + gamepak_size, 0xE000000);
  }
}

void init_gamepak_buffer(void)
{
  gamepak_ram_buffer_size = 32 * 1024 * 1024;
  gamepak_rom = (u8 *)memalign(MEM_ALIGN, gamepak_ram_buffer_size);

  while (gamepak_rom == NULL)
  {
    gamepak_ram_buffer_size >>= 1;

    if (gamepak_ram_buffer_size == 0)
    {
      error_msg(MSG[MSG_ERR_MALLOC], CONFIRMATION_QUIT);
      quit();
    }

    gamepak_rom = (u8 *)memalign(MEM_ALIGN, gamepak_ram_buffer_size);
  }

  memset(gamepak_rom, 0, gamepak_ram_buffer_size);

  // Here's assuming we'll have enough memory left over for this,
  // and that the above succeeded (if not we're in trouble all around)
  gamepak_ram_pages = gamepak_ram_buffer_size / (32 * 1024);
  gamepak_memory_map = (GamepakSwapEntryType *)safe_malloc(sizeof(GamepakSwapEntryType) * gamepak_ram_pages);

  memset(gamepak_memory_map, 0, sizeof(GamepakSwapEntryType) * gamepak_ram_pages);
}


void init_memory(void)
{
  u32 i = 0;

  // Fill memory map regions, areas marked as NULL must be checked directly
  MAP_REGION(read, 0x0000000, 0x1000000, 1, bios.rom);
  MAP_NULL(read, 0x1000000, 0x2000000);
  MAP_REGION(read, 0x2000000, 0x3000000, 8, ewram);
  MAP_REGION(read, 0x3000000, 0x4000000, 1, iwram);
  MAP_NULL(read, 0x4000000, 0x5000000);
  MAP_NULL(read, 0x5000000, 0x6000000);
  MAP_VRAM(read);
  MAP_NULL(read, 0x7000000, 0x8000000);
  init_memory_gamepak();
  MAP_NULL(read, 0xE000000, 0x10000000);

  // Fill memory map regions, areas marked as NULL must be checked directly
  MAP_NULL(write, 0x0000000, 0x2000000);
  MAP_REGION(write, 0x2000000, 0x3000000, 8, ewram);
  MAP_REGION(write, 0x3000000, 0x4000000, 1, iwram);
  MAP_NULL(write, 0x4000000, 0x5000000);
  MAP_NULL(write, 0x5000000, 0x6000000);
  MAP_VRAM(write);
  MAP_NULL(write, 0x7000000, 0x8000000);
  MAP_NULL(write, 0x8000000, 0xE000000);
  MAP_NULL(write, 0xE000000, 0x10000000);

  memset(iwram, 0, sizeof(iwram));
  memset(ewram, 0, sizeof(ewram));
  memset(vram,  0, sizeof(vram));

  memset(io_registers, 0, sizeof(io_registers));
  memset(oam_ram,      0, sizeof(oam_ram));
  memset(palette_ram,  0, sizeof(palette_ram));

  io_registers[REG_DISPCNT]   = 0x0080;
  io_registers[REG_DISPSTAT]  = 0x0000;
  io_registers[REG_VCOUNT]    = option_boot_mode ? 0x0000 : 0x007e;
  io_registers[REG_P1]        = 0x03FF;
  io_registers[REG_BG2PA]     = 0x0100;
  io_registers[REG_BG2PD]     = 0x0100;
  io_registers[REG_BG3PA]     = 0x0100;
  io_registers[REG_BG3PD]     = 0x0100;
  io_registers[REG_RCNT]      = 0x800F;//0x8000
  io_registers[REG_SOUNDBIAS] = 0x0200;

  sio_control(0x0004);//add
  waitstate_control(0x0000);

  obj_address = 0x10000;

  affine_reference_x[0] = 0;
  affine_reference_x[1] = 0;
  affine_reference_y[0] = 0;
  affine_reference_y[1] = 0;

  oam_update = 1;

  //sram_size = SRAM_SIZE_32KB;//
  //flash_size = FLASH_SIZE_64KB;//

  flash_mode = FLASH_BASE_MODE;
  flash_bank = 0;
  flash_command_position = 0;
  //eeprom_size = EEPROM_512_BYTE;//
  eeprom_mode = EEPROM_BASE_MODE;
  eeprom_address = 0;
  eeprom_counter = 0;

  rtc_state = RTC_DISABLED;
  rtc_status = 0x40;
  memset(rtc_registers, 0, sizeof(rtc_registers));

  bios_read_protect = 0xe129f000;

  enable_tilt_sensor = 0;

  read_rom_region  = 0xFFFFFFFF;
  read_ram_region  = 0xFFFFFFFF;

  iwram_control = 0x0d000020;

  for(i = 0; i < 0x400; i++)
    io_readable[i] = 1;
  for(i = 0x10; i < 0x48; i++)
    io_readable[i] = 0;
  for(i = 0x4c; i < 0x50; i++)
    io_readable[i] = 0;
  for(i = 0x54; i < 0x60; i++)
    io_readable[i] = 0;
  for(i = 0x8c; i < 0x90; i++)
    io_readable[i] = 0;
  for(i = 0xa0; i < 0xb8; i++)
    io_readable[i] = 0;
  for(i = 0xbc; i < 0xc4; i++)
    io_readable[i] = 0;
  for(i = 0xc8; i < 0xd0; i++)
    io_readable[i] = 0;
  for(i = 0xd4; i < 0xdc; i++)
    io_readable[i] = 0;
  for(i = 0xe0; i < 0x100; i++)
    io_readable[i] = 0;
  for(i = 0x110; i < 0x120; i++)
    io_readable[i] = 0;
  for(i = 0x12c; i < 0x130; i++)
    io_readable[i] = 0;
  for(i = 0x138; i < 0x140; i++)
    io_readable[i] = 0;
  for(i = 0x144; i < 0x150; i++)
    io_readable[i] = 0;
  for(i = 0x15c; i < 0x200; i++)
    io_readable[i] = 0;
  for(i = 0x20c; i < 0x300; i++)
    io_readable[i] = 0;
  for(i = 0x304; i < 0x400; i++)
    io_readable[i] = 0;
}


void memory_term(void)
{
  if (FILE_CHECK_VALID(gamepak_file_large))
  {
    FILE_CLOSE(gamepak_file_large);
  }

  if (gamepak_memory_map != NULL)
  {
    free(gamepak_memory_map);
    gamepak_memory_map = NULL;
  }

  if (gamepak_rom != NULL)
  {
    free(gamepak_rom);
    gamepak_rom = NULL;
  }
}


static void load_backup_id(void)
{
  u32 addr = 0x08000000 + gamepak_size - 4;

  u8 *block = NULL;
  u32 *data = NULL;

  u32 region = 0xFFFFFFFF;
  u32 new_region = 0;

  init_memory_gamepak();

  backup_type = BACKUP_NONE;

  sram_size   = SRAM_SIZE_32KB;
  flash_size  = FLASH_SIZE_64KB;
  eeprom_size = EEPROM_512_BYTE;

//  backup_id[0] = 0;

  for ( ; addr > 0x08000000; addr -= 4)
  {
    new_region = addr >> 15;

    if (new_region != region)
    {
      region = new_region;
      block = memory_map_read[region];

      if (block == NULL)
        block = load_gamepak_page(region & 0x3FF);
    }

    data = (u32 *)(block + (addr & 0x7FFC));

    switch (data[0])
    {
      case ('E' | ('E' << 8) | ('P' << 16) | ('R' << 24)):
      {
        // EEPROM_Vxxx : EEPROM 512 bytes or 8 Kbytes (4Kbit or 64Kbit)
        if (memcmp(data, "EEPROM_V", 8) == 0)
        {
          backup_type = BACKUP_EEPROM;
/*
          memcpy(backup_id, data, 11);
          backup_id[11] = 0;
*/
          return;
        }
      }
      break;

      case ('S' | ('R' << 8) | ('A' << 16) | ('M' << 24)):
      {
        // SRAM_Vxxx : SRAM 32 Kbytes (256Kbit)
        if (memcmp(data, "SRAM_V", 6) == 0)
        {
          backup_type = BACKUP_SRAM;
          sram_size   = SRAM_SIZE_32KB;
/*
          memcpy(backup_id, data, 9);
          backup_id[9] = 0;
*/
          return;
        }

        // SRAM_F_Vxxx : FRAM 32 Kbytes (256Kbit)
        if (memcmp(data, "SRAM_F", 6) == 0)
        {
          backup_type = BACKUP_SRAM;
          sram_size   = SRAM_SIZE_32KB;
/*
          memcpy(backup_id, data, 11);
          backup_id[11] = 0;
*/
          return;
        }
      }
      break;

      case ('F' | ('L' << 8) | ('A' << 16) | ('S' << 24)):
      {
        // FLASH_Vxxx : FLASH 64 Kbytes (512Kbit) (ID used in older files)
        if (memcmp(data, "FLASH_V", 7) == 0)
        {
          backup_type = BACKUP_FLASH;
          flash_size  = FLASH_SIZE_64KB;
          flash_device_id = FLASH_DEVICE_PANASONIC_64KB;
          flash_manufacturer_id = FLASH_MANUFACTURER_PANASONIC;
/*
          memcpy(backup_id, data, 10);
          backup_id[10] = 0;
*/
          return;
        }

        // FLASH512_Vxxx : FLASH 64 Kbytes (512Kbit) (ID used in newer files)
        if (memcmp(data, "FLASH512", 8) == 0)
        {
          backup_type = BACKUP_FLASH;
          flash_size  = FLASH_SIZE_64KB;
          flash_device_id = FLASH_DEVICE_PANASONIC_64KB;
          flash_manufacturer_id = FLASH_MANUFACTURER_PANASONIC;
/*
          memcpy(backup_id, data, 13);
          backup_id[13] = 0;
*/
          return;
        }

        // FLASH1M_Vxxx : FLASH 128 Kbytes (1Mbit)
        if (memcmp(data, "FLASH1M_", 8) == 0)
        {
          backup_type = BACKUP_FLASH;
          flash_size  = FLASH_SIZE_128KB;
          flash_device_id = FLASH_DEVICE_SANYO_128KB;
          flash_manufacturer_id = FLASH_MANUFACTURER_SANYO;
/*
          memcpy(backup_id, data, 12);
          backup_id[12] = 0;
*/
          return;
        }
      }
      break;
    }
  }
}

s32 load_backup(char *name)
{
  SceUID backup_file;
  char backup_path[MAX_PATH];

  memset(gamepak_backup, 0xFF, 1024 * 128);

  sprintf(backup_path, "%s%s", dir_save, name);

  FILE_OPEN(backup_file, backup_path, READ);

  if (FILE_CHECK_VALID(backup_file))
  {
    u32 backup_size = file_length(backup_path);

    FILE_READ(backup_file, gamepak_backup, backup_size);
    FILE_CLOSE(backup_file);
/*
    if (backup_type != BACKUP_NONE)
    {
      switch (backup_size)
      {
        case 0x200:
          eeprom_size = EEPROM_512_BYTE;
          break;

        case 0x2000:
          eeprom_size = EEPROM_8_KBYTE;
          break;
      }
    }
    else*/
    {
      // The size might give away what kind of backup it is.
      switch (backup_size)
      {
        case 0x200:
          backup_type = BACKUP_EEPROM;
          eeprom_size = EEPROM_512_BYTE;
          break;

        case 0x2000:
          backup_type = BACKUP_EEPROM;
          eeprom_size = EEPROM_8_KBYTE;
          break;

        case 0x8000:
          backup_type = BACKUP_SRAM;
          sram_size   = SRAM_SIZE_32KB;
          break;

        // Could be either flash or SRAM, go with flash
        case 0x10000:
          sram_size  = BACKUP_FLASH;
          flash_size = FLASH_SIZE_64KB;
          flash_device_id = FLASH_DEVICE_PANASONIC_64KB;
          flash_manufacturer_id = FLASH_MANUFACTURER_PANASONIC;
          break;

        case 0x20000:
          backup_type = BACKUP_FLASH;
          flash_size  = FLASH_SIZE_128KB;
          flash_device_id = FLASH_DEVICE_SANYO_128KB;
          flash_manufacturer_id = FLASH_MANUFACTURER_SANYO;
          break;
      }
	}

    return 0;
  }

    return -1;
}

static u32 save_backup(char *name)
{
  SceUID backup_file;
  char backup_path[MAX_PATH];

  u32 backup_size = 0;

  if (backup_type != BACKUP_NONE)
  {
    sprintf(backup_path, "%s%s", dir_save, name);

    scePowerLock(0);

    FILE_OPEN(backup_file, backup_path, WRITE);

    if (FILE_CHECK_VALID(backup_file))
    {
      switch (backup_type)
      {
        case BACKUP_SRAM:
          backup_size = sram_size;
          break;

        case BACKUP_FLASH:
          backup_size = flash_size;
          break;

        case BACKUP_EEPROM:
          backup_size = eeprom_size;
          break;

        default:
        case BACKUP_NONE:
          backup_size = 0x8000;
          break;
      }

      FILE_WRITE(backup_file, gamepak_backup, backup_size);
      FILE_CLOSE(backup_file);
    }

    scePowerUnlock(0);
  }

  return backup_size;
}

void update_backup(void)
{
  if (backup_update != (WRITE_BACKUP_DELAY + 1))
    backup_update--;

  if (backup_update == 0)
  {
    save_backup(backup_filename);
    backup_update = WRITE_BACKUP_DELAY + 1;
  }
}

void update_backup_immediately(void)
{
  if (backup_update != (WRITE_BACKUP_DELAY + 1))
  {
    save_backup(backup_filename);
    backup_update = WRITE_BACKUP_DELAY + 1;
  }
}


static char *skip_spaces(char *line_ptr)
{
  while (*line_ptr == ' ')
    line_ptr++;

  return line_ptr;
}


s32 parse_config_line(char *current_line, char *current_variable, char *current_value)
{
  char *line_ptr = current_line;
  char *line_ptr_new;

  if ((current_line[0] == 0) || (current_line[0] == '#'))
    return -1;

  line_ptr_new = strchr(line_ptr, ' ');
  if (line_ptr_new == NULL)
    return -1;

  *line_ptr_new = 0;
  strcpy(current_variable, line_ptr);
  line_ptr_new = skip_spaces(line_ptr_new + 1);

  if (*line_ptr_new != '=')
    return -1;

  line_ptr_new = skip_spaces(line_ptr_new + 1);
  strcpy(current_value, line_ptr_new);
  line_ptr_new = current_value + strlen(current_value) - 1;
  if (*line_ptr_new == '\n')
  {
    line_ptr_new--;
    *line_ptr_new = 0;
  }

  if (*line_ptr_new == '\r')
    *line_ptr_new = 0;

  return 0;
}

static s32 load_game_config(char *gamepak_title, char *gamepak_code, char *gamepak_maker)
{
  char current_line[256];
  char current_variable[256];
  char current_value[256];
  char config_path[MAX_PATH];
  FILE *config_file;

  u32 i;

  idle_loop_targets = 0;
  for (i = 0; i < MAX_IDLE_LOOPS; i++)
    idle_loop_target_pc[i] = 0xFFFFFFFF;

  iwram_stack_optimize = 1;

  bios.rom[0x39] = 0x00;
  bios.rom[0x2C] = 0x00;

  //translation_gate_targets = 0;
/*fix no game_code*/
  //flash_device_id = FLASH_DEVICE_MACRONIX_64KB;
  //backup_type = BACKUP_NONE;

  sprintf(config_path, "%s%s", main_path, CONFIG_FILENAME);

  config_file = fopen(config_path, "rb");

  if (config_file)
  {
    while (fgets(current_line, 256, config_file))
    {
      if (parse_config_line(current_line, current_variable, current_value) != -1)
      {
        if (strcasecmp(current_variable, "game_name") ||
            strcasecmp(current_value, gamepak_title))
        {
          continue;
        }
        if (!fgets(current_line, 256, config_file) ||
            (parse_config_line(current_line, current_variable, current_value) == -1) ||
            strcasecmp(current_variable, "game_code") ||
            strcasecmp(current_value, gamepak_code))
        {
          continue;
        }
        if (!fgets(current_line, 256, config_file) ||
            (parse_config_line(current_line, current_variable, current_value) == -1) ||
            strcasecmp(current_variable, "vender_code") ||
            strcasecmp(current_value, gamepak_maker))
        {
          continue;
        }

        while (fgets(current_line, 256, config_file))
        {
          if (parse_config_line(current_line, current_variable, current_value) != -1)
          {
            if (!strcasecmp(current_variable, "game_name"))
            {
              fclose(config_file);
              return 0;
            }

            if (!strcasecmp(current_variable, "idle_loop_eliminate_target"))
            {
              if (idle_loop_targets < MAX_IDLE_LOOPS)
              {
                idle_loop_target_pc[idle_loop_targets] = strtol(current_value, NULL, 16);
                idle_loop_targets++;
              }
            }

            if (!strcasecmp(current_variable, "iwram_stack_optimize") &&
                !strcasecmp(current_value, "no"))
            {
              iwram_stack_optimize = 0;
            }
/*
            if (!strcasecmp(current_variable, "flash_rom_type") && !strcasecmp(current_value, "128KB"))
            {
              flash_device_id = FLASH_DEVICE_MACRONIX_128KB;
            }

            // DBZLGCYGOKU2 のプロテクト回避
            // EEPROM_V124で特殊な物(現在判別不可) で指定すれば動作可
            if (!strcasecmp(current_variable, "save_type"))
            {
              if (!strcasecmp(current_value, "sram"))
                backup_type = BACKUP_SRAM;
              else
              if (!strcasecmp(current_value, "flash"))
                backup_type = BACKUP_FLASH;
              else
              if (!strcasecmp(current_value, "eeprom"))
                backup_type = BACKUP_EEPROM;
            }
*/
            if (!strcasecmp(current_variable, "bios_rom_hack_39") &&
                !strcasecmp(current_value, "yes"))
            {
              bios.rom[0x39] = 0xC0;
            }

            if (!strcasecmp(current_variable, "bios_rom_hack_2C") &&
                !strcasecmp(current_value, "yes"))
            {
               bios.rom[0x2C] = 0x02;
            }
          }
        }

        fclose(config_file);

        return 0;
      }
    }

    fclose(config_file);
  }

  return -1;
}


static s32 load_gamepak_raw(char *name)
{
  SceUID gamepak_file;

  FILE_OPEN(gamepak_file, name, READ);

  if (FILE_CHECK_VALID(gamepak_file))
  {
    u32 _gamepak_size = file_length(name);

    // If it's a big file size keep it don't close it, we'll
    // probably want to load it later
    if (_gamepak_size <= gamepak_ram_buffer_size)
    {
      FILE_READ(gamepak_file, gamepak_rom, _gamepak_size);
      FILE_CLOSE(gamepak_file);
    }
    else
    {
      // Read in just enough for the header
      FILE_READ(gamepak_file, gamepak_rom, 0x100);

      gamepak_file_large = gamepak_file;

      if (strrchr(name, '/') != NULL)
      {
        strcpy(gamepak_filename_raw, name);
      }
      else
      {
        char current_dir[MAX_PATH];

        getcwd(current_dir, MAX_PATH);
        sprintf(gamepak_filename_raw, "%s/%s", current_dir, name);
      }
    }

    return _gamepak_size;
  }

  return -1;
}

s32 load_gamepak(char *name)
{
  char *dot_position = strrchr(name, '.');
  char cheats_filename[MAX_FILE];

  s32 file_size = -1;
  gamepak_file_large = -1;

  draw_box_alpha(110, 50, 370, 220, 0xBF000000);
  draw_box_line(120, 60, 360, 210, COLOR15_WHITE);
  if (option_language == 0)
	print_string(MSG[MSG_LOADING_ROM], X_POS_CENTER, 100, COLOR15_WHITE, BG_NO_FILL);
  else
	print_string_gbk(MSG[MSG_LOADING_ROM], X_POS_CENTER, 100, COLOR15_WHITE, BG_NO_FILL);
  flip_screen(1);
  draw_box_alpha(110, 50, 370, 220, 0xBF000000);
  draw_box_line(120, 60, 360, 210, COLOR15_WHITE);
  if (option_language == 0)
	print_string(MSG[MSG_LOADING_ROM], X_POS_CENTER, 100, COLOR15_WHITE, BG_NO_FILL);
  else
	print_string_gbk(MSG[MSG_LOADING_ROM], X_POS_CENTER, 100, COLOR15_WHITE, BG_NO_FILL);

  scePowerLock(0);
  set_cpu_clock(PSP_CLOCK_333);

  if (!strcasecmp(dot_position, ".zip") || !strcasecmp(dot_position, ".gbz"))
  {
    file_size = load_file_zip(name);
  }
  else

  if (!strcasecmp(dot_position, ".gba") || !strcasecmp(dot_position, ".agb") || !strcasecmp(dot_position, ".bin"))
  {
    file_size = load_gamepak_raw(name);
	if (option_language == 0)
		print_string(MSG[MSG_SEARCHING_BACKUP_ID], X_POS_CENTER, 148, COLOR15_WHITE, BG_NO_FILL);
    else
		print_string_gbk(MSG[MSG_SEARCHING_BACKUP_ID], X_POS_CENTER, 148, COLOR15_WHITE, BG_NO_FILL);
    flip_screen(1);
  }

  if (file_size > 0)
  {
    gamepak_size = (file_size + 0x7FFF) & ~0x7FFF;

    char *p = strrchr(name, '/');
    if (p != NULL)
      name = p + 1;

    sprintf(gamepak_filename, "%s", name);

    char game_title[13];
    char game_code[5];
    char maker_code[3];

    memcpy(game_title, gamepak_rom + 0xA0, 12);
    memcpy(game_code,  gamepak_rom + 0xAC,  4);
    memcpy(maker_code, gamepak_rom + 0xB0,  2);
    game_title[12] = 0;
    game_code[4] = 0;
    maker_code[2] = 0;
	//load_backup_id();//up order to fix no game_code
    load_game_config(game_title, game_code, maker_code);
    load_game_config_file();

	load_backup_id();
    change_ext(gamepak_filename, backup_filename, ".sav");
    load_backup(backup_filename);

    change_ext(gamepak_filename, cheats_filename, ".cht");
    add_cheats(cheats_filename);
  }

  set_cpu_clock(PSP_CLOCK_222);
  scePowerUnlock(0);

  return file_size;

}


s32 load_bios(char *name)
{
  SceUID bios_file;

  FILE_OPEN(bios_file, name, READ);

  if (FILE_CHECK_VALID(bios_file))
  {
    FILE_READ(bios_file, bios.rom, 0x4000);
    FILE_CLOSE(bios_file);

    return 0;
  }

  return -1;
}


#define SAVESTATE_BLOCK(type)                                                 \
  cpu_##type##_savestate(savestate_file);                                     \
  input_##type##_savestate(savestate_file);                                   \
  main_##type##_savestate(savestate_file);                                    \
  memory_##type##_savestate(savestate_file);                                  \
  sound_##type##_savestate(savestate_file);                                   \
  video_##type##_savestate(savestate_file);                                   \

void load_state(char *savestate_filename)
{
  SceUID savestate_file;
  char savestate_path[MAX_PATH];

  sprintf(savestate_path, "%s%s", dir_state, savestate_filename);

  scePowerLock(0);

  FILE_OPEN(savestate_file, savestate_path, READ);

  if (FILE_CHECK_VALID(savestate_file))
  {
    FILE_SEEK(savestate_file, GBA_SCREEN_SIZE + sizeof(u64), SEEK_SET);

    SAVESTATE_BLOCK(read);
    FILE_CLOSE(savestate_file);

    clear_metadata_area(METADATA_AREA_EWRAM, CLEAR_REASON_LOADING_STATE);
    clear_metadata_area(METADATA_AREA_IWRAM, CLEAR_REASON_LOADING_STATE);
    clear_metadata_area(METADATA_AREA_VRAM,  CLEAR_REASON_LOADING_STATE);

    oam_update = 1;
    gbc_sound_update = 1;
    reg[CHANGED_PC_STATUS] = 1;
  }

  scePowerUnlock(0);
}

void save_state(char *savestate_filename, u16 *screen_capture)
{
  SceUID savestate_file;
  char savestate_path[MAX_PATH];

  u8 *savestate_write_buffer;

  sprintf(savestate_path, "%s%s", dir_state, savestate_filename);

  savestate_write_buffer = (u8 *)safe_malloc(SAVESTATE_SIZE);
  memset(savestate_write_buffer, 0, SAVESTATE_SIZE);

  write_mem_ptr = savestate_write_buffer;

  scePowerLock(0);

  FILE_OPEN(savestate_file, savestate_path, WRITE);

  if (FILE_CHECK_VALID(savestate_file))
  {
    FILE_WRITE_MEM(savestate_file, screen_capture, GBA_SCREEN_SIZE);

    u64 current_time = ticker();
    FILE_WRITE_MEM_VARIABLE(savestate_file, current_time);

    SAVESTATE_BLOCK(write_mem);
    FILE_WRITE(savestate_file, savestate_write_buffer, SAVESTATE_SIZE);
    FILE_CLOSE(savestate_file);
  }

  scePowerUnlock(0);

  free(savestate_write_buffer);
}


#define MEMORY_SAVESTATE_BODY(type)                                           \
{                                                                             \
  FILE_##type##_VARIABLE(savestate_file, backup_type);                        \
                                                                              \
  FILE_##type##_VARIABLE(savestate_file, sram_size);                          \
                                                                              \
  FILE_##type##_VARIABLE(savestate_file, flash_size);                         \
  FILE_##type##_VARIABLE(savestate_file, flash_mode);                         \
  FILE_##type##_VARIABLE(savestate_file, flash_bank);                         \
  FILE_##type##_VARIABLE(savestate_file, flash_device_id);                    \
  FILE_##type##_VARIABLE(savestate_file, flash_manufacturer_id);              \
  FILE_##type##_VARIABLE(savestate_file, flash_command_position);             \
                                                                              \
  FILE_##type##_VARIABLE(savestate_file, eeprom_size);                        \
  FILE_##type##_VARIABLE(savestate_file, eeprom_mode);                        \
  FILE_##type##_VARIABLE(savestate_file, eeprom_address_length);              \
  FILE_##type##_VARIABLE(savestate_file, eeprom_address);                     \
  FILE_##type##_VARIABLE(savestate_file, eeprom_counter);                     \
  FILE_##type##_ARRAY(savestate_file, eeprom_buffer);                         \
                                                                              \
  FILE_##type##_VARIABLE(savestate_file, rtc_state);                          \
  FILE_##type##_VARIABLE(savestate_file, rtc_write_mode);                     \
  FILE_##type##_VARIABLE(savestate_file, rtc_command);                        \
  FILE_##type##_VARIABLE(savestate_file, rtc_status);                         \
  FILE_##type##_VARIABLE(savestate_file, rtc_data_bytes);                     \
  FILE_##type##_VARIABLE(savestate_file, rtc_bit_count);                      \
  FILE_##type##_ARRAY(savestate_file, rtc_registers);                         \
  FILE_##type##_ARRAY(savestate_file, rtc_data);                              \
                                                                              \
  FILE_##type##_ARRAY(savestate_file, dma);                                   \
                                                                              \
  FILE_##type(savestate_file, iwram, 0x8000);                                 \
  FILE_##type(savestate_file, ewram, 0x40000);                                \
  FILE_##type(savestate_file, vram, 0x18000);                                 \
  FILE_##type(savestate_file, oam_ram, 0x400);                                \
  FILE_##type(savestate_file, palette_ram, 0x400);                            \
  FILE_##type(savestate_file, io_registers, 0x400);                           \
}                                                                             \

static void memory_read_savestate(SceUID savestate_file)
{
  MEMORY_SAVESTATE_BODY(READ);
}

static void memory_write_mem_savestate(SceUID savestate_file)
{
  MEMORY_SAVESTATE_BODY(WRITE_MEM);
}

