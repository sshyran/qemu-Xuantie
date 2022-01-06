#include "qemu/osdep.h"
#include "qemu/bitops.h"
#include "hw/irq.h"
#include "hw/sysbus.h"
#include "migration/vmstate.h"
#include "hw/qdev-core.h"
#include "hw/pci/pci.h"
#include "cpu.h"
#include "trace.h"
#include "qemu/log.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "hw/qdev-properties.h"

#include "hw/riscv/xmmuv1.h"
#include "xmmuv1-internal.h"
#include "xmmu-internal.h"

#define DTE_V_BIT (1U << 0)
#define DTE_S_BIT (1U << 1)
#define DTE_F_BIT (1U << 2)

static void xmmuv1_init_regs(XMMUv1State *s)
{
    if (s->rsiddiv >= RSIDLEN) {
        s->rsiddiv = RSIDLEN;
    }
}

static bool riscv_one_stage(void *opaque, hwaddr *physical, uint64_t satp,
                            dma_addr_t addr, IOMMUAccessFlags flags)
{
    XMMUv1State *s = (XMMUv1State *)opaque;
    hwaddr base;
    int levels = 0, ptidxbits = 0, ptesize = 0, vm = 0;
    int i, ptshift;
    bool succeed = false;
    hwaddr pte_addr;
    target_ulong pte;
    hwaddr ppn;
    base = (hwaddr)get_field(satp, SATP64_PPN) << PGSHIFT;
    vm = get_field(satp, SATP64_MODE);

    switch (vm) {
    case VM_1_10_SV32:
        levels = 2; ptidxbits = 10; ptesize = 4; break;
    case VM_1_10_SV39:
        levels = 3; ptidxbits = 9; ptesize = 8; break;
    case VM_1_10_SV48:
        levels = 4; ptidxbits = 9; ptesize = 8; break;
    case VM_1_10_SV57:
        levels = 5; ptidxbits = 9; ptesize = 8; break;
    case VM_1_10_MBARE:
        return addr;
    default:
        g_assert_not_reached();
    }

    ptshift = (levels - 1) * ptidxbits;
    for (i = 0; i < levels; i++, ptshift -= ptidxbits) {
        target_ulong idx = (addr >> (PGSHIFT + ptshift)) &
                           ((1 << ptidxbits) - 1);
        /* check that physical address of PTE is legal */
        pte_addr = base + idx * ptesize;
        dma_memory_read(&address_space_memory, pte_addr, &pte, 8,
                        MEMTXATTRS_UNSPECIFIED);
        ppn = (pte & (~0xffc0000000000000ll)) >> PTE_PPN_SHIFT;
        if (!(pte & PTE_V)) {
            /* Invalid PTE */
            s->fault_cause = IOMMU_S1_PTE_INVALID_FAULT;
        } else if (!(pte & (PTE_R | PTE_W | PTE_X))) {
            /* Inner PTE, continue walking */
            base = ppn << PGSHIFT;
        } else if ((pte & (PTE_R | PTE_W | PTE_X)) == PTE_W) {
            /* Reserved leaf PTE flags: PTE_W */
            s->fault_cause = IOMMU_S1_PTE_RESERVED_FAULT;
        } else if ((pte & (PTE_R | PTE_W | PTE_X)) == (PTE_W | PTE_X)) {
            /* Reserved leaf PTE flags: PTE_W + PTE_X */
            s->fault_cause = IOMMU_S1_PTE_RESERVED_FAULT;
        } else if (ppn & ((1ULL << ptshift) - 1)) {
            /* Misaligned PPN */
            s->fault_cause = IOMMU_S1_PTE_MISALIGN_PPN_FAULT;
        } else if ((flags & IOMMU_RO) && !(pte & PTE_R)) {
            /* Read access check failed */
            s->fault_cause = IOMMU_S1_PTE_NO_READ_FAULT;
        } else if ((flags & IOMMU_WO) && !(pte & PTE_W)) {
            /* Write access check failed */
            s->fault_cause = IOMMU_S1_PTE_NO_RW_FAULT;
        } else {
            target_ulong vpn = addr >> PGSHIFT;
            *physical = ((ppn | (vpn & ((1L << ptshift) - 1))) << PGSHIFT) |
                            (addr & ~TARGET_PAGE_MASK);
            succeed = true;
        }
        if (s->fault_cause) {
            break;
        }
    }
    return succeed;
}

