#include "qemu/osdep.h"
#include "trace.h"
#include "exec/target_page.h"
#include "hw/core/cpu.h"
#include "hw/qdev-properties.h"
#include "qapi/error.h"
#include "qemu/jhash.h"
#include "qemu/module.h"

#include "qemu/error-report.h"
#include "hw/riscv/xmmu-common.h"
#include "xmmu-internal.h"

XMMUPciBus *xmmu_find_xmmu_pcibus(XMMUState *s, uint8_t bus_num)
{
    XMMUPciBus *xmmu_pci_bus = s->xmmu_pcibus_by_bus_num[bus_num];
    GHashTableIter iter;

    if (xmmu_pci_bus) {
        return xmmu_pci_bus;
    }

    g_hash_table_iter_init(&iter, s->xmmu_pcibus_by_busptr);
    while (g_hash_table_iter_next(&iter, NULL, (void **)&xmmu_pci_bus)) {
        if (pci_bus_num(xmmu_pci_bus->bus) == bus_num) {
            s->xmmu_pcibus_by_bus_num[bus_num] = xmmu_pci_bus;
            return xmmu_pci_bus;
        }
    }

    return NULL;
}

static AddressSpace *xmmu_find_add_as(PCIBus *bus, void *opaque, int devfn)
{
    XMMUState *s = opaque;
    XMMUPciBus *sbus = g_hash_table_lookup(s->xmmu_pcibus_by_busptr, bus);
    XMMUDevice *sdev;
    static unsigned int index;

    if (!sbus) {
        sbus = g_malloc0(sizeof(XMMUPciBus) +
                         sizeof(XMMUDevice *) * XMMU_PCI_DEVFN_MAX);
        sbus->bus = bus;
        g_hash_table_insert(s->xmmu_pcibus_by_busptr, bus, sbus);
    }

    sdev = sbus->pbdev[devfn];
    if (!sdev) {
        char *name = g_strdup_printf("%s-%d-%d", s->mrtypename, devfn, index++);

        sdev = sbus->pbdev[devfn] = g_new0(XMMUDevice, 1);

        sdev->xmmu = s;
        sdev->bus = bus;
        sdev->devfn = devfn;

        memory_region_init_iommu(&sdev->iommu, sizeof(sdev->iommu),
                                 s->mrtypename,
                                 OBJECT(s), name, 1ULL << XMMU_MAX_VA_BITS);
        address_space_init(&sdev->as,
                           MEMORY_REGION(&sdev->iommu), name);
        g_free(name);
    }

    return &sdev->as;
}

IOMMUMemoryRegion *xmmu_iommu_mr(XMMUState *s, uint32_t sid)
{
    uint8_t bus_n, devfn;
    XMMUPciBus *xmmu_bus;
    XMMUDevice *xmmu;

    bus_n = PCI_BUS_NUM(sid);
    xmmu_bus = xmmu_find_xmmu_pcibus(s, bus_n);
    if (xmmu_bus) {
        devfn = XMMU_PCI_DEVFN(sid);
        xmmu = xmmu_bus->pbdev[devfn];
        if (xmmu) {
            return &xmmu->iommu;
        }
    }
    return NULL;
}

/* Unmap the whole notifier's range */
static void xmmu_unmap_notifier_range(IOMMUNotifier *n)
{
    IOMMUTLBEvent event;

    event.type = IOMMU_NOTIFIER_UNMAP;
    event.entry.target_as = &address_space_memory;
    event.entry.iova = n->start;
    event.entry.perm = IOMMU_NONE;
    event.entry.addr_mask = n->end - n->start;

    memory_region_notify_iommu_one(n, &event);
}

/* Unmap all notifiers attached to @mr */
inline void xmmu_inv_notifiers_mr(IOMMUMemoryRegion *mr)
{
    IOMMUNotifier *n;

    IOMMU_NOTIFIER_FOREACH(n, mr) {
        xmmu_unmap_notifier_range(n);
    }
}

/* Unmap all notifiers of all mr's */
void xmmu_inv_notifiers_all(XMMUState *s)
{
    XMMUDevice *sdev;

    QLIST_FOREACH(sdev, &s->devices_with_notifiers, next) {
        xmmu_inv_notifiers_mr(&sdev->iommu);
    }
}

static void xmmu_base_realize(DeviceState *dev, Error **errp)
{
    XMMUState *s = RISCV_XMMU(dev);
    XMMUBaseClass *sbc = RISCV_XMMU_GET_CLASS(dev);
    Error *local_err = NULL;

    sbc->parent_realize(dev, &local_err);
    if (local_err) {
        error_propagate(errp, local_err);
        return;
    }
    s->xmmu_pcibus_by_busptr = g_hash_table_new(NULL, NULL);

    if (s->primary_bus) {
        pci_setup_iommu(s->primary_bus, xmmu_find_add_as, s);
    } else {
        error_setg(errp, "Xuantie IOMMU is not attached to any PCI bus!");
    }
}

static void xmmu_base_reset(DeviceState *dev)
{
}

static Property xmmu_dev_properties[] = {
    DEFINE_PROP_UINT8("bus_num", XMMUState, bus_num, 0),
    DEFINE_PROP_LINK("primary-bus", XMMUState, primary_bus, "PCI", PCIBus *),
    DEFINE_PROP_END_OF_LIST(),
};

static void xmmu_base_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    XMMUBaseClass *xbc = RISCV_XMMU_CLASS(klass);

    device_class_set_props(dc, xmmu_dev_properties);
    device_class_set_parent_realize(dc, xmmu_base_realize,
                                    &xbc->parent_realize);
    dc->reset = xmmu_base_reset;
}

static const TypeInfo xmmu_base_info = {
    .name          = TYPE_RISCV_XMMU,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(XMMUState),
    .class_data    = NULL,
    .class_size    = sizeof(XMMUBaseClass),
    .class_init    = xmmu_base_class_init,
    .abstract      = true,
};

static void xmmu_base_register_types(void)
{
    type_register_static(&xmmu_base_info);
}

type_init(xmmu_base_register_types)
