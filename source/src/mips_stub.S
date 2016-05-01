# unofficial gameplaySP kai
#
# Copyright (C) 2006 Exophase <exophase@gmail.com>
# Copyright (C) 2007 takka <takka@tfact.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

.align 4

.global mips_update_gba
.global mips_indirect_branch_arm
.global mips_indirect_branch_thumb
.global mips_indirect_branch_dual
.global mips_indirect_branch_arm_no_update_gba
.global mips_indirect_branch_thumb_no_update_gba
.global mips_indirect_branch_dual_no_update_gba

.global execute_load_u8
.global execute_load_u16
.global execute_load_u32
.global execute_load_s8
.global execute_load_s16
.global execute_store_u8
.global execute_store_u16
.global execute_store_u32
.global execute_aligned_load32
.global execute_aligned_store32
.global execute_read_cpsr
.global execute_read_spsr

.global execute_swi
.global execute_spsr_restore
.global execute_store_cpsr
.global execute_store_spsr
.global execute_lsl_flags_reg
.global execute_lsr_flags_reg
.global execute_asr_flags_reg
.global execute_ror_flags_reg
.global execute_arm_translate
.global invalidate_icache_region
.global invalidate_all_cache

.global execute_force_user_mode_prologue
.global execute_force_user_mode_epilogue

.global execute_branch_ticks_arm
.global execute_branch_ticks_thumb
.global execute_branch_ticks_dual
.global execute_multiply_ticks

.global memory_map_read
.global memory_map_write
.global reg

.extern spsr

# MIPS register layout:

# $0  - constant zero
# $1  - temporary
# $2  - temporary / return value
# $3  - ARM r0 (not saved)
# $4  - temporary / function argument 0
# $5  - temporary / function argument 1
# $6  - temporary / function argument 2
# $7  - ARM r1 (not saved)
# $8  - ARM r2 (not saved)
# $9  - ARM r3 (not saved)
# $10 - ARM r4 (not saved)
# $11 - ARM r5 (not saved)
# $12 - ARM r6 (not saved)
# $13 - ARM r7 (not saved)
# $14 - ARM r8 (not saved)
# $15 - ARM r9 (not saved)
# $16 - ARM machine state pointer (saved)
# $17 - cycle counter (saved)
# $18 - ARM r10 (saved)
# $19 - block start address (roughly r15) (saved)
# $20 - ARM negative register (saved)
# $21 - ARM zero register (saved)
# $22 - ARM carry register (saved)
# $23 - ARM overflow register (saved)
# $24 - ARM r11 (not saved)
# $25 - ARM r12 (not saved)
# $26 - kernel temporary 0
# $27 - kernel temporary 1
# $28 - ARM r13 (saved)
# $29 - stack pointer
# $30 - ARM r14 (saved)
# $31 - return address

.equ REG_R0,              (0 * 4)
.equ REG_R1,              (1 * 4)
.equ REG_R2,              (2 * 4)
.equ REG_R3,              (3 * 4)
.equ REG_R4,              (4 * 4)
.equ REG_R5,              (5 * 4)
.equ REG_R6,              (6 * 4)
.equ REG_R7,              (7 * 4)
.equ REG_R8,              (8 * 4)
.equ REG_R9,              (9 * 4)
.equ REG_R10,             (10 * 4)
.equ REG_R11,             (11 * 4)
.equ REG_R12,             (12 * 4)
.equ REG_R13,             (13 * 4)
.equ REG_R14,             (14 * 4)
.equ REG_LR,              (14 * 4)
.equ REG_PC,              (15 * 4)
.equ REG_N_FLAG,          (16 * 4)
.equ REG_Z_FLAG,          (17 * 4)
.equ REG_C_FLAG,          (18 * 4)
.equ REG_V_FLAG,          (19 * 4)
.equ REG_CPSR,            (20 * 4)
.equ REG_SAVE,            (21 * 4)
.equ REG_SAVE2,           (22 * 4)
.equ REG_SAVE3,           (23 * 4)

.equ CPU_MODE,            (29 * 4)
.equ CPU_HALT_STATE,      (30 * 4)
.equ CHANGED_PC_STATUS,   (31 * 4)
.equ GP_SAVE,             (32 * 4)

.equ EXECUTE_CYCLES,      (60 * 4)

.equ CPU_DMA_HACK,        (61 * 4)
.equ CPU_DMA_LAST,        (62 * 4)

.equ REG_TMP,             (63 * 4)

.equ SUPERVISOR_LR,       (reg_mode + (3 * (7 * 4)) + (6 * 4))
.equ SUPERVISOR_SPSR,     (spsr + (3 * 4))

.set noat
.set noreorder

# make sure $16 has the register base for these macros

.macro collapse_flag flag_reg, shift
  ins $2, $\flag_reg, \shift, 1   # insert flag into CPSR
.endm

.macro collapse_flags_body        # insert flag into $2
  collapse_flag 20, 31            # store flags
  collapse_flag 21, 30
  collapse_flag 22, 29
  collapse_flag 23, 28
.endm

.macro collapse_flags
  lw $2, REG_CPSR($16)            # load CPSR
  collapse_flags_body
  sw $2, REG_CPSR($16)            # store CPSR
.endm

.macro extract_flag shift, flag_reg
  ext $\flag_reg, $1, \shift, 1   # extract flag from CPSR
.endm

.macro extract_flags_body         # extract flags from $1
  extract_flag 31, 20             # load flags
  extract_flag 30, 21
  extract_flag 29, 22
  extract_flag 28, 23
.endm

.macro extract_flags
  lw $1, REG_CPSR($16)            # load CPSR
  extract_flags_body
.endm

.macro save_registers_0
  sw $3,  REG_R0($16)
  sw $7,  REG_R1($16)
  sw $8,  REG_R2($16)
  sw $9,  REG_R3($16)
  sw $10, REG_R4($16)
  sw $11, REG_R5($16)
  sw $12, REG_R6($16)
  sw $13, REG_R7($16)
  sw $14, REG_R8($16)
  sw $15, REG_R9($16)
  sw $24, REG_R11($16)
  sw $25, REG_R12($16)

  sw $18, REG_R10($16)
  sw $28, REG_R13($16)
  sw $30, REG_R14($16)
.endm

.macro save_registers_1
  lw $28, GP_SAVE($16)
.endm

.macro save_registers
  save_registers_0
  save_registers_1
.endm

.macro restore_registers_0
  lw $3,  REG_R0($16)
  lw $7,  REG_R1($16)
  lw $8,  REG_R2($16)
  lw $9,  REG_R3($16)
  lw $10, REG_R4($16)
  lw $11, REG_R5($16)
  lw $12, REG_R6($16)
  lw $13, REG_R7($16)
  lw $14, REG_R8($16)
  lw $15, REG_R9($16)
  lw $24, REG_R11($16)
  lw $25, REG_R12($16)

  lw $18, REG_R10($16)
  lw $28, REG_R13($16)
.endm

.macro restore_registers_1
  lw $30, REG_R14($16)
.endm

.macro restore_registers
  restore_registers_0
  restore_registers_1
.endm


# Process a hardware event. Since an interrupt might be
# raised we have to check if the PC has changed.

# $4: next address
# $16: register base
# $17: cycle counter

.balign 64

mips_update_gba:
  lw $2, REG_CPSR($16)            # load CPSR
  sw $4, REG_PC($16)              # current PC = $4
  sw $0, CHANGED_PC_STATUS($16)

  addiu $sp, $sp, -4              # make room on the stack
  sw $ra, ($sp)                   # save return address

  save_registers                  # save registers
  collapse_flags_body             # insert flag into $2

  jal update_gba                  # process the next event
  sw $2, REG_CPSR($16)            # store CPSR (delay)

  lw $1, CHANGED_PC_STATUS($16)

  lw $ra, ($sp)                   # restore return address
  addiu $sp, $sp, 4               # fix stack

  bne $1, $0, lookup_pc_changed
  addu $17, $2, $0                # $17 = new cycle count (delay slot)

  restore_registers_0             # restore registers 1/2
  jr $ra                          # if not, go back to caller
  restore_registers_1             # restore registers 2/2 (delay)


branch_arm_update_gba:
  lw $2, REG_CPSR($16)            # load CPSR
  sw $4, REG_PC($16)              # next PC = $4

  save_registers                  # save registers
  collapse_flags_body             # insert flag into $2

  jal update_gba                  # process the next event
  sw $2, REG_CPSR($16)            # store CPSR (delay)

  lw $1, CHANGED_PC_STATUS($16)

  bne $1, $0, lookup_pc_changed
  addu $17, $2, $0                # $17 = new cycle count (delay slot)

  jal block_lookup_address_arm    # $2 = MIPS address to jump to
  lw $4, REG_PC($16)              # restore branch address (delay)

  restore_registers_0             # restore registers 1/2
  jr $2                           # jump to it
  restore_registers_1             # restore registers 2/2 (delay)

branch_thumb_update_gba:
  lw $2, REG_CPSR($16)            # load CPSR
  sw $4, REG_PC($16)              # next PC = $4

  save_registers                  # save registers
  collapse_flags_body             # insert flag into $2

  jal update_gba                  # process the next event
  sw $2, REG_CPSR($16)            # store CPSR (delay)

  lw $1, CHANGED_PC_STATUS($16)

  bne $1, $0, lookup_pc_changed
  addu $17, $2, $0                # $17 = new cycle count (delay slot)

  jal block_lookup_address_thumb  # $2 = MIPS address to jump to
  lw $4, REG_PC($16)              # restore branch address (delay)

  restore_registers_0             # restore registers 1/2
  jr $2                           # jump to it
  restore_registers_1             # restore registers 2/2 (delay)

branch_dual_update_gba:
  lw $2, REG_CPSR($16)            # load CPSR
  sw $4, REG_PC($16)              # next PC = $4

  save_registers                  # save registers

  andi $1, $4, 0x01
  ins $2, $1, 5, 1                # state bit = address & 0x01
  collapse_flags_body             # insert flag into $2

  jal update_gba                  # process the next event
  sw $2, REG_CPSR($16)            # store CPSR (delay)

  lw $1, CHANGED_PC_STATUS($16)

  bne $1, $0, lookup_pc_changed
  addu $17, $2, $0                # $17 = new cycle count (delay slot)

  jal block_lookup_address_dual   # $2 = MIPS address to jump to
  lw $4, REG_PC($16)              # restore branch address (delay)

  restore_registers_0             # restore registers 1/2
  jr $2                           # jump to it
  restore_registers_1             # restore registers 2/2 (delay)


# Perform an indirect branch.

# $4: GBA address to branch to

mips_indirect_branch_arm:
  blez $17, branch_arm_update_gba
  sw $0, CHANGED_PC_STATUS($16)

mips_indirect_branch_arm_no_update_gba:
  save_registers_0                # save registers 1/2
  jal block_lookup_address_arm    # $2 = MIPS address to jump to
  save_registers_1                # save registers 2/2 (delay)

  restore_registers_0             # restore registers 1/2
  jr $2                           # jump to it
  restore_registers_1             # restore registers 2/2 (delay)

mips_indirect_branch_thumb:
  blez $17, branch_thumb_update_gba
  sw $0, CHANGED_PC_STATUS($16)

mips_indirect_branch_thumb_no_update_gba:
  save_registers_0                # save registers 1/2
  jal block_lookup_address_thumb  # $2 = MIPS address to jump to
  save_registers_1                # save registers 2/2 (delay)

  restore_registers_0             # restore registers 1/2
  jr $2                           # jump to it
  restore_registers_1             # restore registers 2/2 (delay)

mips_indirect_branch_dual:
  blez $17, branch_dual_update_gba
  sw $0, CHANGED_PC_STATUS($16)

mips_indirect_branch_dual_no_update_gba:
  save_registers_0                # save registers 1/2
  jal block_lookup_address_dual   # $2 = MIPS address to jump to
  save_registers_1                # save registers 2/2 (delay)

  restore_registers_0             # restore registers 1/2
  jr $2                           # jump to it
  restore_registers_1             # restore registers 2/2 (delay)


# $4: address to write to
# $5: current PC

# Will patch the return address with a call to the correct handler as
# listed in the given table.

# Value will be set to force_open if it's open

.macro patch_handler ftable, force_open
  srl $1, $4, 24                  # $1 = address region
  sltiu $2, $1, 0x0F              # check if the value is open
  sll $1, $1, 2                   # make address word indexed

  beql $2, $0, 01f
  addiu $1, $0, (\force_open * 4) # (delay Likely)

01:
  lui $2, %hi(\ftable)
  addu $2, $2, $1
  lw $2, %lo(\ftable)($2)         # new function handler is in $2

  lui $1, %hi(3 << 26)            # $1 = 3 (JAL opcode)
  srl $2, $2, 2                   # remove lower two bits
  ins $1, $2, 0, 26               # insert offset into jal

  addiu $ra, $ra, -8              # rewind return address to function call
  sw $1, ($ra)                    # modify to call new handler

  cache 0x1a, ($ra)               # hit writeback dcache line
  sync
  cache 0x08, ($ra)               # hit invalidate icache line

  jr $ra                          # return
  nop                             # wary of putting cache here
.endm


# Like the above, but will use the table of the proper alignment,
# The tables should be ordered by alignment

.macro patch_handler_align ftable, alignment
  srl $1, $4, 24                  # $1 = address region
  sltiu $2, $1, 0x0F              # check if the value is open
  sll $1, $1, 2                   # make address word indexed

  beql $2, $0, 01f
  addiu $1, $0, 4                 # force address to 0x1 (open) delay slot, Likely

01:
  ins $1, $4, 6, \alignment       # place alignment bits into offset

  lui $2, %hi(\ftable)
  addu $2, $2, $1
  lw $2, %lo(\ftable)($2)         # new function handler is in $2

  lui $1, %hi(3 << 26)            # $1 = 3 (JAL opcode)
  srl $2, $2, 2                   # remove lower two bits
  ins $1, $2, 0, 26               # insert offset into jal

  addiu $ra, $ra, -8              # rewind return address to function call
  sw $1, ($ra)                    # modify to call new handler

  cache 0x1a, ($ra)               # hit writeback dcache line
  sync
  cache 0x08, ($ra)               # hit invalidate icache line

  jr $ra                          # return
  nop                             # wary of putting cache here
.endm


.macro region_check region, patch_handler
  srl $1, $4, 24                  # check upper 8bits of address
  xor $1, $1, \region             # see if it is the given region
  bne $1, $0, \patch_handler      # if not repatch/try again
.endm

.macro region_check_open patch_handler
  srl $1, $4, 24                  # check upper 8bits of address
  sltiu $2, $1, 0x0F              # true if it is a low address
  addiu $1, $1, -1                # non-zero if it is not a low open
  sltu $1, $0, $1                 # true if lower bits != 1
  and $1, $1, $2                  # true if low address and not open
  bne $1, $0, \patch_handler      # if above is true, patch
.endm


.macro region_check_align region, align_bits, alignment, patch_handler
  srl $1, $4, 24                  # check upper 8bits of address
  ins $1, $4, 8, \align_bits      # look at lower bits of address too
  # See if it is the given region and alignment
  xori $1, $1, (\region | (\alignment << 8))
  bne $1, $0, \patch_handler      # if not repatch/try again
.endm

.macro region_check_open_align align_bits, alignment, patch_handler
  srl $1, $4, 24                  # check upper 8bits of address
  sltiu $2, $1, 0x0F              # true if it is a low address
  addiu $1, $1, -1                # non-zero if it is not a low open
  sltu $1, $0, $1                 # true if $1 != 0
  and $1, $1, $2                  # true if low address and not open
  ext $2, $4, 0, \align_bits      # $2 = low bits of 4
  xori $2, $2, \alignment         # true if alignment doesn't match
  or $1, $1, $2                   # align failure will trigger too
  bne $1, $0, \patch_handler      # if above is true, patch
.endm


.macro ignore_region region, patch_handler
  region_check \region, \patch_handler
  nop
  jr $ra
  addiu $17, $17, -1              # waitstate cycles (delay)