static IOMMUTLBEntry xmmuv1_translate(IOMMUMemoryRegion *mr, hwaddr addr,
                                      IOMMUAccessFlags flag, int iommu_idx)
{
    XMMUDevice *sdev = container_of(mr, XMMUDevice, iommu);
    XMMUv1State *s = sdev->xmmu;
    IOMMUTLBEntry entry = {
         .target_as = &address_space_memory,
         .iova = addr,
         .translated_addr = addr,
         .addr_mask = (hwaddr)0,
         .perm = flag,
    };
    uint32_t rsidhi, rsidlo, rsid = xmmu_get_rsid(sdev);
    uint64_t out_dte, dev_conf;
    bool do_stage_one = false, do_stage_two = false;
    uint64_t s1_atp, s2_atp;
    uint64_t dte_addr;

    if (!s->enable) {
        entry.translated_addr = addr;
        return entry;
    }
    if (s->rsiddiv) {
        rsidlo = extract32(rsid, 0, s->rsiddiv);
        rsidhi = extract32(rsid, s->rsiddiv, RSIDLEN - s->rsiddiv);

        dte_addr = s->dtbase + (rsidhi << 3);
        dma_memory_read(&address_space_memory, dte_addr, &out_dte, 8,
                        MEMTXATTRS_UNSPECIFIED);
        if (out_dte & 0x7) {
            s->fault_pending = true;
            s->fault_rsid = rsid;
            s->fault_cause = IOMMU_DTE_RESERVED_NOZERO_FAULT;
            s->ftval = addr;
            if (s->irq_enable) {
                qemu_set_irq(s->irq, 1);
            }
        }
        dte_addr = out_dte + (rsidlo << 3);
        dma_memory_read(&address_space_memory, dte_addr, &out_dte, 8,
                        MEMTXATTRS_UNSPECIFIED);
    } else {
        uint64_t dte_addr = s->dtbase + (rsid << 3);
        dma_memory_read(&address_space_memory, dte_addr, &out_dte, 8,
                        MEMTXATTRS_UNSPECIFIED);
    }

    if (!(out_dte & DTE_V_BIT)) {
        entry.translated_addr = addr;
        return entry;
    }

    uint64_t desc_addr = out_dte & ~0x7;
    dma_memory_read(&address_space_memory, desc_addr + 16, &dev_conf, 8,
                    MEMTXATTRS_UNSPECIFIED);

    if (out_dte & DTE_F_BIT) {
        do_stage_one = true;
        dma_memory_read(&address_space_memory, desc_addr, &s1_atp, 8,
                        MEMTXATTRS_UNSPECIFIED);
    }

    if (out_dte & DTE_S_BIT) {
        do_stage_two = true;
        dma_memory_read(&address_space_memory, desc_addr + 8, &s2_atp, 8,
                        MEMTXATTRS_UNSPECIFIED);
    }

    assert(!do_stage_two);

    if (!do_stage_one) {
        entry.translated_addr = addr;
    } else { /* Stage-2 disabled, Stage-1 enabled */
        if (!riscv_one_stage(s, &entry.translated_addr, s1_atp, addr, flag)) {
            entry.perm = IOMMU_NONE;
            s->fault_pending = true;
            s->fault_rsid = rsid;
            s->ftval = addr;
            if (s->irq_enable) {
                qemu_set_irq(s->irq, 1);
            }
        }
    }
    return entry;
}

