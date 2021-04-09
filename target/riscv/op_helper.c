/*
 * RISC-V Emulation Helpers for QEMU.
 *
 * Copyright (c) 2016-2017 Sagar Karandikar, sagark@eecs.berkeley.edu
 * Copyright (c) 2017-2018 SiFive, Inc.
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

#include "qemu/osdep.h"
#include "cpu.h"
#include "qemu/main-loop.h"
#include "exec/exec-all.h"
#include "exec/helper-proto.h"
#include "exec/cpu_ldst.h"

#if !defined(CONFIG_USER_ONLY)
#include "hw/intc/riscv_clic.h"
#endif

/* Exceptions processing helpers */
void QEMU_NORETURN riscv_raise_exception(CPURISCVState *env,
                                          uint32_t exception, uintptr_t pc)
{
    CPUState *cs = env_cpu(env);
    cs->exception_index = exception;
    cpu_loop_exit_restore(cs, pc);
}

void helper_raise_exception(CPURISCVState *env, uint32_t exception)
{
    riscv_raise_exception(env, exception, 0);
}

target_ulong helper_csrrw(CPURISCVState *env, target_ulong src,
        target_ulong csr)
{
    target_ulong val = 0;
    RISCVException ret = riscv_csrrw(env, csr, &val, src, -1);

    if (ret != RISCV_EXCP_NONE) {
        riscv_raise_exception(env, ret, GETPC());
    }
    return val;
}

target_ulong helper_csrrs(CPURISCVState *env, target_ulong src,
        target_ulong csr, target_ulong rs1_pass)
{
    target_ulong val = 0;
    RISCVException ret = riscv_csrrw(env, csr, &val, -1, rs1_pass ? src : 0);

    if (ret != RISCV_EXCP_NONE) {
        riscv_raise_exception(env, ret, GETPC());
    }
    return val;
}

target_ulong helper_csrrc(CPURISCVState *env, target_ulong src,
        target_ulong csr, target_ulong rs1_pass)
{
    target_ulong val = 0;
    RISCVException ret = riscv_csrrw(env, csr, &val, 0, rs1_pass ? src : 0);

    if (ret != RISCV_EXCP_NONE) {
        riscv_raise_exception(env, ret, GETPC());
    }
    return val;
}

#ifndef CONFIG_USER_ONLY

target_ulong helper_sret(CPURISCVState *env, target_ulong cpu_pc_deb)
{
    uint64_t mstatus;
    target_ulong prev_priv, prev_virt;

    if (!(env->priv >= PRV_S)) {
        riscv_raise_exception(env, RISCV_EXCP_ILLEGAL_INST, GETPC());
    }

    target_ulong retpc = env->sepc;
    if (!riscv_has_ext(env, RVC) && (retpc & 0x3)) {
        riscv_raise_exception(env, RISCV_EXCP_INST_ADDR_MIS, GETPC());
    }

    if (get_field(env->mstatus, MSTATUS_TSR) && !(env->priv >= PRV_M)) {
        riscv_raise_exception(env, RISCV_EXCP_ILLEGAL_INST, GETPC());
    }

    if (riscv_has_ext(env, RVH) && riscv_cpu_virt_enabled(env) &&
        get_field(env->hstatus, HSTATUS_VTSR)) {
        riscv_raise_exception(env, RISCV_EXCP_VIRT_INSTRUCTION_FAULT, GETPC());
    }

    mstatus = env->mstatus;

    if (riscv_has_ext(env, RVH) && !riscv_cpu_virt_enabled(env)) {
        /* We support Hypervisor extensions and virtulisation is disabled */
        target_ulong hstatus = env->hstatus;

        prev_priv = get_field(mstatus, MSTATUS_SPP);
        prev_virt = get_field(hstatus, HSTATUS_SPV);

        hstatus = set_field(hstatus, HSTATUS_SPV, 0);
        mstatus = set_field(mstatus, MSTATUS_SPP, 0);
        mstatus = set_field(mstatus, SSTATUS_SIE,
                            get_field(mstatus, SSTATUS_SPIE));
        mstatus = set_field(mstatus, SSTATUS_SPIE, 1);

        env->mstatus = mstatus;
        env->hstatus = hstatus;

        if (prev_virt) {
            riscv_cpu_swap_hypervisor_regs(env);
        }

        riscv_cpu_set_virt_enabled(env, prev_virt);
    } else {
        prev_priv = get_field(mstatus, MSTATUS_SPP);

        mstatus = set_field(mstatus, MSTATUS_SIE,
                            get_field(mstatus, MSTATUS_SPIE));
        mstatus = set_field(mstatus, MSTATUS_SPIE, 1);
        mstatus = set_field(mstatus, MSTATUS_SPP, PRV_U);
        env->mstatus = mstatus;

        if (riscv_clic_is_clic_mode(env)) {
            CPUState *cs = env_cpu(env);
            target_ulong spil = get_field(env->scause, SCAUSE_SPIL);
            env->mintstatus = set_field(env->mintstatus, MINTSTATUS_SIL, spil);
            env->scause = set_field(env->scause, SCAUSE_SPIE, 0);
            env->scause = set_field(env->scause, SCAUSE_SPP, PRV_U);
            qemu_mutex_lock_iothread();
            riscv_clic_get_next_interrupt(env->clic, cs->cpu_index);
            qemu_mutex_unlock_iothread();
        }
    }

    riscv_cpu_set_mode(env, prev_priv);

    return retpc;
}

