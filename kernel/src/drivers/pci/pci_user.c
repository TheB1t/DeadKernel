#include <drivers/pci/pci_user.h>

uint32_t user_PCIDirectScan(PCIDevice_t* user_devices) {
    PCIDevice_t devices[256];

    uint32_t count = PCIDirectScan(devices);

    copyToUser(user_devices, devices, sizeof(PCIDevice_t) * count);

    return count;
}