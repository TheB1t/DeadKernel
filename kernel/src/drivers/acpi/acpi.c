#include <drivers/acpi/acpi.h>
#include <io/screen.h>
#include <memory_managment/paging.h>

static inline uint8_t checksum(const uint8_t *ptr, uint32_t size) {
    uint8_t sum = 0;
    for (uint32_t i = 0; i < size; i++)
        sum += ptr[i];
    return sum;
}

static inline bool doChecksum(SDTHeader_t* header) {
    return checksum((uint8_t*)header, header->length) == 0;
}

extern RSDP_t* ll_find_rsdp(void);

RSDP_t* findRSDP(void) {
    RSDP_t* ptr = ll_find_rsdp();
    if (!ptr)
        return NULL;

    if (strncmp((char*)&ptr->signature, RSDP_SIGNATURE, 8) == 0) {
        if (checksum((uint8_t*)ptr, sizeof(RSDP_t)) == 0)
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
    if (strncmp((char*)&rsdt->header.signature, RSDT_SIGNATURE, 4) == 0) {
        if (doChecksum(&rsdt->header))
            return rsdt;
    }
    return NULL;     
}

XSDT_t* findXSDT(RSDP2_t* rsdp) {
    if (rsdp == NULL)
        return NULL;

    XSDT_t* xsdt = (XSDT_t*)rsdp->XSDTAddress.low;    
    if (strncmp((char*)&xsdt->header.signature, XSDT_SIGNATURE, 4) == 0) {
        if (doChecksum(&xsdt->header))
            return xsdt;
    }
    return NULL;     
}

void* findInSDT(char* signature, SDTHeader_t* header) {
    
    if (strncmp((char*)&header->signature, XSDT_SIGNATURE, 4) == 0) {
        XSDT_t* xsdt = (XSDT_t*)header;
        uint64_t* table = &xsdt->entry0;
        int entries = (xsdt->header.length - sizeof(uint64_t)) / sizeof(uint64_t);
        
        for (int i = 0; i < entries; i++) {
            SDTHeader_t* header = (SDTHeader_t*)table[i].low;

            if (strncmp((char*)header->signature, signature, 4) == 0)
                if (doChecksum(header))
                    return (void*)header;
        }
    } else {
        RSDT_t* rsdt = (RSDT_t*)header;
        uint32_t* table = &rsdt->entry0;
        int entries = (rsdt->header.length - sizeof(uint32_t)) / sizeof(uint32_t);

        for (int i = 0; i < entries; i++) {
            SDTHeader_t* header = (SDTHeader_t*)table[i];

            if (strncmp((char*)header->signature, signature, 4) == 0)
                if (doChecksum(header))
                    return (void*)header;
        }
    }

    return NULL;
}

void* printSDT(SDTHeader_t* header) {
    uint8_t signature[5];

    if (strncmp((char*)&header->signature, XSDT_SIGNATURE, 4) == 0) {
        XSDT_t* xsdt = (XSDT_t*)header;
        uint64_t* table = &xsdt->entry0;
        int entries = (xsdt->header.length - sizeof(uint64_t)) / sizeof(uint64_t);
        
        printf("XSDT entries: ");

        for (int i = 0; i < entries; i++) {
            SDTHeader_t* header = (SDTHeader_t*)table[i].low;
            
            memcpy((void*)signature, (void*)header->signature, 4);
            signature[4] = '\0';

            printf("%s ", signature);
        }
    } else {
        RSDT_t* rsdt = (RSDT_t*)header;
        uint32_t* table = &rsdt->entry0;
        int entries = (rsdt->header.length - sizeof(uint32_t)) / sizeof(uint32_t);

        printf("RSDT entries: ");

        for (int i = 0; i < entries; i++) {
            SDTHeader_t* header = (SDTHeader_t*)table[i];

            memcpy((void*)signature, (void*)header->signature, 4);
            signature[4] = '\0';

            printf("%s ", signature);
        }
    }

    printf("\n");

    return NULL;
}