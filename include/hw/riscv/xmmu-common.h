#ifndef HW_RISCV_XMMU_COMMON_H
#define HW_RISCV_XMMU_COMMON_H

#include "hw/sysbus.h"
#include "hw/pci/pci.h"
#include "qom/object.h"

#define XMMU_PCI_BUS_MAX      256
#define XMMU_PCI_DEVFN_MAX    256
#define XMMU_PCI_DEVFN(sid)   (sid & 0xFF)

#define XMMU_MAX_VA_BITS      48

typedef struct XMMUDevice {
    void               *xmmu;
    PCIBus             *bus;
    int                devfn;
    IOMMUMemoryRegion  iommu;
    AddressSpace       as;
    QLIST_ENTRY(XMMUDevice) next;
} XMMUDevice;

typedef struct XMMUPciBus {
    PCIBus       *bus;
    XMMUDevice   *pbdev[]; /* Parent array is sparse, so dynamically alloc */
} XMMUPciBus;

struct XMMUState {
    /* <private> */
    SysBusDevice  dev;
    const char *mrtypename;
    MemoryRegion iomem;

    GHashTable *xmmu_pcibus_by_busptr;
    GHashTable *configs; /* cache for configuration data */
    GHashTable *iotlb;
    XMMUPciBus *xmmu_pcibus_by_bus_num[XMMU_PCI_BUS_MAX];
    PCIBus *pci_bus;
    QLIST_HEAD(, XMMUDevice) devices_with_notifiers;
    uint8_t bus_num;
    PCIBus *primary_bus;
};

struct XMMUBaseClass {
    /* <private> */
    SysBusDeviceClass parent_class;

    /*< public >*/

    DeviceRealize parent_realize;

};

#define TYPE_RISCV_XMMU "riscv-xmmu"
OBJECT_DECLARE_TYPE(XMMUState, XMMUBaseClass, RISCV_XMMU)

/* Return the XMMUPciBus handle associated to a PCI bus number */
XMMUPciBus *xmmu_find_xmmu_pcibus(XMMUState *s, uint8_t bus_num);

/* Return the stream ID of an XMMU device */
static inline uint16_t xmmu_get_rsid(XMMUDevice *sdev)
{
    return PCI_BUILD_BDF(pci_bus_num(sdev->bus), sdev->devfn);
}

/* Return the iommu mr associated to @sid, or NULL if none */
IOMMUMemoryRegion *xmmu_iommu_mr(XMMUState *s, uint32_t rsid);

/* Unmap the range of all the notifiers registered to any IOMMU mr */
void xmmu_inv_notifiers_all(XMMUState *s);

/* Unmap the range of all the notifiers registered to @mr */
void xmmu_inv_notifiers_mr(IOMMUMemoryRegion *mr);

#endif /* HW_RISCV_XMMU_COMMON_H */
