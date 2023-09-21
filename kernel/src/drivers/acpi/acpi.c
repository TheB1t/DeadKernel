#include <drivers/acpi/acpi.h>
#include <io/screen.h>
#include <memory_managment/paging.h>

#define RSDP_SIGNATURE  "RSD PTR "
#define RSDP2_SIGNATURE "XSDT"
#define FACP_SIGNATURE  "FACP"

static inline uint8_t checksum(const uint8_t *ptr, uint32_t size) {
    uint8_t sum = 0;
    for (uint32_t i = 0; i < size; i++)
        sum += ptr[i];
    return sum;
}

extern RSDP_t* ll_find_rsdp(void);

RSDP_t* findRSDP(void) {
    RSDP_t* ptr = ll_find_rsdp();
    if (!ptr)
        return NULL;

    if (strncmp((char*)&ptr->signature, RSDP_SIGNATURE, 8) == 0) {
        if (checksum(ptr, sizeof(RSDP_t)) == 0)
            return (RSDP_t *)ptr;
    }

    return NULL;
}

RSDP2_t* findRSDP2(RSDP_t* rsdp) {
    if (rsdp == NULL)
        return NULL;

    if (rsdp->revision != 2)
        return NULL;

    if (checksum((uint8_t*)rsdp, sizeof(RSDP2_t)) == 0)
        return (RSDP2_t *)rsdp;   

    return NULL;     
}

RSDT_t* findRSDT(RSDP_t* rsdp) {
    if (rsdp == NULL)
        return NULL;

    RSDT_t* rsdt = (RSDT_t*)rsdp->RSTDAddress;
    allocFramesMirrored(rsdp->RSTDAddress, rsdp->RSTDAddress + sizeof(RSDT_t), 1, 0, 1);

    if (strncmp((char*)&rsdt->header.signature, RSDP_SIGNATURE, 4) == 0) {
        if (checksum((uint8_t*)rsdt, sizeof(RSDT_t)) == 0)
            return rsdt;
    }

    freeFrames(rsdp->RSTDAddress, rsdp->RSTDAddress + sizeof(RSDT_t));
    return NULL;     
}


FADT_t* findFACP(RSDT_t* rootSDT) {
    int entries = (rootSDT->header.length - sizeof(rootSDT->header)) / 4;
    
    LOG_INFO("RootSDT: 0x%08x", rootSDT);
    for (int i = 0; i < entries; i++) {
        uint32_t* entry0 = &rootSDT->sdt0;
        SDT_Header_t* header = (SDT_Header_t *)entry0[i];     
        LOG_INFO("ENTRY0: 0x%08x HEADER: 0x%08x I: %d", entry0, header, i);

        // allocFramesMirrored(entry0[i], entry0[i] + sizeof(FADT_t), 1, 0, 1); 

        if (strncmp((char*)header->signature, FACP_SIGNATURE, 4))
            return (FADT_t*)header;

        // freeFrames(entry0[i], entry0[i] + sizeof(FADT_t));
    }
 
    // No FACP found
    return NULL;
}

// uint8_t *find_table(RSDP2_t* rsdp2, const char *signature) {
//     RSDP2_t *rsdp2 = find_rsdp2();
//     if (rsdp2 == NULL)
//         return NULL;
//     // Search for the table signature.
//     uint8_t *start = (uint8_t *)(uintptr_t)rsdp2->XSDTAddress[0];
//     uint8_t *end = start + rsdp2->length;
//     for (uint8_t *ptr = start; ptr < end; ptr += 4) {
//         if (strncmp((char *)ptr, signature, 4) == 0)
//             return ptr;
//     }

//     return NULL;
// }

// int get_table_length(const uint8_t *table) {
//     return *(uint32_t *)(table + 4);
// }

// uint8_t *get_table_entry(const uint8_t *table, int index) {
//     int entry_count = (get_table_length(table) - sizeof(uint32_t)) / sizeof(uint32_t);
//     if (index < 0 || index >= entry_count)
//     return NULL;
//     return *(uint8_t **)(table + sizeof(uint32_t) + index * sizeof(uint32_t));
// }

// int get_table_entry_count(const uint8_t *table) {
//     return (get_table_length(table) - sizeof(uint32_t)) / sizeof(uint32_t);
// }

// const char *acpi_get_version(void) {
//     RSDP2_t *rsdp2 = find_rsdp2();
//     if (rsdp2 == NULL)
//         return NULL;

//     return rsdp2->firstPart.revision == 0 ? "1.0" : "2.0 or later";
// }

// const char *acpi_get_firmware_vendor(void) {
//     uint8_t *rsdt = find_table(RSDT_SIGNATURE);
//     if (rsdt == NULL)
//         return NULL;

//     uint8_t *entry = get_table_entry(rsdt, 0);
//     if (entry == NULL)
//         return NULL;
        
//     return (const char *)entry + 4;
// }