#ifndef HW_RISCV_XMMUV1_H
#define HW_RISCV_XMMUV1_H

#include "hw/riscv/xmmu-common.h"
#include "hw/registerfields.h"
#include "qom/object.h"

#define TYPE_XMMUV1_IOMMU_MEMORY_REGION "xmmuv1-iommu-memory-region"

enum XMMUv1Fault {
    IOMMU_DTE_RESERVED_NOZERO_FAULT = 1,
    IOMMU_DTE_F_ONE_S1MODE_ZERO_FAULT,
    IOMMU_DTE_S_ONE_S2MODE_ZERO_FAULT,

    IOMMU_S1_PTE_NO_EXEC_FAULT = 12,
    IOMMU_S1_PTE_NO_READ_FAULT,
    IOMMU_S1_PTE_RESERVED_FAULT,
    IOMMU_S1_PTE_NO_RW_FAULT,
    IOMMU_S1_PTE_INVALID_FAULT,
    IOMMU_S1_PTE_MISALIGN_PPN_FAULT,
    IOMMU_S1_PTE_USER_FOR_PRIV_FAULT,
    IOMMU_S1_PTE_NO_USER_FOR_UNPRIV_FAULT,
    IOMMU_S1_PMP_ACCESS_FAULT,
    IOMMU_S2_PTE_NO_EXEC_FAULT,
    IOMMU_S2_PTE_NO_READ_FAULT,
    IOMMU_S2_PTE_RESERVED_FAULT,
    IOMMU_S2_PTE_NO_RW_FAULT,
    IOMMU_S2_PTE_INVALID_FAULT,
    IOMMU_S2_PTE_MISALIGN_PPN_FAULT,
    IOMMU_S2_PTE_NO_USER_FAULT,
};

struct XMMUv1State {
    XMMUState xmmu_state;
    uint64_t  dtbase;  /* Phys_addr of the level-1 device table */
    uint64_t  rsiddiv; /* Split point of RSID */
    uint64_t  ftval;   /* The address that caused the fault */
    bool      enable;  /* the IOMMU is active or not */
    bool      irq_enable;  /* Enable the interrupt from the IOMMU */
    bool      stage_two;
    uint64_t  irq_num;     /* Interrupt number of IOMMU */
    bool      fault_pending; /* There is a pending fault or not */
    uint64_t  fault_rsid;    /* Fault RSID */
    enum XMMUv1Fault fault_cause;   /* Fault cause */
    qemu_irq  irq;
    QemuMutex mutex;
};

struct XMMUv1Class {
    /*< private >*/
    XMMUBaseClass xmmu_base_class;
    /*< public >*/

    DeviceRealize parent_realize;
    DeviceReset   parent_reset;
};

#define TYPE_RISCV_XMMUV1  "riscv-xmmuv1"
OBJECT_DECLARE_TYPE(XMMUv1State, XMMUv1Class, RISCV_XMMUV1)

#endif