.endm

.macro ignore_high patch_handler
  srl $1, $4, 24                  # check upper 8bits of address
  sltiu $1, $1, 0x0F              # see if it is not high
  bne $1, $0, \patch_handler      # if not repatch/try again
  nop
  jr $ra
  addiu $17, $17, -1              # waitstate cycles (delay)
.endm


.macro translate_region_core base, size
  lui $2, %hi(\base)              # generate upper address
  andi $4, $4, \size              # generate offset
  addu $2, $2, $4                 # add ptr upper and offset
.endm

.macro translate_region region, patch_handler, base, size
  region_check \region, \patch_handler
  translate_region_core \base, \size
.endm

# I refuse to have > 80 char lines, and GAS has a problem with the param
# list spilling over (grumble)

.macro translate_region_align region, a_b, alignment, p_h, base, size
  region_check_align \region, \a_b, \alignment, \p_h
  translate_region_core \base, \size
.endm


.macro translate_region_ewram_core mask
  lui  $1, 3                      # size of EWRAM = 0x40000; load 3 in 31..16 (delay)
  ori  $1, $1, \mask              # now append the rest of the mask, 0x3XXXX
  and  $4, $4, $1
  lui  $2, %hi(ewram)             # generate upper address
  addu $2, $2, $4
.endm

.macro translate_region_ewram patch_handler
  region_check 2, \patch_handler
  translate_region_ewram_core 0xFFFF
.endm

.macro translate_region_ewram_load_align align_bits, alignment, patch_handler
  region_check_align 2, \align_bits, \alignment, \patch_handler
  translate_region_ewram_core 0xFFFF
.endm

.macro translate_region_ewram_load_align16 align_bits, alignment, patch_handler
  region_check_align 2, \align_bits, \alignment, \patch_handler
  translate_region_ewram_core 0xFFFE
.endm

.macro translate_region_ewram_load_align32 align_bits, alignment, patch_handler
  region_check_align 2, \align_bits, \alignment, \patch_handler
  translate_region_ewram_core 0xFFFC
.endm

.macro translate_region_ewram_store_align16 patch_handler
  region_check 2, \patch_handler
  translate_region_ewram_core 0xFFFE
.endm

.macro translate_region_ewram_store_align32 patch_handler
  region_check 2, \patch_handler
  translate_region_ewram_core 0xFFFC
.endm


.macro translate_region_vram_core
  ext $4, $4, 0, 17               # generate 17bit offset
  bnel $2, $0, 01f                # if address >= 0x10000
  ins $4, $0, 15, 1               # mask out bit 15 of address (delay Likely)

01:
  lui $1, %hi(vram)               # start loading vram address
  addu $2, $1, $4                 # $2 = (hi)vram + address
.endm

.macro translate_region_vram patch_handler
  region_check 6, \patch_handler
  ext $2, $4, 16, 1               # $2 = bit 16 of address (delay)
  translate_region_vram_core
.endm

.macro translate_region_vram_load_align align_bits, alignment, patch_handler
  region_check_align 6, \align_bits, \alignment, \patch_handler
  ext $2, $4, 16, 1               # $2 = bit 16 of address (delay)
  translate_region_vram_core
.endm

.macro translate_region_vram_load_align16 align_bits, alignment, patch_handler
  region_check_align 6, \align_bits, \alignment, \patch_handler
  ext $2, $4, 16, 1               # $2 = bit 16 of address (delay)
  ins $4, $0, 0, 1                # mask out lower bit of address
  translate_region_vram_core
.endm

.macro translate_region_vram_load_align32 align_bits, alignment, patch_handler
  region_check_align 6, \align_bits, \alignment, \patch_handler
  ext $2, $4, 16, 1               # $2 = bit 16 of address (delay)
  ins $4, $0, 0, 2                # mask out lower two bits of address
  translate_region_vram_core
.endm

.macro translate_region_vram_store_align16 patch_handler
  region_check 6, \patch_handler
  ext $2, $4, 16, 1               # $2 = bit 16 of address (delay)
  ins $4, $0, 0, 1                # mask out lower bit of address
  translate_region_vram_core
.endm

.macro translate_region_vram_store_align32 patch_handler
  region_check 6, \patch_handler
  ext $2, $4, 16, 1               # $2 = bit 16 of address (delay)
  ins $4, $0, 0, 2                # mask out lower two bits of address
  translate_region_vram_core
.endm


.macro translate_region_gamepak_core mask
  srl $2, $4, 15                  # $2 = page number of address (delay)
  sll $2, $2, 2                   # adjust to word index
  addu $2, $2, $16                # $2 = memory_map_read[address >> 15]
  lw $2, -32768($2)

  bne $2, $0, 01f                 # if it's non-NULL continue
  andi $1, $4, \mask              # $1 = low 15bits of address (delay slot)

  ext $4, $4, 15, 10              # $4 = (address >> 15) & 0x3FF

  sw $1, REG_SAVE($16)            # save offset
  sw $ra, REG_SAVE2($16)          # save return address

  save_registers_0                # save registers 1/2
  jal load_gamepak_page           # get page in $2
  save_registers_1                # save registers 2/2 (delay)

  lw $1, REG_SAVE($16)            # restore offset
  lw $ra, REG_SAVE2($16)          # restore return address
  restore_registers               # restore registers

01:
  addu $2, $2, $1                 # add the memory map offset
.endm

.macro translate_region_gamepak region, patch_handler
  region_check \region, \patch_handler
  translate_region_gamepak_core 0x7FFF
.endm

.macro translate_region_gamepak_align region, a_b, alignment, patch_handler
  region_check_align \region, \a_b, \alignment, \patch_handler
  translate_region_gamepak_core 0x7FFF
.endm

.macro translate_region_gamepak_align16 region, a_b, alignment, patch_handler
  region_check_align \region, \a_b, \alignment, \patch_handler
  translate_region_gamepak_core 0x7FFE
.endm

.macro translate_region_gamepak_align32 region, a_b, alignment, patch_handler
  region_check_align \region, \a_b, \alignment, \patch_handler
  translate_region_gamepak_core 0x7FFC
.endm


.macro translate_region_gamepak_a region, patch_handler
  region_check \region, \patch_handler
  srl $2, $4, 15                  # $2 = page number of address (delay)
  sll $2, $2, 2                   # adjust to word index
  addu $2, $2, $16                # $2 = memory_map_read[address >> 15]
  lw $2, -32768($2)

  bne $2, $0, 01f                 # if it's non-NULL continue
  andi $1, $4, 0x7FFC             # $1 = low 15bits of address (delay slot)

  ext $4, $4, 15, 10              # $4 = (address >> 15) & 0x3FF

  sw $1, REG_SAVE($16)            # save offset
  sw $ra, REG_SAVE2($16)          # save return address
  sw $6, REG_SAVE3($16)           # save a2

  save_registers_0                # save registers 1/2
  jal load_gamepak_page           # get page in $2
  save_registers_1                # save registers 2/2 (delay)

  lw $1, REG_SAVE($16)            # restore offset
  lw $ra, REG_SAVE2($16)          # restore return address
  lw $6, REG_SAVE3($16)           # restore a2
  restore_registers               # restore registers

01:
  addu $2, $2, $1                 # add the memory map offset
.endm


.macro eeprom_load_core
  sw $ra, REG_SAVE($16)           # save the return address (delay)

  save_registers_0                # save registers 1/2
  jal read_eeprom                 # get eeprom value in $2
  save_registers_1                # save registers 2/2 (delay)

  addiu $17, $17, -9              # waitstate cycles

  lw $ra, REG_SAVE($16)           # restore return address
  restore_registers_0             # restore registers 1/2
  jr $ra                          # return
  restore_registers_1             # restore registers 2/2 (delay)
.endm

.macro eeprom_load_align align_bits, alignment, patch_handler
  region_check_align 0xD, \align_bits, \alignment, \patch_handler
  eeprom_load_core
.endm

.macro eeprom_load_align16 align_bits, alignment, patch_handler
  eeprom_load_align \align_bits, \alignment, \patch_handler
.endm


.macro backup_load_core
  save_registers_0                # save registers 1/2
  jal read_backup                 # get backup value in $2
  save_registers_1                # save registers 2/2 (delay)

  addiu $17, $17, -9              # waitstate cycles

  lw $ra, REG_SAVE($16)           # restore return address
  restore_registers               # restore registers
.endm

.macro backup_load_a patch_handler
  region_check 0xE, \patch_handler
  sw $ra, REG_SAVE($16)           # save return address (delay)
  sw $6, REG_SAVE2($16)           # save a2

  save_registers_0                # save registers 1/2
  jal read_backup                 # get backup value in $2
  save_registers_1                # save registers 2/2 (delay)

  ins $2, $2, 8, 8
  ins $2, $2, 16, 16              # result = result * 0x01010101

  addiu $17, $17, -9              # waitstate cycles

  lw $ra, REG_SAVE($16)           # restore return address
  lw $6, REG_SAVE2($16)           # restore a2 (delay)
  restore_registers_0             # restore registers 1/2
  jr $ra                          # return
  restore_registers_1             # restore registers 2/2
.endm


.macro backup_load patch_handler
  region_check 0xE, \patch_handler
  sw $ra, REG_SAVE($16)           # save the return address (delay)
  backup_load_core
  jr $ra                          # return
.endm

.macro backup_load_align align_bits, alignment, patch_handler
  region_check_align 0xE, \align_bits, \alignment, \patch_handler
  sw $ra, REG_SAVE($16)           # save the return address (delay)
  backup_load_core
  jr $ra                          # return
.endm

.macro backup_load_align16 align_bits, alignment, patch_handler
  region_check_align 0xE, \align_bits, \alignment, \patch_handler
  sw $ra, REG_SAVE($16)           # save the return address (delay)
  ins $4, $0, 0, 1                # mask out lower bit
  backup_load_core
  ins $2, $2, 8, 8                # result = result * 0x0101
  jr $ra                          # return
.endm

.macro backup_load_align32 align_bits, alignment, patch_handler
  region_check_align 0xE, \align_bits, \alignment, \patch_handler
  sw $ra, REG_SAVE($16)           # save the return address (delay)
  ins $4, $0, 0, 2                # mask out lower two bits
  backup_load_core
  ins $2, $2, 8, 8
  ins $2, $2, 16, 16              # result = result * 0x01010101
  jr $ra                          # return
.endm


.macro open_load8_core
  lw $2, CPU_DMA_HACK($16)
  lw $1, REG_CPSR($16)            # $1 = CPSR

  andi $4, $4, 0x03               # in ARM mode, isolate lower 2bits from address
  addiu $17, $17, -1              # waitstate cycles

  bnel $2, $0, 02f
  lbu $2, CPU_DMA_LAST($16)       # DMA Prefetch (delay Likely)

  andi $1, $1, 0x20               # test T bit
  bnel $1, $0, 01f                # branch if Thumb mode
  andi $4, $4, 0x01               # in Thumb mode, isolate one more bit (delay Likely)

01:
  addu $4, $5, $4                 # a0 = PC + low bits of address

  sw $ra, REG_SAVE($16)           # save the return address
  save_registers_0                # save registers 1/2
  jal read_open_memory8           # get instruction at PC
  save_registers_1                # save registers 2/2 (delay)

  lw $ra, REG_SAVE($16)           # restore return address
  restore_registers               # restore registers

02:
  jr $ra                          # return
.endm

.macro open_load8 patch_handler
  region_check_open \patch_handler
  open_load8_core
.endm


.macro open_load16_core
  lw $2, CPU_DMA_HACK($16)
  lw $1, REG_CPSR($16)            # $1 = CPSR

  andi $4, $4, 0x02               # in ARM mode, isolate bit 1 from address
  addiu $17, $17, -1              # waitstate cycles

  bnel $2, $0, 02f
  lhu $2, CPU_DMA_LAST($16)       # DMA Prefetch (delay Likely)

  andi $1, $1, 0x20               # test T bit
  bnel $1, $0, 01f                # branch if Thumb mode
  addu $4, $0, $0                 # in Thumb mode, zero out address bit (delay Likely)

01:
  addu $4, $5, $4                 # a0 = PC + low bits of address

  sw $ra, REG_SAVE($16)           # save the return address
  save_registers_0                # save registers 1/2
  jal read_open_memory16          # get instruction at PC
  save_registers_1                # save registers 2/2 (delay)

  lw $ra, REG_SAVE($16)           # restore return address
  restore_registers               # restore registers

02:
  jr $ra                          # return
.endm

.macro open_load16_align align_bits, alignment, patch_handler
  region_check_open_align \align_bits, \alignment, \patch_handler
  open_load16_core
.endm

.macro open_load16_align16 align_bits, alignment, patch_handler
  open_load16_align \align_bits, \alignment, \patch_handler
.endm


.macro open_load32_core
  lw $2, CPU_DMA_HACK($16)
  lw $1, REG_CPSR($16)            # $2 = CPSR

  addiu $17, $17, -1              # waitstate cycles

  bnel $2, $0, 03f
  lw $2, CPU_DMA_LAST($16)        # DMA Prefetch (delay Likely)

  save_registers                  # save registers

  andi $1, $1, 0x20               # test T bit
  beq $1, $0, 01f                 # branch if ARM mode
  sw $ra, REG_SAVE($16)           # save the return address (delay)

  jal read_open_memory16          # get instruction at PC
  addu $4, $5, $0                 # a0 = PC (delay)

  j 02f
  ins $2, $2, 16, 16              # result = (result << 16) | result (delay)

01:
  jal read_open_memory32          # get instruction at PC
  addu $4, $5, $0                 # a0 = PC (delay)

02:                               # join point
  lw $ra, REG_SAVE($16)           # restore return address
  restore_registers               # restore registers

03:
  jr $ra                          # return
.endm

.macro open_load32a_core
  lw $2, CPU_DMA_HACK($16)
  lw $1, REG_CPSR($16)            # $1 = CPSR
  sw $6, REG_SAVE2($16)           # save a2

  beq $2, $0, 01f
  addiu $17, $17, -1              # waitstate cycles (delay)

  jr $ra
  lw $2, CPU_DMA_LAST($16)        # DMA Prefetch (delay)

01:
  save_registers                  # save registers

  andi $1, $1, 0x20               # test T bit
  beq $1, $0, 02f                 # branch if ARM mode
  sw $ra, REG_SAVE($16)           # save the return address (delay)

  jal read_open_memory16          # get instruction at PC
  addu $4, $5, $0                 # a0 = PC (delay)

  j 03f
  ins $2, $2, 16, 16              # result = (result << 16) | result (delay)

02:
  jal read_open_memory32          # get instruction at PC
  addu $4, $5, $0                 # a0 = PC (delay)

03:
  lw $ra, REG_SAVE($16)           # restore return address
  restore_registers               # restore registers

  jr $ra                          # return
  lw $6, REG_SAVE2($16)           # restore a2 (delay)
.endm

.macro open_load32_a patch_handler
  region_check_open \patch_handler
  open_load32a_core
.endm

.macro open_load32_align align_bits, alignment, patch_handler
  region_check_open_align \align_bits, \alignment, \patch_handler
  open_load32_core
.endm

.macro open_load32_align32 align_bits, alignment, patch_handler
  open_load32_align \align_bits, \alignment, \patch_handler
.endm


.macro store_function function, region, patch_handler, mask
  region_check \region, \patch_handler
  sw $ra, REG_SAVE($16)           # save the return address (delay)
  save_registers                  # save registers

  jal \function                   # store value out
  andi $4, $4, \mask              # mask address (delay)

  addiu $17, $17, -9              # waitstate cycles

  lw $ra, REG_SAVE($16)           # restore return address
  restore_registers_0             # restore registers 1/2
  jr $ra                          # return
  restore_registers_1             # restore registers 2/2 (delay)
.endm


