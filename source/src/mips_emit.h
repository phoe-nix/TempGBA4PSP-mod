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

#ifndef MIPS_EMIT_H
#define MIPS_EMIT_H


u32 mips_update_gba(u32 pc);

// Although these are defined as a function, don't call them as
// such (jump to it instead)
void mips_indirect_branch_arm(u32 address);
void mips_indirect_branch_thumb(u32 address);
void mips_indirect_branch_dual(u32 address);

void mips_indirect_branch_arm_no_update_gba(u32 address);
void mips_indirect_branch_thumb_no_update_gba(u32 address);
void mips_indirect_branch_dual_no_update_gba(u32 address);

u32 execute_read_cpsr(void);
u32 execute_read_spsr(void);
void execute_swi(u32 pc);

u32 execute_spsr_restore(u32 address);
void execute_store_cpsr(u32 new_cpsr, u32 store_mask);
void execute_store_spsr(u32 new_spsr, u32 store_mask);

u32 execute_spsr_restore_body(u32 address);
u32 execute_store_cpsr_body(u32 _cpsr, u32 store_mask);

u32 execute_lsl_flags_reg(u32 value, u32 shift);
u32 execute_lsr_flags_reg(u32 value, u32 shift);
u32 execute_asr_flags_reg(u32 value, u32 shift);
u32 execute_ror_flags_reg(u32 value, u32 shift);

void execute_aligned_store32(u32 address, u32 value);
u32 execute_aligned_load32(u32 address);

void execute_branch_ticks_arm(u32 branch_address);
void execute_branch_ticks_thumb(u32 branch_address);
void execute_branch_ticks_dual(u32 branch_address);

void execute_multiply_ticks(u32 value);

void execute_force_user_mode_prologue(void);
void execute_force_user_mode_epilogue(void);
void force_user_mode_body(CPU_MODE_TYPE cpu_mode, CPU_MODE_TYPE new_mode);


typedef enum
{
  mips_reg_zero, // 0
  mips_reg_at,   // 1
  mips_reg_v0,   // 2
  mips_reg_v1,   // 3
  mips_reg_a0,   // 4
  mips_reg_a1,   // 5
  mips_reg_a2,   // 6
  mips_reg_a3,   // 7
  mips_reg_t0,   // 8
  mips_reg_t1,   // 9
  mips_reg_t2,   // 10
  mips_reg_t3,   // 11
  mips_reg_t4,   // 12
  mips_reg_t5,   // 13
  mips_reg_t6,   // 14
  mips_reg_t7,   // 15
  mips_reg_s0,   // 16
  mips_reg_s1,   // 17
  mips_reg_s2,   // 18
  mips_reg_s3,   // 19
  mips_reg_s4,   // 20
  mips_reg_s5,   // 21
  mips_reg_s6,   // 22
  mips_reg_s7,   // 23
  mips_reg_t8,   // 24
  mips_reg_t9,   // 25
  mips_reg_k0,   // 26
  mips_reg_k1,   // 27
  mips_reg_gp,   // 28
  mips_reg_sp,   // 29
  mips_reg_fp,   // 30
  mips_reg_ra    // 31
} MIPS_REG_NUMBER;

typedef enum
{
  mips_special_sll       = 0x00,
  mips_special_srl       = 0x02,
  mips_special_sra       = 0x03,
  mips_special_sllv      = 0x04,
  mips_special_srlv      = 0x06,
  mips_special_srav      = 0x07,
  mips_special_jr        = 0x08,
  mips_special_jalr      = 0x09,
  mips_special_movz      = 0x0A,
  mips_special_movn      = 0x0B,
  mips_special_mfhi      = 0x10,
  mips_special_mthi      = 0x11,
  mips_special_mflo      = 0x12,
  mips_special_mtlo      = 0x13,
  mips_special_mult      = 0x18,
  mips_special_multu     = 0x19,
  mips_special_div       = 0x1A,
  mips_special_divu      = 0x1B,
  mips_special_madd      = 0x1C,
  mips_special_maddu     = 0x1D,
  mips_special_add       = 0x20,
  mips_special_addu      = 0x21,
  mips_special_sub       = 0x22,
  mips_special_subu      = 0x23,
  mips_special_and       = 0x24,
  mips_special_or        = 0x25,
  mips_special_xor       = 0x26,
  mips_special_nor       = 0x27,
  mips_special_slt       = 0x2A,
  mips_special_sltu      = 0x2B
} MIPS_FUNCTION_SPECIAL;

typedef enum
{
  mips_special3_ext      = 0x00,
  mips_special3_ins      = 0x04,
  mips_special3_bshfl    = 0x20
} MIPS_FUNCTION_SPECIAL3;

typedef enum
{
  mips_regimm_bltz       = 0x00,
  mips_regimm_bltzal     = 0x10
} MIPS_FUNCTION_REGIMM;

typedef enum
{
  mips_opcode_special    = 0x00,
  mips_opcode_regimm     = 0x01,
  mips_opcode_j          = 0x02,
  mips_opcode_jal        = 0x03,
  mips_opcode_beq        = 0x04,
  mips_opcode_bne        = 0x05,
  mips_opcode_blez       = 0x06,
  mips_opcode_bgtz       = 0x07,
  mips_opcode_addi       = 0x08,
  mips_opcode_addiu      = 0x09,
  mips_opcode_slti       = 0x0A,
  mips_opcode_sltiu      = 0x0B,
  mips_opcode_andi       = 0x0C,
  mips_opcode_ori        = 0x0D,
  mips_opcode_xori       = 0x0E,
  mips_opcode_lui        = 0x0F,
  mips_opcode_llo        = 0x18,
  mips_opcode_lhi        = 0x19,
  mips_opcode_trap       = 0x1A,
  mips_opcode_special2   = 0x1C,
  mips_opcode_special3   = 0x1F,
  mips_opcode_lb         = 0x20,
  mips_opcode_lh         = 0x21,
  mips_opcode_lw         = 0x23,
  mips_opcode_lbu        = 0x24,
  mips_opcode_lhu        = 0x25,
  mips_opcode_sb         = 0x28,
  mips_opcode_sh         = 0x29,
  mips_opcode_sw         = 0x2B,
} MIPS_OPCODE;