static MemTxResult xmmu_write_mmio(void *opaque, hwaddr offset, uint64_t data,
                                   unsigned size, MemTxAttrs attrs)
{
    XMMUState *sys = opaque;
    XMMUv1State *s = RISCV_XMMUV1(sys);
    MemTxResult r = MEMTX_OK;

    offset &= ~0x10000;
    offset = offset >> 3;

    if (size == 8) {
        switch (offset) {
        case R_DTBASE:
            s->dtbase = data;
            break;
        case R_IOMMUCAP:
            if (FIELD_EX64(data, IOMMUCAP, STAGE_TWO)) {
                qemu_log_mask(LOG_GUEST_ERROR,
                              "XMMU: stage two is unimplemented\n");
                r = MEMTX_ERROR;
            }
            break;
        case R_IOMMUCAPEN:
            s->enable = FIELD_EX64(data, IOMMUCAPEN, ENABLE);
            break;
        case R_IOMMUINTEN:
            s->irq_enable = FIELD_EX64(data, IOMMUINTEN, ENABLE);
            break;
        case R_RESUME:
            s->fault_pending = false;
            s->fault_rsid = 0;
            s->fault_cause = 0;
            qemu_set_irq(s->irq, 0);
            break;
        case R_RSIDDIV:
        case R_INVLTLB:
        case R_IOMMUCAUSE:
        case R_IOMMUINTNO:
        case R_ERSIDDIV:
        case R_FTVAL:
        default:
            r = MEMTX_ERROR;
            break;
        }
    }
    return r;
}

static MemTxResult xmmu_read_mmio(void *opaque, hwaddr offset, uint64_t *data,
                                   unsigned size, MemTxAttrs attrs)
{
    XMMUState *sys = opaque;
    XMMUv1State *s = RISCV_XMMUV1(sys);
    MemTxResult r = MEMTX_OK;
    *data = 0;

    /* CONSTRAINED UNPREDICTABLE choice to have page0/1 be exact aliases */
    offset &= ~0x10000;
    offset = offset >> 3;

    if (size == 8) {
        switch (offset) {
        case R_DTBASE:
            *data = s->dtbase;
           break;
        case R_IOMMUCAP:
            *data = FIELD_DP64(*data, IOMMUCAP, STAGE_TWO, s->stage_two);
            break;
        case R_RSIDDIV: /* Todo: Add a property or add a create function */
            *data = s->rsiddiv;
            break;
        case R_FTVAL:
            *data = s->ftval;
            break;
        case R_IOMMUCAPEN:
            *data = FIELD_DP64(*data, IOMMUCAPEN, ENABLE, s->enable);
            break;
        case R_IOMMUINTEN:
            *data = FIELD_DP64(*data, IOMMUINTEN, ENABLE, s->irq_enable);
            break;
        case R_IOMMUINTNO:
            *data = s->irq_num;
            break;
        case R_IOMMUCAUSE:
            *data = FIELD_DP64(*data, IOMMUCAUSE, V, s->fault_pending);
            *data = FIELD_DP64(*data, IOMMUCAUSE, RSID, s->fault_rsid);
            *data = FIELD_DP64(*data, IOMMUCAUSE, CAUSE, s->fault_cause);
            break;
        case R_ERSIDDIV:
        default:
            r = MEMTX_ERROR;
            break;
        }
    }

    return r;
}

static const MemoryRegionOps xmmu_mem_ops = {
    .read_with_attrs = xmmu_read_mmio,
    .write_with_attrs = xmmu_write_mmio,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
    .impl = {
        .min_access_size = 4,
        .max_access_size = 8,
    },
};

static void xmmu_init_irq(XMMUv1State *s, SysBusDevice *dev)
{
    sysbus_init_irq(dev, &s->irq);
}

static void xmmu_reset(DeviceState *dev)
{
    XMMUv1State *s = RISCV_XMMUV1(dev);
    XMMUv1Class *c = RISCV_XMMUV1_GET_CLASS(s);

    c->parent_reset(dev);

    xmmuv1_init_regs(s);
}

static void xmmu_realize(DeviceState *d, Error **errp)
{
    XMMUState *sys = RISCV_XMMU(d);
    XMMUv1State *s = RISCV_XMMUV1(sys);
    XMMUv1Class *c = RISCV_XMMUV1_GET_CLASS(s);
    SysBusDevice *dev = SYS_BUS_DEVICE(d);
    Error *local_err = NULL;

    c->parent_realize(d, &local_err);
    if (local_err) {
        error_propagate(errp, local_err);
        return;
    }

    qemu_mutex_init(&s->mutex);
    memory_region_init_io(&sys->iomem, OBJECT(s),
                          &xmmu_mem_ops, sys, TYPE_RISCV_XMMUV1, 0x20000);

    sys->mrtypename = TYPE_XMMUV1_IOMMU_MEMORY_REGION;
    sysbus_init_mmio(dev, &sys->iomem);
    xmmu_init_irq(s, dev);
}