.macro backup_store region, patch_handler
  region_check \region, \patch_handler
  sw $ra, REG_SAVE($16)           # save the return address (delay)

  save_registers_0                # save registers 1/2
  jal write_backup                # store value out
  save_registers_1                # save registers 2/2 (delay)

  addiu $17, $17, -9              # waitstate cycles

  lw $ra, REG_SAVE($16)           # restore return address
  restore_registers_0             # restore registers 1/2
  jr $ra                          # return
  restore_registers_1             # restore registers 2/2 (delay)
.endm

.macro rtc_store region, patch_handler
  region_check \region, \patch_handler
  sw $ra, REG_SAVE($16)           # save the return address (delay)
  save_registers                  # save registers

  jal write_rtc                   # store value out
  andi $4, $4, 0xFE               # mask address (delay)

  addiu $17, $17, -4              # waitstate cycles

  lw $ra, REG_SAVE($16)           # restore return address
  restore_registers_0             # restore registers 1/2
  jr $ra                          # return
  restore_registers_1             # restore registers 2/2 (delay)
.endm


.macro load_u8 base, waitstate
  lbu $2, %lo(\base)($2)          # return base[offset]
  jr $ra                          # return
  addiu $17, $17, \waitstate      # waitstate cycles (delay)
.endm

.macro load_s8 base, waitstate
  lb $2, %lo(\base)($2)           # return base[offset]
  jr $ra                          # return
  addiu $17, $17, \waitstate      # waitstate cycles (delay)
.endm

.macro load_u16 base, waitstate
  lhu $2, %lo(\base)($2)          # return base[offset]
  jr $ra                          # return
  addiu $17, $17, \waitstate      # waitstate cycles (delay)
.endm

.macro load_s16 base, waitstate
  lh $2, %lo(\base)($2)           # return base[offset]
  jr $ra                          # return
  addiu $17, $17, \waitstate      # waitstate cycles (delay)
.endm

.macro load_u32 base, waitstate
  lw $2, %lo(\base)($2)           # return base[offset]
  jr $ra                          # return
  addiu $17, $17, \waitstate      # waitstate cycles (delay)
.endm


# 16bit unaligned load will always have a 1 in the LSB;
# should have already been taken care of in indexing.

.macro load_u16_unaligned base, waitstate
  lhu $2, %lo(\base)($2)          # load base[offset]
  addiu $17, $17, \waitstate      # waitstate cycles
  jr $ra                          # return
  ror $2, $2, 8                   # rotate value by 8bits
.endm

# This is technically the same as load_s8, but kept to
# avoid confusion.

.macro load_s16_unaligned base, waitstate
  lb $2, %lo(\base)($2)           # return base[offset]
  jr $ra                          # return
  addiu $17, $17, \waitstate      # waitstate cycles (delay)
.endm

# Unalignment must be known statically (use the tables to
# patch correctly)

.macro load_u32_unaligned base, alignment, waitstate
  lw $2, %lo(\base)($2)           # load base[offset]
  addiu $17, $17, \waitstate      # waitstate cycles
  jr $ra                          # return
  ror $2, $2, (\alignment * 8)    # rotate value by 8bits
.endm


.macro store_u8 base, waitstate
  sb $5, %lo(\base)($2)           # store value at base[offset]
  addiu $17, $17, \waitstate      # waitstate cycles

  blez $17, store_update_gba
  sw $0, CHANGED_PC_STATUS($16)

  jr $ra                          # return
  nop
.endm

.macro store_u16 base, waitstate
  sh $5, %lo(\base)($2)           # store value at base[offset]
  addiu $17, $17, \waitstate      # waitstate cycles

  blez $17, store_update_gba
  sw $0, CHANGED_PC_STATUS($16)

  jr $ra                          # return
  nop
.endm

.macro store_u32 base, waitstate
  sw $5, %lo(\base)($2)           # store value at base[offset]
  addiu $17, $17, \waitstate      # waitstate cycles

  blez $17, store_update_gba
  sw $0, CHANGED_PC_STATUS($16)

  jr $ra                          # return
  nop
.endm

.macro store_u32a base, waitstate
  sw $5, %lo(\base)($2)           # store value at base[offset]
  jr $ra                          # return
  addiu $17, $17, \waitstate      # waitstate cycles (delay)
.endm


# Store the value double mirrored (u16)

.macro store_u8_double base, waitstate
  ins $5, $5, 8, 8                # value = (value << 8) | value
  sh $5, %lo(\base)($2)           # store value at base[offset]
  addiu $17, $17, \waitstate      # waitstate cycles

  blez $17, store_update_gba
  sw $0, CHANGED_PC_STATUS($16)

  jr $ra                          # return
  nop
.endm


store_update_gba:
  lw $2, REG_CPSR($16)            # load CPSR
  sw $6, REG_PC($16)              # next PC = $6

  addiu $sp, $sp, -4              # make room on the stack
  sw $ra, ($sp)                   # save return address

  save_registers                  # save registers
  collapse_flags_body             # insert flag into $2

  jal update_gba                  # process the next event
  sw $2, REG_CPSR($16)            # store CPSR (delay)

  lw $1, CHANGED_PC_STATUS($16)

  lw $ra, ($sp)                   # restore return address
  addiu $sp, $sp, 4               # fix stack

  bne $1, $0, lookup_pc_changed
  addu $17, $2, $0                # $17 = new cycle count (delay slot)

  restore_registers_0             # restore registers 1/2
  jr $ra
  restore_registers_1             # restore registers 2/2 (delay)


# Store the values and check if it overwrote code there, by way of a Metadata
# Area. A Metadata Area has 4 half-words for each word of the Data Area it
# describes, so data[0] is at metadata[0] and data[4] is at metadata[8] and so
# on, when the offsets are taken as bytes. The entries all have [3]'s bit 0
# stating whether the Data Word is a code word: 1 if it is, 0 if it isn't.
# [3] is at byte offset 6 because this is an assembly file.

# Register assignment:
#   $1 available
#   $2 available
#   $4 (invariant) = Data Area offset
#   $5 (outgoing)  = memory region
#   $6 (invariant) = PC
# See smc_write for its register assignment.
.macro post_write_metadata_core base, metabase, memory_region
  srl $2, $4, 2                   # $2 = (address & ~3) [>> 2]
  sll $2, $2, 3                   # byte offset into a 16-bit array: * 2
  la $1, \metabase                # load the Metadata Area's address
  addu $1, $1, $2                 # $1 = &metabase[offset] (u16)

  lhu $2, 6($1)                   # load the code modification status
  andi $2, $2, 0x3                # and extract the Currently Code bits

  bne $2, $0, smc_write           # if there has been code there, go flush RAM
  addiu $5, $0, \memory_region    # $5 = memory region (delay)

  blez $17, store_update_gba
  sw $0, CHANGED_PC_STATUS($16)

  jr $ra                          # if there has not been code there, return
  nop                             # cannot usefully delay here
.endm

post_write_metadata_ewram:
  post_write_metadata_core ewram, ewram_metadata, 0x02

post_write_metadata_iwram:
  post_write_metadata_core iwram, iwram_metadata, 0x03

post_write_metadata_vram:
  post_write_metadata_core vram, vram_metadata, 0x06

.macro store_u8_metadata base, waitstate, post_function
  sb $5, %lo(\base)($2)           # store value at base[offset]
  j \post_function
  addiu $17, $17, \waitstate      # waitstate cycles (delay)
.endm

.macro store_u16_metadata base, waitstate, post_function
  sh $5, %lo(\base)($2)           # store value at base[offset]
  j \post_function
  addiu $17, $17, \waitstate      # waitstate cycles (delay)
.endm

.macro store_u32_metadata base, waitstate, post_function
  sw $5, %lo(\base)($2)           # store value at base[offset] (delay)
  j \post_function
  addiu $17, $17, \waitstate      # waitstate cycles (delay)
.endm


# Unsigned 8bit load handlers

execute_load_bios_u8:
  region_check 0, patch_load_u8
  srl $2, $4, 14                  # check if address is in BIOS region (delay)
  bne $2, $0, 2f                  # if not, perform open read
  srl $1, $5, 14                  # check if PC is in BIOS region (delay)
  bne $1, $0, 1f                  # if not, perform BIOS protected read

  lui $2, %hi(bios)               # generate upper address (delay)
  ins $2, $4, 0, 14               # generate offset
  load_u8 bios, -1

1:
  lui $2, %hi(bios_read_protect)  # generate upper address
  ins $2, $4, 0, 2                # lower 2 bits address contributes
  load_u8 bios_read_protect, -1

2:
  open_load8_core
  nop

execute_load_ewram_u8:
  translate_region_ewram patch_load_u8
  load_u8 ewram, -3

# Put the generic address over the handler you want to be default
# IWRAM is typically the most frequently read and written to.

execute_load_u8:
execute_load_iwram_u8:
  translate_region 3, patch_load_u8, iwram, 0x7FFF
  load_u8 iwram, -1

execute_load_io_u8:
  region_check 4, patch_load_u8

  addiu $2, $0, (0x0800 >> 2)     # $2 = 0x0800 >> 2 (delay)
  ext $1, $4, 2, 14               # $1 = (address & 0xFFFC) >> 2
  beq $1, $2, 1f

  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)
  bne $2, $0, 2f

  lui $2, %hi(io_registers)       # generate upper address (delay)
  ins $2, $4, 0, 10               # generate offset

  lbu $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 2f
  nop
  load_u8 io_registers, -1

1:
  lui $2, %hi(iwram_control)      # generate upper address
  ins $2, $4, 0, 2                # lower 2 bits address contributes
  load_u8 iwram_control, -1

2:
  open_load8_core
  nop

execute_load_palette_u8:
  translate_region 5, patch_load_u8, palette_ram, 0x3FF
  load_u8 palette_ram, -1

execute_load_vram_u8:
  translate_region_vram patch_load_u8
  load_u8 vram, -1

execute_load_oam_u8:
  translate_region 7, patch_load_u8, oam_ram, 0x3FF
  load_u8 oam_ram, -1

execute_load_gamepak8_u8:
  translate_region_gamepak 8, patch_load_u8
  load_u8 0, -4

execute_load_gamepak9_u8:
  translate_region_gamepak 9, patch_load_u8
  load_u8 0, -4

execute_load_gamepakA_u8:
  translate_region_gamepak 10, patch_load_u8
  load_u8 0, -4

execute_load_gamepakB_u8:
  translate_region_gamepak 11, patch_load_u8
  load_u8 0, -4

execute_load_gamepakC_u8:
  translate_region_gamepak 12, patch_load_u8
  load_u8 0, -4

execute_load_gamepakD_u8:
  translate_region_gamepak 13, patch_load_u8
  load_u8 0, -4

execute_load_backup_u8:
  backup_load patch_load_u8
  nop

execute_load_open_u8:
  open_load8 patch_load_u8
  nop

load_u8_ftable:
  .long execute_load_bios_u8      # 0x00 BIOS
  .long execute_load_open_u8      # 0x01 open address
  .long execute_load_ewram_u8     # 0x02 EWRAM
  .long execute_load_iwram_u8     # 0x03 IWRAM
  .long execute_load_io_u8        # 0x04 I/O registers
  .long execute_load_palette_u8   # 0x05 Palette RAM
  .long execute_load_vram_u8      # 0x06 VRAM
  .long execute_load_oam_u8       # 0x07 OAM RAM
  .long execute_load_gamepak8_u8  # 0x08 gamepak
  .long execute_load_gamepak9_u8  # 0x09 gamepak
  .long execute_load_gamepakA_u8  # 0x0A gamepak
  .long execute_load_gamepakB_u8  # 0x0B gamepak
  .long execute_load_gamepakC_u8  # 0x0C gamepak
  .long execute_load_gamepakD_u8  # 0x0D gamepak/eeprom
  .long execute_load_backup_u8    # 0x0E Flash ROM/SRAM
  .long execute_load_open_u8      # 0x0F open address

patch_load_u8:
  patch_handler load_u8_ftable, 0x01



# Signed 8bit load handlers

execute_load_bios_s8:
  region_check 0, patch_load_s8
  srl $2, $4, 14                  # check if address is in BIOS region (delay)
  bne $2, $0, 2f
  srl $1, $5, 14                  # check if PC is in BIOS region (delay)
  bne $1, $0, 1f                  # if not, perform BIOS protected read

  lui $2, %hi(bios)               # generate upper address (delay)
  ins $2, $4, 0, 14
  load_s8 bios, -1

1:
  lui $2, %hi(bios_read_protect)  # generate upper address
  ins $2, $4, 0, 2                # lower 2 bits contribute
  load_s8 bios_read_protect, -1

2:
  open_load8_core
  seb $2, $2

execute_load_ewram_s8:
  translate_region_ewram patch_load_s8
  load_s8 ewram, -3

execute_load_s8:
execute_load_iwram_s8:
  translate_region 3, patch_load_s8, iwram, 0x7FFF
  load_s8 iwram, -1

execute_load_io_s8:
  region_check 4, patch_load_s8

  addiu $2, $0, (0x0800 >> 2)     # $2 = 0x0800 >> 2 (delay)
  ext $1, $4, 2, 14               # $1 = (address & 0xFFFC) >> 2
  beq $1, $2, 1f

  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)
  bne $2, $0, 2f

  lui $2, %hi(io_registers)       # generate upper address (delay)
  ins $2, $4, 0, 10               # generate offset

  lbu $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 2f
  nop
  load_s8 io_registers, -1

1:
  lui $2, %hi(iwram_control)      # generate upper address
  ins $2, $4, 0, 2                # lower 2 bits contribute
  load_s8 iwram_control, -1

2:
  open_load8_core
  seb $2, $2

execute_load_palette_s8:
  translate_region 5, patch_load_s8, palette_ram, 0x3FF
  load_s8 palette_ram, -1

execute_load_vram_s8:
  translate_region_vram patch_load_s8
  load_s8 vram, -1

execute_load_oam_s8:
  translate_region 7, patch_load_s8, oam_ram, 0x3FF
  load_s8 oam_ram, -1

execute_load_gamepak8_s8:
  translate_region_gamepak 8, patch_load_s8
  load_s8 0, -4

execute_load_gamepak9_s8:
  translate_region_gamepak 9, patch_load_s8
  load_s8 0, -4

execute_load_gamepakA_s8:
  translate_region_gamepak 10, patch_load_s8
  load_s8 0, -4

execute_load_gamepakB_s8:
  translate_region_gamepak 11, patch_load_s8
  load_s8 0, -4

execute_load_gamepakC_s8:
  translate_region_gamepak 12, patch_load_s8
  load_s8 0, -4

execute_load_gamepakD_s8:
  translate_region_gamepak 13, patch_load_s8
  load_s8 0, -4

execute_load_backup_s8:
  backup_load patch_load_s8
  seb $2, $2                      # sign extend result (delay)

execute_load_open_s8:
  open_load8 patch_load_s8
  seb $2, $2                      # sign extend result (delay)

load_s8_ftable:
  .long execute_load_bios_s8      # 0x00 BIOS
  .long execute_load_open_s8      # 0x01 open address
  .long execute_load_ewram_s8     # 0x02 EWRAM
  .long execute_load_iwram_s8     # 0x03 IWRAM
  .long execute_load_io_s8        # 0x04 I/O registers
  .long execute_load_palette_s8   # 0x05 Palette RAM
  .long execute_load_vram_s8      # 0x06 VRAM
  .long execute_load_oam_s8       # 0x07 OAM RAM
  .long execute_load_gamepak8_s8  # 0x08 gamepak
  .long execute_load_gamepak9_s8  # 0x09 gamepak
  .long execute_load_gamepakA_s8  # 0x0A gamepak
  .long execute_load_gamepakB_s8  # 0x0B gamepak
  .long execute_load_gamepakC_s8  # 0x0C gamepak
  .long execute_load_gamepakD_s8  # 0x0D gamepak/eeprom
  .long execute_load_backup_s8    # 0x0E Flash ROM/SRAM
  .long execute_load_open_s8      # 0x0F open address

patch_load_s8:
  patch_handler load_s8_ftable, 1



# Unsigned aligned 16bit load handlers

