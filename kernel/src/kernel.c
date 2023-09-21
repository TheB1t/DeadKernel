
#include <utils/common.h>
#include <io/screen.h>
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
#include <utils/bool.h>

#include <io/keyboard.h>
#include <io/serial.h>

extern ELF32Header_t* testModule;

int32_t kernel_main() {
	initSysTimer();
	initPaging();

	//uint32_t d = *(uint32_t*)0xD0000000;
	
	if (BIOS32Find()) {
		LOG_INFO("BIOS32 found on 0x%08x", BIOS32GetAddress());
		
		uint8_t	HWMech, majorVer, minorVer;
		ASSERT(PCICheckSupport(&majorVer, &minorVer, &HWMech));
		LOG_INFO("PCI BIOS found %02x ver %02x.%02x", HWMech, majorVer, minorVer);
		
		LOG_INFO("Scan PCI bus...");
		PCIDevice_t devices[256];
		memset(devices, 0, sizeof(PCIDevice_t) * 256);

		uint32_t count = PCIDirectScan(devices);
		LOG_INFO("Found %d devices!", count);
		
		// PCIDevice_t* device = devices;
		// while (device->vendor) {
		// 	LOG_INFO("	%03d:%02d:%01d [%04x:%04x] %s", device->bus, device->dev, device->fn, device->vendor, device->device, PCIGetClassName(device->class));
		// 	device++;
		// }
	} else {
		LOG_INFO("BIOS32 instance not found!");
	}

	RSDP_t* rsdp = findRSDP();
	if (rsdp) {
		LOG_INFO("RSDP Found. v%d OEM: 0x%02x%02x%02x%02x%02x%02x", rsdp->revision, rsdp->OEMID[0], rsdp->OEMID[1], rsdp->OEMID[1], rsdp->OEMID[3], rsdp->OEMID[4], rsdp->OEMID[5]);
		RSDP2_t* rsdp2 = findRSDP2(rsdp);
		if (rsdp2) {
			LOG_INFO("RSDP2 Found");
		} else {
			LOG_INFO("RSDP2 Not found");
			RSDT_t* rsdt = findRSDT(rsdp);
			if (rsdt) {
				LOG_INFO("RSDT Found");
				FADT_t* fadt = findFACP(rsdt);
				if (fadt) {
					LOG_INFO("FADT Found");
				} else {
					LOG_INFO("FADT Not found");
				}
			} else {
				LOG_INFO("RSDT Not found");
			}
		}
	} else {
		LOG_INFO("RSDP Not found");
	}
	serialInit(COM1, UART_BAUD_9600);

	initSysCalls();
	initTasking();
    
    LOG_INFO("Keyboard: %s", initKeyboard() == 0 ? "initialized" : "initialization failed");

	Task_t* t1 = makeTaskFromELF(testModule);
	runTask(t1);

	uint32_t pid = getPID();

    sleep(2000);
	LOG_INFO("Message from kernel! PID %d Ring %d", pid, getCPL());

    while (true) {
        sleep(2000);
    }

	LOG_INFO("Upime %d seconds", getUptime() / 1000);

	return 0xDEADBABA;
}