static target_ulong do_excp_return(CPURISCVState *env, target_ulong cpu_pc_deb,
                                   target_ulong ra)
{
    if (!(env->priv >= PRV_M)) {
        riscv_raise_exception(env, RISCV_EXCP_ILLEGAL_INST, ra);
    }

    target_ulong retpc = env->mepc;
    if (!riscv_has_ext(env, RVC) && (retpc & 0x3)) {
        riscv_raise_exception(env, RISCV_EXCP_INST_ADDR_MIS, ra);
    }

    /* if CLIC mode, copy mcause.mpil into minstatus.mil */
    if ((env->mtvec & 0b111110) == 0b000010) {
        target_ulong mpil = get_field(env->mcause, MCAUSE_MPIL);
        env->mintstatus = set_field(env->mintstatus, MINTSTATUS_MIL, mpil);
        if ((mpil == 0) && (env->mexstatus & MEXSTATUS_SPSWAP)
            && ((target_long)env->mcause < 0)) {
            target_ulong tmp = env->mscratch;
            env->mscratch = env->gpr[2];
            env->gpr[2] = tmp;
        }
    }

    uint64_t mstatus = env->mstatus;
    target_ulong prev_priv = get_field(mstatus, MSTATUS_MPP);

    if (!pmp_get_num_rules(env) && (prev_priv != PRV_M)) {
        riscv_raise_exception(env, RISCV_EXCP_ILLEGAL_INST, ra);
    }

    target_ulong prev_virt = get_field(env->mstatus, MSTATUS_MPV);
    mstatus = set_field(mstatus, MSTATUS_MIE,
                        get_field(mstatus, MSTATUS_MPIE));
    mstatus = set_field(mstatus, MSTATUS_MPIE, 1);
    mstatus = set_field(mstatus, MSTATUS_MPP, PRV_U);
    mstatus = set_field(mstatus, MSTATUS_MPV, 0);
    env->mstatus = mstatus;
    riscv_cpu_set_mode(env, prev_priv);

    if (riscv_has_ext(env, RVH)) {
        if (prev_virt) {
            riscv_cpu_swap_hypervisor_regs(env);
        }

        riscv_cpu_set_virt_enabled(env, prev_virt);
    }

    if (riscv_clic_is_clic_mode(env)) {
        CPUState *cs = env_cpu(env);
        target_ulong mpil = get_field(env->mcause, MCAUSE_MPIL);
        env->mintstatus = set_field(env->mintstatus, MINTSTATUS_MIL, mpil);
        env->mcause = set_field(env->mcause, MCAUSE_MPIE, 0);
        env->mcause = set_field(env->mcause, MCAUSE_MPP, PRV_U);
        qemu_mutex_lock_iothread();
        riscv_clic_get_next_interrupt(env->clic, cs->cpu_index);
        qemu_mutex_unlock_iothread();
    }
    return retpc;
}