execute_load_bios_u16:
  region_check_align 0, 1, 0, patch_load_u16
  srl $2, $4, 14                  # check if address is in BIOS region (delay)
  bne $2, $0, 2f                  # if not, perform open read
  srl $1, $5, 14                  # check if PC is in BIOS region (delay)
  bne $1, $0, 1f                  # if not, perform BIOS protected read

  lui $2, %hi(bios)               # generate upper address (delay)
  ins $2, $4, 0, 14
  load_u16 bios, -1

1:
  lui $2, %hi(bios_read_protect)  # generate upper address
  ins $2, $4, 0, 2                # bit 1 contributes
  load_u16 bios_read_protect, -1

2:
  open_load16_core
  nop

execute_load_ewram_u16:
  translate_region_ewram_load_align16 1, 0, patch_load_u16
  load_u16 ewram, -3

execute_load_u16:
execute_load_iwram_u16:
  translate_region_align 3, 1, 0, patch_load_u16, iwram, 0x7FFe
  load_u16 iwram, -1

execute_load_io_u16:
  region_check_align 4, 1, 0, patch_load_u16

  addiu $2, $0, (0x0800 >> 2)     # $2 = 0x0800 >> 2 (delay)
  ext $1, $4, 2, 14               # $1 = (address & 0xFFFC) >> 2
  beq $1, $2, 2f

  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)
  bne $2, $0, 3f

  lui $2, %hi(io_registers)       # generate upper address (delay)
  ins $2, $4, 0, 10               # generate offset

  lbu $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 1f
  nop
  load_u16 io_registers, -1

1:
  ins $2, $0, 0, 2
  lbu $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 3f

  addu $2, $0, $0                 # value = 0 (delay)
  jr $ra
  addiu $17, $17, -1              # waitstate cycles (delay)

2:
  lui $2, %hi(iwram_control)      # generate upper address
  ins $2, $4, 0, 2                # bit 1 contributes
  load_u16 iwram_control, -1

3:
  open_load16_core
  nop

execute_load_palette_u16:
  translate_region_align 5, 1, 0, patch_load_u16, palette_ram, 0x3Fe
  load_u16 palette_ram, -1

execute_load_vram_u16:
  translate_region_vram_load_align16 1, 0, patch_load_u16
  load_u16 vram, -1

execute_load_oam_u16:
  translate_region_align 7, 1, 0, patch_load_u16, oam_ram, 0x3Fe
  load_u16 oam_ram, -1

execute_load_gamepak8_u16:
  translate_region_gamepak_align16 8, 1, 0, patch_load_u16
  load_u16 0, -4

execute_load_gamepak9_u16:
  translate_region_gamepak_align16 9, 1, 0, patch_load_u16
  load_u16 0, -4

execute_load_gamepakA_u16:
  translate_region_gamepak_align16 10, 1, 0, patch_load_u16
  load_u16 0, -4

execute_load_gamepakB_u16:
  translate_region_gamepak_align16 11, 1, 0, patch_load_u16
  load_u16 0, -4

execute_load_gamepakC_u16:
  translate_region_gamepak_align16 12, 1, 0, patch_load_u16
  load_u16 0, -4

execute_load_eeprom_u16:
  eeprom_load_align 1, 0, patch_load_u16

execute_load_backup_u16:
  backup_load_align16 1, 0, patch_load_u16
  nop

execute_load_open_u16:
  open_load16_align 1, 0, patch_load_u16
  nop



# Unsigned unaligned 16bit load handlers

execute_load_bios_u16u:
  region_check_align 0, 1, 1, patch_load_u16
  srl $2, $4, 14                  # check if address is in BIOS region (delay)
  bne $2, $0, 2f                  # if not, perform open read
  srl $1, $5, 14                  # check if PC is in BIOS region (delay)
  bne $1, $0, 1f                  # if not, perform BIOS protected read

  lui $2, %hi(bios)               # generate upper address (delay)
  andi $4, $4, 0x3FFE             # generate offset
  addu $2, $2, $4
  load_u16_unaligned bios, -1

1:
  lui $2, %hi(bios_read_protect)  # generate upper address
  andi $1, $4, 0x02
  addu $2, $2, $1                 # bit 1 contributes
  load_u16_unaligned bios_read_protect, -1

2:
  open_load16_core
  ror $2, $2, 8

execute_load_ewram_u16u:
  translate_region_ewram_load_align16 1, 1, patch_load_u16
  load_u16_unaligned ewram, -3

execute_load_iwram_u16u:
  translate_region_align 3, 1, 1, patch_load_u16, iwram, 0x7FFE
  load_u16_unaligned iwram, -1

execute_load_io_u16u:
  region_check_align 4, 1, 1, patch_load_u16

  addiu $2, $0, (0x0800 >> 2)     # $2 = 0x0800 >> 2 (delay)
  ext $1, $4, 2, 14               # $1 = (address & 0xFFFC) >> 2
  beq $1, $2, 2f

  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)
  bne $2, $0, 3f

  lui $2, %hi(io_registers)       # generate upper address (delay)
  andi $4, $4, 0x3FFE             # generate offset
  addu $2, $2, $4

  lbu $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 1f
  nop
  load_u16_unaligned io_registers, -1

1:
  ins $2, $0, 0, 2
  lbu $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 3f

  addu $2, $0, $0                 # value = 0 (delay)
  jr $ra
  addiu $17, $17, -1              # waitstate cycles (delay)

2:
  lui $2, %hi(iwram_control)      # generate upper address
  andi $1, $4, 0x02
  addu $2, $2, $1                 # bit 1 contributes
  load_u16_unaligned iwram_control, -1

3:
  open_load16_core
  ror $2, $2, 8

execute_load_palette_u16u:
  translate_region_align 5, 1, 1, patch_load_u16, palette_ram, 0x3FE
  load_u16_unaligned palette_ram, -1

execute_load_vram_u16u:
  translate_region_vram_load_align16 1, 1, patch_load_u16
  load_u16_unaligned vram, -1

execute_load_oam_u16u:
  translate_region_align 7, 1, 1, patch_load_u16, oam_ram, 0x3FE
  load_u16_unaligned oam_ram, -1

execute_load_gamepak8_u16u:
  translate_region_gamepak_align16 8, 1, 1, patch_load_u16
  load_u16_unaligned 0, -4

execute_load_gamepak9_u16u:
  translate_region_gamepak_align16 9, 1, 1, patch_load_u16
  load_u16_unaligned 0, -4

execute_load_gamepakA_u16u:
  translate_region_gamepak_align16 10, 1, 1, patch_load_u16
  load_u16_unaligned 0, -4

execute_load_gamepakB_u16u:
  translate_region_gamepak_align16 11, 1, 1, patch_load_u16
  load_u16_unaligned 0, -4

execute_load_gamepakC_u16u:
  translate_region_gamepak_align16 12, 1, 1, patch_load_u16
  load_u16_unaligned 0, -4

execute_load_eeprom_u16u:
  eeprom_load_align16 1, 1, patch_load_u16

execute_load_backup_u16u:
  backup_load_align16 1, 1, patch_load_u16
  nop

execute_load_open_u16u:
  open_load16_align16 1, 1, patch_load_u16
  ror $2, $2, 8                   # rotate value by 8bits

load_u16_ftable:
  .long execute_load_bios_u16     # 0x00 BIOS
  .long execute_load_open_u16     # 0x01 open address
  .long execute_load_ewram_u16    # 0x02 EWRAM
  .long execute_load_iwram_u16    # 0x03 IWRAM
  .long execute_load_io_u16       # 0x04 I/O registers
  .long execute_load_palette_u16  # 0x05 Palette RAM
  .long execute_load_vram_u16     # 0x06 VRAM
  .long execute_load_oam_u16      # 0x07 OAM RAM
  .long execute_load_gamepak8_u16 # 0x08 gamepak
  .long execute_load_gamepak9_u16 # 0x09 gamepak
  .long execute_load_gamepakA_u16 # 0x0A gamepak
  .long execute_load_gamepakB_u16 # 0x0B gamepak
  .long execute_load_gamepakC_u16 # 0x0C gamepak
  .long execute_load_eeprom_u16   # 0x0D gamepak/eeprom
  .long execute_load_backup_u16   # 0x0E Flash ROM/SRAM
  .long execute_load_open_u16     # 0x0F open

  .long execute_load_bios_u16u    # 0x00 BIOS unaligned
  .long execute_load_open_u16u    # 0x01 open address unaligned
  .long execute_load_ewram_u16u   # 0x02 EWRAM unaligned
  .long execute_load_iwram_u16u   # 0x03 IWRAM unaligned
  .long execute_load_io_u16u      # 0x04 I/O registers unaligned
  .long execute_load_palette_u16u # 0x05 Palette RAM unaligned
  .long execute_load_vram_u16u    # 0x06 VRAM unaligned
  .long execute_load_oam_u16u     # 0x07 OAM RAM unaligned
  .long execute_load_gamepak8_u16u# 0x08 gamepak unaligned
  .long execute_load_gamepak9_u16u# 0x09 gamepak unaligned
  .long execute_load_gamepakA_u16u# 0x0A gamepak unaligned
  .long execute_load_gamepakB_u16u# 0x0B gamepak unaligned
  .long execute_load_gamepakC_u16u# 0x0C gamepak unaligned
  .long execute_load_eeprom_u16u  # 0x0D gamepak/eeprom unaligned
  .long execute_load_backup_u16u  # 0x0E Flash ROM/SRAM unaligned
  .long execute_load_open_u16u    # 0x0F open unaligned


patch_load_u16:
  patch_handler_align load_u16_ftable, 1

# Signed aligned 16bit load handlers

execute_load_bios_s16:
  region_check_align 0, 1, 0, patch_load_s16
  srl $2, $4, 14                  # check if address is in BIOS region (delay)
  bne $2, $0, 2f                  # if not, perform open read
  srl $1, $5, 14                  # check if PC is in BIOS region (delay)
  bne $1, $0, 1f                  # if not, perform BIOS protected read

  lui $2, %hi(bios)               # generate upper address (delay)
  ins $2, $4, 0, 14               # generate offset
  load_s16 bios, -1

1:
  lui $2, %hi(bios_read_protect)  # generate upper address
  ins $2, $4, 0, 2                # bit 2 contributes
  load_s16 bios_read_protect, -1

2:
  open_load16_core
  seh $2, $2

execute_load_ewram_s16:
  translate_region_ewram_load_align16 1, 0, patch_load_s16
  load_s16 ewram, -3

execute_load_s16:
execute_load_iwram_s16:
  translate_region_align 3, 1, 0, patch_load_s16, iwram, 0x7FFe
  load_s16 iwram, -1

execute_load_io_s16:
  region_check_align 4, 1, 0, patch_load_s16

  addiu $2, $0, (0x0800 >> 2)     # $2 = 0x0800 >> 2 (delay)
  ext $1, $4, 2, 14               # $1 = (address & 0xFFFC) >> 2
  beq $1, $2, 2f

  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)
  bne $2, $0, 3f

  lui $2, %hi(io_registers)       # generate upper address (delay)
  ins $2, $4, 0, 10               # generate offset

  lbu $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 1f
  nop
  load_s16 io_registers, -1

1:
  ins $2, $0, 0, 2
  lbu $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 3f

  addu $2, $0, $0                 # value = 0 (delay)
  jr $ra
  addiu $17, $17, -1              # waitstate cycles (delay)

2:
  lui $2, %hi(iwram_control)      # generate upper address
  ins $2, $4, 0, 2                # bit 2 contributes
  load_s16 iwram_control, -1

3:
  open_load16_core
  seh $2, $2

execute_load_palette_s16:
  translate_region_align 5, 1, 0, patch_load_s16, palette_ram, 0x3Fe
  load_s16 palette_ram, -1

execute_load_vram_s16:
  translate_region_vram_load_align16 1, 0, patch_load_s16
  load_s16 vram, -1

execute_load_oam_s16:
  translate_region_align 7, 1, 0, patch_load_s16, oam_ram, 0x3Fe
  load_s16 oam_ram, -1

execute_load_gamepak8_s16:
  translate_region_gamepak_align16 8, 1, 0, patch_load_s16
  load_s16 0, -4

execute_load_gamepak9_s16:
  translate_region_gamepak_align16 9, 1, 0, patch_load_s16
  load_s16 0, -4

execute_load_gamepakA_s16:
  translate_region_gamepak_align16 10, 1, 0, patch_load_s16
  load_s16 0, -4

execute_load_gamepakB_s16:
  translate_region_gamepak_align16 11, 1, 0, patch_load_s16
  load_s16 0, -4

execute_load_gamepakC_s16:
  translate_region_gamepak_align16 12, 1, 0, patch_load_s16
  load_s16 0, -4

execute_load_eeprom_s16:
  eeprom_load_align 1, 0, patch_load_s16

execute_load_backup_s16:
  backup_load_align16 1, 0, patch_load_s16
  seh $2, $2

execute_load_open_s16:
  open_load16_align 1, 0, patch_load_s16
  seh $2, $2


# Signed unaligned 16bit load handlers

execute_load_bios_s16u:
  region_check_align 0, 1, 1, patch_load_s16
  srl $2, $4, 14                  # check if address is in BIOS region (delay)
  bne $2, $0, 2f                  # if not, perform open read
  srl $1, $5, 14                  # check if PC is in BIOS region (delay)
  bne $1, $0, 1f                  # if not, perform BIOS protected read

  lui $2, %hi(bios)               # generate upper address (delay)
  ins $2, $4, 0, 14               # generate offset
  load_s16_unaligned bios, -1

1:
  lui $2, %hi(bios_read_protect)  # generate upper address
  ins $2, $4, 0, 2                # bit 2 contributes
  load_s16_unaligned bios_read_protect, -1

2:
  open_load8_core
  seb $2, $2

execute_load_ewram_s16u:
  translate_region_ewram_load_align 1, 1, patch_load_s16
  load_s16_unaligned ewram, -3

execute_load_iwram_s16u:
  translate_region_align 3, 1, 1, patch_load_s16, iwram, 0x7FFF
  load_s16_unaligned iwram, -1

execute_load_io_s16u:
  region_check_align 4, 1, 1, patch_load_s16

  addiu $2, $0, (0x0800 >> 2)     # $2 = 0x0800 >> 2 (delay)
  ext $1, $4, 2, 14               # $1 = (address & 0xFFFC) >> 2
  beq $1, $2, 1f

  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)
  bne $2, $0, 2f

  lui $2, %hi(io_registers)       # generate upper address (delay)
  ins $2, $4, 0, 10               # generate offset

  lbu $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 2f
  nop
  load_s16_unaligned io_registers, -1

1:
  lui $2, %hi(iwram_control)      # generate upper address
  ins $2, $4, 0, 2                # bit 2 contributes
  load_s16_unaligned iwram_control, -1

2:
  open_load8_core
  seb $2, $2

execute_load_palette_s16u:
  translate_region_align 5, 1, 1, patch_load_s16, palette_ram, 0x3FF
  load_s16_unaligned palette_ram, -1

execute_load_vram_s16u:
  translate_region_vram_load_align 1, 1, patch_load_s16
  load_s16_unaligned vram, -1

execute_load_oam_s16u:
  translate_region_align 7, 1, 1, patch_load_s16, oam_ram, 0x3FF
  load_s16_unaligned oam_ram, -1

execute_load_gamepak8_s16u:
  translate_region_gamepak_align 8, 1, 1, patch_load_s16
  load_s16_unaligned 0, -4

execute_load_gamepak9_s16u:
  translate_region_gamepak_align 9, 1, 1, patch_load_s16
  load_s16_unaligned 0, -4

execute_load_gamepakA_s16u:
  translate_region_gamepak_align 10, 1, 1, patch_load_s16
  load_s16_unaligned 0, -4

execute_load_gamepakB_s16u:
  translate_region_gamepak_align 11, 1, 1, patch_load_s16
  load_s16_unaligned 0, -4

execute_load_gamepakC_s16u:
  translate_region_gamepak_align 12, 1, 1, patch_load_s16
  load_s16_unaligned 0, -4

