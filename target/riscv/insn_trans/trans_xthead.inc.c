/*
 * RISC-V translation routines for the T-Head Extend Instruction Set.
 *
 * Copyright (c) 2020 T-Head Limited, ltd
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define GET_GPR_R                    \
    TCGv src1, src2, dst;            \
    src1 = tcg_temp_new();           \
    src2 = tcg_temp_new();           \
    dst = tcg_temp_new();            \
    gen_get_gpr(src1, a->rs1);       \
    gen_get_gpr(src2, a->rs2);       \
    gen_get_gpr(dst, a->rd);

#define FREE_GPR_R                   \
    tcg_temp_free(src1);             \
    tcg_temp_free(src2);             \
    tcg_temp_free(dst);

#define SET_GPR_R  gen_set_gpr(a->rd, dst);

#define GET_GPR_R2                  \
    TCGv src1, dst;                 \
    src1 = tcg_temp_new();          \
    dst = tcg_temp_new();           \
    gen_get_gpr(src1, a->rs1);      \
    gen_get_gpr(dst, a->rd);

#define FREE_GPR_R2                 \
    tcg_temp_free(src1);            \
    tcg_temp_free(dst);

#define SET_GPR_R2                  \
    gen_set_gpr(a->rs1, src1);      \
    gen_set_gpr(a->rd, dst);

#define SET_GPR_RS1                 \
    gen_set_gpr(a->rs1, src1);

static bool trans_addsl(DisasContext *ctx, arg_addsl *a)
{
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(dst, src1, src2);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_srri(DisasContext *ctx, arg_srri *a)
{
    TCGv src1, dst;
    TCGv imm = tcg_temp_new();
    src1 = tcg_temp_new();
    dst = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst, a->rd);
    tcg_gen_movi_tl(imm, a->imm6);
    gen_helper_srri(dst, src1, imm);
    SET_GPR_R
    tcg_temp_free(imm);
    tcg_temp_free(src1);
    tcg_temp_free(dst);
    return true;
}

static bool trans_tstnbz(DisasContext *ctx, arg_tstnbz *a)
{
    TCGv src1, dst;
    src1 = tcg_temp_new();
    dst = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst, a->rd);
    gen_helper_tstnbz(dst, src1);
    SET_GPR_R
    tcg_temp_free(src1);
    tcg_temp_free(dst);
    return true;
}

static bool trans_rev(DisasContext *ctx, arg_rev *a)
{
    TCGv src1, dst;
    src1 = tcg_temp_new();
    dst = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst, a->rd);

    tcg_gen_bswap_tl(dst, src1);
    SET_GPR_R
    tcg_temp_free(src1);
    tcg_temp_free(dst);
    return true;
}

static bool trans_ff0(DisasContext *ctx, arg_ff0 *a)
{
    TCGv src1, dst;
    src1 = tcg_temp_new();
    dst = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst, a->rd);
    gen_helper_ff0(dst, src1);
    SET_GPR_R
    tcg_temp_free(src1);
    tcg_temp_free(dst);
    return true;
}

static bool trans_ff1(DisasContext *ctx, arg_ff1 *a)
{
    TCGv src1, dst;
    src1 = tcg_temp_new();
    dst = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst, a->rd);
    gen_helper_ff1(dst, src1);
    SET_GPR_R
    tcg_temp_free(src1);
    tcg_temp_free(dst);
    return true;
}

static bool trans_tst(DisasContext *ctx, arg_tst *a)
{
    TCGv src1, dst;
    TCGv imm = tcg_temp_new();
    src1 = tcg_temp_new();
    dst = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst, a->rd);
    tcg_gen_movi_tl(imm, a->imm6);
    gen_helper_tst(dst, src1, imm);
    SET_GPR_R
    tcg_temp_free(imm);
    tcg_temp_free(src1);
    tcg_temp_free(dst);
    return true;
}

static bool trans_mveqz(DisasContext *ctx, arg_mveqz *a)
{
    TCGv src1 ,src2, dst;
    TCGLabel *l1 = NULL;
    l1 = gen_new_label();
    src1 = tcg_temp_local_new();
    dst = tcg_temp_local_new();
    src2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(src2, a->rs2);
    gen_get_gpr(dst, a->rd);
    tcg_gen_brcondi_tl(TCG_COND_NE, src2, 0, l1);
    tcg_gen_mov_tl(dst, src1);
    SET_GPR_R
    gen_set_label(l1);
    FREE_GPR_R
    return true;
}

static bool trans_mvnez(DisasContext *ctx, arg_mvnez *a)
{
    TCGv src1 ,src2, dst;
    TCGLabel *l1 = NULL;
    l1 = gen_new_label();
    src1 = tcg_temp_local_new();
    dst = tcg_temp_local_new();
    src2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(src2, a->rs2);
    gen_get_gpr(dst, a->rd);
    tcg_gen_brcondi_tl(TCG_COND_EQ, src2, 0, l1);
    tcg_gen_mov_tl(dst, src1);
    SET_GPR_R
    gen_set_label(l1);
    FREE_GPR_R
    return true;
}

static bool trans_mula(DisasContext *ctx, arg_mula *a)
{
    GET_GPR_R
    tcg_gen_mulu2_tl(src1, src2, src1, src2);
    tcg_gen_add_tl(dst, dst, src1);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_muls(DisasContext *ctx, arg_muls *a)
{
    GET_GPR_R
    tcg_gen_mulu2_tl(src1, src2, src1, src2);
    tcg_gen_sub_tl(dst, dst, src1);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_mulah(DisasContext *ctx, arg_mulah *a)
{
    GET_GPR_R
    gen_helper_mulah(dst, src1, src2, dst);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_mulsh(DisasContext *ctx, arg_mulsh *a)
{
    GET_GPR_R
    gen_helper_mulsh(dst, src1, src2, dst);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_ext(DisasContext *ctx, arg_ext *a)
{
    TCGv src1, dst;
    src1 = tcg_temp_new();
    dst = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst, a->rd);
    if (a->msb < a->lsb) {
        tcg_gen_movi_tl(dst, 0);
    } else {
        tcg_gen_shli_tl(src1, src1, TARGET_LONG_BITS - 1 - a->msb);
        tcg_gen_sari_tl(dst, src1, TARGET_LONG_BITS - 1 - a->msb + a->lsb);
    }
    SET_GPR_R
    tcg_temp_free(src1);
    tcg_temp_free(dst);
    return true;
}

static bool trans_extu(DisasContext *ctx, arg_extu *a)
{
    TCGv src1, dst;
    src1 = tcg_temp_new();
    dst = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst, a->rd);
    if (a->msb < a->lsb) {
        tcg_gen_movi_tl(dst, 0);
    } else {
        tcg_gen_shli_tl(src1, src1, TARGET_LONG_BITS - 1 - a->msb);
        tcg_gen_shri_tl(dst, src1, TARGET_LONG_BITS - 1 - a->msb + a->lsb);
    }
    SET_GPR_R
    tcg_temp_free(src1);
    tcg_temp_free(dst);
    return true;
}

static bool trans_lrb(DisasContext *ctx, arg_lrb *a)
{
    int memop = MO_SB;
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lrh(DisasContext *ctx, arg_lrh *a)
{
    int memop = MO_TESW;
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lrw(DisasContext *ctx, arg_lrw *a)
{
    int memop = MO_TESL;
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lrbu(DisasContext *ctx, arg_lrbu *a)
{
    int memop = MO_UB;
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lrhu(DisasContext *ctx, arg_lrhu *a)
{
    int memop = MO_TEUW;
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static inline target_long sign_ext(int imm, int len)
{
    target_long temp = imm;
    int width = sizeof(target_long) * 8 - len;

    return (temp << width) >> width;
}

static bool trans_lbib(DisasContext *ctx, arg_lbib *a)
{
    int memop = MO_SB;
    GET_GPR_R2
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lhib(DisasContext *ctx, arg_lhib *a)
{
    int memop = MO_TESW;
    GET_GPR_R2
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lwib(DisasContext *ctx, arg_lwib *a)
{
    int memop = MO_TESL;
    GET_GPR_R2
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lbuib(DisasContext *ctx, arg_lbuib *a)
{
    int memop = MO_UB;
    GET_GPR_R2
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lhuib(DisasContext *ctx, arg_lhuib *a)
{
    int memop = MO_TEUW;
    GET_GPR_R2
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lbia(DisasContext *ctx, arg_lbia *a)
{
    int memop = MO_SB;
    GET_GPR_R2
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lhia(DisasContext *ctx, arg_lhia *a)
{
    int memop = MO_TESW;
    GET_GPR_R2
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lwia(DisasContext *ctx, arg_lwia *a)
{
    int memop = MO_TESL;
    GET_GPR_R2
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lbuia(DisasContext *ctx, arg_lbuia *a)
{
    int memop = MO_UB;
    GET_GPR_R2
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lhuia(DisasContext *ctx, arg_lhuia *a)
{
    int memop = MO_TEUW;
    GET_GPR_R2
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lwd(DisasContext *ctx, arg_lwd *a)
{
    uint8_t offset = a->imm2 << 3;
    TCGv src1, dst1, dst2;
    src1 = tcg_temp_new();
    dst1 = tcg_temp_new();
    dst2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    tcg_gen_addi_tl(src1, src1, offset);
    tcg_gen_qemu_ld_tl(dst1, src1, ctx->mem_idx, MO_TESL);
    tcg_gen_addi_tl(src1,src1, 4);
    tcg_gen_qemu_ld_tl(dst2, src1, ctx->mem_idx, MO_TESL);
    gen_set_gpr(a->rd1, dst1);
    gen_set_gpr(a->rd2, dst2);
    tcg_temp_free(src1);
    tcg_temp_free(dst1);
    tcg_temp_free(dst2);
    return true;
}

static bool trans_srb(DisasContext *ctx, arg_srb *a)
{
    int memop = MO_SB;
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    FREE_GPR_R
    return true;
}

static bool trans_srh(DisasContext *ctx, arg_srh *a)
{
    int memop = MO_TESW;
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    FREE_GPR_R
    return true;
}

static bool trans_srw(DisasContext *ctx, arg_srw *a)
{
    int memop = MO_TESL;
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    FREE_GPR_R
    return true;
}

static bool trans_sbia(DisasContext *ctx, arg_sbia *a)
{
    int memop = MO_SB;
    GET_GPR_R2
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    SET_GPR_RS1
    FREE_GPR_R2
    return true;
}

static bool trans_shia(DisasContext *ctx, arg_shia *a)
{
    int memop = MO_TESW;
    GET_GPR_R2
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    SET_GPR_RS1
    FREE_GPR_R2
    return true;
}

static bool trans_swia(DisasContext *ctx, arg_swia *a)
{
    int memop = MO_TESL;
    GET_GPR_R2
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    SET_GPR_RS1
    FREE_GPR_R2
    return true;
}

static bool trans_sbib(DisasContext *ctx, arg_sbib *a)
{
    int memop = MO_SB;
    GET_GPR_R2
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_RS1
    FREE_GPR_R2
    return true;
}

static bool trans_shib(DisasContext *ctx, arg_shib *a)
{
    int memop = MO_TESW;
    GET_GPR_R2
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_RS1
    FREE_GPR_R2
    return true;
}

static bool trans_swib(DisasContext *ctx, arg_swib *a)
{
    int memop = MO_TESL;
    GET_GPR_R2
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_RS1
    FREE_GPR_R2
    return true;
}

static bool trans_swd(DisasContext *ctx, arg_swd *a)
{
    uint8_t offset = a->imm2 << 3;
    TCGv src1, dst1, dst2;
    src1 = tcg_temp_new();
    dst1 = tcg_temp_new();
    dst2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst1, a->rd1);
    gen_get_gpr(dst2, a->rd2);
    tcg_gen_addi_tl(src1, src1, offset);
    tcg_gen_qemu_st_tl(dst1, src1, ctx->mem_idx, MO_TESL);
    tcg_gen_addi_tl(src1, src1, 4);
    tcg_gen_qemu_st_tl(dst2, src1, ctx->mem_idx, MO_TESL);
    tcg_temp_free(src1);
    tcg_temp_free(dst1);
    tcg_temp_free(dst2);
    return true;
}

static bool trans_flrw(DisasContext *ctx, arg_flrw *a)
{
    TCGv src1, src2;
    TCGv_i64 one;
    if (ctx->mstatus_fs == 0) {
        gen_exception_illegal(ctx);
    }
    src1 = tcg_temp_new();
    src2 = tcg_temp_new();
    one = tcg_const_i64(0xffffffff);
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(src2, a->rs2);

    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_i64(cpu_fpr[a->rd], src1, ctx->mem_idx, MO_TEUL);
    tcg_gen_deposit_i64(cpu_fpr[a->rd], cpu_fpr[a->rd], one, 32, 32);

    tcg_temp_free(src1);
    tcg_temp_free(src2);
    tcg_temp_free_i64(one);
    mark_fs_dirty(ctx);
    return true;
}

static bool trans_flrd(DisasContext *ctx, arg_flrd *a)
{
    TCGv src1, src2;
    if (ctx->mstatus_fs == 0) {
        gen_exception_illegal(ctx);
    }
    src1 = tcg_temp_new();
    src2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(src2, a->rs2);

    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_i64(cpu_fpr[a->rd], src1, ctx->mem_idx, MO_TEQ);

    tcg_temp_free(src1);
    tcg_temp_free(src2);
    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fsrw(DisasContext *ctx, arg_fsrw *a)
{
    TCGv src1, src2;
    if (ctx->mstatus_fs == 0) {
        gen_exception_illegal(ctx);
    }
    src1 = tcg_temp_new();
    src2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(src2, a->rs2);

    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_i64(cpu_fpr[a->rd], src1, ctx->mem_idx, MO_TEUL);

    tcg_temp_free(src1);
    tcg_temp_free(src2);
    return true;
}

static bool trans_fsrd(DisasContext *ctx, arg_fsrd *a)
{
    TCGv src1, src2;
    if (ctx->mstatus_fs == 0) {
        gen_exception_illegal(ctx);
    }
    src1 = tcg_temp_new();
    src2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(src2, a->rs2);

    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_i64(cpu_fpr[a->rd], src1, ctx->mem_idx, MO_TEQ);

    tcg_temp_free(src1);
    tcg_temp_free(src2);
    return true;
}

static bool trans_ignore(DisasContext *ctx, arg_ignore *a)
{
    tcg_gen_movi_tl(cpu_pc, ctx->pc_succ_insn);
    exit_tb(ctx);
    ctx->base.is_jmp = DISAS_NORETURN;
    return true;
}

static bool trans_srriw(DisasContext *ctx, arg_srriw *a)
{
    REQUIRE_64BIT(ctx);
    TCGv src1, dst;
    TCGv imm = tcg_temp_new();
    src1 = tcg_temp_new();
    dst = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst, a->rd);
    tcg_gen_movi_tl(imm, a->imm5);
    gen_helper_srriw(dst, src1, imm);
    SET_GPR_R
    tcg_temp_free(imm);
    tcg_temp_free(src1);
    tcg_temp_free(dst);
    return true;
}

static bool trans_revw(DisasContext *ctx, arg_revw *a)
{
    REQUIRE_64BIT(ctx);
    TCGv src1, dst;
    src1 = tcg_temp_new();
    dst = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst, a->rd);

    tcg_gen_bswap32_tl(dst, src1, TCG_BSWAP_OS);
    SET_GPR_R
    tcg_temp_free(src1);
    tcg_temp_free(dst);
    return true;
}

static bool trans_mulaw(DisasContext *ctx, arg_mulaw *a)
{
    REQUIRE_64BIT(ctx);
    GET_GPR_R
    tcg_gen_ext32u_tl(src1, src1);
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_ext32u_tl(dst, dst);
    tcg_gen_muls2_tl(src1, src2, src1, src2);
    tcg_gen_ext32u_tl(src1, src1);
    tcg_gen_add_tl(dst, dst, src1);
    tcg_gen_ext32s_tl(dst, dst);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_mulsw(DisasContext *ctx, arg_mulsw *a)
{
    REQUIRE_64BIT(ctx);
    GET_GPR_R
    tcg_gen_ext32u_tl(src1, src1);
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_ext32u_tl(dst, dst);
    tcg_gen_muls2_tl(src1, src2, src1, src2);
    tcg_gen_ext32u_tl(src1, src1);
    tcg_gen_sub_tl(dst, dst, src1);
    tcg_gen_ext32s_tl(dst, dst);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lrd(DisasContext *ctx, arg_lrd *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEQ;
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lrwu(DisasContext *ctx, arg_lrwu *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEUL;
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lurb(DisasContext *ctx, arg_lurb *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_SB;
    GET_GPR_R
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lurh(DisasContext *ctx, arg_lurh *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TESW;
    GET_GPR_R
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lurw(DisasContext *ctx, arg_lurw *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TESL;
    GET_GPR_R
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lurd(DisasContext *ctx, arg_lurd *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEQ;
    GET_GPR_R
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lurbu(DisasContext *ctx, arg_lurbu *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_UB;
    GET_GPR_R
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lurhu(DisasContext *ctx, arg_lurhu *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEUW;
    GET_GPR_R
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}

static bool trans_lurwu(DisasContext *ctx, arg_lurwu *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEUL;
    GET_GPR_R
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R
    FREE_GPR_R
    return true;
}
static bool trans_ldia(DisasContext *ctx, arg_ldia *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEQ;
    GET_GPR_R2
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_ldib(DisasContext *ctx, arg_ldib *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEQ;
    GET_GPR_R2
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lwuia(DisasContext *ctx, arg_lwuia *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEUL;
    GET_GPR_R2
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lwuib(DisasContext *ctx, arg_lwuib *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEUL;
    GET_GPR_R2
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    tcg_gen_qemu_ld_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_R2
    FREE_GPR_R2
    return true;
}

static bool trans_lwud(DisasContext *ctx, arg_lwud *a)
{
    REQUIRE_64BIT(ctx);
    uint8_t offset = a->imm2 << 3;
    TCGv src1, dst1, dst2;
    src1 = tcg_temp_new();
    dst1 = tcg_temp_new();
    dst2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    tcg_gen_addi_tl(src1, src1, offset);
    tcg_gen_qemu_ld_tl(dst1, src1, ctx->mem_idx, MO_TEUL);
    tcg_gen_addi_tl(src1, src1, 4);
    tcg_gen_qemu_ld_tl(dst2, src1, ctx->mem_idx, MO_TEUL);
    gen_set_gpr(a->rd1, dst1);
    gen_set_gpr(a->rd2, dst2);
    tcg_temp_free(src1);
    tcg_temp_free(dst1);
    tcg_temp_free(dst2);
    return true;
}

static bool trans_ldd(DisasContext *ctx, arg_ldd *a)
{
    REQUIRE_64BIT(ctx);
    uint8_t offset = a->imm2 << 4;
    TCGv src1, dst1, dst2;
    src1 = tcg_temp_new();
    dst1 = tcg_temp_new();
    dst2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    tcg_gen_addi_tl(src1, src1, offset);
    tcg_gen_qemu_ld_tl(dst1, src1, ctx->mem_idx, MO_TEQ);
    tcg_gen_addi_tl(src1, src1, 8);
    tcg_gen_qemu_ld_tl(dst2, src1, ctx->mem_idx, MO_TEQ);
    gen_set_gpr(a->rd1, dst1);
    gen_set_gpr(a->rd2, dst2);
    tcg_temp_free(src1);
    tcg_temp_free(dst1);
    tcg_temp_free(dst2);
    return true;
}

static bool trans_srd(DisasContext *ctx, arg_srd *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEQ;
    GET_GPR_R
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    FREE_GPR_R
    return true;
}

static bool trans_surb(DisasContext *ctx, arg_surb *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_SB;
    GET_GPR_R
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    FREE_GPR_R
    return true;
}

static bool trans_surh(DisasContext *ctx, arg_surh *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TESW;
    GET_GPR_R
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    FREE_GPR_R
    return true;
}

static bool trans_surw(DisasContext *ctx, arg_surw *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TESL;
    GET_GPR_R
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    FREE_GPR_R
    return true;
}

static bool trans_surd(DisasContext *ctx, arg_surd *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEQ;
    GET_GPR_R
    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    FREE_GPR_R
    return true;
}

static bool trans_sdia(DisasContext *ctx, arg_sdia *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEQ;
    GET_GPR_R2
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    SET_GPR_RS1
    FREE_GPR_R2
    return true;
}

static bool trans_sdib(DisasContext *ctx, arg_sdib *a)
{
    REQUIRE_64BIT(ctx);
    int memop = MO_TEQ;
    GET_GPR_R2
    tcg_gen_addi_tl(src1, src1, sign_ext(a->imm5, 5) << a->imm2);
    tcg_gen_qemu_st_tl(dst, src1, ctx->mem_idx, memop);
    SET_GPR_RS1
    FREE_GPR_R2
    return true;
}

static bool trans_sdd(DisasContext *ctx, arg_sdd *a)
{
    REQUIRE_64BIT(ctx);
    uint8_t offset = a->imm2 << 4;
    TCGv src1, dst1, dst2;
    src1 = tcg_temp_new();
    dst1 = tcg_temp_new();
    dst2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(dst1, a->rd1);
    gen_get_gpr(dst2, a->rd2);
    tcg_gen_addi_tl(src1, src1, offset);
    tcg_gen_qemu_st_tl(dst1, src1, ctx->mem_idx, MO_TEQ);
    tcg_gen_addi_tl(src1, src1, 8);
    tcg_gen_qemu_st_tl(dst2, src1, ctx->mem_idx, MO_TEQ);
    tcg_temp_free(src1);
    tcg_temp_free(dst1);
    tcg_temp_free(dst2);
    return true;
}

static bool trans_flurw(DisasContext *ctx, arg_flurw *a)
{
    REQUIRE_64BIT(ctx);
    TCGv src1, src2;
    if (ctx->mstatus_fs == 0) {
        gen_exception_illegal(ctx);
    }
    src1 = tcg_temp_new();
    src2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(src2, a->rs2);

    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_i64(cpu_fpr[a->rd], src1, ctx->mem_idx, MO_TEUL);
    gen_nanbox_s(cpu_fpr[a->rd], cpu_fpr[a->rd]);

    tcg_temp_free(src1);
    tcg_temp_free(src2);
    mark_fs_dirty(ctx);
    return true;
}

static bool trans_flurd(DisasContext *ctx, arg_flurd *a)
{
    REQUIRE_64BIT(ctx);
    TCGv src1, src2;
    if (ctx->mstatus_fs == 0) {
        gen_exception_illegal(ctx);
    }
    src1 = tcg_temp_new();
    src2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(src2, a->rs2);

    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_ld_i64(cpu_fpr[a->rd], src1, ctx->mem_idx, MO_TEQ);

    tcg_temp_free(src1);
    tcg_temp_free(src2);
    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fsurw(DisasContext *ctx, arg_fsurw *a)
{
    REQUIRE_64BIT(ctx);
    TCGv src1, src2;
    if (ctx->mstatus_fs == 0) {
        gen_exception_illegal(ctx);
    }
    src1 = tcg_temp_new();
    src2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(src2, a->rs2);

    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_i64(cpu_fpr[a->rd], src1, ctx->mem_idx, MO_TEUL);

    tcg_temp_free(src1);
    tcg_temp_free(src2);
    return true;
}

static bool trans_fsurd(DisasContext *ctx, arg_fsurd *a)
{
    REQUIRE_64BIT(ctx);
    TCGv src1, src2;
    if (ctx->mstatus_fs == 0) {
        gen_exception_illegal(ctx);
    }
    src1 = tcg_temp_new();
    src2 = tcg_temp_new();
    gen_get_gpr(src1, a->rs1);
    gen_get_gpr(src2, a->rs2);

    tcg_gen_ext32u_tl(src2, src2);
    tcg_gen_shli_tl(src2, src2, a->imm2);
    tcg_gen_add_tl(src1, src1, src2);
    tcg_gen_qemu_st_i64(cpu_fpr[a->rd], src1, ctx->mem_idx, MO_TEQ);

    tcg_temp_free(src1);
    tcg_temp_free(src2);
    return true;
}

static bool trans_flh(DisasContext *ctx, arg_flh *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);
    TCGv t0 = tcg_temp_new();
    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, a->imm);

    tcg_gen_qemu_ld_i64(cpu_fpr[a->rd], t0, ctx->mem_idx, MO_TEUW);
    gen_nanbox_h(cpu_fpr[a->rd], cpu_fpr[a->rd]);

    tcg_temp_free(t0);
    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fsh(DisasContext *ctx, arg_fsh *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    TCGv t0 = tcg_temp_new();
    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, a->imm);

    tcg_gen_qemu_st_i64(cpu_fpr[a->rs2], t0, ctx->mem_idx, MO_TEUW);

    tcg_temp_free(t0);
    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fmadd_h(DisasContext *ctx, arg_fmadd_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fmadd_h(cpu_fpr[a->rd], cpu_env, cpu_fpr[a->rs1],
                       cpu_fpr[a->rs2], cpu_fpr[a->rs3]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fmsub_h(DisasContext *ctx, arg_fmsub_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fmsub_h(cpu_fpr[a->rd], cpu_env, cpu_fpr[a->rs1],
                       cpu_fpr[a->rs2], cpu_fpr[a->rs3]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fnmsub_h(DisasContext *ctx, arg_fnmsub_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fnmsub_h(cpu_fpr[a->rd], cpu_env, cpu_fpr[a->rs1],
                        cpu_fpr[a->rs2], cpu_fpr[a->rs3]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fnmadd_h(DisasContext *ctx, arg_fnmadd_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fnmadd_h(cpu_fpr[a->rd], cpu_env, cpu_fpr[a->rs1],
                        cpu_fpr[a->rs2], cpu_fpr[a->rs3]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fadd_h(DisasContext *ctx, arg_fadd_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fadd_h(cpu_fpr[a->rd], cpu_env,
                      cpu_fpr[a->rs1], cpu_fpr[a->rs2]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fsub_h(DisasContext *ctx, arg_fsub_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fsub_h(cpu_fpr[a->rd], cpu_env,
                      cpu_fpr[a->rs1], cpu_fpr[a->rs2]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fmul_h(DisasContext *ctx, arg_fmul_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fmul_h(cpu_fpr[a->rd], cpu_env,
                      cpu_fpr[a->rs1], cpu_fpr[a->rs2]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fdiv_h(DisasContext *ctx, arg_fdiv_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fdiv_h(cpu_fpr[a->rd], cpu_env,
                      cpu_fpr[a->rs1], cpu_fpr[a->rs2]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fsqrt_h(DisasContext *ctx, arg_fsqrt_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fsqrt_h(cpu_fpr[a->rd], cpu_env, cpu_fpr[a->rs1]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fmv_x_h(DisasContext *ctx, arg_fmv_x_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);
    TCGv_i64 t_64 = tcg_temp_new_i64();
    TCGv t = tcg_temp_new();

    tcg_gen_ext16s_i64(t_64, cpu_fpr[a->rs1]);
    tcg_gen_trunc_i64_tl(t, t_64);
    gen_set_gpr(a->rd, t);

    tcg_temp_free(t);
    tcg_temp_free_i64(t_64);
    return true;
}

static bool trans_fmv_h_x(DisasContext *ctx, arg_fmv_h_x *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);
    TCGv t0 = tcg_temp_new();
    gen_get_gpr(t0, a->rs1);

    tcg_gen_extu_tl_i64(cpu_fpr[a->rd], t0);
    gen_nanbox_h(cpu_fpr[a->rd], cpu_fpr[a->rd]);

    mark_fs_dirty(ctx);
    tcg_temp_free(t0);
    return true;
}

static bool trans_fsgnj_h(DisasContext *ctx, arg_fsgnj_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    if (a->rs1 == a->rs2) { /* FMOV */
        gen_check_nanbox_h(cpu_fpr[a->rd], cpu_fpr[a->rs1]);
    } else { /* FSGNJ */
	    TCGv_i64 rs1 = tcg_temp_new_i64();
        TCGv_i64 rs2 = tcg_temp_new_i64();

        gen_check_nanbox_h(rs1, cpu_fpr[a->rs1]);
        gen_check_nanbox_h(rs2, cpu_fpr[a->rs2]);

        /* This formulation retains the nanboxing of rs2. */
        tcg_gen_deposit_i64(cpu_fpr[a->rd], rs2, rs1, 0, 15);
        tcg_temp_free_i64(rs1);
        tcg_temp_free_i64(rs2);
    }

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fsgnjn_h(DisasContext *ctx, arg_fsgnjn_h *a)
{
    TCGv_i64 rs1, rs2, mask;
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    rs1 = tcg_temp_new_i64();
    gen_check_nanbox_h(rs1, cpu_fpr[a->rs1]);

    if (a->rs1 == a->rs2) { /* FNEG */
        tcg_gen_xori_i64(cpu_fpr[a->rd], rs1, MAKE_64BIT_MASK(15, 1));
    } else {
        rs2 = tcg_temp_new_i64();
        gen_check_nanbox_h(rs2, cpu_fpr[a->rs2]);

        /*
         * Replace bit 15 in rs1 with inverse in rs2.
         * This formulation retains the nanboxing of rs1.
         */
        mask = tcg_const_i64(~MAKE_64BIT_MASK(15, 1));
        tcg_gen_nor_i64(rs2, rs2, mask);
        tcg_gen_and_i64(rs1, mask, rs1);
        tcg_gen_or_i64(cpu_fpr[a->rd], rs1, rs2);

        tcg_temp_free_i64(mask);
        tcg_temp_free_i64(rs2);
    }
    tcg_temp_free_i64(rs1);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fsgnjx_h(DisasContext *ctx, arg_fsgnjx_h *a)
{
    TCGv_i64 rs1, rs2;
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    rs1 = tcg_temp_new_i64();
    gen_check_nanbox_h(rs1, cpu_fpr[a->rs1]);

    if (a->rs1 == a->rs2) { /* FABS */
        tcg_gen_andi_i64(cpu_fpr[a->rd], rs1, ~MAKE_64BIT_MASK(15, 1));
    } else {
        rs2 = tcg_temp_new_i64();
        gen_check_nanbox_s(rs2, cpu_fpr[a->rs2]);

        /*
         * Xor bit 15 in rs1 with that in rs2.
         * This formulation retains the nanboxing of rs1.
         */
        tcg_gen_andi_i64(rs2, rs2, MAKE_64BIT_MASK(15, 1));
        tcg_gen_xor_i64(cpu_fpr[a->rd], rs1, rs2);

        tcg_temp_free_i64(rs2);
    }
    tcg_temp_free_i64(rs1);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fmin_h(DisasContext *ctx, arg_fmin_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_helper_fmin_h(cpu_fpr[a->rd], cpu_env, cpu_fpr[a->rs1],
                      cpu_fpr[a->rs2]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fmax_h(DisasContext *ctx, arg_fmax_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_helper_fmax_h(cpu_fpr[a->rd], cpu_env, cpu_fpr[a->rs1],
                      cpu_fpr[a->rs2]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fcvt_w_h(DisasContext *ctx, arg_fcvt_w_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    TCGv t0 = tcg_temp_new();
    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_w_h(t0, cpu_env, cpu_fpr[a->rs1]);
    gen_set_gpr(a->rd, t0);
    tcg_temp_free(t0);

    return true;
}

static bool trans_fcvt_wu_h(DisasContext *ctx, arg_fcvt_wu_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    TCGv t0 = tcg_temp_new();
    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_wu_h(t0, cpu_env, cpu_fpr[a->rs1]);
    gen_set_gpr(a->rd, t0);
    tcg_temp_free(t0);

    return true;
}

static bool trans_feq_h(DisasContext *ctx, arg_feq_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);
    TCGv t0 = tcg_temp_new();
    gen_helper_feq_h(t0, cpu_env, cpu_fpr[a->rs1], cpu_fpr[a->rs2]);
    gen_set_gpr(a->rd, t0);
    tcg_temp_free(t0);
    return true;
}

static bool trans_flt_h(DisasContext *ctx, arg_flt_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    TCGv t0 = tcg_temp_new();
    gen_helper_flt_h(t0, cpu_env, cpu_fpr[a->rs1], cpu_fpr[a->rs2]);
    gen_set_gpr(a->rd, t0);
    tcg_temp_free(t0);

    return true;
}

static bool trans_fle_h(DisasContext *ctx, arg_fle_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    TCGv t0 = tcg_temp_new();
    gen_helper_fle_h(t0, cpu_env, cpu_fpr[a->rs1], cpu_fpr[a->rs2]);
    gen_set_gpr(a->rd, t0);

    tcg_temp_free(t0);
    return true;
}

static bool trans_fclass_h(DisasContext *ctx, arg_fclass_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);
    TCGv t0 = tcg_temp_new();

    gen_helper_fclass_h(t0, cpu_fpr[a->rs1]);
    gen_set_gpr(a->rd, t0);

    tcg_temp_free(t0);
    return true;
}

static bool trans_fcvt_h_w(DisasContext *ctx, arg_fcvt_h_w *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    TCGv t0 = tcg_temp_new();
    gen_get_gpr(t0, a->rs1);

    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_h_w(cpu_fpr[a->rd], cpu_env, t0);

    mark_fs_dirty(ctx);
    tcg_temp_free(t0);

    return true;
}

static bool trans_fcvt_h_wu(DisasContext *ctx, arg_fcvt_h_wu *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    TCGv t0 = tcg_temp_new();
    gen_get_gpr(t0, a->rs1);

    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_h_wu(cpu_fpr[a->rd], cpu_env, t0);

    mark_fs_dirty(ctx);
    tcg_temp_free(t0);
    return true;
}

static bool trans_fcvt_l_h(DisasContext *ctx, arg_fcvt_l_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    TCGv t0 = tcg_temp_new();
    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_l_h(t0, cpu_env, cpu_fpr[a->rs1]);
    gen_set_gpr(a->rd, t0);

    tcg_temp_free(t0);
    return true;
}

static bool trans_fcvt_lu_h(DisasContext *ctx, arg_fcvt_lu_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    TCGv t0 = tcg_temp_new();
    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_lu_h(t0, cpu_env, cpu_fpr[a->rs1]);
    gen_set_gpr(a->rd, t0);

    tcg_temp_free(t0);
    return true;
}

static bool trans_fcvt_h_l(DisasContext *ctx, arg_fcvt_h_l *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    TCGv t0 = tcg_temp_new();
    gen_get_gpr(t0, a->rs1);

    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_h_l(cpu_fpr[a->rd], cpu_env, t0);

    mark_fs_dirty(ctx);
    tcg_temp_free(t0);
    return true;
}

static bool trans_fcvt_h_lu(DisasContext *ctx, arg_fcvt_h_lu *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    TCGv t0 = tcg_temp_new();
    gen_get_gpr(t0, a->rs1);

    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_h_lu(cpu_fpr[a->rd], cpu_env, t0);

    mark_fs_dirty(ctx);
    tcg_temp_free(t0);
    return true;
}

static bool trans_fcvt_s_h(DisasContext *ctx, arg_fcvt_s_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_s_h(cpu_fpr[a->rd], cpu_env, cpu_fpr[a->rs1]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fcvt_h_s(DisasContext *ctx, arg_fcvt_h_s *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_h_s(cpu_fpr[a->rd], cpu_env, cpu_fpr[a->rs1]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fcvt_h_d(DisasContext *ctx, arg_fcvt_h_d *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_h_d(cpu_fpr[a->rd], cpu_env, cpu_fpr[a->rs1]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_fcvt_d_h(DisasContext *ctx, arg_fcvt_d_h *a)
{
    REQUIRE_FPU;
    REQUIRE_64BIT(ctx);

    gen_set_rm(ctx, a->rm);
    gen_helper_fcvt_d_h(cpu_fpr[a->rd], cpu_env, cpu_fpr[a->rs1]);

    mark_fs_dirty(ctx);
    return true;
}

static bool trans_ipush(DisasContext *ctx, arg_ipush *a)
{
#ifndef CONFIG_USER_ONLY
    gen_helper_ipush(cpu_env);
    return true;
#else
    return false;
#endif
}

static bool trans_ipop(DisasContext *ctx, arg_ipop *a)
{
#ifndef CONFIG_USER_ONLY
    tcg_gen_movi_tl(cpu_pc, ctx->base.pc_next);
    gen_helper_ipop(cpu_pc, cpu_env, cpu_pc);
    exit_tb(ctx); /* no chaining */
    ctx->base.is_jmp = DISAS_NORETURN;
    return true;
#else
    return false;
#endif
}

static bool trans_fmv_hw_x(DisasContext *ctx, arg_fmv_hw_x *a)
{
#if defined(TARGET_RISCV32)
    REQUIRE_FPU;
    REQUIRE_EXT(ctx, RVD);

    TCGv_i32 t0 = tcg_temp_new_i32();
    TCGv_i64 t1 = tcg_temp_new_i64();
    gen_get_gpr(t0, a->rs1);

    tcg_gen_extu_i32_i64(t1, t0);
    tcg_gen_deposit_i64(cpu_fpr[a->rd], cpu_fpr[a->rd], t1, 32, 32);
    tcg_temp_free_i32(t0);
    tcg_temp_free_i64(t1);
    mark_fs_dirty(ctx);
#else
    gen_exception_illegal(ctx);
#endif
    return true;
}

static bool trans_fmv_x_hw(DisasContext *ctx, arg_fmv_x_hw *a)
{
#if defined(TARGET_RISCV32)
    REQUIRE_FPU;
    REQUIRE_EXT(ctx, RVD);

    TCGv_i32 t0 = tcg_temp_new_i32();
    TCGv_i64 t1 = tcg_const_i64(0);

    tcg_gen_extract_i64(t1, cpu_fpr[a->rs1], 32, 32);
    tcg_gen_extrl_i64_i32(t0, t1);
    gen_set_gpr(a->rd, t0);
    tcg_temp_free_i32(t0);
    tcg_temp_free_i64(t1);
    mark_fs_dirty(ctx);
#else
    gen_exception_illegal(ctx);
#endif
    return true;
}