static const VMStateDescription vmstate_xmmuv1 = {
    .name = "xmmuv1",
    .version_id = 1,
    .minimum_version_id = 1,
    .priority = MIG_PRI_IOMMU,
    .fields = (VMStateField[]) {
        VMSTATE_UINT64(rsiddiv, XMMUv1State),
        VMSTATE_UINT64(dtbase, XMMUv1State),
        VMSTATE_END_OF_LIST(),
    },
};

static void xmmuv1_instance_init(Object *obj)
{
    /* Nothing much to do here as of now */
}

static Property xmmu_properties[] = {
    DEFINE_PROP_UINT64("rsiddiv", XMMUv1State, rsiddiv, 0),
    DEFINE_PROP_UINT64("irq", XMMUv1State, irq_num, 0),
    DEFINE_PROP_END_OF_LIST(),
};

static void xmmuv1_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    XMMUv1Class *c = RISCV_XMMUV1_CLASS(klass);

    dc->vmsd = &vmstate_xmmuv1;
    device_class_set_parent_reset(dc, xmmu_reset, &c->parent_reset);
    c->parent_realize = dc->realize;
    dc->realize = xmmu_realize;
    device_class_set_props(dc, xmmu_properties);
}

static int xmmuv1_notify_flag_changed(IOMMUMemoryRegion *iommu,
                                      IOMMUNotifierFlag old,
                                      IOMMUNotifierFlag new,
                                      Error **errp)
{
    XMMUDevice *sdev = container_of(iommu, XMMUDevice, iommu);
    XMMUv1State *s1 = sdev->xmmu;
    XMMUState *s = &(s1->xmmu_state);

    if (new & IOMMU_NOTIFIER_DEVIOTLB_UNMAP) {
        error_setg(errp, "XMMUv1 does not support dev-iotlb yet");
        return -EINVAL;
    }

    if (new & IOMMU_NOTIFIER_MAP) {
        error_setg(errp,
                   "device %02x.%02x.%x requires iommu MAP notifier which is "
                   "not currently supported", pci_bus_num(sdev->bus),
                   PCI_SLOT(sdev->devfn), PCI_FUNC(sdev->devfn));
        return -EINVAL;
    }

    if (old == IOMMU_NOTIFIER_NONE) {
        QLIST_INSERT_HEAD(&s->devices_with_notifiers, sdev, next);
    } else if (new == IOMMU_NOTIFIER_NONE) {
        QLIST_REMOVE(sdev, next);
    }
    return 0;
}

static void xmmuv1_iommu_memory_region_class_init(ObjectClass *klass,
                                                  void *data)
{
    IOMMUMemoryRegionClass *imrc = IOMMU_MEMORY_REGION_CLASS(klass);

    imrc->translate = xmmuv1_translate;
    imrc->notify_flag_changed = xmmuv1_notify_flag_changed;
}

static const TypeInfo xmmuv1_type_info = {
    .name          = TYPE_RISCV_XMMUV1,
    .parent        = TYPE_RISCV_XMMU,
    .instance_size = sizeof(XMMUv1State),
    .instance_init = xmmuv1_instance_init,
    .class_size    = sizeof(XMMUv1Class),
    .class_init    = xmmuv1_class_init,
};

static const TypeInfo xmmuv1_iommu_memory_region_info = {
    .parent = TYPE_IOMMU_MEMORY_REGION,
    .name = TYPE_XMMUV1_IOMMU_MEMORY_REGION,
    .class_init = xmmuv1_iommu_memory_region_class_init,
};

static void xmmuv1_register_types(void)
{
    type_register(&xmmuv1_type_info);
    type_register(&xmmuv1_iommu_memory_region_info);
}

type_init(xmmuv1_register_types)