execute_load_eeprom_s16u:
  eeprom_load_align 1, 1, patch_load_s16

execute_load_backup_s16u:
  backup_load_align 1, 1, patch_load_s16
  seb $2, $2                      # sign extend result from 8bits

execute_load_open_s16u:
  region_check_open_align 1, 1, patch_load_s16
  open_load8_core
  seb $2, $2                      # sign extend result from 8bits

load_s16_ftable:
  .long execute_load_bios_s16     # 0x00 BIOS
  .long execute_load_open_s16     # 0x01 open address
  .long execute_load_ewram_s16    # 0x02 EWRAM
  .long execute_load_iwram_s16    # 0x03 IWRAM
  .long execute_load_io_s16       # 0x04 I/O registers
  .long execute_load_palette_s16  # 0x05 Palette RAM
  .long execute_load_vram_s16     # 0x06 VRAM
  .long execute_load_oam_s16      # 0x07 OAM RAM
  .long execute_load_gamepak8_s16 # 0x08 gamepak
  .long execute_load_gamepak9_s16 # 0x09 gamepak
  .long execute_load_gamepakA_s16 # 0x0A gamepak
  .long execute_load_gamepakB_s16 # 0x0B gamepak
  .long execute_load_gamepakC_s16 # 0x0C gamepak
  .long execute_load_eeprom_s16   # 0x0D gamepak/eeprom
  .long execute_load_backup_s16   # 0x0E Flash ROM/SRAM
  .long execute_load_open_s16     # 0x0F open unaligned

  .long execute_load_bios_s16u    # 0x00 BIOS unaligned
  .long execute_load_open_s16u    # 0x01 open address unaligned
  .long execute_load_ewram_s16u   # 0x02 EWRAM unaligned
  .long execute_load_iwram_s16u   # 0x03 IWRAM unaligned
  .long execute_load_io_s16u      # 0x04 I/O registers unaligned
  .long execute_load_palette_s16u # 0x05 Palette RAM unaligned
  .long execute_load_vram_s16u    # 0x06 VRAM unaligned
  .long execute_load_oam_s16u     # 0x07 OAM RAM unaligned
  .long execute_load_gamepak8_s16u# 0x08 gamepak unaligned
  .long execute_load_gamepak9_s16u# 0x09 gamepak unaligned
  .long execute_load_gamepakA_s16u# 0x0A gamepak unaligned
  .long execute_load_gamepakB_s16u# 0x0B gamepak unaligned
  .long execute_load_gamepakC_s16u# 0x0C gamepak unaligned
  .long execute_load_eeprom_s16u  # 0x0D gamepak/eeprom unaligned
  .long execute_load_backup_s16u  # 0x0E Flash ROM/SRAM unaligned
  .long execute_load_open_s16u    # 0x0F open unaligned

patch_load_s16:
  patch_handler_align load_s16_ftable, 1



# Unsigned aligned 32bit load handlers

execute_load_bios_u32:
  region_check_align 0, 2, 0, patch_load_u32
  srl $2, $4, 14                  # check if address is in BIOS region (delay)
  bne $2, $0, 2f                  # if not, perform open read
  srl $1, $5, 14                  # check if PC is in BIOS region (delay)
  bne $1, $0, 1f                  # if not, perform BIOS protected read

  lui $2, %hi(bios)               # generate upper address (delay)
  ins $2, $4, 0, 14               # generate offset
  load_u32 bios, -1

1:
  lui $2, %hi(bios_read_protect)  # generate upper address
  load_u32 bios_read_protect, -1

2:
  open_load32_core
  nop

execute_load_ewram_u32:
  translate_region_ewram_load_align32 2, 0, patch_load_u32
  load_u32 ewram, -6

execute_load_u32:
execute_load_iwram_u32:
  translate_region_align 3, 2, 0, patch_load_u32, iwram, 0x7FFC
  load_u32 iwram, -1

execute_load_io_u32:
  region_check_align 4, 2, 0, patch_load_u32

  addiu $2, $0, (0x0800 >> 2)     # $2 = 0x0800 >> 2 (delay)
  ext $1, $4, 2, 14               # $1 = (address & 0xFFFC) >> 2
  beq $2, $1, 2f

  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)
  bne $2, $0, 3f

  lui $2, %hi(io_registers)       # generate upper address (delay)
  ins $2, $4, 0, 10               # generate offset

  lw $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 3f

  srl $1, $1, 16
  beq $1, $0, 1f
  nop
  load_u32 io_registers, -1

1:
  load_u16 io_registers, -1

2:
  lui $2, %hi(iwram_control)      # generate upper address
  load_u32 iwram_control, -1

3:
  open_load32_core
  nop

execute_load_palette_u32:
  translate_region_align 5, 2, 0, patch_load_u32, palette_ram, 0x3FC
  load_u32 palette_ram, -2

execute_load_vram_u32:
  translate_region_vram_load_align32 2, 0, patch_load_u32
  load_u32 vram, -2

execute_load_oam_u32:
  translate_region_align 7, 2, 0, patch_load_u32, oam_ram, 0x3FC
  load_u32 oam_ram, -2

execute_load_gamepak8_u32:
  translate_region_gamepak_align32 8, 2, 0, patch_load_u32
  load_u32 0, -6

execute_load_gamepak9_u32:
  translate_region_gamepak_align32 9, 2, 0, patch_load_u32
  load_u32 0, -6

execute_load_gamepakA_u32:
  translate_region_gamepak_align32 10, 2, 0, patch_load_u32
  load_u32 0, -6

execute_load_gamepakB_u32:
  translate_region_gamepak_align32 11, 2, 0, patch_load_u32
  load_u32 0, -6

execute_load_gamepakC_u32:
  translate_region_gamepak_align32 12, 2, 0, patch_load_u32
  load_u32 0, -6

execute_load_gamepakD_u32:
  translate_region_gamepak_align32 13, 2, 0, patch_load_u32
  load_u32 0, -6

execute_load_backup_u32:
  backup_load_align32 2, 0, patch_load_u32
  nop

execute_load_open_u32:
  open_load32_align 2, 0, patch_load_u32
  nop


# Unsigned unaligned (by 1) 32bit load handlers

execute_load_bios_u32u1:
  region_check_align 0, 2, 1, patch_load_u32
  srl $2, $4, 14                  # check if address is in BIOS region (delay)
  bne $2, $0, 2f                  # if not, perform open read
  srl $1, $5, 14                  # check if PC is in BIOS region (delay)
  bne $1, $0, 1f                  # if not, perform BIOS protected read

  lui $2, %hi(bios)               # generate upper address (delay)
  andi $4, $4, 0x3FFC             # generate offset
  addu $2, $2, $4
  load_u32_unaligned bios, 1, -1

1:
  lui $2, %hi(bios_read_protect)  # generate upper address
  load_u32_unaligned bios_read_protect, 1, -1

2:
  open_load32_core
  ror $2, $2, 8

execute_load_ewram_u32u1:
  translate_region_ewram_load_align32 2, 1, patch_load_u32
  load_u32_unaligned ewram, 1, -6

execute_load_iwram_u32u1:
  translate_region_align 3, 2, 1, patch_load_u32, iwram, 0x7FFC
  load_u32_unaligned iwram, 1, -1

execute_load_io_u32u1:
  region_check_align 4, 2, 1, patch_load_u32

  addiu $2, $0, (0x0800 >> 2)     # $2 = 0x0800 >> 2 (delay)
  ext $1, $4, 2, 14               # $1 = (address & 0xFFFC) >> 2
  beq $1, $2, 2f

  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)
  bne $2, $0, 3f

  lui $2, %hi(io_registers)       # generate upper address (delay)
  andi $4, $4, 0x3FFC             # generate offset
  addu $2, $2, $4

  lw $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 3f

  srl $1, $1, 16
  beq $1, $0, 1f
  nop
  load_u32_unaligned io_registers, 1, -1

1:
  lhu $2, %lo(io_registers)($2)   # load base[offset]
  addiu $17, $17, -1              # waitstate cycles
  jr $ra                          # return
  ror $2, $2, 8                   # rotate value by 8bits

2:
  lui $2, %hi(iwram_control)      # generate upper address
  load_u32_unaligned iwram_control, 1, -1

3:
  open_load32_core
  ror $2, $2, 8

execute_load_palette_u32u1:
  translate_region_align 5, 2, 1, patch_load_u32, palette_ram, 0x3FC
  load_u32_unaligned palette_ram, 1, -2

execute_load_vram_u32u1:
  translate_region_vram_load_align32 2, 1, patch_load_u32
  load_u32_unaligned vram, 1, -2

execute_load_oam_u32u1:
  translate_region_align 7, 2, 1, patch_load_u32, oam_ram, 0x3FC
  load_u32_unaligned oam_ram, 1, -2

execute_load_gamepak8_u32u1:
  translate_region_gamepak_align32 8, 2, 1, patch_load_u32
  load_u32_unaligned 0, 1, -6

execute_load_gamepak9_u32u1:
  translate_region_gamepak_align32 9, 2, 1, patch_load_u32
  load_u32_unaligned 0, 1, -6

execute_load_gamepakA_u32u1:
  translate_region_gamepak_align32 10, 2, 1, patch_load_u32
  load_u32_unaligned 0, 1, -6

execute_load_gamepakB_u32u1:
  translate_region_gamepak_align32 11, 2, 1, patch_load_u32
  load_u32_unaligned 0, 1, -6

execute_load_gamepakC_u32u1:
  translate_region_gamepak_align32 12, 2, 1, patch_load_u32
  load_u32_unaligned 0, 1, -6

execute_load_gamepakD_u32u1:
  translate_region_gamepak_align32 13, 2, 1, patch_load_u32
  load_u32_unaligned 0, 1, -6

execute_load_backup_u32u1:
  backup_load_align32 2, 1, patch_load_u32
  nop

execute_load_open_u32u1:
  open_load32_align32 2, 1, patch_load_u32
  ror $2, $2, 8                   # rotate value by 8bits


# Unsigned unaligned (by 2) 32bit load handlers

execute_load_bios_u32u2:
  region_check_align 0, 2, 2, patch_load_u32
  srl $2, $4, 14                  # check if address is in BIOS region (delay)
  bne $2, $0, 2f                  # if not, perform open read
  srl $1, $5, 14                  # check if PC is in BIOS region (delay)
  bne $1, $0, 1f                  # if not, perform BIOS protected read

  lui $2, %hi(bios)               # generate upper address (delay)
  andi $4, $4, 0x3FFC             # generate offset
  addu $2, $2, $4
  load_u32_unaligned bios, 2, -1

1:
  lui $2, %hi(bios_read_protect)  # generate upper address
  load_u32_unaligned bios_read_protect, 2, -1

2:
  open_load32_core
  ror $2, $2, 16

execute_load_ewram_u32u2:
  translate_region_ewram_load_align32 2, 2, patch_load_u32
  load_u32_unaligned ewram, 2, -6

execute_load_iwram_u32u2:
  translate_region_align 3, 2, 2, patch_load_u32, iwram, 0x7FFC
  load_u32_unaligned iwram, 2, -1

execute_load_io_u32u2:
  region_check_align 4, 2, 2, patch_load_u32

  addiu $2, $0, (0x0800 >> 2)     # $2 = 0x0800 >> 2 (delay)
  ext $1, $4, 2, 14               # $1 = (address & 0xFFFC) >> 2
  beq $1, $2, 2f

  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)
  bne $2, $0, 3f

  lui $2, %hi(io_registers)       # generate upper address (delay)
  andi $4, $4, 0x3FFC             # generate offset
  addu $2, $2, $4

  lw $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 3f

  srl $1, $1, 16
  beq $1, $0, 1f
  nop
  load_u32_unaligned io_registers, 2, -1

1:
  lhu $2, %lo(io_registers)($2)   # load base[offset]
  addiu $17, $17, -1              # waitstate cycles
  jr $ra                          # return
  ror $2, $2, 16

2:
  lui $2, %hi(iwram_control)      # generate upper address
  load_u32_unaligned iwram_control, 2, -1

3:
  open_load32_core
  ror $2, $2, 16

execute_load_palette_u32u2:
  translate_region_align 5, 2, 2, patch_load_u32, palette_ram, 0x3FC
  load_u32_unaligned palette_ram, 2, -2

execute_load_vram_u32u2:
  translate_region_vram_load_align32 2, 2, patch_load_u32
  load_u32_unaligned vram, 2, -2

execute_load_oam_u32u2:
  translate_region_align 7, 2, 2, patch_load_u32, oam_ram, 0x3FC
  load_u32_unaligned oam_ram, 2, -2

execute_load_gamepak8_u32u2:
  translate_region_gamepak_align32 8, 2, 2, patch_load_u32
  load_u32_unaligned 0, 2, -6

execute_load_gamepak9_u32u2:
  translate_region_gamepak_align32 9, 2, 2, patch_load_u32
  load_u32_unaligned 0, 2, -6

execute_load_gamepakA_u32u2:
  translate_region_gamepak_align32 10, 2, 2, patch_load_u32
  load_u32_unaligned 0, 2, -6

execute_load_gamepakB_u32u2:
  translate_region_gamepak_align32 11, 2, 2, patch_load_u32
  load_u32_unaligned 0, 2, -6

execute_load_gamepakC_u32u2:
  translate_region_gamepak_align32 12, 2, 2, patch_load_u32
  load_u32_unaligned 0, 2, -6

execute_load_gamepakD_u32u2:
  translate_region_gamepak_align32 13, 2, 2, patch_load_u32
  load_u32_unaligned 0, 2, -6

execute_load_backup_u32u2:
  backup_load_align32 2, 2, patch_load_u32
  nop

execute_load_open_u32u2:
  open_load32_align32 2, 2, patch_load_u32
  ror $2, $2, 16                  # rotate value by 16bits

# Unsigned unaligned (by 1) 32bit load handlers

execute_load_bios_u32u3:
  region_check_align 0, 2, 3, patch_load_u32
  srl $2, $4, 14                  # check if address is in BIOS region (delay)
  bne $2, $0, 2f                  # if not, perform open read
  srl $1, $5, 14                  # check if PC is in BIOS region (delay)
  bne $1, $0, 1f                  # if not, perform BIOS protected read

  lui $2, %hi(bios)               # generate upper address (delay)
  andi $4, $4, 0x3FFC             # generate offset
  addu $2, $2, $4
  load_u32_unaligned bios, 3, -1

1:
  lui $2, %hi(bios_read_protect)  # generate upper address
  load_u32_unaligned bios_read_protect, 3, -1

2:
  open_load32_core
  ror $2, $2, 24

execute_load_ewram_u32u3:
  translate_region_ewram_load_align32 2, 3, patch_load_u32
  load_u32_unaligned ewram, 3, -6

execute_load_iwram_u32u3:
  translate_region_align 3, 2, 3, patch_load_u32, iwram, 0x7FFC
  load_u32_unaligned iwram, 3, -1

execute_load_io_u32u3:
  region_check_align 4, 2, 3, patch_load_u32

  addiu $2, $0, (0x0800 >> 2)     # $2 = 0x0800 >> 2 (delay)
  ext $1, $4, 2, 14               # $1 = (address & 0xFFFC) >> 2
  beq $1, $2, 2f

  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)
  bne $2, $0, 3f

  lui $2, %hi(io_registers)       # generate upper address (delay)
  andi $4, $4, 0x3FFC             # generate offset
  addu $2, $2, $4

  lw $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 3f

  srl $1, $1, 16
  beq $1, $0, 1f
  nop
  load_u32_unaligned io_registers, 3, -1

1:
  lhu $2, %lo(io_registers)($2)   # load base[offset]
  addiu $17, $17, -1              # waitstate cycles
  jr $ra                          # return
  ror $2, $2, 24

2:
  lui $2, %hi(iwram_control)      # generate upper address
  load_u32_unaligned iwram_control, 3, -1

3:
  open_load32_core
  ror $2, $2, 24

execute_load_palette_u32u3:
  translate_region_align 5, 2, 3, patch_load_u32, palette_ram, 0x3FC
  load_u32_unaligned palette_ram, 3, -2