#define mips_emit_reg(opcode, rs, rt, rd, shift, function)                    \
  *((u32 *)translation_ptr) = (mips_opcode_##opcode << 26) | ((rs) << 21) | ((rt) << 16) | ((rd) << 11) | ((shift) << 6) | function; \
  translation_ptr += 4;                                                       \

#define mips_emit_special(function, rs, rt, rd, shift)                        \
  *((u32 *)translation_ptr) = (mips_opcode_special  << 26) | ((rs) << 21) | ((rt) << 16) | ((rd) << 11) | ((shift) << 6) | mips_special_##function; \
  translation_ptr += 4;                                                       \

#define mips_emit_special3(function, rs, rt, imm_a, imm_b)                    \
  *((u32 *)translation_ptr) = (mips_opcode_special3 << 26) | ((rs) << 21) | ((rt) << 16) | ((imm_a) << 11) | ((imm_b) << 6) | mips_special3_##function; \
  translation_ptr += 4;                                                       \

#define mips_emit_imm(opcode, rs, rt, immediate)                              \
  *((u32 *)translation_ptr) = (mips_opcode_##opcode << 26) | ((rs) << 21) | ((rt) << 16) | ((immediate) & 0xFFFF); \
  translation_ptr += 4;                                                       \

#define mips_emit_regimm(function, rs, immediate)                             \
  *((u32 *)translation_ptr) = (mips_opcode_regimm   << 26) | ((rs) << 21) | (mips_regimm_##function << 16) | ((immediate) & 0xFFFF); \
  translation_ptr += 4;                                                       \

#define mips_emit_jump(opcode, offset)                                        \
  *((u32 *)translation_ptr) = (mips_opcode_##opcode << 26) | ((offset) & 0x3FFFFFF); \
  translation_ptr += 4;                                                       \

#define mips_relative_offset(source, offset)                                  \
  (((u32)(offset) - ((u32)(source) + 4)) >> 2)                                \

#define mips_absolute_offset(offset)                                          \
  ((u32)(offset) >> 2)                                                        \

// ADDU rd, rs, rt
#define mips_emit_addu(rd, rs, rt)                                            \
  mips_emit_special(addu, rs, rt, rd, 0)                                      \

#define mips_emit_subu(rd, rs, rt)                                            \
  mips_emit_special(subu, rs, rt, rd, 0)                                      \

#define mips_emit_xor(rd, rs, rt)                                             \
  mips_emit_special(xor, rs, rt, rd, 0)                                       \

#define mips_emit_add(rd, rs, rt)                                             \
  mips_emit_special(add, rs, rt, rd, 0)                                       \

#define mips_emit_sub(rd, rs, rt)                                             \
  mips_emit_special(sub, rs, rt, rd, 0)                                       \

#define mips_emit_and(rd, rs, rt)                                             \
  mips_emit_special(and, rs, rt, rd, 0)                                       \

#define mips_emit_or(rd, rs, rt)                                              \
  mips_emit_special(or, rs, rt, rd, 0)                                        \

#define mips_emit_nor(rd, rs, rt)                                             \
  mips_emit_special(nor, rs, rt, rd, 0)                                       \

#define mips_emit_slt(rd, rs, rt)                                             \
  mips_emit_special(slt, rs, rt, rd, 0)                                       \

#define mips_emit_sltu(rd, rs, rt)                                            \
  mips_emit_special(sltu, rs, rt, rd, 0)                                      \

#define mips_emit_sllv(rd, rt, rs)                                            \
  mips_emit_special(sllv, rs, rt, rd, 0)                                      \

#define mips_emit_srlv(rd, rt, rs)                                            \
  mips_emit_special(srlv, rs, rt, rd, 0)                                      \

#define mips_emit_srav(rd, rt, rs)                                            \
  mips_emit_special(srav, rs, rt, rd, 0)                                      \

#define mips_emit_rotrv(rd, rt, rs)                                           \
  mips_emit_special(srlv, rs, rt, rd, 1)                                      \

#define mips_emit_sll(rd, rt, shift)                                          \
  mips_emit_special(sll, 0, rt, rd, shift)                                    \

#define mips_emit_srl(rd, rt, shift)                                          \
  mips_emit_special(srl, 0, rt, rd, shift)                                    \

#define mips_emit_sra(rd, rt, shift)                                          \
  mips_emit_special(sra, 0, rt, rd, shift)                                    \

#define mips_emit_rotr(rd, rt, shift)                                         \
  mips_emit_special(srl, 1, rt, rd, shift)                                    \

#define mips_emit_mfhi(rd)                                                    \
  mips_emit_special(mfhi, 0, 0, rd, 0)                                        \

#define mips_emit_mflo(rd)                                                    \
  mips_emit_special(mflo, 0, 0, rd, 0)                                        \

#define mips_emit_mthi(rs)                                                    \
  mips_emit_special(mthi, rs, 0, 0, 0)                                        \

#define mips_emit_mtlo(rs)                                                    \
  mips_emit_special(mtlo, rs, 0, 0, 0)                                        \

#define mips_emit_mult(rs, rt)                                                \
  mips_emit_special(mult, rs, rt, 0, 0)                                       \

#define mips_emit_multu(rs, rt)                                               \
  mips_emit_special(multu, rs, rt, 0, 0)                                      \

#define mips_emit_div(rs, rt)                                                 \
  mips_emit_special(div, rs, rt, 0, 0)                                        \

#define mips_emit_divu(rs, rt)                                                \
  mips_emit_special(divu, rs, rt, 0, 0)                                       \

#define mips_emit_madd(rs, rt)                                                \
  mips_emit_special(madd, rs, rt, 0, 0)                                       \

#define mips_emit_maddu(rs, rt)                                               \
  mips_emit_special(maddu, rs, rt, 0, 0)                                      \

#define mips_emit_movn(rd, rs, rt)                                            \
  mips_emit_special(movn, rs, rt, rd, 0)                                      \

#define mips_emit_movz(rd, rs, rt)                                            \
  mips_emit_special(movz, rs, rt, rd, 0)                                      \

#define mips_emit_lb(rt, rs, offset)                                          \
  mips_emit_imm(lb, rs, rt, offset)                                           \

#define mips_emit_lbu(rt, rs, offset)                                         \
  mips_emit_imm(lbu, rs, rt, offset)                                          \

#define mips_emit_lh(rt, rs, offset)                                          \
  mips_emit_imm(lh, rs, rt, offset)                                           \

#define mips_emit_lhu(rt, rs, offset)                                         \
  mips_emit_imm(lhu, rs, rt, offset)                                          \

#define mips_emit_lw(rt, rs, offset)                                          \
  mips_emit_imm(lw, rs, rt, offset)                                           \

#define mips_emit_sb(rt, rs, offset)                                          \
  mips_emit_imm(sb, rs, rt, offset)                                           \

#define mips_emit_sh(rt, rs, offset)                                          \
  mips_emit_imm(sh, rs, rt, offset)                                           \

#define mips_emit_sw(rt, rs, offset)                                          \
  mips_emit_imm(sw, rs, rt, offset)                                           \

#define mips_emit_lui(rt, imm)                                                \
  mips_emit_imm(lui, 0, rt, imm)                                              \

#define mips_emit_addiu(rt, rs, imm)                                          \
  mips_emit_imm(addiu, rs, rt, imm)                                           \

#define mips_emit_xori(rt, rs, imm)                                           \
  mips_emit_imm(xori, rs, rt, imm)                                            \

#define mips_emit_ori(rt, rs, imm)                                            \
  mips_emit_imm(ori, rs, rt, imm)                                             \

#define mips_emit_andi(rt, rs, imm)                                           \
  mips_emit_imm(andi, rs, rt, imm)                                            \

#define mips_emit_slti(rt, rs, imm)                                           \
  mips_emit_imm(slti, rs, rt, imm)                                            \

#define mips_emit_sltiu(rt, rs, imm)                                          \
  mips_emit_imm(sltiu, rs, rt, imm)                                           \

#define mips_emit_ext(rt, rs, pos, size)                                      \
  mips_emit_special3(ext, rs, rt, (size) - 1, pos)                            \

#define mips_emit_ins(rt, rs, pos, size)                                      \
  mips_emit_special3(ins, rs, rt, (pos) + (size) - 1, pos)                    \

// Breaks down if the backpatch offset is greater than 16bits, take care
// when using (should be okay if limited to conditional instructions)

#define mips_emit_b_filler(type, rs, rt, writeback_location)                  \
  (writeback_location) = translation_ptr;                                     \
  mips_emit_imm(type, rs, rt, 0);                                             \

// The backpatch code for this has to be handled differently than the above

#define mips_emit_j_filler(writeback_location)                                \
  (writeback_location) = translation_ptr;                                     \
  mips_emit_jump(j, 0);                                                       \

#define mips_emit_b(type, rs, rt, offset)                                     \
  mips_emit_imm(type, rs, rt, offset)                                         \

#define mips_emit_j(offset)                                                   \
  mips_emit_jump(j, offset)                                                   \

#define mips_emit_jal(offset)                                                 \
  mips_emit_jump(jal, offset)                                                 \

#define mips_emit_jr(rs)                                                      \
  mips_emit_special(jr, rs, 0, 0, 0)                                          \

#define mips_emit_bltzal(rs, offset)                                          \
  mips_emit_regimm(bltzal, rs, offset)                                        \

#define mips_emit_nop()                                                       \
  mips_emit_sll(reg_zero, reg_zero, 0)                                        \

#define reg_base    mips_reg_s0
#define reg_cycles  mips_reg_s1
#define reg_a0      mips_reg_a0
#define reg_a1      mips_reg_a1
#define reg_a2      mips_reg_a2
#define reg_rv      mips_reg_v0
#define reg_pc      mips_reg_s3
#define reg_temp    mips_reg_at
#define reg_zero    mips_reg_zero

#define reg_n_cache mips_reg_s4
#define reg_z_cache mips_reg_s5
#define reg_c_cache mips_reg_s6
#define reg_v_cache mips_reg_s7

#define reg_r0      mips_reg_v1
#define reg_r1      mips_reg_a3
#define reg_r2      mips_reg_t0
#define reg_r3      mips_reg_t1
#define reg_r4      mips_reg_t2
#define reg_r5      mips_reg_t3
#define reg_r6      mips_reg_t4
#define reg_r7      mips_reg_t5
#define reg_r8      mips_reg_t6
#define reg_r9      mips_reg_t7
#define reg_r10     mips_reg_s2
#define reg_r11     mips_reg_t8
#define reg_r12     mips_reg_t9
#define reg_r13     mips_reg_gp
#define reg_r14     mips_reg_fp

// Writing to r15 goes straight to a0, to be chained with other ops

const u8 arm_to_mips_reg[] =
{
  reg_r0,
  reg_r1,
  reg_r2,
  reg_r3,
  reg_r4,
  reg_r5,
  reg_r6,
  reg_r7,
  reg_r8,
  reg_r9,
  reg_r10,
  reg_r11,
  reg_r12,
  reg_r13,
  reg_r14,
  reg_a0,
  reg_a1,
  reg_a2,
  reg_temp
};

#define arm_reg_a0   15
#define arm_reg_a1   16
#define arm_reg_a2   17
#define arm_reg_temp 18

#define generate_load_reg(ireg, reg_index)                                    \
  mips_emit_addu(ireg, arm_to_mips_reg[reg_index], reg_zero)                  \

#define generate_load_imm(ireg, imm)                                          \
  if (((s32)imm >= -32768) && ((s32)imm <= 32767))                            \
  {                                                                           \
    mips_emit_addiu(ireg, reg_zero, imm);                                     \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    if (((u32)imm >> 16) == 0x0000)                                           \
    {                                                                         \
      mips_emit_ori(ireg, reg_zero, imm);                                     \
    }                                                                         \
    else                                                                      \
    {                                                                         \
      mips_emit_lui(ireg, imm >> 16);                                         \
                                                                              \
      if (((u32)imm & 0x0000FFFF) != 0x00000000)                              \
      {                                                                       \
        mips_emit_ori(ireg, ireg, imm & 0xFFFF);                              \
      }                                                                       \
    }                                                                         \
  }                                                                           \

#define generate_load_pc(ireg, new_pc)                                        \
{                                                                             \
  s32 pc_delta = new_pc - stored_pc;                                          \
  if ((pc_delta >= -32768) && (pc_delta <= 32767))                            \
  {                                                                           \
    mips_emit_addiu(ireg, reg_pc, pc_delta);                                  \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_imm(ireg, new_pc);                                          \
  }                                                                           \
}                                                                             \

#define generate_store_reg(ireg, reg_index)                                   \
  mips_emit_addu(arm_to_mips_reg[reg_index], ireg, reg_zero)                  \

#define generate_shift_left(ireg, imm)                                        \
  mips_emit_sll(ireg, ireg, imm)                                              \

#define generate_shift_right(ireg, imm)                                       \
  mips_emit_srl(ireg, ireg, imm)                                              \

#define generate_shift_right_arithmetic(ireg, imm)                            \
  mips_emit_sra(ireg, ireg, imm)                                              \

#define generate_rotate_right(ireg, imm)                                      \
  mips_emit_rotr(ireg, ireg, imm)                                             \

#define generate_add(ireg_dest, ireg_src)                                     \
  mips_emit_addu(ireg_dest, ireg_dest, ireg_src)                              \

#define generate_sub(ireg_dest, ireg_src)                                     \
  mips_emit_subu(ireg_dest, ireg_dest, ireg_src)                              \

#define generate_or(ireg_dest, ireg_src)                                      \
  mips_emit_or(ireg_dest, ireg_dest, ireg_src)                                \

#define generate_xor(ireg_dest, ireg_src)                                     \
  mips_emit_xor(ireg_dest, ireg_dest, ireg_src)                               \

#define generate_alu_imm(imm_type, reg_type, ireg_dest, ireg_src, imm)        \
  if (((s32)imm >= -32768) && ((s32)imm <= 32767))                            \
  {                                                                           \
    mips_emit_##imm_type(ireg_dest, ireg_src, imm);                           \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_imm(reg_temp, imm);                                         \
    mips_emit_##reg_type(ireg_dest, ireg_src, reg_temp);                      \
  }                                                                           \

#define generate_alu_immu(imm_type, reg_type, ireg_dest, ireg_src, imm)       \
  if (((u32)imm >= 0) && ((u32)imm <= 65535))                                 \
  {                                                                           \
    mips_emit_##imm_type(ireg_dest, ireg_src, imm);                           \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_imm(reg_temp, imm);                                         \
    mips_emit_##reg_type(ireg_dest, ireg_src, reg_temp);                      \
  }                                                                           \

#define generate_add_imm(ireg, imm)                                           \
  generate_alu_imm(addiu, add, ireg, ireg, imm)                               \

#define generate_sub_imm(ireg, imm)                                           \
  generate_alu_imm(addiu, add, ireg, ireg, -imm)                              \

#define generate_xor_imm(ireg, imm)                                           \
  generate_alu_immu(xori, xor, ireg, ireg, imm)                               \

#define generate_add_reg_reg_imm(ireg_dest, ireg_src, imm)                    \
  generate_alu_imm(addiu, add, ireg_dest, ireg_src, imm)                      \

#define generate_and_imm(ireg, imm)                                           \
  generate_alu_immu(andi, and, ireg, ireg, imm)                               \

#define generate_mov(ireg_dest, ireg_src)                                     \
  mips_emit_addu(ireg_dest, ireg_src, reg_zero)                               \

#define generate_multiply_s64()                                               \
  mips_emit_mult(arm_to_mips_reg[rm], arm_to_mips_reg[rs])                    \

#define generate_multiply_u64()                                               \
  mips_emit_multu(arm_to_mips_reg[rm], arm_to_mips_reg[rs])                   \

#define generate_multiply_s64_add()                                           \
  mips_emit_madd(arm_to_mips_reg[rm], arm_to_mips_reg[rs])                    \

#define generate_multiply_u64_add()                                           \
  mips_emit_maddu(arm_to_mips_reg[rm], arm_to_mips_reg[rs])                   \

#define generate_function_call(function_location)                             \
  mips_emit_jal(mips_absolute_offset(function_location));                     \
  mips_emit_nop();                                                            \

#define generate_function_call_swap_delay(function_location)                  \
{                                                                             \
  u32 delay_instruction = ADDRESS32(translation_ptr, -4);                     \
  translation_ptr -= 4;                                                       \
  mips_emit_jal(mips_absolute_offset(function_location));                     \
  ADDRESS32(translation_ptr, 0) = delay_instruction;                          \
  translation_ptr += 4;                                                       \
}                                                                             \

#define generate_swap_delay()                                                 \
{                                                                             \
  u32 delay_instruction = ADDRESS32(translation_ptr, -8);                     \
  u32 branch_instruction = ADDRESS32(translation_ptr, -4);                    \
  branch_instruction = (branch_instruction & 0xFFFF0000) | (((branch_instruction & 0x0000FFFF) + 1) & 0x0000FFFF); \
  ADDRESS32(translation_ptr, -8) = branch_instruction;                        \
  ADDRESS32(translation_ptr, -4) = delay_instruction;                         \
}                                                                             \

#define generate_cycle_update()                                               \
  if (cycle_count != 0)                                                       \
  {                                                                           \
    mips_emit_addiu(reg_cycles, reg_cycles, -cycle_count);                    \
    cycle_count = 0;                                                          \
  }                                                                           \

#define generate_cycle_update_force()                                         \
  mips_emit_addiu(reg_cycles, reg_cycles, -cycle_count);                      \
  cycle_count = 0;                                                            \

#define check_cycle_counter(_pc)                                              \
  generate_cycle_update_force();                                              \
  mips_emit_b(bgtz, reg_cycles, reg_zero, 3);                                 \
  mips_emit_lui(reg_temp, (_pc) >> 16);                                       \
  mips_emit_jal(mips_absolute_offset(mips_update_gba));                       \
  mips_emit_ori(reg_a0, reg_temp, (_pc) & 0xFFFF);                            \

#define generate_branch_patch_conditional(dest, offset)                       \
  *((u16 *)(dest)) = mips_relative_offset(dest, offset)                       \

#define generate_branch_patch_unconditional(dest, offset)                     \
  *((u32 *)(dest)) = (mips_opcode_j << 26) | ((mips_absolute_offset(offset)) & 0x3FFFFFF) \

#define generate_branch_no_cycle_update(type, writeback_location, new_pc)     \
{                                                                             \
  s32 i;                                                                      \
  u32 flag = 0;                                                               \
                                                                              \
  for (i = 0; i < idle_loop_targets; i++)                                     \
  {                                                                           \
    if (pc == idle_loop_target_pc[i])                                         \
    {                                                                         \
      flag = 1;                                                               \
      break;                                                                  \
    }                                                                         \
                                                                              \
    if (idle_loop_target_pc[i] == 0xFFFFFFFF)                                 \
      break;                                                                  \
  }                                                                           \
                                                                              \
  generate_load_pc(reg_a0, new_pc);                                           \
                                                                              \
  if (flag != 0)                                                              \
  {                                                                           \
    generate_function_call_swap_delay(mips_update_gba);                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_bltzal(reg_cycles, mips_relative_offset(translation_ptr, update_trampoline)); \
    generate_swap_delay();                                                    \
  }                                                                           \
                                                                              \
  /* This uses variables from cpu_asm.c's translate_block_builder /           \
   * translate_block_arm / translate_block_thumb functions. Basically,        \
   * if we're emitting a jump from a read-only area (BIOS or ROM) and         \
   * the branch target is in a read-only area (BIOS or ROM), we can link      \
   * statically and backpatch all we like, but if we're emitting a branch     \
   * towards a basic block that's in writable (GBA) memory, that block is     \
   * OFF LIMITS and that branch must be issued indirectly and resolved at     \
   * branch time. This allows us to efficiently clear SOME of the RAM         \
   * code cache after SOME of it has been modified. Ideally, that's one       \
   * basic block. */                                                          \
  if ((new_pc >= block_start_pc && new_pc < block_end_pc)                     \
   || (new_pc <  0x00004000) /* BIOS */                                       \
   || (new_pc >= 0x08000000 && new_pc < 0x0E000000) /* Game Pak ROM */)       \
  {                                                                           \
    mips_emit_j_filler(writeback_location);                                   \
    mips_emit_nop();                                                          \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_pc(reg_a0, new_pc);                                         \
    generate_function_call_swap_delay(mips_indirect_branch_##type##_no_update_gba); \
  }                                                                           \
}                                                                             \

#define generate_branch_cycle_update(type, writeback_location, new_pc)        \
  generate_cycle_update();                                                    \
  generate_branch_no_cycle_update(type, writeback_location, new_pc);          \

#define generate_conditional_branch(ireg_a, ireg_b, type, writeback_location) \
  generate_branch_filler_##type(ireg_a, ireg_b, writeback_location)           \

// a0 holds the destination

#define generate_indirect_branch_no_cycle_update(type)                        \
  mips_emit_j(mips_absolute_offset(mips_indirect_branch_##type));             \
  mips_emit_nop();                                                            \

#define generate_indirect_branch_cycle_update(type)                           \
  mips_emit_j(mips_absolute_offset(mips_indirect_branch_##type));             \
  generate_cycle_update_force();                                              \

#define generate_block_prologue()                                             \
  update_trampoline = translation_ptr;                                        \
                                                                              \
  mips_emit_j(mips_absolute_offset(mips_update_gba));                         \
  mips_emit_nop();                                                            \
  generate_load_imm(reg_pc, stored_pc);                                       \

#define translate_invalidate_dcache()                                         \
  sceKernelDcacheWritebackAll()                                               \

#define block_prologue_size 8
#define CODE_ALIGN_SIZE 4

#define check_generate_n_flag                                                 \
  (flag_status & 0x08)                                                        \

#define check_generate_z_flag                                                 \
  (flag_status & 0x04)                                                        \

#define check_generate_c_flag                                                 \
  (flag_status & 0x02)                                                        \

#define check_generate_v_flag                                                 \
  (flag_status & 0x01)                                                        \

#define generate_load_reg_pc(ireg, reg_index, pc_offset)                      \
  if (reg_index == REG_PC)                                                    \
  {                                                                           \
    generate_load_pc(ireg, (pc + pc_offset));                                 \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_load_reg(ireg, reg_index);                                       \
  }                                                                           \

#define check_load_reg_pc(arm_reg, reg_index, pc_offset)                      \
  if (reg_index == REG_PC)                                                    \
  {                                                                           \
    reg_index = arm_reg;                                                      \
    generate_load_pc(arm_to_mips_reg[arm_reg], (pc + pc_offset));             \
  }                                                                           \

#define check_store_reg_pc_no_flags(reg_index)                                \
  if (reg_index == REG_PC)                                                    \
  {                                                                           \
    generate_indirect_branch_cycle_update(arm);                               \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    cycle_count += pFETCH_WS32S(pc >> 24);                                    \
  }                                                                           \

#define check_store_reg_pc_flags(reg_index)                                   \
  if (reg_index == REG_PC)                                                    \
  {                                                                           \
    mips_emit_jal(mips_absolute_offset(execute_spsr_restore));                \
    generate_cycle_update_force();                                            \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    cycle_count += pFETCH_WS32S(pc >> 24);                                    \
  }                                                                           \

#define generate_shift_imm_lsl_no_flags(arm_reg, _rm, _shift)                 \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if (_shift != 0)                                                            \
  {                                                                           \
    mips_emit_sll(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
    _rm = arm_reg;                                                            \
  }                                                                           \

#define generate_shift_imm_lsr_no_flags(arm_reg, _rm, _shift)                 \
  if (_shift != 0)                                                            \
  {                                                                           \
    check_load_reg_pc(arm_reg, _rm, 8);                                       \
    mips_emit_srl(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_addu(arm_to_mips_reg[arm_reg], reg_zero, reg_zero);             \
  }                                                                           \
  _rm = arm_reg;                                                              \

#define generate_shift_imm_asr_no_flags(arm_reg, _rm, _shift)                 \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if (_shift != 0)                                                            \
  {                                                                           \
    mips_emit_sra(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_sra(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], 31);        \
  }                                                                           \
  _rm = arm_reg;                                                              \

#define generate_shift_imm_ror_no_flags(arm_reg, _rm, _shift)                 \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if (_shift != 0)                                                            \
  {                                                                           \
    mips_emit_rotr(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_srl(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], 1);         \
    mips_emit_ins(arm_to_mips_reg[arm_reg], reg_c_cache, 31, 1);              \
  }                                                                           \
  _rm = arm_reg;                                                              \

#define generate_shift_imm_lsl_flags(arm_reg, _rm, _shift)                    \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if (_shift != 0)                                                            \
  {                                                                           \
    mips_emit_ext(reg_c_cache, arm_to_mips_reg[_rm], (32 - _shift), 1);       \
    mips_emit_sll(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
    _rm = arm_reg;                                                            \
  }                                                                           \

#define generate_shift_imm_lsr_flags(arm_reg, _rm, _shift)                    \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if (_shift != 0)                                                            \
  {                                                                           \
    mips_emit_ext(reg_c_cache, arm_to_mips_reg[_rm], (_shift - 1), 1);        \
    mips_emit_srl(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_ext(reg_c_cache, arm_to_mips_reg[_rm], 31, 1);                  \
    mips_emit_addu(arm_to_mips_reg[arm_reg], reg_zero, reg_zero);             \
  }                                                                           \
  _rm = arm_reg;                                                              \

#define generate_shift_imm_asr_flags(arm_reg, _rm, _shift)                    \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if (_shift != 0)                                                            \
  {                                                                           \
    mips_emit_ext(reg_c_cache, arm_to_mips_reg[_rm], (_shift - 1), 1);        \
    mips_emit_sra(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);    \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_sra(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], 31);        \
    mips_emit_andi(reg_c_cache, arm_to_mips_reg[arm_reg], 1);                 \
  }                                                                           \
  _rm = arm_reg;                                                              \

#define generate_shift_imm_ror_flags(arm_reg, _rm, _shift)                    \
  check_load_reg_pc(arm_reg, _rm, 8);                                         \
  if (_shift != 0)                                                            \
  {                                                                           \
    mips_emit_ext(reg_c_cache, arm_to_mips_reg[_rm], (_shift - 1), 1);        \
    mips_emit_rotr(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], _shift);   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_andi(reg_temp, arm_to_mips_reg[_rm], 1);                        \
    mips_emit_srl(arm_to_mips_reg[arm_reg], arm_to_mips_reg[_rm], 1);         \
    mips_emit_ins(arm_to_mips_reg[arm_reg], reg_c_cache, 31, 1);              \
    mips_emit_addu(reg_c_cache, reg_temp, reg_zero);                          \
  }                                                                           \
  _rm = arm_reg;                                                              \

#define generate_load_reg_lower_byte(ireg, reg_index)                         \
  mips_emit_andi(ireg, arm_to_mips_reg[reg_index], 0xFF)                      \

#define generate_shift_reg_lsl_no_flags(_rm, _rs)                             \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  generate_load_reg_lower_byte(reg_a1, _rs);                                  \
  mips_emit_sltiu(reg_temp, reg_a1, 32);                                      \
  mips_emit_sllv(reg_a0, reg_a0, reg_a1);                                     \
  mips_emit_movz(reg_a0, reg_zero, reg_temp);                                 \

#define generate_shift_reg_lsr_no_flags(_rm, _rs)                             \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  generate_load_reg_lower_byte(reg_a1, _rs);                                  \
  mips_emit_sltiu(reg_temp, reg_a1, 32);                                      \
  mips_emit_srlv(reg_a0, reg_a0, reg_a1);                                     \
  mips_emit_movz(reg_a0, reg_zero, reg_temp);                                 \

#define generate_shift_reg_asr_no_flags(_rm, _rs)                             \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  generate_load_reg_lower_byte(reg_a1, _rs);                                  \
  mips_emit_sltiu(reg_temp, reg_a1, 32);                                      \
  mips_emit_b(bne, reg_temp, reg_zero, 2);                                    \
  mips_emit_srav(reg_a0, reg_a0, reg_a1);                                     \
  mips_emit_sra(reg_a0, reg_a0, 31);                                          \

#define generate_shift_reg_ror_no_flags(_rm, _rs)                             \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  generate_load_reg_lower_byte(reg_a1, _rs);                                  \
  mips_emit_rotrv(reg_a0, reg_a0, reg_a1);                                    \

#define generate_shift_reg_lsl_flags(_rm, _rs)                                \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  mips_emit_jal(mips_absolute_offset(execute_lsl_flags_reg));                 \
  generate_load_reg_lower_byte(reg_a1, _rs);                                  \

#define generate_shift_reg_lsr_flags(_rm, _rs)                                \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  mips_emit_jal(mips_absolute_offset(execute_lsr_flags_reg));                 \
  generate_load_reg_lower_byte(reg_a1, _rs);                                  \

#define generate_shift_reg_asr_flags(_rm, _rs)                                \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  mips_emit_jal(mips_absolute_offset(execute_asr_flags_reg));                 \
  generate_load_reg_lower_byte(reg_a1, _rs);                                  \

#define generate_shift_reg_ror_flags(_rm, _rs)                                \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  generate_load_reg_lower_byte(reg_a1, _rs);                                  \
  mips_emit_b(beq, reg_a1, reg_zero, 3);                                      \
  mips_emit_addiu(reg_temp, reg_a1, -1);                                      \
  mips_emit_srlv(reg_temp, reg_a0, reg_temp);                                 \
  mips_emit_andi(reg_c_cache, reg_temp, 1);                                   \
  mips_emit_rotrv(reg_a0, reg_a0, reg_a1);                                    \

/*
#define generate_shift_reg_ror_flags(_rm, _rs)                                \
  generate_load_reg_pc(reg_a0, _rm, 12);                                      \
  mips_emit_jal(mips_absolute_offset(execute_ror_flags_reg));                 \
  generate_load_reg_lower_byte(reg_a1, _rs);                                  \
*/

#define generate_shift_imm(arm_reg, name, flags_op)                           \
  u32 shift = (opcode >> 7) & 0x1F;                                           \
  generate_shift_imm_##name##_##flags_op(arm_reg, rm, shift);                 \

#define generate_shift_reg(arm_reg, name, flags_op)                           \
  u8 rs = (opcode >> 8) & 0x0F;                                               \
  generate_shift_reg_##name##_##flags_op(rm, rs);                             \
  rm = arm_reg;                                                               \

// Made functions due to the macro expansion getting too large.
// Returns a new rm if it redirects it (which will happen on most of these
// cases)

#define generate_load_rm_sh_builder(flags_op)                                 \
u32 generate_load_rm_sh_##flags_op(u32 rm)                                    \
{                                                                             \
  switch ((opcode >> 4) & 0x07)                                               \
  {                                                                           \
    /* LSL imm */                                                             \
    case 0x0:                                                                 \
    {                                                                         \
      generate_shift_imm(arm_reg_a0, lsl, flags_op);                          \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSL reg */                                                             \
    case 0x1:                                                                 \
    {                                                                         \
      generate_shift_reg(arm_reg_a0, lsl, flags_op);                          \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR imm */                                                             \
    case 0x2:                                                                 \
    {                                                                         \
      generate_shift_imm(arm_reg_a0, lsr, flags_op);                          \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* LSR reg */                                                             \
    case 0x3:                                                                 \
    {                                                                         \
      generate_shift_reg(arm_reg_a0, lsr, flags_op);                          \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR imm */                                                             \
    case 0x4:                                                                 \
    {                                                                         \
      generate_shift_imm(arm_reg_a0, asr, flags_op);                          \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ASR reg */                                                             \
    case 0x5:                                                                 \
    {                                                                         \
      generate_shift_reg(arm_reg_a0, asr, flags_op);                          \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR imm */                                                             \
    case 0x6:                                                                 \
    {                                                                         \
      generate_shift_imm(arm_reg_a0, ror, flags_op);                          \
      break;                                                                  \
    }                                                                         \
                                                                              \
    /* ROR reg */                                                             \
    case 0x7:                                                                 \
    {                                                                         \
      generate_shift_reg(arm_reg_a0, ror, flags_op);                          \
      break;                                                                  \
    }                                                                         \
  }                                                                           \
                                                                              \
  return rm;                                                                  \
}                                                                             \


#define generate_block_extra_vars()                                           \
  u32 stored_pc = pc;                                                         \
  u8 *update_trampoline;                                                      \

#define generate_block_extra_vars_arm()                                       \
  generate_block_extra_vars();                                                \
                                                                              \
  auto u32 generate_load_rm_sh_flags(u32 rm);                                 \
  auto u32 generate_load_rm_sh_no_flags(u32 rm);                              \
  auto u32 generate_load_offset_sh(u32 rm);                                   \
                                                                              \
  generate_load_rm_sh_builder(flags);                                         \
  generate_load_rm_sh_builder(no_flags);                                      \
                                                                              \
  u32 generate_load_offset_sh(u32 rm)                                         \
  {                                                                           \
    switch ((opcode >> 5) & 0x03)                                             \
    {                                                                         \
      /* LSL imm */                                                           \
      case 0x0:                                                               \
      {                                                                       \
        generate_shift_imm(arm_reg_a1, lsl, no_flags);                        \
        break;                                                                \
      }                                                                       \
                                                                              \
      /* LSR imm */                                                           \
      case 0x1:                                                               \
      {                                                                       \
        generate_shift_imm(arm_reg_a1, lsr, no_flags);                        \
        break;                                                                \
      }                                                                       \
                                                                              \
      /* ASR imm */                                                           \
      case 0x2:                                                               \
      {                                                                       \
        generate_shift_imm(arm_reg_a1, asr, no_flags);                        \
        break;                                                                \
      }                                                                       \
                                                                              \
      /* ROR imm */                                                           \
      case 0x3:                                                               \
      {                                                                       \
        generate_shift_imm(arm_reg_a1, ror, no_flags);                        \
        break;                                                                \
      }                                                                       \
    }                                                                         \
                                                                              \
    return rm;                                                                \
  }                                                                           \

#define generate_block_extra_vars_thumb()                                     \
  generate_block_extra_vars()                                                 \

// It should be okay to still generate result flags, spsr will overwrite them.
// This is pretty infrequent (returning from interrupt handlers, et al) so
// probably not worth optimizing for.

u32 execute_spsr_restore_body(u32 address)
{
  set_cpu_mode(cpu_modes[reg[REG_CPSR] & 0x1F]);

  if (((pIO_REG(REG_IE) & pIO_REG(REG_IF)) != 0) && GBA_IME_STATE && ARM_IRQ_STATE)
  {
    address |= 0x80000000;
  }

  return address;
}


/* EQ  Z=1 */
#define generate_condition_eq()                                               \
  mips_emit_b_filler(beq, reg_z_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force();                                              \

/* NE  Z=0 */
#define generate_condition_ne()                                               \
  mips_emit_b_filler(bne, reg_z_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force();                                              \

/* CS  C=1 */
#define generate_condition_cs()                                               \
  mips_emit_b_filler(beq, reg_c_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force();                                              \

/* CC  C=0 */
#define generate_condition_cc()                                               \
  mips_emit_b_filler(bne, reg_c_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force();                                              \

/* MI  N=1 */
#define generate_condition_mi()                                               \
  mips_emit_b_filler(beq, reg_n_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force();                                              \

/* PL  N=0 */
#define generate_condition_pl()                                               \
  mips_emit_b_filler(bne, reg_n_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force();                                              \

/* VS  V=1 */
#define generate_condition_vs()                                               \
  mips_emit_b_filler(beq, reg_v_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force();                                              \

/* VC  V=0 */
#define generate_condition_vc()                                               \
  mips_emit_b_filler(bne, reg_v_cache, reg_zero, backpatch_address);          \
  generate_cycle_update_force();                                              \

/* HI  C=1 and Z=0 */
#define generate_condition_hi()                                               \
  mips_emit_xori(reg_temp, reg_c_cache, 1);                                   \
  mips_emit_or(reg_temp, reg_temp, reg_z_cache);                              \
  mips_emit_b_filler(bne, reg_temp, reg_zero, backpatch_address);             \
  generate_cycle_update_force();                                              \

/* LS  C=0 or Z=1 */
#define generate_condition_ls()                                               \
  mips_emit_xori(reg_temp, reg_c_cache, 1);                                   \
  mips_emit_or(reg_temp, reg_temp, reg_z_cache);                              \
  mips_emit_b_filler(beq, reg_temp, reg_zero, backpatch_address);             \
  generate_cycle_update_force();                                              \

/* GE  N=V */
#define generate_condition_ge()                                               \
  mips_emit_b_filler(bne, reg_n_cache, reg_v_cache, backpatch_address);       \
  generate_cycle_update_force();                                              \

/* LT  N<>V */
#define generate_condition_lt()                                               \
  mips_emit_b_filler(beq, reg_n_cache, reg_v_cache, backpatch_address);       \
  generate_cycle_update_force();                                              \

/* GT  Z=0 and N=V */
#define generate_condition_gt()                                               \
  mips_emit_xor(reg_temp, reg_n_cache, reg_v_cache);                          \
  mips_emit_or(reg_temp, reg_temp, reg_z_cache);                              \
  mips_emit_b_filler(bne, reg_temp, reg_zero, backpatch_address);             \
  generate_cycle_update_force();                                              \

/* LE  Z=1 or N<>V */
#define generate_condition_le()                                               \
  mips_emit_xor(reg_temp, reg_n_cache, reg_v_cache);                          \
  mips_emit_or(reg_temp, reg_temp, reg_z_cache);                              \
  mips_emit_b_filler(beq, reg_temp, reg_zero, backpatch_address);             \
  generate_cycle_update_force();                                              \

/* NV  never */
#define generate_condition_nv()                                               \
  cycle_count += pMEMORY_WS32S(pc >> 24);                                     \
  mips_emit_b_filler(beq, reg_zero, reg_zero, backpatch_address);             \
  generate_cycle_update_force();                                              \

#define generate_condition()                                                  \
  switch (condition)                                                          \
  {                                                                           \
    case 0x0:                                                                 \
      generate_condition_eq();                                                \
      break;                                                                  \
                                                                              \
    case 0x1:                                                                 \
      generate_condition_ne();                                                \
      break;                                                                  \
                                                                              \
    case 0x2:                                                                 \
      generate_condition_cs();                                                \
      break;                                                                  \
                                                                              \
    case 0x3:                                                                 \
      generate_condition_cc();                                                \
      break;                                                                  \
                                                                              \
    case 0x4:                                                                 \
      generate_condition_mi();                                                \
      break;                                                                  \
                                                                              \
    case 0x5:                                                                 \
      generate_condition_pl();                                                \
      break;                                                                  \
                                                                              \
    case 0x6:                                                                 \
      generate_condition_vs();                                                \
      break;                                                                  \
                                                                              \
    case 0x7:                                                                 \
      generate_condition_vc();                                                \
      break;                                                                  \
                                                                              \
    case 0x8:                                                                 \
      generate_condition_hi();                                                \
      break;                                                                  \
                                                                              \
    case 0x9:                                                                 \
      generate_condition_ls();                                                \
      break;                                                                  \
                                                                              \
    case 0xA:                                                                 \
      generate_condition_ge();                                                \
      break;                                                                  \
                                                                              \
    case 0xB:                                                                 \
      generate_condition_lt();                                                \
      break;                                                                  \
                                                                              \
    case 0xC:                                                                 \
      generate_condition_gt();                                                \
      break;                                                                  \
                                                                              \
    case 0xD:                                                                 \
      generate_condition_le();                                                \
      break;                                                                  \
                                                                              \
    case 0xE:                                                                 \
      break;                                                                  \
                                                                              \
    case 0xF:                                                                 \
      generate_condition_nv();                                                \
      break;                                                                  \
  }                                                                           \

#define generate_branch(type)                                                 \
{                                                                             \
  generate_branch_cycle_update(type, block_exits[block_exit_position].branch_source, block_exits[block_exit_position].branch_target); \
  block_exit_position++;                                                      \
}                                                                             \

#define generate_op_and_reg(_rd, _rn, _rm)                                    \
  mips_emit_and(_rd, _rn, _rm)                                                \

#define generate_op_orr_reg(_rd, _rn, _rm)                                    \
  mips_emit_or(_rd, _rn, _rm)                                                 \

#define generate_op_eor_reg(_rd, _rn, _rm)                                    \
  mips_emit_xor(_rd, _rn, _rm)                                                \

#define generate_op_bic_reg(_rd, _rn, _rm)                                    \
  mips_emit_nor(reg_temp, _rm, reg_zero);                                     \
  mips_emit_and(_rd, _rn, reg_temp);                                          \

#define generate_op_sub_reg(_rd, _rn, _rm)                                    \
  mips_emit_subu(_rd, _rn, _rm)                                               \

#define generate_op_rsb_reg(_rd, _rn, _rm)                                    \
  mips_emit_subu(_rd, _rm, _rn)                                               \

/* SBC Rd = Rn - <Oprnd> - NOT(Carry) */
#define generate_op_sbc_reg(_rd, _rn, _rm)                                    \
  mips_emit_subu(_rd, _rn, _rm);                                              \
  mips_emit_xori(reg_temp, reg_c_cache, 1);                                   \
  mips_emit_subu(_rd, _rd, reg_temp);                                         \

/* RSC Rd = <Oprnd> - Rn - NOT(Carry) */
#define generate_op_rsc_reg(_rd, _rn, _rm)                                    \
  mips_emit_subu(_rd, _rm, _rn);                                              \
  mips_emit_xori(reg_temp, reg_c_cache, 1);                                   \
  mips_emit_subu(_rd, _rd, reg_temp);                                         \

#define generate_op_add_reg(_rd, _rn, _rm)                                    \
  mips_emit_addu(_rd, _rn, _rm)                                               \

#define generate_op_adc_reg(_rd, _rn, _rm)                                    \
  mips_emit_addu(reg_temp, _rm, reg_c_cache);                                 \
  mips_emit_addu(_rd, _rn, reg_temp);                                         \

#define generate_op_mov_reg(_rd, _rn, _rm)                                    \
  mips_emit_addu(_rd, _rm, reg_zero)                                          \

#define generate_op_mvn_reg(_rd, _rn, _rm)                                    \
  mips_emit_nor(_rd, _rm, reg_zero)                                           \

#define generate_op_imm_wrapper(name, _rd, _rn)                               \
  if (imm != 0)                                                               \
  {                                                                           \
    generate_load_imm(reg_a0, imm);                                           \
    generate_op_##name##_reg(_rd, _rn, reg_a0);                               \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_op_##name##_reg(_rd, _rn, reg_zero);                             \
  }                                                                           \

#define generate_op_and_imm(_rd, _rn)                                         \
  generate_alu_immu(andi, and, _rd, _rn, imm)                                 \

#define generate_op_orr_imm(_rd, _rn)                                         \
  generate_alu_immu(ori, or, _rd, _rn, imm)                                   \

#define generate_op_eor_imm(_rd, _rn)                                         \
  generate_alu_immu(xori, xor, _rd, _rn, imm)                                 \

#define generate_op_bic_imm(_rd, _rn)                                         \
  generate_alu_immu(andi, and, _rd, _rn, (~imm))                              \

#define generate_op_sub_imm(_rd, _rn)                                         \
  generate_alu_imm(addiu, addu, _rd, _rn, (-imm))                             \

#define generate_op_rsb_imm(_rd, _rn)                                         \
  if (imm != 0)                                                               \
  {                                                                           \
    generate_load_imm(reg_temp, imm);                                         \
    mips_emit_subu(_rd, reg_temp, _rn);                                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_subu(_rd, reg_zero, _rn);                                       \
  }                                                                           \

#define generate_op_sbc_imm(_rd, _rn)                                         \
  generate_op_imm_wrapper(sbc, _rd, _rn)                                      \

#define generate_op_rsc_imm(_rd, _rn)                                         \
  generate_op_imm_wrapper(rsc, _rd, _rn)                                      \

#define generate_op_add_imm(_rd, _rn)                                         \
  generate_alu_imm(addiu, addu, _rd, _rn, imm)                                \

#define generate_op_adc_imm(_rd, _rn)                                         \
  generate_op_imm_wrapper(adc, _rd, _rn)                                      \

#define generate_op_mov_imm(_rd, _rn)                                         \
  generate_load_imm(_rd, imm)                                                 \

#define generate_op_mvn_imm(_rd, _rn)                                         \
  generate_load_imm(_rd, (~imm))                                              \

#define generate_op_logic_flags(_rd)                                          \
  if (check_generate_n_flag != 0)                                             \
  {                                                                           \
    mips_emit_ext(reg_n_cache, _rd, 31, 1);                                   \
  }                                                                           \
  if (check_generate_z_flag != 0)                                             \
  {                                                                           \
    mips_emit_sltiu(reg_z_cache, _rd, 1);                                     \
  }                                                                           \

#define generate_op_sub_flags_prologue(_rn, _rm)                              \
  if (check_generate_c_flag != 0)                                             \
  {                                                                           \
    mips_emit_sltu(reg_c_cache, _rn, _rm);                                    \
    mips_emit_xori(reg_c_cache, reg_c_cache, 1);                              \
  }                                                                           \
  if (check_generate_v_flag != 0)                                             \
  {                                                                           \
    mips_emit_slt(reg_v_cache, _rn, _rm);                                     \
  }                                                                           \

#define generate_op_sub_flags_epilogue(_rd)                                   \
  generate_op_logic_flags(_rd);                                               \
  if (check_generate_v_flag != 0)                                             \
  {                                                                           \
    if (check_generate_n_flag == 0)                                           \
    {                                                                         \
      mips_emit_ext(reg_n_cache, _rd, 31, 1);                                 \
    }                                                                         \
    mips_emit_xor(reg_v_cache, reg_v_cache, reg_n_cache);                     \
  }                                                                           \

#define generate_add_flags_prologue(_rn, _rm)                                 \
  if ((check_generate_c_flag | check_generate_v_flag) != 0)                   \
  {                                                                           \
    mips_emit_addu(reg_c_cache, _rn, reg_zero);                               \
  }                                                                           \
  if (check_generate_v_flag != 0)                                             \
  {                                                                           \
    mips_emit_slt(reg_v_cache, _rm, reg_zero);                                \
  }                                                                           \

#define generate_add_flags_epilogue(_rd)                                      \
  if (check_generate_v_flag != 0)                                             \
  {                                                                           \
    mips_emit_slt(reg_a0, _rd, reg_c_cache);                                  \
    mips_emit_xor(reg_v_cache, reg_v_cache, reg_a0);                          \
  }                                                                           \
  if ((check_generate_c_flag | check_generate_v_flag) != 0)                   \
  {                                                                           \
    mips_emit_sltu(reg_c_cache, _rd, reg_c_cache);                            \
  }                                                                           \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_ands_reg(_rd, _rn, _rm)                                   \
  mips_emit_and(_rd, _rn, _rm);                                               \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_orrs_reg(_rd, _rn, _rm)                                   \
  mips_emit_or(_rd, _rn, _rm);                                                \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_eors_reg(_rd, _rn, _rm)                                   \
  mips_emit_xor(_rd, _rn, _rm);                                               \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_bics_reg(_rd, _rn, _rm)                                   \
  mips_emit_nor(reg_temp, _rm, reg_zero);                                     \
  mips_emit_and(_rd, _rn, reg_temp);                                          \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_subs_reg(_rd, _rn, _rm)                                   \
  generate_op_sub_flags_prologue(_rn, _rm);                                   \
  mips_emit_subu(_rd, _rn, _rm);                                              \
  generate_op_sub_flags_epilogue(_rd);                                        \

#define generate_op_rsbs_reg(_rd, _rn, _rm)                                   \
  generate_op_sub_flags_prologue(_rm, _rn);                                   \
  mips_emit_subu(_rd, _rm, _rn);                                              \
  generate_op_sub_flags_epilogue(_rd);                                        \

/* SBCS Rd = Rn - <Oprnd> - NOT(Carry) */
#define generate_op_sbcs_reg(_rd, _rn, _rm)                                   \
  mips_emit_xori(reg_temp, reg_c_cache, 1);                                   \
  mips_emit_addu(reg_temp, _rm, reg_temp);                                    \
  generate_op_sub_flags_prologue(_rn, reg_temp);                              \
  mips_emit_subu(_rd, _rn, reg_temp);                                         \
  generate_op_sub_flags_epilogue(_rd);                                        \

/* RSCS Rd = <Oprnd> - Rn - NOT(Carry) */
#define generate_op_rscs_reg(_rd, _rn, _rm)                                   \
  mips_emit_xori(reg_temp, reg_c_cache, 1);                                   \
  mips_emit_addu(reg_temp, _rn, reg_temp);                                    \
  generate_op_sub_flags_prologue(_rm, reg_temp);                              \
  mips_emit_subu(_rd, _rm, reg_temp);                                         \
  generate_op_sub_flags_epilogue(_rd);                                        \

#define generate_op_adds_reg(_rd, _rn, _rm)                                   \
  generate_add_flags_prologue(_rn, _rm);                                      \
  mips_emit_addu(_rd, _rn, _rm);                                              \
  generate_add_flags_epilogue(_rd);                                           \

#define generate_op_adcs_reg(_rd, _rn, _rm)                                   \
  mips_emit_addu(reg_temp, _rm, reg_c_cache);                                 \
  generate_add_flags_prologue(_rn, reg_temp);                                 \
  mips_emit_addu(_rd, _rn, reg_temp);                                         \
  generate_add_flags_epilogue(_rd);                                           \

#define generate_op_movs_reg(_rd, _rn, _rm)                                   \
  mips_emit_addu(_rd, _rm, reg_zero);                                         \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_mvns_reg(_rd, _rn, _rm)                                   \
  mips_emit_nor(_rd, _rm, reg_zero);                                          \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_neg_reg(_rd, _rn, _rm)                                    \
  generate_op_subs_reg(_rd, reg_zero, _rm)                                    \

#define generate_op_muls_reg(_rd, _rn, _rm)                                   \
  mips_emit_multu(_rn, _rm);                                                  \
  mips_emit_mflo(_rd);                                                        \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_cmp_reg(_rd, _rn, _rm)                                    \
  generate_op_subs_reg(reg_temp, _rn, _rm)                                    \

#define generate_op_cmn_reg(_rd, _rn, _rm)                                    \
  generate_op_adds_reg(reg_temp, _rn, _rm)                                    \

#define generate_op_tst_reg(_rd, _rn, _rm)                                    \
  generate_op_ands_reg(reg_temp, _rn, _rm)                                    \

#define generate_op_teq_reg(_rd, _rn, _rm)                                    \
  generate_op_eors_reg(reg_temp, _rn, _rm)                                    \

#define generate_op_ands_imm(_rd, _rn)                                        \
  generate_alu_immu(andi, and, _rd, _rn, imm);                                \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_orrs_imm(_rd, _rn)                                        \
  generate_alu_immu(ori, or, _rd, _rn, imm);                                  \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_eors_imm(_rd, _rn)                                        \
  generate_alu_immu(xori, xor, _rd, _rn, imm);                                \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_bics_imm(_rd, _rn)                                        \
  generate_alu_immu(andi, and, _rd, _rn, (~imm));                             \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_subs_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(subs, _rd, _rn)                                     \

#define generate_op_rsbs_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(rsbs, _rd, _rn)                                     \

#define generate_op_sbcs_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(sbcs, _rd, _rn)                                     \

#define generate_op_rscs_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(rscs, _rd, _rn)                                     \

#define generate_op_adds_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(adds, _rd, _rn)                                     \

#define generate_op_adcs_imm(_rd, _rn)                                        \
  generate_op_imm_wrapper(adcs, _rd, _rn)                                     \

#define generate_op_movs_imm(_rd, _rn)                                        \
  generate_load_imm(_rd, imm);                                                \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_mvns_imm(_rd, _rn)                                        \
  generate_load_imm(_rd, (~imm));                                             \
  generate_op_logic_flags(_rd);                                               \

#define generate_op_cmp_imm(_rd, _rn)                                         \
  generate_op_imm_wrapper(cmp, _rd, _rn)                                      \

#define generate_op_cmn_imm(_rd, _rn)                                         \
  generate_op_imm_wrapper(cmn, _rd, _rn)                                      \

#define generate_op_tst_imm(_rd, _rn)                                         \
  generate_op_ands_imm(reg_temp, _rn)                                         \

#define generate_op_teq_imm(_rd, _rn)                                         \
  generate_op_eors_imm(reg_temp, _rn)                                         \

#define arm_generate_op_load_yes()                                            \
  generate_load_reg_pc(reg_a1, rn, 8)                                         \

#define arm_generate_op_load_no()                                             \

#define arm_op_check_yes()                                                    \
  check_load_reg_pc(arm_reg_a1, rn, 8)                                        \

#define arm_op_check_no()                                                     \

#define arm_generate_op_reg_flags(name, load_op)                              \
  if (check_generate_c_flag != 0)                                             \
  {                                                                           \
    rm = generate_load_rm_sh_flags(rm);                                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    rm = generate_load_rm_sh_no_flags(rm);                                    \
  }                                                                           \
  arm_op_check_##load_op();                                                   \
  generate_op_##name##_reg(arm_to_mips_reg[rd], arm_to_mips_reg[rn], arm_to_mips_reg[rm]); \
  cycle_count++;                                                              \

#define arm_generate_op_reg(name, load_op)                                    \
  rm = generate_load_rm_sh_no_flags(rm);                                      \
  arm_op_check_##load_op();                                                   \
  generate_op_##name##_reg(arm_to_mips_reg[rd], arm_to_mips_reg[rn], arm_to_mips_reg[rm]); \
  cycle_count++;                                                              \

#define arm_generate_op_imm_flags(name, load_op)                              \
  arm_op_check_##load_op();                                                   \
  if ((check_generate_c_flag != 0) && (shift != 0))                           \
  {                                                                           \
    mips_emit_addiu(reg_c_cache, reg_zero, ((op2_imm >> (shift - 1)) & 0x01));\
  }                                                                           \
  generate_op_##name##_imm(arm_to_mips_reg[rd], arm_to_mips_reg[rn]);         \

#define arm_generate_op_imm(name, load_op)                                    \
  arm_op_check_##load_op();                                                   \
  generate_op_##name##_imm(arm_to_mips_reg[rd], arm_to_mips_reg[rn]);         \

#define check_store_reg_pc_reg_no_flags(reg_index)                            \
  if (reg_index == REG_PC)                                                    \
  {                                                                           \
    generate_indirect_branch_cycle_update(arm);                               \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    cycle_count += pFETCH_WS32S(pc >> 24);                                    \
  }                                                                           \

#define check_store_reg_pc_reg_flags(reg_index)                               \
  if (reg_index == REG_PC)                                                    \
  {                                                                           \
    mips_emit_jal(mips_absolute_offset(execute_spsr_restore));                \
    generate_cycle_update_force();                                            \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    cycle_count += pFETCH_WS32S(pc >> 24);                                    \
  }                                                                           \

#define check_store_reg_pc_reg_flags_flags(reg_index)                         \
  check_store_reg_pc_reg_flags(reg_index)                                     \

#define check_store_reg_pc_imm_no_flags(reg_index)                            \
  if (reg_index == REG_PC)                                                    \
  {                                                                           \
    generate_indirect_branch_cycle_update(arm);                               \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    cycle_count += pMEMORY_WS32S(pc >> 24);                                   \
  }                                                                           \

#define check_store_reg_pc_imm_flags(reg_index)                               \
  if (reg_index == REG_PC)                                                    \
  {                                                                           \
    mips_emit_jal(mips_absolute_offset(execute_spsr_restore));                \
    generate_cycle_update_force();                                            \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    cycle_count += pMEMORY_WS32S(pc >> 24);                                   \
  }                                                                           \

#define check_store_reg_pc_imm_flags_flags(reg_index)                         \
  check_store_reg_pc_imm_flags(reg_index)                                     \

#define arm_data_proc(name, type, flags_op)                                   \
{                                                                             \
  arm_decode_data_proc_##type();                                              \
  arm_generate_op_##type(name, yes);                                          \
  check_store_reg_pc_##type##_##flags_op(rd);                                 \
}                                                                             \

#define cycle_arm_data_proc_test_reg()                                        \
  cycle_count += pFETCH_WS32S(pc >> 24)                                       \

#define cycle_arm_data_proc_test_reg_flags()                                  \
  cycle_arm_data_proc_test_reg()                                              \

#define cycle_arm_data_proc_test_imm()                                        \
  cycle_count += pMEMORY_WS32S(pc >> 24)                                      \

#define cycle_arm_data_proc_test_imm_flags()                                  \
  cycle_arm_data_proc_test_imm()                                              \

#define arm_data_proc_test(name, type)                                        \
{                                                                             \
  arm_decode_data_proc_test_##type();                                         \
  arm_generate_op_##type(name, yes);                                          \
                                                                              \
  cycle_arm_data_proc_test_##type();                                          \
}                                                                             \

#define arm_data_proc_unary(name, type, flags_op)                             \
{                                                                             \
  arm_decode_data_proc_unary_##type();                                        \
  arm_generate_op_##type(name, no);                                           \
  check_store_reg_pc_##type##_##flags_op(rd);                                 \
}                                                                             \


/* 1S+mI */
#define cycle_arm_multiply_add_no()                                           \
  mips_emit_jal(mips_absolute_offset(execute_multiply_ticks));                \
  generate_load_reg(reg_a0, rs);                                              \
  cycle_count += pFETCH_WS32S(pc >> 24);                                      \

/* 1S+mI+1I */
#define cycle_arm_multiply_add_yes()                                          \
  mips_emit_jal(mips_absolute_offset(execute_multiply_ticks));                \
  generate_load_reg(reg_a0, rs);                                              \
  cycle_count += pFETCH_WS32S(pc >> 24) + 1;                                  \

#define arm_multiply_flags_yes(_rd)                                           \
  generate_op_logic_flags(_rd)                                                \

#define arm_multiply_flags_no(_rd)                                            \

#define arm_multiply_add_no()                                                 \
  mips_emit_mflo(arm_to_mips_reg[rd])                                         \

#define arm_multiply_add_yes()                                                \
  mips_emit_mflo(reg_temp);                                                   \
  mips_emit_addu(arm_to_mips_reg[rd], reg_temp, arm_to_mips_reg[rn]);         \

#define arm_multiply(add_op, flags)                                           \
{                                                                             \
  arm_decode_multiply_add_##add_op();                                         \
  mips_emit_multu(arm_to_mips_reg[rm], arm_to_mips_reg[rs]);                  \
                                                                              \
  cycle_arm_multiply_add_##add_op();                                          \
                                                                              \
  arm_multiply_add_##add_op();                                                \
  arm_multiply_flags_##flags(arm_to_mips_reg[rd]);                            \
}                                                                             \


/* 1S+mI+1I */
#define cycle_arm_multiply_long_add_no()                                      \
  generate_load_reg(reg_a0, rs);                                              \
  cycle_count += pFETCH_WS32S(pc >> 24) + 1;                                  \

/* 1S+mI+2I */
#define cycle_arm_multiply_long_add_yes()                                     \
  generate_load_reg(reg_a0, rs);                                              \
  cycle_count += pFETCH_WS32S(pc >> 24) + 2;                                  \

#define arm_multiply_long_flags_yes(_rdlo, _rdhi)                             \
  mips_emit_sltiu(reg_z_cache, _rdlo, 1);                                     \
  mips_emit_sltiu(reg_a0, _rdhi, 1);                                          \
  mips_emit_and(reg_z_cache, reg_z_cache, reg_a0);                            \
  mips_emit_srl(reg_n_cache, _rdhi, 31);                                      \

#define arm_multiply_long_flags_no(_rdlo, _rdhi)                              \

#define arm_multiply_long_add_yes(name)                                       \
  mips_emit_mtlo(arm_to_mips_reg[rdlo]);                                      \
  mips_emit_mthi(arm_to_mips_reg[rdhi]);                                      \
  generate_multiply_##name();                                                 \

#define arm_multiply_long_add_no(name)                                        \
  generate_multiply_##name()                                                  \

#define arm_multiply_long(name, add_op, flags)                                \
{                                                                             \
  arm_decode_multiply_long();                                                 \
  arm_multiply_long_add_##add_op(name);                                       \
                                                                              \
  cycle_arm_multiply_long_add_##add_op();                                     \
                                                                              \
  mips_emit_mflo(arm_to_mips_reg[rdlo]);                                      \
  mips_emit_mfhi(arm_to_mips_reg[rdhi]);                                      \
  arm_multiply_long_flags_##flags(arm_to_mips_reg[rdlo], arm_to_mips_reg[rdhi]); \
}                                                                             \


#define arm_psr_read(op_type, psr_reg)                                        \
  generate_function_call(execute_read_##psr_reg);                             \
  generate_store_reg(reg_rv, rd);                                             \

u32 execute_store_cpsr_body(u32 _cpsr, u32 store_mask)
{
  if ((store_mask & 0xFF) != 0)
  {
    set_cpu_mode(cpu_modes[_cpsr & 0x1F]);

    if (((pIO_REG(REG_IE) & pIO_REG(REG_IF)) != 0) && GBA_IME_STATE && ARM_IRQ_STATE)
    {
      return 1;
    }
  }

  return 0;
}

#define arm_psr_load_new_reg()                                                \
  generate_load_reg(reg_a0, rm)                                               \

#define arm_psr_load_new_imm()                                                \
  generate_load_imm(reg_a0, imm)                                              \

#define arm_psr_store(op_type, psr_reg)                                       \
  arm_psr_load_new_##op_type();                                               \
  generate_load_pc(reg_a2, (pc + 4));                                         \
  generate_load_imm(reg_a1, psr_masks[psr_field]);                            \
  mips_emit_jal(mips_absolute_offset(execute_store_##psr_reg));               \
  generate_cycle_update_force();                                              \

#define arm_psr(op_type, transfer_type, psr_reg)                              \
{                                                                             \
  arm_decode_psr_##transfer_type##_##op_type();                               \
  cycle_count += pMEMORY_WS32S(pc >> 24);                                     \
                                                                              \
  arm_psr_##transfer_type(op_type, psr_reg);                                  \
}                                                                             \

/* LDR: 1S+1N+1I. LDR PC: 2S+2N+1I. STR: 2N. */
#define arm_access_memory_load(mem_type)                                      \
  cycle_count++;                                                              \
  generate_load_pc(reg_a1, (pc + 8));                                         \
  generate_function_call_swap_delay(execute_load_##mem_type);                 \
  generate_store_reg(reg_rv, rd);                                             \
  check_store_reg_pc_no_flags(rd);                                            \

#define arm_access_memory_store(mem_type)                                     \
  cycle_count += pFETCH_WS32N(pc >> 24);                                      \
  generate_load_pc(reg_a2, (pc + 4));                                         \
  generate_load_reg_pc(reg_a1, rd, 12);                                       \
  mips_emit_jal(mips_absolute_offset(execute_store_##mem_type));              \
  generate_cycle_update_force();                                              \

#define arm_access_memory_reg_pre_up()                                        \
  mips_emit_addu(reg_a0, arm_to_mips_reg[rn], arm_to_mips_reg[rm])            \

#define arm_access_memory_reg_pre_down()                                      \
  mips_emit_subu(reg_a0, arm_to_mips_reg[rn], arm_to_mips_reg[rm])            \

#define arm_access_memory_reg_pre(direction)                                  \
  check_load_reg_pc(arm_reg_a0, rn, 8);                                       \
  arm_access_memory_reg_pre_##direction();                                    \

#define arm_access_memory_reg_pre_wb(direction)                               \
  arm_access_memory_reg_pre(direction);                                       \
  generate_store_reg(reg_a0, rn);                                             \

#define arm_access_memory_reg_post_up()                                       \
  mips_emit_addu(arm_to_mips_reg[rn], arm_to_mips_reg[rn], arm_to_mips_reg[rm]) \

#define arm_access_memory_reg_post_down()                                     \
  mips_emit_subu(arm_to_mips_reg[rn], arm_to_mips_reg[rn], arm_to_mips_reg[rm]) \

#define arm_access_memory_reg_post(direction)                                 \
  generate_load_reg(reg_a0, rn);                                              \
  arm_access_memory_reg_post_##direction();                                   \

#define arm_access_memory_imm_pre_up()                                        \
  mips_emit_addiu(reg_a0, arm_to_mips_reg[rn], offset)                        \

#define arm_access_memory_imm_pre_down()                                      \
  mips_emit_addiu(reg_a0, arm_to_mips_reg[rn], -offset)                       \

#define arm_access_memory_imm_pre(direction)                                  \
  check_load_reg_pc(arm_reg_a0, rn, 8);                                       \
  arm_access_memory_imm_pre_##direction();                                    \

#define arm_access_memory_imm_pre_wb(direction)                               \
  arm_access_memory_imm_pre(direction);                                       \
  generate_store_reg(reg_a0, rn);                                             \

#define arm_access_memory_imm_post_up()                                       \
  mips_emit_addiu(arm_to_mips_reg[rn], arm_to_mips_reg[rn], offset)           \

#define arm_access_memory_imm_post_down()                                     \
  mips_emit_addiu(arm_to_mips_reg[rn], arm_to_mips_reg[rn], -offset)          \

#define arm_access_memory_imm_post(direction)                                 \
  generate_load_reg(reg_a0, rn);                                              \
  arm_access_memory_imm_post_##direction();                                   \

#define arm_data_trans_reg(adjust_op, direction)                              \
  arm_decode_data_trans_reg();                                                \
  rm = generate_load_offset_sh(rm);                                           \
  arm_access_memory_reg_##adjust_op(direction);                               \

#define arm_data_trans_imm(adjust_op, direction)                              \
  arm_decode_data_trans_imm();                                                \
  arm_access_memory_imm_##adjust_op(direction);                               \

#define arm_data_trans_half_reg(adjust_op, direction)                         \
  arm_decode_half_trans_r();                                                  \
  arm_access_memory_reg_##adjust_op(direction);                               \

#define arm_data_trans_half_imm(adjust_op, direction)                         \
  arm_decode_half_trans_of();                                                 \
  arm_access_memory_imm_##adjust_op(direction);                               \

#define arm_access_memory(access_type, direction, adjust_op, mem_type, offset_type) \
{                                                                             \
  arm_data_trans_##offset_type(adjust_op, direction);                         \
  arm_access_memory_##access_type(mem_type);                                  \
}                                                                             \


#define word_bit_count(word)                                                  \
  (bit_count[(word) >> 8] + bit_count[(word) & 0xFF])                         \

void force_user_mode_body(CPU_MODE_TYPE cpu_mode, CPU_MODE_TYPE new_mode)
{
  u32 i;
  u32 *reg_base;

  reg_base = reg_mode[cpu_mode];

  if (new_mode != MODE_FIQ)
  {
    reg_base[5] = reg[REG_SP];
    reg_base[6] = reg[REG_LR];
  }
  else
  {
    for (i = 8; i < 15; i++)
      reg_base[i - 8] = reg[i];
  }

  reg_base = reg_mode[new_mode];

  if (cpu_mode != MODE_FIQ)
  {
    reg[REG_SP] = reg_base[5];
    reg[REG_LR] = reg_base[6];
  }
  else
  {
    for (i = 8; i < 15; i++)
      reg[i] = reg_base[i - 8];
  }
}

#define arm_block_memory_force_user_yes_load_prologue()                       \
  if ((reg_list & 0x8000) == 0)                                               \
  {                                                                           \
    generate_function_call(execute_force_user_mode_prologue);                 \
  }                                                                           \

#define arm_block_memory_force_user_no_load_prologue()                        \

#define arm_block_memory_force_user_yes_store_prologue()                      \
  generate_function_call(execute_force_user_mode_prologue)                    \

#define arm_block_memory_force_user_no_store_prologue()                       \

#define arm_block_memory_force_user_yes_load_epilogue()                       \
  if ((reg_list & 0x8000) == 0)                                               \
  {                                                                           \
    generate_function_call(execute_force_user_mode_epilogue);                 \
  }                                                                           \

#define arm_block_memory_force_user_no_load_epilogue()                        \

#define arm_block_memory_force_user_yes_store_epilogue()                      \
  generate_function_call(execute_force_user_mode_epilogue)                    \

#define arm_block_memory_force_user_no_store_epilogue()                       \

/* LDM, nS+1N+1I. LDM PC, (n+1)S+2N+1I. STM (n-1)S+2N. */
#define arm_block_memory_load()                                               \
  generate_load_pc(reg_a1, (pc + 8));                                         \
  generate_function_call_swap_delay(execute_aligned_load32);                  \
  generate_store_reg(reg_rv, i);                                              \

#define arm_block_memory_store()                                              \
  generate_load_reg_pc(reg_a1, i, 12);                                        \
  generate_function_call_swap_delay(execute_aligned_store32);                 \

#define arm_block_memory_final_load()                                         \
  generate_load_pc(reg_a1, (pc + 8));                                         \
  generate_function_call_swap_delay(execute_load_u32);                        \
  generate_store_reg(reg_rv, i);                                              \

#define arm_block_memory_final_store()                                        \
  generate_load_pc(reg_a2, (pc + 4));                                         \
  generate_load_reg_pc(reg_a1, i, 12);                                        \
  mips_emit_jal(mips_absolute_offset(execute_store_u32));                     \
  generate_cycle_update_force();                                              \

#define arm_block_memory_load_psr_yes()                                       \
  generate_mov(reg_a0, reg_rv);                                               \
  mips_emit_jal(mips_absolute_offset(execute_spsr_restore));                  \
  generate_cycle_update_force();                                              \

#define arm_block_memory_load_psr_no()                                        \
  generate_mov(reg_a0, reg_rv);                                               \
  generate_indirect_branch_cycle_update(arm);                                 \

#define arm_block_memory_adjust_pc_load(s_bit)                                \
  if ((reg_list & 0x8000) != 0)                                               \
  {                                                                           \
    cycle_count++;                                                            \
    arm_block_memory_load_psr_##s_bit();                                      \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    cycle_count += pFETCH_WS32S(pc >> 24) + 1;                                \
  }                                                                           \

#define arm_block_memory_adjust_pc_store(s_bit)                               \
  cycle_count += pFETCH_WS32N(pc >> 24);                                      \

#define arm_block_memory_sp_load()                                            \
  mips_emit_lw(arm_to_mips_reg[i], reg_a1, offset);                           \

#define arm_block_memory_sp_store()                                           \
{                                                                             \
  u32 store_reg = i;                                                          \
  check_load_reg_pc(arm_reg_a0, store_reg, 12);                               \
  mips_emit_sw(arm_to_mips_reg[store_reg], reg_a1, offset);                   \
}                                                                             \

#define arm_block_memory_sp_load_psr_yes()                                    \
  mips_emit_jal(mips_absolute_offset(execute_spsr_restore));                  \
  generate_cycle_update_force();                                              \

#define arm_block_memory_sp_load_psr_no()                                     \
  generate_indirect_branch_cycle_update(arm);                                 \

#define arm_block_memory_sp_adjust_pc_load(s_bit)                             \
  if ((reg_list & 0x8000) != 0)                                               \
  {                                                                           \
    cycle_count++;                                                            \
    arm_block_memory_sp_load_psr_##s_bit();                                   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    cycle_count += pFETCH_WS32S(pc >> 24) + 1;                                \
  }                                                                           \

#define arm_block_memory_sp_adjust_pc_store(s_bit)                            \
  cycle_count += pFETCH_WS32N(pc >> 24)                                       \

#define arm_block_memory_address_offset_no()                                  \
  mips_emit_addu(reg_a2, base_reg, reg_zero)                                  \

#define arm_block_memory_address_offset_up()                                  \
  mips_emit_addiu(reg_a2, base_reg, 4)                                        \

#define arm_block_memory_address_offset_down_a()                              \
  mips_emit_addiu(reg_a2, base_reg, (-((word_bit_count(reg_list) - 1) << 2))) \

#define arm_block_memory_address_offset_down_b()                              \
  mips_emit_addiu(reg_a2, base_reg, (-(word_bit_count(reg_list) << 2)))       \

#define arm_block_memory_address_writeback_no()

#define arm_block_memory_address_writeback_up()                               \
  mips_emit_addiu(base_reg, base_reg, (word_bit_count(reg_list) << 2))        \

#define arm_block_memory_address_writeback_down()                             \
  mips_emit_addiu(base_reg, base_reg, (-(word_bit_count(reg_list) << 2)))     \

#define arm_block_memory_address_load(offset_type, writeback_type)            \
  arm_block_memory_address_offset_##offset_type();                            \
  if (((reg_list >> rn) & 0x01) == 0)                                         \
  {                                                                           \
    arm_block_memory_address_writeback_##writeback_type();                    \
    mips_emit_addu(arm_to_mips_reg[rn], base_reg, reg_zero);                  \
  }                                                                           \

#define arm_block_memory_address_store(offset_type, writeback_type)           \
  arm_block_memory_address_offset_##offset_type();                            \
  arm_block_memory_address_writeback_##writeback_type();                      \

#define arm_block_memory_writeback_no()

#define arm_block_memory_writeback_up()                                       \
  if (offset == 0)                                                            \
  {                                                                           \
    mips_emit_addu(arm_to_mips_reg[rn], base_reg, reg_zero);                  \
  }                                                                           \

#define arm_block_memory_writeback_down()                                     \
  arm_block_memory_writeback_up()                                             \

#define arm_block_memory_writeback_load(writeback_type)                       \

#define arm_block_memory_writeback_store(writeback_type)                      \
  arm_block_memory_writeback_##writeback_type()                               \

#define arm_block_memory(access_type, offset_type, writeback_type, s_bit)     \
{                                                                             \
  arm_decode_block_trans();                                                   \
  u32 i;                                                                      \
  u32 offset = 0;                                                             \
  u32 base_reg = arm_to_mips_reg[rn];                                         \
                                                                              \
  arm_block_memory_force_user_##s_bit##_##access_type##_prologue();           \
  arm_block_memory_address_##access_type(offset_type, writeback_type);        \
                                                                              \
  if ((rn == REG_SP) && ((iwram_stack_optimize & option_stack_optimize) != 0))\
  {                                                                           \
    mips_emit_andi(reg_a1, reg_a2, 0x7FFC);                                   \
    generate_load_imm(reg_a0, (u32)iwram);                                    \
    mips_emit_addu(reg_a1, reg_a1, reg_a0);                                   \
                                                                              \
    for (i = 0; i < 16; i++)                                                  \
    {                                                                         \
      if (((reg_list >> i) & 0x01) != 0)                                      \
      {                                                                       \
        cycle_count++;                                                        \
        arm_block_memory_sp_##access_type();                                  \
        arm_block_memory_writeback_##access_type(writeback_type);             \
        offset += 4;                                                          \
      }                                                                       \
    }                                                                         \
                                                                              \
    arm_block_memory_sp_adjust_pc_##access_type(s_bit);                       \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_ins(reg_a2, reg_zero, 0, 2);                                    \
                                                                              \
    for (i = 0; i < 16; i++)                                                  \
    {                                                                         \
      if (((reg_list >> i) & 0x01) != 0)                                      \
      {                                                                       \
        mips_emit_addiu(reg_a0, reg_a2, offset);                              \
                                                                              \
        if ((reg_list & ~((2 << i) - 1)) != 0)                                \
        {                                                                     \
          arm_block_memory_##access_type();                                   \
          arm_block_memory_writeback_##access_type(writeback_type);           \
          offset += 4;                                                        \
        }                                                                     \
        else                                                                  \
        {                                                                     \
          arm_block_memory_final_##access_type();                             \
          break;                                                              \
        }                                                                     \
      }                                                                       \
    }                                                                         \
                                                                              \
    arm_block_memory_adjust_pc_##access_type(s_bit);                          \
  }                                                                           \
                                                                              \
  arm_block_memory_force_user_##s_bit##_##access_type##_epilogue();           \
}                                                                             \


/* 1S+2N+1I */
#define arm_swap(type)                                                        \
{                                                                             \
  arm_decode_swap();                                                          \
  cycle_count += pFETCH_WS32S(pc >> 24) + 1;                                  \
                                                                              \
  generate_load_pc(reg_a1, (pc + 8));                                         \
  mips_emit_jal(mips_absolute_offset(execute_load_##type));                   \
  generate_load_reg(reg_a0, rn);                                              \
                                                                              \
  generate_load_pc(reg_a2, (pc + 4));                                         \
  generate_load_reg(reg_a0, rn);                                              \
  generate_load_reg(reg_a1, rm);                                              \
  generate_store_reg(reg_rv, rd);                                             \
  mips_emit_jal(mips_absolute_offset(execute_store_##type));                  \
  generate_cycle_update_force();                                              \
}                                                                             \


#define thumb_generate_op_load_yes(_rs)                                       \
  generate_load_reg(reg_a1, _rs)                                              \

#define thumb_generate_op_load_no(_rs)                                        \

#define thumb_generate_op_reg(name, _rd, _rs, _rn)                            \
  generate_op_##name##_reg(arm_to_mips_reg[_rd], arm_to_mips_reg[_rs], arm_to_mips_reg[_rn]); \

#define thumb_generate_op_imm(name, _rd, _rs, _rn)                            \
  generate_op_##name##_imm(arm_to_mips_reg[_rd], arm_to_mips_reg[_rs]);       \

// Types: add_sub, add_sub_imm, alu_op, imm
// Affects N/Z/C/V flags

#define thumb_data_proc(type, name, rn_type, _rd, _rs, _rn)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_generate_op_##rn_type(name, _rd, _rs, _rn);                           \
                                                                              \
  cycle_count += pMEMORY_WS16S(pc >> 24);                                     \
}                                                                             \

/* 1S+mI */
#define thumb_data_proc_muls(_rd, _rs, _rn)                                   \
{                                                                             \
  thumb_decode_alu_op();                                                      \
  mips_emit_multu(arm_to_mips_reg[rd], arm_to_mips_reg[rs]);                  \
                                                                              \
  mips_emit_jal(mips_absolute_offset(execute_multiply_ticks));                \
  generate_load_reg(reg_a0, rs);                                              \
                                                                              \
  mips_emit_mflo(arm_to_mips_reg[rd]);                                        \
  generate_op_logic_flags(arm_to_mips_reg[rd]);                               \
                                                                              \
  cycle_count += pFETCH_WS16S(pc >> 24);                                      \
}                                                                             \

#define thumb_data_proc_test(type, name, rn_type, _rs, _rn)                   \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_generate_op_##rn_type(name, 0, _rs, _rn);                             \
                                                                              \
  cycle_count += pMEMORY_WS16S(pc >> 24);                                     \
}                                                                             \

#define thumb_data_proc_unary(type, name, rn_type, _rd, _rn)                  \
{                                                                             \
  thumb_decode_##type();                                                      \
  thumb_generate_op_##rn_type(name, _rd, 0, _rn);                             \
                                                                              \
  cycle_count += pMEMORY_WS16S(pc >> 24);                                     \
}                                                                             \

#define check_store_reg_pc_thumb(_rd)                                         \
  if (_rd == REG_PC)                                                          \
  {                                                                           \
    generate_function_call(execute_branch_ticks_thumb);                       \
    generate_indirect_branch_cycle_update(thumb);                             \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    cycle_count += pMEMORY_WS16S(pc >> 24);                                   \
  }                                                                           \

#define thumb_data_proc_hi(name)                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  u32 dest_rd = rd;                                                           \
  check_load_reg_pc(arm_reg_a0, rs, 4);                                       \
  check_load_reg_pc(arm_reg_a1, rd, 4);                                       \
  generate_op_##name##_reg(arm_to_mips_reg[dest_rd], arm_to_mips_reg[rd], arm_to_mips_reg[rs]); \
  check_store_reg_pc_thumb(dest_rd);                                          \
}                                                                             \

/*

#define thumb_data_proc_hi(name)                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  check_load_reg_pc(arm_reg_a0, rs, 4);                                       \
  check_load_reg_pc(arm_reg_a1, rd, 4);                                       \
  generate_op_##name##_reg(arm_to_mips_reg[rd], arm_to_mips_reg[rd], arm_to_mips_reg[rs]); \
  check_store_reg_pc_thumb(rd);                                               \
}                                                                             \

*/

#define thumb_data_proc_test_hi(name)                                         \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  check_load_reg_pc(arm_reg_a0, rs, 4);                                       \
  check_load_reg_pc(arm_reg_a1, rd, 4);                                       \
  generate_op_##name##_reg(reg_temp, arm_to_mips_reg[rd], arm_to_mips_reg[rs]); \
                                                                              \
  cycle_count += pMEMORY_WS16S(pc >> 24);                                     \
}                                                                             \

#define thumb_data_proc_mov_hi()                                              \
{                                                                             \
  thumb_decode_hireg_op();                                                    \
  check_load_reg_pc(arm_reg_a0, rs, 4);                                       \
  mips_emit_addu(arm_to_mips_reg[rd], arm_to_mips_reg[rs], reg_zero);         \
  check_store_reg_pc_thumb(rd);                                               \
}                                                                             \

#define thumb_load_pc(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  generate_load_pc(arm_to_mips_reg[_rd], ((pc & ~2) + 4 + (imm << 2)));       \
                                                                              \
  cycle_count += pMEMORY_WS16S(pc >> 24);                                     \
}                                                                             \

#define thumb_load_sp(_rd)                                                    \
{                                                                             \
  thumb_decode_imm();                                                         \
  mips_emit_addiu(arm_to_mips_reg[_rd], reg_r13, (imm << 2));                 \
                                                                              \
  cycle_count += pMEMORY_WS16S(pc >> 24);                                     \
}                                                                             \

#define thumb_adjust_sp(value)                                                \
{                                                                             \
  thumb_decode_add_sp();                                                      \
  mips_emit_addiu(reg_r13, reg_r13, (value));                                 \
                                                                              \
  cycle_count += pMEMORY_WS16S(pc >> 24);                                     \
}                                                                             \

// Decode types: shift, alu_op
// Operation types: lsl, lsr, asr, ror
// Affects N/Z/C flags

#define thumb_generate_shift_imm(name)                                        \
  if (check_generate_c_flag != 0)                                             \
  {                                                                           \
    generate_shift_imm_##name##_flags(rd, rs, imm);                           \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_shift_imm_##name##_no_flags(rd, rs, imm);                        \
  }                                                                           \
  if (rs != rd)                                                               \
  {                                                                           \
    mips_emit_addu(arm_to_mips_reg[rd], arm_to_mips_reg[rs], reg_zero);       \
  }                                                                           \

#define thumb_generate_shift_reg(name)                                        \
{                                                                             \
  u32 original_rd = rd;                                                       \
  if (check_generate_c_flag != 0)                                             \
  {                                                                           \
    generate_shift_reg_##name##_flags(rd, rs);                                \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    generate_shift_reg_##name##_no_flags(rd, rs);                             \
  }                                                                           \
                                                                              \
  mips_emit_addu(arm_to_mips_reg[original_rd], reg_a0, reg_zero);             \
}                                                                             \

/* 1S */
#define cycle_thumb_shift_shift()                                             \
  cycle_count += pMEMORY_WS16S(pc >> 24)                                      \

/* ALU operations LSL,LSR,ASR,ROR  1S+1I */
#define cycle_thumb_shift_alu_op()                                            \
  cycle_count += pFETCH_WS16S(pc >> 24) + 1                                   \

#define thumb_shift(decode_type, op_type, value_type)                         \
{                                                                             \
  thumb_decode_##decode_type();                                               \
                                                                              \
  thumb_generate_shift_##value_type(op_type);                                 \
  generate_op_logic_flags(arm_to_mips_reg[rd]);                               \
                                                                              \
  cycle_thumb_shift_##decode_type();                                          \
}                                                                             \

// Operation types: imm, mem_reg, mem_imm

/* LDR 1S+1N+1I, STR 2N */
#define thumb_access_memory_load(mem_type, reg_rd)                            \
  cycle_count += pFETCH_WS16S(pc >> 24) + 1;                                  \
  generate_load_pc(reg_a1, (pc + 4));                                         \
  generate_function_call_swap_delay(execute_load_##mem_type);                 \
  generate_store_reg(reg_rv, reg_rd);                                         \

#define thumb_access_memory_store(mem_type, reg_rd)                           \
  cycle_count += pFETCH_WS16N(pc >> 24);                                      \
  generate_load_pc(reg_a2, (pc + 2));                                         \
  generate_load_reg(reg_a1, reg_rd);                                          \
  mips_emit_jal(mips_absolute_offset(execute_store_##mem_type));              \
  generate_cycle_update_force();                                              \

#define thumb_access_memory_generate_address_pc_relative(offset, reg_rb, reg_ro) \
  generate_load_pc(reg_a0, (offset))                                          \

#define thumb_access_memory_generate_address_reg_imm(offset, reg_rb, reg_ro)  \
  mips_emit_addiu(reg_a0, arm_to_mips_reg[reg_rb], (offset))                  \

#define thumb_access_memory_generate_address_reg_imm_sp(offset, reg_rb, reg_ro) \
  mips_emit_addiu(reg_a0, arm_to_mips_reg[reg_rb], ((offset) << 2))           \

#define thumb_access_memory_generate_address_reg_reg(offset, reg_rb, reg_ro)  \
  mips_emit_addu(reg_a0, arm_to_mips_reg[reg_rb], arm_to_mips_reg[reg_ro])    \

#define thumb_access_memory(access_type, op_type, reg_rd, reg_rb, reg_ro, address_type, offset, mem_type) \
{                                                                             \
  thumb_decode_##op_type();                                                   \
  thumb_access_memory_generate_address_##address_type(offset, reg_rb, reg_ro);\
  thumb_access_memory_##access_type(mem_type, reg_rd);                        \
}                                                                             \


/* nS+1N+1I for LDM, (n-1)S+2N for STM */
/* nS+1N+1I (POP), (n+1)S+2N+1I (POP PC), (n-1)S+2N (PUSH) */
#define thumb_block_memory_load()                                             \
  generate_load_pc(reg_a1, (pc + 4));                                         \
  generate_function_call_swap_delay(execute_aligned_load32);                  \
  generate_store_reg(reg_rv, i);                                              \

#define thumb_block_memory_store()                                            \
  mips_emit_jal(mips_absolute_offset(execute_aligned_store32));               \
  generate_load_reg(reg_a1, i);                                               \

#define thumb_block_memory_final_load()                                       \
  generate_load_pc(reg_a1, (pc + 4));                                         \
  generate_function_call_swap_delay(execute_load_u32);                        \
  generate_store_reg(reg_rv, i);                                              \

#define thumb_block_memory_final_store()                                      \
  generate_load_pc(reg_a2, (pc + 2));                                         \
  generate_load_reg(reg_a1, i);                                               \
  mips_emit_jal(mips_absolute_offset(execute_store_u32));                     \
  generate_cycle_update_force();                                              \

#define thumb_block_memory_final_up_a(access_type)                            \
  thumb_block_memory_final_##access_type()                                    \

#define thumb_block_memory_final_down_b(access_type)                          \
  thumb_block_memory_final_##access_type()                                    \

#define thumb_block_memory_final_push_lr(access_type)                         \
  thumb_block_memory_##access_type()                                          \

#define thumb_block_memory_final_pop_pc(access_type)                          \
  thumb_block_memory_##access_type()                                          \

#define base_cycle_thumb_block_memory_load()                                  \
  cycle_count += pFETCH_WS16S(pc >> 24) + 1                                   \

#define base_cycle_thumb_block_memory_store()                                 \
  cycle_count += pFETCH_WS16N(pc >> 24)                                       \

#define thumb_block_memory_extra_up_a(access_type)                            \
  base_cycle_thumb_block_memory_##access_type()                               \

#define thumb_block_memory_extra_down_b(access_type)                          \
  base_cycle_thumb_block_memory_##access_type()                               \

#define thumb_block_memory_extra_push_lr(access_type)                         \
  mips_emit_addiu(reg_a0, reg_a2, (bit_count[reg_list] << 2));                \
  mips_emit_jal(mips_absolute_offset(execute_aligned_store32));               \
  generate_load_reg(reg_a1, REG_LR);                                          \
  base_cycle_thumb_block_memory_##access_type();                              \

#define thumb_block_memory_extra_pop_pc(access_type)                          \
  cycle_count++;                                                              \
  generate_load_pc(reg_a1, (pc + 4));                                         \
  mips_emit_jal(mips_absolute_offset(execute_aligned_load32));                \
  mips_emit_addiu(reg_a0, reg_a2, (bit_count[reg_list] << 2));                \
                                                                              \
  generate_mov(reg_a0, reg_rv);                                               \
  generate_indirect_branch_cycle_update(thumb);                               \

#define thumb_block_memory_sp_load()                                          \
  mips_emit_lw(arm_to_mips_reg[i], reg_a1, offset)                            \

#define thumb_block_memory_sp_store()                                         \
  mips_emit_sw(arm_to_mips_reg[i], reg_a1, offset)                            \

#define thumb_block_memory_sp_extra_up_a(access_type)                         \
  base_cycle_thumb_block_memory_##access_type()                               \

#define thumb_block_memory_sp_extra_down_b(access_type)                       \
  base_cycle_thumb_block_memory_##access_type()                               \

#define thumb_block_memory_sp_extra_push_lr(access_type)                      \
  mips_emit_sw(reg_r14, reg_a1, (bit_count[reg_list] << 2));                  \
  cycle_count++;                                                              \
  base_cycle_thumb_block_memory_##access_type();                              \

#define thumb_block_memory_sp_extra_pop_pc(access_type)                       \
  cycle_count++;                                                              \
  mips_emit_lw(reg_a0, reg_a1, (bit_count[reg_list] << 2));                   \
  generate_indirect_branch_cycle_update(thumb);                               \

#define thumb_block_memory_address_offset_up_a()                              \
  mips_emit_addu(reg_a2, base_reg, reg_zero)                                  \

#define thumb_block_memory_address_offset_down_b()                            \
  mips_emit_addiu(reg_a2, base_reg, (-(bit_count[reg_list] << 2)))            \

#define thumb_block_memory_address_offset_push_lr()                           \
  mips_emit_addiu(reg_a2, base_reg, (-((bit_count[reg_list] + 1) << 2)))      \

#define thumb_block_memory_address_offset_pop_pc()                            \
  mips_emit_addu(reg_a2, base_reg, reg_zero)                                  \

#define thumb_block_memory_address_writeback_up_a()                           \
  mips_emit_addiu(base_reg, base_reg, (bit_count[reg_list] << 2))             \

#define thumb_block_memory_address_writeback_down_b()                         \
  mips_emit_addiu(base_reg, base_reg, (-(bit_count[reg_list] << 2)))          \

#define thumb_block_memory_address_writeback_push_lr()                        \
  mips_emit_addiu(base_reg, base_reg, (-((bit_count[reg_list] + 1) << 2)))    \

#define thumb_block_memory_address_writeback_pop_pc()                         \
  mips_emit_addiu(base_reg, base_reg, ((bit_count[reg_list] + 1) << 2))       \

#define thumb_block_memory_offset_load(offset_type)                           \
  thumb_block_memory_address_offset_##offset_type();                          \
  if (((reg_list >> rn) & 0x01) == 0)                                         \
  {                                                                           \
    thumb_block_memory_address_writeback_##offset_type();                     \
    mips_emit_addu(arm_to_mips_reg[rn], base_reg, reg_zero);                  \
  }                                                                           \

#define thumb_block_memory_offset_store(offset_type)                          \
  thumb_block_memory_address_offset_##offset_type();                          \
  thumb_block_memory_address_writeback_##offset_type();                       \

#define thumb_block_memory_writeback_load()                                   \

#define thumb_block_memory_writeback_store()                                  \
  if (offset == 0)                                                            \
  {                                                                           \
    mips_emit_addu(arm_to_mips_reg[rn], base_reg, reg_zero);                  \
  }                                                                           \

#define thumb_block_memory_sp(access_type, offset_type)                       \
{                                                                             \
  thumb_decode_rlist();                                                       \
                                                                              \
  u32 i;                                                                      \
  u32 offset = 0;                                                             \
  u32 rn = REG_SP;                                                            \
  u32 base_reg = arm_to_mips_reg[rn];                                         \
                                                                              \
  thumb_block_memory_offset_##access_type(offset_type);                       \
                                                                              \
  if ((iwram_stack_optimize & option_stack_optimize) != 0)                    \
  {                                                                           \
    mips_emit_andi(reg_a1, reg_a2, 0x7FFC);                                   \
    generate_load_imm(reg_a0, (u32)iwram);                                    \
    generate_add(reg_a1, reg_a0);                                             \
                                                                              \
    for (i = 0; i < 8; i++)                                                   \
    {                                                                         \
      if (((reg_list >> i) & 0x01) != 0)                                      \
      {                                                                       \
        cycle_count++;                                                        \
        thumb_block_memory_sp_##access_type();                                \
        thumb_block_memory_writeback_##access_type();                         \
        offset += 4;                                                          \
      }                                                                       \
    }                                                                         \
                                                                              \
    thumb_block_memory_sp_extra_##offset_type(access_type);                   \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    mips_emit_ins(reg_a2, reg_zero, 0, 2);                                    \
                                                                              \
    for (i = 0; i < 8; i++)                                                   \
    {                                                                         \
      if (((reg_list >> i) & 0x01) != 0)                                      \
      {                                                                       \
        mips_emit_addiu(reg_a0, reg_a2, offset);                              \
                                                                              \
        if ((reg_list & ~((2 << i) - 1)) != 0)                                \
        {                                                                     \
          thumb_block_memory_##access_type();                                 \
          thumb_block_memory_writeback_##access_type();                       \
          offset += 4;                                                        \
        }                                                                     \
        else                                                                  \
        {                                                                     \
          thumb_block_memory_final_##offset_type(access_type);                \
          break;                                                              \
        }                                                                     \
      }                                                                       \
    }                                                                         \
                                                                              \
    thumb_block_memory_extra_##offset_type(access_type);                      \
  }                                                                           \
}                                                                             \

#define thumb_block_memory(access_type, offset_type)                          \
{                                                                             \
  thumb_decode_rlist();                                                       \
                                                                              \
  u32 i;                                                                      \
  u32 offset = 0;                                                             \
  u32 rn = (opcode >> 8) & 0x07;                                              \
  u32 base_reg = arm_to_mips_reg[rn];                                         \
                                                                              \
  thumb_block_memory_offset_##access_type(offset_type);                       \
                                                                              \
  mips_emit_ins(reg_a2, reg_zero, 0, 2);                                      \
                                                                              \
  for (i = 0; i < 8; i++)                                                     \
  {                                                                           \
    if (((reg_list >> i) & 0x01) != 0)                                        \
    {                                                                         \
      mips_emit_addiu(reg_a0, reg_a2, offset);                                \
                                                                              \
      if ((reg_list & ~((2 << i) - 1)) != 0)                                  \
      {                                                                       \
        thumb_block_memory_##access_type();                                   \
        thumb_block_memory_writeback_##access_type();                         \
        offset += 4;                                                          \
      }                                                                       \
      else                                                                    \
      {                                                                       \
        thumb_block_memory_final_##offset_type(access_type);                  \
        break;                                                                \
      }                                                                       \
    }                                                                         \
  }                                                                           \
                                                                              \
  thumb_block_memory_extra_##offset_type(access_type);                        \
}                                                                             \


/* condition true 2S+1N, false 1S */
#define thumb_conditional_branch(condition)                                   \
{                                                                             \
  u8 region = block_exits[block_exit_position].branch_target >> 24;           \
  cycle_count += 2 + (pMEMORY_WS16S(region) << 1) + pMEMORY_WS16N(region);    \
                                                                              \
  generate_condition_##condition();                                           \
  generate_branch_no_cycle_update(thumb,                                      \
   block_exits[block_exit_position].branch_source,                            \
   block_exits[block_exit_position].branch_target);                           \
  generate_branch_patch_conditional(backpatch_address, translation_ptr);      \
  block_exit_position++;                                                      \
}                                                                             \

/* condition false 1S */
#define arm_conditional_block_header()                                        \
{                                                                             \
  generate_condition();                                                       \
}                                                                             \


#define arm_b()                                                               \
{                                                                             \
  u8 region = block_exits[block_exit_position].branch_target >> 24;           \
  cycle_count += 2 + (pMEMORY_WS32S(region) << 1) + pMEMORY_WS32N(region);    \
                                                                              \
  generate_branch(arm);                                                       \
}                                                                             \

#define arm_bl()                                                              \
{                                                                             \
  generate_load_pc(reg_r14, (pc + 4));                                        \
                                                                              \
  u8 region = block_exits[block_exit_position].branch_target >> 24;           \
  cycle_count += 2 + (pMEMORY_WS32S(region) << 1) + pMEMORY_WS32N(region);    \
                                                                              \
  generate_branch(arm);                                                       \
}                                                                             \

#define arm_bx()                                                              \
{                                                                             \
  arm_decode_branchx();                                                       \
  generate_load_reg_pc(reg_a0, rn, 8);                                        \
  generate_indirect_branch_cycle_update(dual);                                \
}                                                                             \

#define arm_swi()                                                             \
{                                                                             \
  bios_read_protect = 0xe3a02004;                                             \
                                                                              \
  cycle_count += 2;                                                           \
  generate_cycle_update_force();                                              \
                                                                              \
  if (((opcode >> 16) & 0xFF) >= 0xF9)                                        \
  {                                                                           \
    /* VBA debug console code */                                              \
    /* Sizuku Advance (PD)(old version) to work */                            \
    break;                                                                    \
  }                                                                           \
  generate_load_pc(reg_a0, (pc + 4));                                         \
  generate_function_call_swap_delay(execute_swi);                             \
                                                                              \
  generate_branch_no_cycle_update(arm, block_exits[block_exit_position].branch_source, block_exits[block_exit_position].branch_target); \
  block_exit_position++;                                                      \
}                                                                             \

#define arm_und()                                                             \
{                                                                             \
}                                                                             \


#define thumb_b()                                                             \
{                                                                             \
  u8 region = block_exits[block_exit_position].branch_target >> 24;           \
  cycle_count += 2 + (pMEMORY_WS16S(region) << 1) + pMEMORY_WS16N(region);    \
                                                                              \
  generate_branch(thumb);                                                     \
}                                                                             \

// BL (First Instruction)
#define thumb_bll()                                                           \
{                                                                             \
  cycle_count += pMEMORY_WS16S(pc >> 24);                                     \
}                                                                             \

// BL (Second Instruction)
#define thumb_bl()                                                            \
{                                                                             \
  generate_load_pc(reg_r14, ((pc + 2) | 0x01));                               \
                                                                              \
  u8 region = block_exits[block_exit_position].branch_target >> 24;           \
  cycle_count += 2 + (pMEMORY_WS16S(region) << 1) + pMEMORY_WS16N(region);    \
                                                                              \
  generate_branch(thumb);                                                     \
}                                                                             \

// BL
#define thumb_blh()                                                           \
{                                                                             \
  thumb_decode_branch();                                                      \
                                                                              \
  generate_alu_imm(addiu, addu, reg_a0, reg_r14, (offset << 1));               \
  generate_load_pc(reg_r14, ((pc + 2) | 0x01));                               \
  generate_indirect_branch_cycle_update(thumb);                               \
}                                                                             \

#define thumb_bx()                                                            \
{                                                                             \
/*  thumb_decode_hireg_op(); */                                               \
  u8 rs = (opcode >> 3) & 0x0F;                                               \
                                                                              \
  generate_load_reg_pc(reg_a0, rs, 4);                                        \
  generate_indirect_branch_cycle_update(dual);                                \
}                                                                             \

#define thumb_swi()                                                           \
{                                                                             \
  bios_read_protect = 0xe3a02004;                                             \
                                                                              \
  cycle_count += 2;                                                           \
  generate_cycle_update_force();                                              \
                                                                              \
  if ((opcode & 0xFF) >= 0xF9)                                                \
  {                                                                           \
    /* VBA debug console code */                                              \
    break;                                                                    \
  }                                                                           \
  generate_load_pc(reg_a0, (pc + 2));                                         \
  generate_function_call_swap_delay(execute_swi);                             \
  /* SWI target == ARM */                                                     \
  generate_branch_no_cycle_update(arm, block_exits[block_exit_position].branch_source, block_exits[block_exit_position].branch_target); \
  block_exit_position++;                                                      \
}                                                                             \

#define thumb_und()                                                           \
{                                                                             \
}                                                                             \


#define generate_translation_gate(type)                                       \
  generate_load_pc(reg_a0, pc);                                               \
  generate_function_call_swap_delay(mips_indirect_branch_##type##_no_update_gba); \

#define generate_update_pc_reg()                                              \
  generate_load_pc(reg_a0, pc);                                               \
  mips_emit_sw(reg_a0, reg_base, (REG_PC * 4))                                \


#endif /* MIPS_EMIT_H */