target_ulong helper_mret(CPURISCVState *env, target_ulong cpu_pc_deb)
{
    return do_excp_return(env, cpu_pc_deb, GETPC());
}

void helper_wfi(CPURISCVState *env)
{
    CPUState *cs = env_cpu(env);
    bool rvs = riscv_has_ext(env, RVS);
    bool prv_u = env->priv == PRV_U;
    bool prv_s = env->priv == PRV_S;

    if (((prv_s || (!rvs && prv_u)) && get_field(env->mstatus, MSTATUS_TW)) ||
        (rvs && prv_u && !riscv_cpu_virt_enabled(env))) {
        riscv_raise_exception(env, RISCV_EXCP_ILLEGAL_INST, GETPC());
    } else if (riscv_cpu_virt_enabled(env) && (prv_u ||
        (prv_s && get_field(env->hstatus, HSTATUS_VTW)))) {
        riscv_raise_exception(env, RISCV_EXCP_VIRT_INSTRUCTION_FAULT, GETPC());
    } else {
        cs->halted = 1;
        cs->exception_index = EXCP_HLT;
        cpu_loop_exit(cs);
    }
}

void helper_tlb_flush(CPURISCVState *env)
{
    CPUState *cs = env_cpu(env);
    if (!(env->priv >= PRV_S) ||
        (env->priv == PRV_S &&
         get_field(env->mstatus, MSTATUS_TVM))) {
        riscv_raise_exception(env, RISCV_EXCP_ILLEGAL_INST, GETPC());
    } else if (riscv_has_ext(env, RVH) && riscv_cpu_virt_enabled(env) &&
               get_field(env->hstatus, HSTATUS_VTVM)) {
        riscv_raise_exception(env, RISCV_EXCP_VIRT_INSTRUCTION_FAULT, GETPC());
    } else {
        tlb_flush(cs);
    }
}

void helper_hyp_tlb_flush(CPURISCVState *env)
{
    CPUState *cs = env_cpu(env);

    if (env->priv == PRV_S && riscv_cpu_virt_enabled(env)) {
        riscv_raise_exception(env, RISCV_EXCP_VIRT_INSTRUCTION_FAULT, GETPC());
    }

    if (env->priv == PRV_M ||
        (env->priv == PRV_S && !riscv_cpu_virt_enabled(env))) {
        tlb_flush(cs);
        return;
    }

    riscv_raise_exception(env, RISCV_EXCP_ILLEGAL_INST, GETPC());
}

void helper_hyp_gvma_tlb_flush(CPURISCVState *env)
{
    if (env->priv == PRV_S && !riscv_cpu_virt_enabled(env) &&
        get_field(env->mstatus, MSTATUS_TVM)) {
        riscv_raise_exception(env, RISCV_EXCP_ILLEGAL_INST, GETPC());
    }

    helper_hyp_tlb_flush(env);
}

target_ulong helper_hyp_hlvx_hu(CPURISCVState *env, target_ulong address)
{
    int mmu_idx = cpu_mmu_index(env, true) | TB_FLAGS_PRIV_HYP_ACCESS_MASK;

    return cpu_lduw_mmuidx_ra(env, address, mmu_idx, GETPC());
}

target_ulong helper_hyp_hlvx_wu(CPURISCVState *env, target_ulong address)
{
    int mmu_idx = cpu_mmu_index(env, true) | TB_FLAGS_PRIV_HYP_ACCESS_MASK;

    return cpu_ldl_mmuidx_ra(env, address, mmu_idx, GETPC());
}

#endif /* !CONFIG_USER_ONLY */

target_ulong helper_mulah(target_ulong src1, target_ulong src2,
                          target_ulong dst)
{
    int16_t a, b;
    int32_t c, d;
    a = (int16_t)(src1);
    b = (int16_t)(src2);
    c = (int32_t)dst;
    d = (int32_t)a * (int32_t)b;
    d = c + d;
    return (target_long)d;
}

target_ulong helper_mulsh(target_ulong src1, target_ulong src2,
                          target_ulong dst)
{
    int16_t a, b;
    int32_t c, d;
    a = (int16_t)(src1);
    b = (int16_t)(src2);
    c = (int32_t)dst;
    d = (int32_t)a * (int32_t)b;
    d = c - d;
    return (target_long)d;
}