execute_load_vram_u32u3:
  translate_region_vram_load_align32 2, 3, patch_load_u32
  load_u32_unaligned vram, 3, -2

execute_load_oam_u32u3:
  translate_region_align 7, 2, 3, patch_load_u32, oam_ram, 0x3FC
  load_u32_unaligned oam_ram, 3, -2

execute_load_gamepak8_u32u3:
  translate_region_gamepak_align32 8, 2, 3, patch_load_u32
  load_u32_unaligned 0, 3, -6

execute_load_gamepak9_u32u3:
  translate_region_gamepak_align32 9, 2, 3, patch_load_u32
  load_u32_unaligned 0, 3, -6

execute_load_gamepakA_u32u3:
  translate_region_gamepak_align32 10, 2, 3, patch_load_u32
  load_u32_unaligned 0, 3, -6

execute_load_gamepakB_u32u3:
  translate_region_gamepak_align32 11, 2, 3, patch_load_u32
  load_u32_unaligned 0, 3, -6

execute_load_gamepakC_u32u3:
  translate_region_gamepak_align32 12, 2, 3, patch_load_u32
  load_u32_unaligned 0, 3, -6

execute_load_gamepakD_u32u3:
  translate_region_gamepak_align32 13, 2, 3, patch_load_u32
  load_u32_unaligned 0, 3, -6

execute_load_backup_u32u3:
  backup_load_align32 2, 3, patch_load_u32
  nop

execute_load_open_u32u3:
  open_load32_align32 2, 3, patch_load_u32
  ror $2, $2, 24                  # rotate value by 24bits


load_u32_ftable:
  .long execute_load_bios_u32     # 0x00 BIOS
  .long execute_load_open_u32     # 0x01 open address
  .long execute_load_ewram_u32    # 0x02 EWRAM
  .long execute_load_iwram_u32    # 0x03 IWRAM
  .long execute_load_io_u32       # 0x04 I/O registers
  .long execute_load_palette_u32  # 0x05 Palette RAM
  .long execute_load_vram_u32     # 0x06 VRAM
  .long execute_load_oam_u32      # 0x07 OAM RAM
  .long execute_load_gamepak8_u32 # 0x08 gamepak
  .long execute_load_gamepak9_u32 # 0x09 gamepak
  .long execute_load_gamepakA_u32 # 0x0A gamepak
  .long execute_load_gamepakB_u32 # 0x0B gamepak
  .long execute_load_gamepakC_u32 # 0x0C gamepak
  .long execute_load_gamepakD_u32 # 0x0D gamepak/eeprom
  .long execute_load_backup_u32   # 0x0E Flash ROM/SRAM
  .long execute_load_open_u32     # 0x0F open

  .long execute_load_bios_u32u1     # 0x00 BIOS unaligned (1b)
  .long execute_load_open_u32u1     # 0x01 open address unaligned (1b)
  .long execute_load_ewram_u32u1    # 0x02 EWRAM unaligned (1b)
  .long execute_load_iwram_u32u1    # 0x03 IWRAM unaligned (1b)
  .long execute_load_io_u32u1       # 0x04 I/O registers unaligned (1b)
  .long execute_load_palette_u32u1  # 0x05 Palette RAM unaligned (1b)
  .long execute_load_vram_u32u1     # 0x06 VRAM unaligned (1b)
  .long execute_load_oam_u32u1      # 0x07 OAM RAM unaligned (1b)
  .long execute_load_gamepak8_u32u1 # 0x08 gamepak unaligned (1b)
  .long execute_load_gamepak9_u32u1 # 0x09 gamepak unaligned (1b)
  .long execute_load_gamepakA_u32u1 # 0x0A gamepak unaligned (1b)
  .long execute_load_gamepakB_u32u1 # 0x0B gamepak unaligned (1b)
  .long execute_load_gamepakC_u32u1 # 0x0C gamepak unaligned (1b)
  .long execute_load_gamepakD_u32u1 # 0x0D gamepak/eeprom unaligned (1b)
  .long execute_load_backup_u32u1   # 0x0E Flash ROM/SRAM unaligned (1b)
  .long execute_load_open_u32u1     # 0x0F open unaligned (1b)

  .long execute_load_bios_u32u2     # 0x00 BIOS unaligned (2b)
  .long execute_load_open_u32u2     # 0x01 open address unaligned (2b)
  .long execute_load_ewram_u32u2    # 0x02 EWRAM unaligned (2b)
  .long execute_load_iwram_u32u2    # 0x03 IWRAM unaligned (2b)
  .long execute_load_io_u32u2       # 0x04 I/O registers unaligned (2b)
  .long execute_load_palette_u32u2  # 0x05 Palette RAM unaligned (2b)
  .long execute_load_vram_u32u2     # 0x06 VRAM unaligned (2b)
  .long execute_load_oam_u32u2      # 0x07 OAM RAM unaligned (2b)
  .long execute_load_gamepak8_u32u2 # 0x08 gamepak unaligned (2b)
  .long execute_load_gamepak9_u32u2 # 0x09 gamepak unaligned (2b)
  .long execute_load_gamepakA_u32u2 # 0x0A gamepak unaligned (2b)
  .long execute_load_gamepakB_u32u2 # 0x0B gamepak unaligned (2b)
  .long execute_load_gamepakC_u32u2 # 0x0C gamepak unaligned (2b)
  .long execute_load_gamepakD_u32u2 # 0x0D gamepak/eeprom unaligned (2b)
  .long execute_load_backup_u32u2   # 0x0E Flash ROM/SRAM unaligned (2b)
  .long execute_load_open_u32u2     # 0x0F open unaligned (2b)

  .long execute_load_bios_u32u3     # 0x00 BIOS unaligned (3b)
  .long execute_load_open_u32u3     # 0x01 open address unaligned (3b)
  .long execute_load_ewram_u32u3    # 0x02 EWRAM unaligned (3b)
  .long execute_load_iwram_u32u3    # 0x03 IWRAM unaligned (3b)
  .long execute_load_io_u32u3       # 0x04 I/O registers unaligned (3b)
  .long execute_load_palette_u32u3  # 0x05 Palette RAM unaligned (3b)
  .long execute_load_vram_u32u3     # 0x06 VRAM unaligned (3b)
  .long execute_load_oam_u32u3      # 0x07 OAM RAM unaligned (3b)
  .long execute_load_gamepak8_u32u3 # 0x08 gamepak unaligned (3b)
  .long execute_load_gamepak9_u32u3 # 0x09 gamepak unaligned (3b)
  .long execute_load_gamepakA_u32u3 # 0x0A gamepak unaligned (3b)
  .long execute_load_gamepakB_u32u3 # 0x0B gamepak unaligned (3b)
  .long execute_load_gamepakC_u32u3 # 0x0C gamepak unaligned (3b)
  .long execute_load_gamepakD_u32u3 # 0x0D gamepak/eeprom unaligned (3b)
  .long execute_load_backup_u32u3   # 0x0E Flash ROM/SRAM unaligned (3b)
  .long execute_load_open_u32u3     # 0x0F open unaligned (3b)

patch_load_u32:
  patch_handler_align load_u32_ftable, 2



# Unsigned always aligned 32bit load handlers

execute_load_bios_u32a:
  region_check 0, patch_load_u32a
  srl $2, $4, 14                  # check if address is in BIOS region (delay)
  bne $2, $0, 2f                  # if not, perform open read
  srl $1, $5, 14                  # check if PC is in BIOS region (delay)
  bne $1, $0, 1f                  # if not, perform BIOS protected read

  lui $2, %hi(bios)               # generate upper address (delay)
  ins $2, $4, 0, 14               # generate offset
  load_u32 bios, -1

1:
  lui $2, %hi(bios_read_protect)  # generate upper address
  load_u32 bios_read_protect, -1

2:
  open_load32a_core

execute_load_ewram_u32a:
  translate_region_ewram patch_load_u32a
  load_u32 ewram, -6

execute_aligned_load32:
execute_load_iwram_u32a:
  translate_region 3, patch_load_u32a, iwram, 0x7FFC
  load_u32 iwram, -1

execute_load_io_u32a:
  region_check 4, patch_load_u32a

  addiu $2, $0, (0x0800 >> 2)     # $2 = 0x0800 >> 2 (delay)
  ext $1, $4, 2, 14               # $1 = (address & 0xFFFC) >> 2
  beq $1, $2, 2f

  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)
  bne $2, $0, 3f

  lui $2, %hi(io_registers)       # generate upper address (delay)
  ins $2, $4, 0, 10               # generate offset

  lw $1, %lo(io_registers + 0x400)($2)
  beq $1, $0, 3f

  srl $1, $1, 16
  beq $1, $0, 1f
  nop
  load_u32 io_registers, -1

1:
  load_u16 io_registers, -1

2:
  lui $2, %hi(iwram_control)      # generate upper address
  load_u32 iwram_control, -1

3:
  open_load32a_core

execute_load_palette_u32a:
  translate_region 5, patch_load_u32a, palette_ram, 0x3FC
  load_u32 palette_ram, -2

execute_load_vram_u32a:
  translate_region_vram patch_load_u32a
  load_u32 vram, -2

execute_load_oam_u32a:
  translate_region 7, patch_load_u32a, oam_ram, 0x3FC
  load_u32 oam_ram, -2

execute_load_gamepak8_u32a:
  translate_region_gamepak_a 8, patch_load_u32a
  load_u32 0, -4

execute_load_gamepak9_u32a:
  translate_region_gamepak_a 9, patch_load_u32a
  load_u32 0, -4

execute_load_gamepakA_u32a:
  translate_region_gamepak_a 10, patch_load_u32a
  load_u32 0, -4

execute_load_gamepakB_u32a:
  translate_region_gamepak_a 11, patch_load_u32a
  load_u32 0, -4

execute_load_gamepakC_u32a:
  translate_region_gamepak_a 12, patch_load_u32a
  load_u32 0, -4

execute_load_gamepakD_u32a:
  translate_region_gamepak_a 13, patch_load_u32a
  load_u32 0, -4

execute_load_backup_u32a:
  backup_load_a patch_load_u32a

execute_load_open_u32a:
  open_load32_a patch_load_u32a

load_u32a_ftable:
  .long execute_load_bios_u32a      # 0x00 BIOS unaligned (3b)
  .long execute_load_open_u32a      # 0x01 open address unaligned (3b)
  .long execute_load_ewram_u32a     # 0x02 EWRAM unaligned (3b)
  .long execute_load_iwram_u32a     # 0x03 IWRAM unaligned (3b)
  .long execute_load_io_u32a        # 0x04 I/O registers unaligned (3b)
  .long execute_load_palette_u32a   # 0x05 Palette RAM unaligned (3b)
  .long execute_load_vram_u32a      # 0x06 VRAM unaligned (3b)
  .long execute_load_oam_u32a       # 0x07 OAM RAM unaligned (3b)
  .long execute_load_gamepak8_u32a  # 0x08 gamepak unaligned (3b)
  .long execute_load_gamepak9_u32a  # 0x09 gamepak unaligned (3b)
  .long execute_load_gamepakA_u32a  # 0x0A gamepak unaligned (3b)
  .long execute_load_gamepakB_u32a  # 0x0B gamepak unaligned (3b)
  .long execute_load_gamepakC_u32a  # 0x0C gamepak unaligned (3b)
  .long execute_load_gamepakD_u32a  # 0x0D gamepak/eeprom unaligned (3b)
  .long execute_load_backup_u32a    # 0x0E Flash ROM/SRAM unaligned (3b)
  .long execute_load_open_u32a      # 0x0F open unaligned (3b)

patch_load_u32a:
  patch_handler load_u32a_ftable, 1


# Unsigned 8bit store handlers

execute_store_ignore0_u8:
  ignore_region 0, patch_store_u8

execute_store_ignore1_u8:
  ignore_region 1, patch_store_u8

execute_store_ewram_u8:
  translate_region_ewram patch_store_u8
  store_u8_metadata ewram, -3, post_write_metadata_ewram

execute_store_u8:
execute_store_iwram_u8:
  translate_region 3, patch_store_u8, iwram, 0x7FFF
  store_u8_metadata iwram, -1, post_write_metadata_iwram

execute_store_io_u8:
  region_check 4, patch_store_u8
  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)

  bne $2, $0, store_ignore        # if not, unwritable
  andi $4, $4, 0x3FF              # wrap around address (delay)
  andi $5, $5, 0xFF               # make value 8bit

  sw $6, REG_PC($16)              # save the PC

  addiu $sp, $sp, -4              # make room on the stack for $ra
  sw $ra, ($sp)

  save_registers_0                # save registers 1/2
  jal write_io_register8          # write the value out
  save_registers_1                # save registers 2/2 (delay)

  j write_io_epilogue             # handle any state changes
  addiu $17, $17, -1              # waitstate cycles (delay)

execute_store_palette_u8:
  translate_region 5, patch_store_u8, palette_ram, 0x3FE
  store_u8_double palette_ram, -1

execute_store_vram_u8:
  region_check 6, patch_store_u8

  lui $2, %hi(obj_address)        # $2 = OBJ Tile address (delay)
  lw $2, %lo(obj_address)($2)

  ext $1, $4, 16, 1               # $1 = bit 16 of address
  ext $4, $4, 0, 17               # generate 17bit offset
  bnel $1, $0, 1f                 # if address >= 0x10000
  ins $4, $0, 15, 1               # mask out bit 15 of address (delay Likely)

1:
  sltu $1, $4, $2                 # if (address < OBJ Tile address) $1 = 1
  beq $1, $0, store_ignore        # byte writes to OBJ are ignored

  ins $5, $5, 8, 8                # value = (value << 8) | value (delay)
  ins $4, $0, 0, 1                # mask out lower bit of address

  lui $2, %hi(vram)               # start loading vram address
  addu $2, $2, $4                 # $2 = (hi)vram + address
  store_u16_metadata vram, -1, post_write_metadata_vram


execute_store_oam_u8:
  ignore_region 7, patch_store_u8 # Write byte datas are ignore

execute_store_ignore8_u8:
  ignore_region 8, patch_store_u8

execute_store_ignore9_u8:
  ignore_region 9, patch_store_u8

execute_store_ignoreA_u8:
  ignore_region 10, patch_store_u8

execute_store_ignoreB_u8:
  ignore_region 11, patch_store_u8

execute_store_ignoreC_u8:
  ignore_region 12, patch_store_u8

execute_store_ignoreD_u8:
  ignore_region 13, patch_store_u8

execute_store_backup_u8:
  backup_store 14, patch_store_u8

execute_store_ignoreF_u8:
  ignore_high patch_store_u8

store_u8_ftable:
  .long execute_store_ignore0_u8  # 0x00 BIOS
  .long execute_store_ignore1_u8  # 0x01 open address
  .long execute_store_ewram_u8    # 0x02 EWRAM
  .long execute_store_iwram_u8    # 0x03 IWRAM
  .long execute_store_io_u8       # 0x04 I/O registers
  .long execute_store_palette_u8  # 0x05 Palette RAM
  .long execute_store_vram_u8     # 0x06 VRAM
  .long execute_store_oam_u8      # 0x07 OAM RAM
  .long execute_store_ignore8_u8  # 0x08 gamepak
  .long execute_store_ignore9_u8  # 0x09 gamepak
  .long execute_store_ignoreA_u8  # 0x0A gamepak
  .long execute_store_ignoreB_u8  # 0x0B gamepak
  .long execute_store_ignoreC_u8  # 0x0C gamepak
  .long execute_store_ignoreD_u8  # 0x0D gamepak/eeprom
  .long execute_store_backup_u8   # 0x0E Flash ROM/SRAM
  .long execute_store_ignoreF_u8  # 0x0F open address

patch_store_u8:
  patch_handler store_u8_ftable, 0x0F


# Unsigned 16bit store handlers

execute_store_ignore0_u16:
  ignore_region 0, patch_store_u16

execute_store_ignore1_u16:
  ignore_region 1, patch_store_u16

