
#include <utils/common.h>
#include <interrupts/descriptor_tables.h>
#include <multitasking/systimer.h>
#include <memory_managment/paging.h>
#include <memory_managment/kheap.h>
#include <multitasking/task.h>
#include <multitasking/syscall.h>
#include <drivers/bios32/bios32.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/pci_ids.h>
#include <drivers/acpi/acpi.h>
#include <modules/modules.h>

#include <io/keyboard.h>

int32_t kernel_main() {
	initSysTimer();
	initPaging();
	
	if (BIOS32Find()) {
		LOG_INFO("BIOS32 found on 0x%08x", BIOS32GetAddress());
	} else {
		LOG_INFO("BIOS32 instance not found!");
	}

	if (PCIInit() == PCI_OP_SUCC) {
		#define MAX_PCI_DEVICES 256
		PCIDevice_t* devices = (PCIDevice_t*)kmalloc(sizeof(PCIDevice_t) * MAX_PCI_DEVICES);
		memset(devices, 0, sizeof(PCIDevice_t) * MAX_PCI_DEVICES);

		uint32_t found = PCIDirectScan(devices);
		if (found) {
			LOG_INFO("[PCI] Found %d devices", found);
		} else {
			LOG_INFO("[PCI] Bus is empty");
		}
	} else {
		LOG_INFO("[PCI] Not supported");
	}

	RSDP_t* rsdp = findRSDP();
	LOG_INFO("RSDP %s", rsdp ? "Found" : "Not found");
	if (rsdp) {
		LOG_INFO("RSDP v%d, OEM: 0x%02x%02x%02x%02x%02x%02x", rsdp->revision, rsdp->OEMID[5], rsdp->OEMID[4], rsdp->OEMID[3], rsdp->OEMID[2], rsdp->OEMID[1], rsdp->OEMID[0]);
		RSDT_t* rsdt = findRSDT(rsdp);
		LOG_INFO("RSDT %s", rsdt ? "Found" : "Not found");

		if (rsdt) {
			printSDT(&rsdt->header);
			FADT_t* fadt = findInSDT(FACP_SIGNATURE, (SDTHeader_t*)rsdt);
			LOG_INFO("FADT %s", rsdt ? "Found" : "Not found");
			if (fadt) {
				asm volatile ("nop");
			}

			HPET_t* hpet = findInSDT(HPET_SIGNATURE, (SDTHeader_t*)rsdt);
			LOG_INFO("HPET %s", hpet ? "Found" : "Not found");
			if (fadt) {
				// LOG_INFO("HPET Info:");
				// printf("Hardware rev: %d\n", hpet->hardware_rev_id);
				// printf("Comparators: %d\n", hpet->comparator_count);
				// printf("Counter size: %d\n", hpet->counter_size);
				// printf("PCI vendor: %s (0x%04x)\n", PCIGetVendorName(hpet->pci_vendor_id), hpet->pci_vendor_id);
			}
		}
	}
	serialInit(COM1, UART_BAUD_9600);

	initSysCalls();
	initTasking();
    
    LOG_INFO("Keyboard: %s", initKeyboard() == 0 ? "initialized" : "initialization failed");

	ELF32Obj_t shell_module = { 0 };

	if (getModuleByName("shell", &shell_module)) {
		Task_t* shell_task = makeTaskFromELF(&shell_module);
		runTask(shell_task);
	}

	LOG_INFO("Message from kernel! PID %d Ring %d", getPID(), getCPL());

    while (true) {
        sleep(2000);
    }

	LOG_INFO("Upime %d seconds", getUptime() / 1000);

	return 0xDEADBABA;
}