target_ulong helper_tstnbz(target_ulong src)
{
    int i;
    target_ulong res = 0;
    for (i = 0; i < sizeof(target_ulong); i++) {
        if ((src & 0xff) == 0) {
            res |= ((target_ulong)0xff << (i * 8));
        }
        src = src >> 8;
    }
    return res;
}

target_ulong helper_ff0(target_ulong src)
{
    int i = 0;
    int bits = sizeof(target_ulong) * 8;

    for (i = 0; i < bits; i++) {
        if ((src & ((target_ulong)1 << (bits - 1 - i))) == 0) {
            break;
        }
    }
    return i;
}

target_ulong helper_ff1(target_ulong src)
{
    int i = 0;
    int bits = sizeof(target_ulong) * 8;

    for (i = 0; i < bits; i++) {
        if ((src & ((target_ulong)1 << (bits - 1 - i))) != 0) {
            break;
        }
    }
    return i;
}

target_ulong helper_srri(target_ulong src, target_ulong imm)
{
    target_ulong i = 0;
    int bits = sizeof(target_ulong) * 8;

    i = (src >> imm) | (src << (bits - imm));
    return i;
}

target_ulong helper_tst(target_ulong src, target_ulong imm)
{
    target_ulong i = 0;

    if (src & (1ULL << imm)) {
        i = 1;
    }

    return i;
}

target_ulong helper_srriw(target_ulong src, target_ulong imm)
{
    target_ulong i = 0;
    uint32_t tmp = src & 0xffffffff;
    tmp = ((tmp >> imm) | (tmp << (32 - imm)));
    if (tmp & 0x80000000) {
        i = 0xffffffff00000000 | tmp;
    } else {
        i = tmp;
    }
    return i;
}

#ifndef CONFIG_USER_ONLY
void helper_ipush(CPURISCVState *env)
{
    target_ulong base = env->gpr[2];
    int i = 4;
    /* TODO: probe the memory */
    for(; i <= 72; i += 4) {
        switch (i) {
        case 4:
            cpu_stl_data(env, base - i, env->mcause);
            break;
        case 8:
            cpu_stl_data(env, base - i, env->mepc);
            break;
        case 12: /* X1 */
            cpu_stl_data(env, base - i, env->gpr[1]);
            break;
        case 16: /* X5-X7 */
        case 20:
        case 24:
            cpu_stl_data(env, base - i, env->gpr[i / 4 + 1]);
            break;
        case 28: /* X10-X17 */
        case 32:
        case 36:
        case 40:
        case 44:
        case 48:
        case 52:
        case 56:
            cpu_stl_data(env, base - i, env->gpr[i / 4 + 3]);
            break;
        case 60: /* X28-X31 */
        case 64:
        case 68:
        case 72:
            cpu_stl_data(env, base - i, env->gpr[i / 4 + 13]);
            break;
        }
    }
    env->gpr[2] -= 72;
}

target_ulong helper_ipop(CPURISCVState *env, target_ulong cpu_pc_deb)
{
    target_ulong base = env->gpr[2];
    int i = 68;
    /* TODO: probe the memory */
    for(; i >= 0; i -= 4) {
        switch (i) {
        case 0: /* X31-X28 */
        case 4:
        case 8:
        case 12:
            env->gpr[31 - i / 4] = cpu_ldl_data(env, base + i);
            break;
        case 16: /* X17-X10 */
        case 20:
        case 24:
        case 28:
        case 32:
        case 36:
        case 40:
        case 44:
            env->gpr[21 - i / 4] = cpu_ldl_data(env, base + i);
            break;
        case 48: /* X7-X5 */
        case 52:
        case 56:
            env->gpr[19 - i / 4] = cpu_ldl_data(env, base + i);
            break;
        case 60: /* X1 */
            env->gpr[1] = cpu_ldl_data(env, base + i);
            break;
        case 64:
            env->mepc = cpu_ldl_data(env, base + i);
            break;
        case 68:
            env->mcause = cpu_ldl_data(env, base + i);
            break;
        }
    }
    env->gpr[2] += 72;
    return do_excp_return(env, cpu_pc_deb, GETPC());
}
#endif