execute_store_ewram_u16:
  translate_region_ewram_store_align16 patch_store_u16
  store_u16_metadata ewram, -3, post_write_metadata_ewram

execute_store_u16:
execute_store_iwram_u16:
  translate_region 3, patch_store_u16, iwram, 0x7FFE
  store_u16_metadata iwram, -1, post_write_metadata_iwram

execute_store_io_u16:
  region_check 4, patch_store_u16
  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)

  bne $2, $0, store_ignore        # if not, unwritable
  andi $4, $4, 0x3Fe              # wrap around/align address (delay)
  ext $5, $5, 0, 16               # make value 16bit

  sw $6, REG_PC($16)              # save the PC

  addiu $sp, $sp, -4              # make room on the stack for $ra
  sw $ra, ($sp)

  save_registers_0                # save registers 1/2
  jal write_io_register16         # write the value out
  save_registers_1                # save registers 2/2 (delay)

  j write_io_epilogue             # handle any state changes
  addiu $17, $17, -1              # waitstate cycles (delay)

execute_store_palette_u16:
  translate_region 5, patch_store_u16, palette_ram, 0x3FE
  store_u16 palette_ram, -1

execute_store_vram_u16:
  translate_region_vram_store_align16 patch_store_u16
  store_u16_metadata vram, -1, post_write_metadata_vram

execute_store_oam_u16:
  translate_region 7, patch_store_u16, oam_ram, 0x3FE
  lui $1, %hi(oam_update)         # write non-zero to oam_update (delay)
  sw $1, %lo(oam_update)($1)      # cheap, but the address is non-zero
  store_u16 oam_ram, -1

execute_store_rtc_u16:
  rtc_store 8, patch_store_u16

execute_store_ignore9_u16:
  ignore_region 9, patch_store_u16

execute_store_ignoreA_u16:
  ignore_region 10, patch_store_u16

execute_store_ignoreB_u16:
  ignore_region 11, patch_store_u16

execute_store_ignoreC_u16:
  ignore_region 12, patch_store_u16

execute_store_ignoreD_u16:
  ignore_region 13, patch_store_u16

execute_store_ignoreE_u16:
  ignore_region 14, patch_store_u16

execute_store_ignoreF_u16:
  ignore_high patch_store_u16


store_u16_ftable:
  .long execute_store_ignore0_u16 # 0x00 BIOS
  .long execute_store_ignore1_u16 # 0x01 open address
  .long execute_store_ewram_u16   # 0x02 EWRAM
  .long execute_store_iwram_u16   # 0x03 IWRAM
  .long execute_store_io_u16      # 0x04 I/O registers
  .long execute_store_palette_u16 # 0x05 Palette RAM
  .long execute_store_vram_u16    # 0x06 VRAM
  .long execute_store_oam_u16     # 0x07 OAM RAM
  .long execute_store_rtc_u16     # 0x08 gamepak
  .long execute_store_ignore9_u16 # 0x09 gamepak
  .long execute_store_ignoreA_u16 # 0x0A gamepak
  .long execute_store_ignoreB_u16 # 0x0B gamepak
  .long execute_store_ignoreC_u16 # 0x0C gamepak
  .long execute_store_ignoreD_u16 # 0x0D gamepak/eeprom
  .long execute_store_ignoreE_u16 # 0x0E Flash ROM/SRAM
  .long execute_store_ignoreF_u16 # 0x0F open address

patch_store_u16:
  patch_handler store_u16_ftable, 0x0F


# Unsigned 32bit store handlers

execute_store_ignore0_u32:
  ignore_region 0, patch_store_u32

execute_store_ignore1_u32:
  ignore_region 1, patch_store_u32

execute_store_ewram_u32:
  translate_region_ewram_store_align32 patch_store_u32
  store_u32_metadata ewram, -6, post_write_metadata_ewram

execute_store_u32:
execute_store_iwram_u32:
  translate_region 3, patch_store_u32, iwram, 0x7FFC
  store_u32_metadata iwram, -1, post_write_metadata_iwram

execute_store_io_u32:
  region_check 4, patch_store_u32
  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)

  bne $2, $0, store_ignore        # if not, unwritable
  andi $4, $4, 0x3FC              # wrap around/align address (delay)

  sw $6, REG_PC($16)              # save the PC

  addiu $sp, $sp, -4              # make room on the stack for $ra
  sw $ra, ($sp)

  save_registers_0                # save registers 1/2
  jal write_io_register32         # write the value out
  save_registers_1                # save registers 2/2 (delay)

  j write_io_epilogue             # handle any state changes
  addiu $17, $17, -1              # waitstate cycles (delay)

execute_store_palette_u32:
  translate_region 5, patch_store_u32, palette_ram, 0x3FC
  store_u32 palette_ram, -2

execute_store_vram_u32:
  translate_region_vram_store_align32 patch_store_u32
  store_u32_metadata vram, -2, post_write_metadata_vram

execute_store_oam_u32:
  translate_region 7, patch_store_u32, oam_ram, 0x3FC
  lui $1, %hi(oam_update)         # write non-zero to oam_update
  sw $1, %lo(oam_update)($1)      # cheap, but the address is non-zero
  store_u32 oam_ram, -2

execute_store_ignore8_u32:
  ignore_region 8, patch_store_u32

execute_store_ignore9_u32:
  ignore_region 9, patch_store_u32

execute_store_ignoreA_u32:
  ignore_region 10, patch_store_u32

execute_store_ignoreB_u32:
  ignore_region 11, patch_store_u32

execute_store_ignoreC_u32:
  ignore_region 12, patch_store_u32

execute_store_ignoreD_u32:
  ignore_region 13, patch_store_u32

execute_store_ignoreE_u32:
  ignore_region 14, patch_store_u32

execute_store_ignoreF_u32:
  ignore_high patch_store_u32

store_u32_ftable:
  .long execute_store_ignore0_u32 # 0x00 BIOS
  .long execute_store_ignore1_u32 # 0x01 open address
  .long execute_store_ewram_u32   # 0x02 EWRAM
  .long execute_store_iwram_u32   # 0x03 IWRAM
  .long execute_store_io_u32      # 0x04 I/O registers
  .long execute_store_palette_u32 # 0x05 Palette RAM
  .long execute_store_vram_u32    # 0x06 VRAM
  .long execute_store_oam_u32     # 0x07 OAM RAM
  .long execute_store_ignore8_u32 # 0x08 gamepak
  .long execute_store_ignore9_u32 # 0x09 gamepak
  .long execute_store_ignoreA_u32 # 0x0A gamepak
  .long execute_store_ignoreB_u32 # 0x0B gamepak
  .long execute_store_ignoreC_u32 # 0x0C gamepak
  .long execute_store_ignoreD_u32 # 0x0D gamepak/eeprom
  .long execute_store_ignoreE_u32 # 0x0E Flash ROM/SRAM
  .long execute_store_ignoreF_u32 # 0x0F open address


patch_store_u32:
  patch_handler store_u32_ftable, 0x0F



# Unsigned always aligned, a2 safe 32bit store handlers

execute_store_ignore0_u32a:
  ignore_region 0, patch_store_u32a

execute_store_ignore1_u32a:
  ignore_region 1, patch_store_u32a

execute_store_ewram_u32a:
  translate_region_ewram_store_align32 patch_store_u32a
  store_u32a ewram, -6

execute_aligned_store32:
execute_store_iwram_u32a:
  translate_region 3, patch_store_u32a, iwram, 0x7FFC
  store_u32a iwram, -1

execute_store_io_u32a:
  region_check 4, patch_store_u32a
  ext $2, $4, 10, 14              # $2 = (address >> 10) & 0x3FFF (delay)

  bne $2, $0, store_ignore        # if not, unwritable
  andi $4, $4, 0x3FC              # wrap around/align address (delay)

  sw $6, REG_SAVE($16)            # save a2
  sw $ra, REG_SAVE2($16)          # save ra

  save_registers_0                # save registers 1/2
  jal write_io_register32         # write the value out
  save_registers_1                # save registers 2/2

  lw $6, REG_SAVE($16)            # restore a2
  lw $ra, REG_SAVE2($16)          # restore ra

  addiu $17, $17, -1              # waitstate cycles

  restore_registers_0             # restore registers 1/2
  jr $ra
  restore_registers_1             # restore registers 2/2

execute_store_palette_u32a:
  translate_region 5, patch_store_u32a, palette_ram, 0x3FC
  store_u32a palette_ram, -2

execute_store_vram_u32a:
  translate_region_vram_store_align32 patch_store_u32a
  store_u32a vram, -2

execute_store_oam_u32a:
  translate_region 7, patch_store_u32a, oam_ram, 0x3FC
  lui $1, %hi(oam_update)         # write non-zero to oam_update
  sw $1, %lo(oam_update)($1)      # cheap, but the address is non-zero
  store_u32a oam_ram, -2

execute_store_ignore8_u32a:
  ignore_region 8, patch_store_u32a

execute_store_ignore9_u32a:
  ignore_region 9, patch_store_u32a

execute_store_ignoreA_u32a:
  ignore_region 10, patch_store_u32a

execute_store_ignoreB_u32a:
  ignore_region 11, patch_store_u32a

execute_store_ignoreC_u32a:
  ignore_region 12, patch_store_u32a

execute_store_ignoreD_u32a:
  ignore_region 13, patch_store_u32a

execute_store_ignoreE_u32a:
  ignore_region 14, patch_store_u32a

execute_store_ignoreF_u32a:
  ignore_high patch_store_u32a

store_u32a_ftable:
  .long execute_store_ignore0_u32a # 0x00 BIOS
  .long execute_store_ignore1_u32a # 0x01 open address
  .long execute_store_ewram_u32a   # 0x02 EWRAM
  .long execute_store_iwram_u32a   # 0x03 IWRAM
  .long execute_store_io_u32a      # 0x04 I/O registers
  .long execute_store_palette_u32a # 0x05 Palette RAM
  .long execute_store_vram_u32a    # 0x06 VRAM
  .long execute_store_oam_u32a     # 0x07 OAM RAM
  .long execute_store_ignore8_u32a # 0x08 gamepak
  .long execute_store_ignore9_u32a # 0x09 gamepak
  .long execute_store_ignoreA_u32a # 0x0A gamepak
  .long execute_store_ignoreB_u32a # 0x0B gamepak
  .long execute_store_ignoreC_u32a # 0x0C gamepak
  .long execute_store_ignoreD_u32a # 0x0D gamepak/eeprom
  .long execute_store_ignoreE_u32a # 0x0E Flash ROM/SRAM
  .long execute_store_ignoreF_u32a # 0x0F open address

patch_store_u32a:
  patch_handler store_u32a_ftable, 0x0F



# General ext memory routines

store_ignore:
  jr $ra                          # ignore these writes
  addiu $17, $17, -1              # waitstate cycles (delay)


# $2: alert type
# bit0 - SMC
# bit1 - IRQ
# bit2 - HALT
# bit3 - timer counter change.
# bit4 - DMA transfers

write_io_epilogue:
  andi $4, $2, 0x01
  bne $4, $0, io_smc_write        # is it an SMC alert?
  andi $4, $2, 0x1e

#  blez $17, io_update_gba         # if cycle counter <= 0
  blez $17, io_alert              # if cycle counter <= 0
  nop

  bne $4, $0, io_alert
  nop

no_alert:
  lw $ra, ($sp)                   # restore return address
  addiu $sp, $sp, 4               # fix the stack

  restore_registers_0             # restore registers 1/2
  jr $ra                          # return
  restore_registers_1             # restore registers 2/2 (delay)

io_alert:
  lw $1, EXECUTE_CYCLES($16)
  lw $2, REG_CPSR($16)            # load CPSR
  sw $0, CHANGED_PC_STATUS($16)

  subu $1, $1, $17                # execute_cycles -= cycle counter
  sw $1, EXECUTE_CYCLES($16)

  collapse_flags_body             # insert flag into $2
  jal update_gba                  # process the next event
  sw $2, REG_CPSR($16)            # store CPSR

  lw $1, CHANGED_PC_STATUS($16)

  lw $ra, ($sp)                   # restore return address
  addiu $sp, $sp, 4               # fix the stack

  bne $1, $0, lookup_pc_changed
  addu $17, $2, $0                # $17 = new cycle count (delay)

  restore_registers_0             # restore registers 1/2
  jr $ra                          # return
  restore_registers_1             # restore registers 2/2 (delay)

io_update_gba:
  lw $2, REG_CPSR($16)            # load CPSR
  sw $0, CHANGED_PC_STATUS($16)

  collapse_flags_body             # insert flag into $2
  jal update_gba                  # process the next event
  sw $2, REG_CPSR($16)            # store CPSR

  lw $1, CHANGED_PC_STATUS($16)

  lw $ra, ($sp)                   # restore return address
  addiu $sp, $sp, 4               # fix the stack

  bne $1, $0, lookup_pc_changed
  addu $17, $2, $0                # $17 = new cycle count (delay)

  restore_registers_0             # restore registers 1/2
  jr $ra                          # return
  restore_registers_1             # restore registers 2/2 (delay)

io_smc_write:
  blez $17, smc_write_update_gba  # if cycle counter <= 0
  addiu $sp, $sp, 4               # fix the stack (delay)

  beq $4, $0, lookup_pc           # alert none
  nop

io_smc_alert:
  lw $1, EXECUTE_CYCLES($16)      # $1 = execute_cycles
  lw $2, REG_CPSR($16)            # load CPSR
  sw $0, CHANGED_PC_STATUS($16)

  subu $1, $1, $17                # execute_cycles -= cycle counter
  sw $1, EXECUTE_CYCLES($16)

  collapse_flags_body             # insert flag into $2
  jal update_gba                  # process the next event
  sw $2, REG_CPSR($16)            # store CPSR

  lw $1, CHANGED_PC_STATUS($16)

  bne $1, $0, lookup_pc_changed
  addu $17, $2, $0                # $17 = new cycle count (delay)

  j lookup_pc
  nop


smc_write_update_gba:
  lw $2, REG_CPSR($16)            # load CPSR
  sw $0, CHANGED_PC_STATUS($16)

  collapse_flags_body             # insert flag into $2
  jal update_gba                  # process the next event
  sw $2, REG_CPSR($16)            # store CPSR (delay)

  lw $1, CHANGED_PC_STATUS($16)

  bne $1, $0, lookup_pc_changed
  addu $17, $2, $0                # $17 = new cycle count (delay)

  j lookup_pc
  nop

# smc_write expects three registers to be set:
# $4 = offset into the Data Area, canonical (address AND (size - 1))
# $5 = GBA memory region (0x02, 0x03, 0x06, etc.)
# $6 = Program Counter, which is immediately looked up
smc_write:
  save_registers
  jal partial_clear_metadata      # flush some of the translation cache
  sw $6, REG_PC($16)              # save PC (delay slot)
  blez $17, smc_write_update_gba  # if counter <= 0
  nop

lookup_pc:
  lw $2, REG_CPSR($16)            # load CPSR
  andi $2, $2, 0x20               # isolate mode bit
  beq $2, $0, lookup_pc_arm       # if T bit is zero use arm handler
  nop

lookup_pc_thumb:
  jal block_lookup_address_thumb  # get Thumb address
  lw $4, REG_PC($16)              # load PC as arg 0 (delay slot)

  restore_registers_0             # restore registers 1/2
  jr $2                           # jump to result
  restore_registers_1             # restore registers 2/2 (delay)

lookup_pc_changed:
  lw $1, REG_CPSR($16)            # load CPSR
  extract_flags_body              # extract flags from $1
  andi $2, $1, 0x20               # isolate mode bit
  bne $2, $0, lookup_pc_thumb     # if T bit is not zero use thumb handler
  nop

lookup_pc_arm:
  jal block_lookup_address_arm    # get ARM address
  lw $4, REG_PC($16)              # load PC as arg 0 (delay slot)

  restore_registers_0             # restore registers 1/2
  jr $2                           # jump to result
  restore_registers_1             # restore registers 2/2 (delay)


# Return the current cpsr

