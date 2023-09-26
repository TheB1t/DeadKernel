#pragma once

#include <utils/common.h>

#define RSDP_SIGNATURE  "RSD PTR "
#define RSDT_SIGNATURE  "RSDT"
#define XSDT_SIGNATURE  "XSDT"
#define FACP_SIGNATURE  "FACP"
#define HPET_SIGNATURE  "HPET"

typedef struct {
	uint8_t     signature[8];
	uint8_t     checksum;
	uint8_t     OEMID[6];
	uint8_t     revision;
	uint32_t    RSTDAddress;
} RSDP_t;

typedef struct {
	RSDP_t      firstPart;
	uint32_t    length;
	uint64_t    XSDTAddress;
	uint8_t     extendedChecksum;
	uint8_t     reserved[3];
} RSDP2_t;

typedef struct {
	uint8_t       signature[4];
	uint32_t      length;
	uint8_t       revision;
	uint8_t       checksum;
	uint8_t       OEMID[6];
	uint8_t       OEMTableID[8];
	uint32_t      OEMRevision;
	uint32_t      CreatorID;
	uint32_t      CreatorRevision;
} SDTHeader_t;

typedef struct {
	SDTHeader_t   header;
	uint32_t      entry0;
} RSDT_t;

typedef struct {
	SDTHeader_t   header;
	uint64_t      entry0;
} XSDT_t;

typedef struct {
	uint8_t       addressSpace;
	uint8_t       bitWidth;
	uint8_t       bitOffset;
	uint8_t       accessSize;
	uint32_t      address[2];
} GenericAddressStructure_t;

typedef struct {
	SDTHeader_t   header;
	uint8_t       hardware_rev_id;
	uint8_t       comparator_count : 5;
	uint8_t       counter_size : 1;
	uint8_t       reserved : 1;
	uint8_t       legacy_replacement : 1;
	uint16_t      pci_vendor_id;
	GenericAddressStructure_t address;
	uint8_t       hpet_number;
	uint16_t      minimum_tick;
	uint8_t       page_protection;
} HPET_t;

typedef struct {
	SDTHeader_t header;
	uint32_t FirmwareCtrl;
	uint32_t Dsdt;
 
	// field used in ACPI 1.0; no longer in use, for compatibility only
	uint8_t  Reserved;
 
	uint8_t  PreferredPowerManagementProfile;
	uint16_t SCI_Interrupt;
	uint32_t SMI_CommandPort;
	uint8_t  AcpiEnable;
	uint8_t  AcpiDisable;
	uint8_t  S4BIOS_REQ;
	uint8_t  PSTATE_Control;
	uint32_t PM1aEventBlock;
	uint32_t PM1bEventBlock;
	uint32_t PM1aControlBlock;
	uint32_t PM1bControlBlock;
	uint32_t PM2ControlBlock;
	uint32_t PMTimerBlock;
	uint32_t GPE0Block;
	uint32_t GPE1Block;
	uint8_t  PM1EventLength;
	uint8_t  PM1ControlLength;
	uint8_t  PM2ControlLength;
	uint8_t  PMTimerLength;
	uint8_t  GPE0Length;
	uint8_t  GPE1Length;
	uint8_t  GPE1Base;
	uint8_t  CStateControl;
	uint16_t WorstC2Latency;
	uint16_t WorstC3Latency;
	uint16_t FlushSize;
	uint16_t FlushStride;
	uint8_t  DutyOffset;
	uint8_t  DutyWidth;
	uint8_t  DayAlarm;
	uint8_t  MonthAlarm;
	uint8_t  Century;
 
	// reserved in ACPI 1.0; used since ACPI 2.0+
	uint16_t BootArchitectureFlags;
 
	uint8_t  Reserved2;
	uint32_t Flags;
 
	// 12 byte structure; see below for details
	GenericAddressStructure_t ResetReg;
 
	uint8_t  ResetValue;
	uint8_t  Reserved3[3];
 
	// 64bit pointers - Available on ACPI 2.0+
	uint64_t                X_FirmwareControl;
	uint64_t                X_Dsdt;
 
	GenericAddressStructure_t X_PM1aEventBlock;
	GenericAddressStructure_t X_PM1bEventBlock;
	GenericAddressStructure_t X_PM1aControlBlock;
	GenericAddressStructure_t X_PM1bControlBlock;
	GenericAddressStructure_t X_PM2ControlBlock;
	GenericAddressStructure_t X_PMTimerBlock;
	GenericAddressStructure_t X_GPE0Block;
	GenericAddressStructure_t X_GPE1Block;
} FADT_t;

RSDP_t* findRSDP(void);
RSDP2_t* findRSDP2(RSDP_t* rsdp);
RSDT_t* findRSDT(RSDP_t* rsdp);
XSDT_t* findXSDT(RSDP2_t* rsdp);
void* findInSDT(char* signature, SDTHeader_t* header);
void* printSDT(SDTHeader_t* header);