execute_read_cpsr:
  collapse_flags                  # fold flags into cpsr, put cpsr into $2
  jr $ra                          # return
  nop

# Return the current spsr

execute_read_spsr:
  lw $1, CPU_MODE($16)            # $1 = cpu_mode

  lui $2, %hi(spsr)
  sll $1, $1, 2                   # adjust to word offset size
  addu $2, $2, $1

  jr $ra                          # return
  lw $2, %lo(spsr)($2)            # $2 = spsr[cpu_mode] (delay slot)


# Switch into SWI, has to collapse flags
# $4: Current pc

execute_swi:
  lw $2, REG_CPSR($16)            # load CPSR

  lui $1, %hi(SUPERVISOR_LR)
  sw  $4, %lo(SUPERVISOR_LR)($1)  # store next PC in the supervisor's LR

  addiu $sp, $sp, -4              # push $ra
  sw $ra, ($sp)

  collapse_flags_body             # insert flag into $2
  lui $5, %hi(SUPERVISOR_SPSR)
  sw  $2, %lo(SUPERVISOR_SPSR)($5)# save cpsr in SUPERVISOR_CPSR

  ins $2, $0, 0, 8                # zero out bottom 8 bits of CPSR
  ori $2, 0x93                    # set mode to supervisor & disable IRQ
  sw $2, REG_CPSR($16)            # write back CPSR

  addiu $4, $0, 3                 # 3 is supervisor mode

  save_registers_0                # save registers 1/2
  jal set_cpu_mode                # set the CPU mode to supervisor
  save_registers_1                # save registers 2/2 (delay)

  lw $ra, ($sp)                   # pop $ra
  addiu $sp, $sp, 4               # fix stack (delay slot)

  restore_registers_0             # restore registers 1/2
  jr $ra                          # return
  restore_registers_1             # restore registers 2/2


# $4: pc to restore to
# returns in $4

execute_spsr_restore:
  lw $1, CPU_MODE($16)            # $1 = cpu_mode

  beq $1, $0, no_spsr_restore     # only restore if the cpu isn't usermode
  lui $2, %hi(spsr)               # start loading SPSR (delay)
  sll $1, $1, 2                   # adjust to word offset size
  addu $2, $2, $1
  lw $1, %lo(spsr)($2)            # $1 = spsr[cpu_mode]

  extract_flags_body              # extract flags from $1
  sw $1, REG_CPSR($16)            # cpsr = spsr[cpu_mode]

  ext $2, $1, 5, 1
  jal execute_branch_ticks_dual
  ins $4, $2, 0, 1                # if Thumb state, address |= 0x01 (delay)

  save_registers_0                # save registers 1/2
  jal execute_spsr_restore_body   # do the dirty work in this C function
  save_registers_1                # save registers 2/2 (delay)

  srl $5, $2, 31                  # bit 31: interrupt flag

  blez $17, 1f
  ext $4, $2, 0, 31               # move return value to $4

  bne $5, $0, 2f
  nop

  jal block_lookup_address_dual   # $2 = MIPS address to jump to
  nop

  restore_registers_0             # restore registers 1/2
  jr $2
  restore_registers_1             # restore registers 2/2 (delay)

1:
  sw $0, CHANGED_PC_STATUS($16)

  jal update_gba                  # process the next event
  sw $4, REG_PC($16)              # next PC = $4 (delay)

  lw $1, CHANGED_PC_STATUS($16)

  bne $1, $0, lookup_pc_changed
  addu $17, $2, $0                # $17 = new cycle count (delay slot)

  jal block_lookup_address_dual   # $2 = MIPS address to jump to
  lw $4, REG_PC($16)              # restore branch address (delay)

  restore_registers_0             # restore registers 1/2
  jr $2                           # jump to it
  restore_registers_1             # restore registers 2/2 (delay)

2:
  lw $1, EXECUTE_CYCLES($16)
  sw $4, REG_PC($16)              # next PC = $4
  sw $0, CHANGED_PC_STATUS($16)

  subu $1, $1, $17
  jal update_gba                  # process the next event
  sw $1, EXECUTE_CYCLES($16)

  lw $1, CHANGED_PC_STATUS($16)

  bne $1, $0, lookup_pc_changed
  addu $17, $2, $0                # $17 = new cycle count (delay slot)

  jal block_lookup_address_dual   # $2 = MIPS address to jump to
  lw $4, REG_PC($16)              # restore branch address (delay)

  restore_registers_0             # restore registers 1/2
  jr $2                           # jump to it
  restore_registers_1             # restore registers 2/2 (delay)

no_spsr_restore:
  jr $ra
  nop

# $4: new cpsr
# $5: store mask
# $6: current PC

execute_store_cpsr:
  lw $2, REG_CPSR($16)            # $2 = current cpsr
  sw $6, REG_PC($16)              # current PC = $6

  addiu $sp, $sp, -4
  sw $ra, ($sp)

  andi $1, $2, 0x0F
  beql $1, $0, 1f
  ins $5, $0, 0, 24               # user mode, only condition code bits (delay Likely)

1:
  and $1, $4, $5                  # $1 = new_cpsr & store_mask
  nor $4, $5, $0                  # $4 = ~store_mask
  and $2, $2, $4                  # $2 = (cpsr & (~store_mask))
  or $1, $1, $2                   # $1 = new cpsr combined with old

  extract_flags_body              # extract flags from $1

  ori $4, $1, 0x10                # fix mode bits
  sw $4, REG_CPSR($16)            # store the new CPSR

  save_registers_0                # save registers 1/2
  jal execute_store_cpsr_body     # do the dirty work in this C function
  save_registers_1                # save registers 2/2 (delay)

  beq $2, $0, 2f
  lw $1, EXECUTE_CYCLES($16)

  sw $0, CHANGED_PC_STATUS($16)

  subu $1, $1, $17
  jal update_gba                  # process the next event
  sw $1, EXECUTE_CYCLES($16)

  lw $1, CHANGED_PC_STATUS($16)
  addu $17, $2, $0                # $17 = new cycle count

  bnel $1, $0, lookup_pc_changed
  addiu $sp, $sp, 4               # fix stack (delay Likely)

2:
  lw $ra, ($sp)
  addiu $sp, $sp, 4

  restore_registers_0             # restore registers 1/2
  jr $ra
  restore_registers_1             # restore registers 2/2 (delay)


# $4: new spsr
# $5: store mask

execute_store_spsr:
  lw $1, CPU_MODE($16)            # $1 = cpu_mode

  lui $2, %hi(spsr)
  sll $1, $1, 2                   # adjust to word offset size
  addu $1, $2, $1
  lw $2, %lo(spsr)($1)            # $2 = spsr[cpu_mode]

  and $4, $4, $5                  # $4 = new_spsr & store_mask
  nor $5, $5, $0                  # $5 = ~store_mask
  and $2, $2, $5                  # $2 = (spsr & (~store_mask))
  or $4, $4, $2                   # $4 = new spsr combined with old

  jr $ra                          # return
  sw $4, %lo(spsr)($1)            # spsr[cpu_mode] = $4 (delay slot)


# $4: value
# $5: shift

execute_lsl_flags_reg:
  beq $5, $0, lsl_shift_zero      # is the shift zero?
  sltiu $1, $5, 32                # $1 = (shift < 32) (delay)
  beq $1, $0, lsl_shift_high      # is the shift >= 32?
  addiu $2, $0, 32                # $2 = 32 (delay)

  subu $2, $2, $5                 # $2 = (32 - shift)
  srlv $2, $4, $2                 # $2 = (value >> (32 - shift))
  andi $22, $2, 1                 # c flag = (value >> (32 - shift)) & 0x01

lsl_shift_zero:
  jr $ra                          # return
  sllv $4, $4, $5                 # return (value << shift) (delay)

lsl_shift_high:
  sltiu $1, $5, 33                # $1 = (shift < 33)
  andi $22, $4, 1                 # c flag = value & 0x01

  beql $1, $0, lsl_shift_done     # jump if shift != 32
  add $22, $0, $0                 # c flag = 0 otherwise (delay Likely)

lsl_shift_done:
  jr $ra                          # return
  add $4, $0, $0                  # value = 0 no matter what (delay)


execute_lsr_flags_reg:
  beq $5, $0, lsr_shift_zero      # is the shift zero?
  sltiu $1, $5, 32                # $1 = (shift < 32) (delay)
  beq $1, $0, lsr_shift_high      # is the shift >= 32?
  addiu $2, $5, -1                # $2 = shift - 1 (delay)

  srlv $2, $4, $2                 # $2 = (value >> (shift - 1))
  andi $22, $2, 1                 # c flag = (value >> (shift - 1)) & 0x01

lsr_shift_zero:
  jr $ra                          # return
  srlv $4, $4, $5                 # return (value >> shift) (delay)

lsr_shift_high:
  sltiu $1, $5, 33                # $1 = (shift < 33)
  ext $22, $4, 31, 1              # c flag = (value >> 31) & 0x01

  beql $1, $0, lsr_shift_done     # jump if shift != 32
  add $22, $0, $0                 # c flag = 0 otherwise (delay Likely)

lsr_shift_done:
  jr $ra                          # return
  add $4, $0, $0                  # value = 0 no matter what


execute_asr_flags_reg:
  beq $5, $0, asr_shift_zero      # is the shift zero?
  sltiu $1, $5, 32                # $1 = (shift < 32) (delay)
  beq $1, $0, asr_shift_high      # is the shift >= 32?
  addiu $2, $5, -1                # $2 = shift - 1 (delay)

  srlv $2, $4, $2                 # $2 = (value >> (shift - 1))
  andi $22, $2, 1                 # c flag = (value >> (shift - 1)) & 0x01

asr_shift_zero:
  jr $ra                          # return
  srav $4, $4, $5                 # return (value >> shift) (delay)

asr_shift_high:
  sra $4, $4, 31                  # value >>= 31
  jr $ra                          # return
  andi $22, $4, 1                 # c flag = value & 0x01


execute_ror_flags_reg:
  beq $5, $0, ror_zero_shift      # is the shift zero?
  sltiu $1, $5, 32                # $1 = (shift < 32) (delay)
  beq $1, $0, ror_shift_high      # is the shift >= 32?
  addiu $1, $5, -1                # $1 = (shift - 1) (delay)

  srav $1, $4, $1                 # $1 = (value >> (shift - 1))
  andi $22, $1, 1                 # c flag = $1 & 1

ror_zero_shift:
  jr $ra                          # return
  rotrv $4, $4, $5                # return (value ror shift) delay

ror_shift_high:
  jr $ra                          # return
  ext $22, $4, 31, 1              # c flag = (value >> 31) & 0x01


# $4: cycle counter argument

execute_arm_translate:
  lui $16, %hi(reg)               # load base register
  addiu $16, $16, %lo(reg)
  addu $17, $4, $0                # load cycle counter register
  extract_flags

  jal block_lookup_address_arm    # lookup initial jump address
  lw $4, REG_PC($16)              # load PC into $4 (delay)

  restore_registers_0             # restore registers 1/2
  jr $2                           # jump to return
  restore_registers_1             # restore registers 2/2 (delay)


# $4 = cpu_mode
# $5 = new_mode

execute_force_user_mode_prologue:
  lw $4, CPU_MODE($16)            # $4 = cpu_mode
  addu $5, $0, $0                 # $5 = new_mode (user mode)

  beq $4, $0, no_change_registers # only change if the cpu isn't usermode
  sw $ra, REG_SAVE($16)           # save the return address (delay)

  save_registers_0                # save registers 1/2
  jal force_user_mode_body        # do the dirty work in this C function
  save_registers_1                # save registers 2/2 (delay)

  lw $ra, REG_SAVE($16)           # restore return address
  restore_registers_0             # restore registers 1/2
  jr $ra
  restore_registers_1             # restore registers 2/2 (delay)

execute_force_user_mode_epilogue:
  lw $5, CPU_MODE($16)            # $5 = new_mode
  addu $4, $0, $0                 # $4 = cpu_mode (user mode)

  beq $5, $0, no_change_registers # only change if the cpu isn't usermode
  sw $ra, REG_SAVE($16)           # save the return address (delay)

  save_registers_0                # save registers 1/2
  jal force_user_mode_body        # do the dirty work in this C function
  save_registers_1                # save registers 2/2 (delay)

  lw $ra, REG_SAVE($16)           # restore return address
  restore_registers_0             # restore registers 1/2
  jr $ra
  restore_registers_1             # restore registers 2/2 (delay)

no_change_registers:
  jr $ra
  nop


.macro count_branch_ticks
  lui $2, %hi(memory_waitstate_s)
  addu $2, $2, $1
  lbu $2, %lo(memory_waitstate_s)($2)
  addiu $17, $17, -2              # CPU cycles
  sll $2, $2, 1
  subu $17, $17, $2               # waitstate 2S

  lui $2, %hi(memory_waitstate_n)
  addu $2, $2, $1
  lbu $2, %lo(memory_waitstate_n)($2)
  jr $ra
  subu $17, $17, $2               # waitstate 1N (delay)
.endm

# $4 = branch address

execute_branch_ticks_arm:
  ext $1, $4, 24, 4               # $1 = (address >> 24) & 0x0F
  addiu $1, $1, 0x10              # ARM state offset +16
  count_branch_ticks

execute_branch_ticks_thumb:
  ext $1, $4, 24, 4               # $1 = (address >> 24) & 0x0F
  count_branch_ticks

execute_branch_ticks_dual:
  ext $1, $4, 24, 4               # $1 = (address >> 24) & 0x0F
  andi $2, $4, 0x01               # address & 0x01
  beql $2, $0, 1f
  addiu $1, $1, 0x10              # if ARM state offset +16 (delay Likely)
1:
  count_branch_ticks


execute_multiply_ticks:
  sra $1, $4, 31
  xor $4, $4, $1                  # if (value < 0) value = ~value;

  ext $1, $4, 8, 24
  beql $1, $0, 1f                 # (value & 0xFFFFFF00) == 0
  addiu $2, $0, 1                 # Interlock = 1 (delay Likely)

  ext $1, $4, 16, 16
  beql $1, $0, 1f                 # (value & 0xFFFF0000) == 0
  addiu $2, $0, 2                 # Interlock = 2 (delay Likely)

  ext $1, $4, 24, 8
  beql $1, $0, 1f                 # (value & 0xFF000000) == 0
  addiu $2, $0, 3                 # Interlock = 3 (delay Likely)

  addiu $2, $0, 4                 # other, Interlock = 4

1:
  jr $ra
  subu $17, $17, $2


# sceKernelInvalidateIcacheRange gives me problems, trying this instead
# Invalidates an n byte region starting at the start address
# $4: start location
# $5: length

invalidate_icache_region:
  addiu $2, $5, 63                # align up to 64 bytes
  srl $2, $2, 6                   # divide by 64
  beq $2, $0, done                # exit early on 0
  ins $4, $0, 0, 6                # align to 64 bytes (delay)

iir_loop:
  cache 0x08, ($4)                # hit invalidate icache line
  addiu $2, $2, -1                # next loop iteration
  bne $2, $0, iir_loop            # loop
  addiu $4, $4, 64                # go to next cache line (delay slot)

done:
  jr $ra                          # return
  nop

# Writes back dcache and invalidates icache.

invalidate_all_cache:
  addu $4, $0, $0                 # $4 = 0
  addiu $5, $0, 0x2000            # $5 = 0x2000 (64 * 256 / 2) line_size * blocks / lines

iac_loop:
  cache 0x14, 0($4)               # index writeback invalidate dcache line 0
  cache 0x14, 0x2000($4)          # index writeback invalidate dcache line 1

  addiu $4, $4, 0x40              # goto next cache line

  cache 0x04, -0x40($4)           # index invalidate icache line 0
  bne $4, $5, iac_loop            # next iteration
  cache 0x04, (0x2000 - 0x40)($4) # index invalidate icache line 1 (delay)

  jr $ra                          # return
  nop

memory_map_read:
  .space 0x8000

reg:
  .space 0x100

memory_map_write:
  .space 0x8000

