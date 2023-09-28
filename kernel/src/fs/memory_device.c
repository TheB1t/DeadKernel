#include <fs/memory_device.h>

int32_t md_read(void* ptr, uint32_t block_number, uint8_t* data) {
    MemoryDevice_t* dev = (MemoryDevice_t*)ptr;

    uint32_t address = dev->address + (dev->device->block_size * block_number);
    uint32_t len = dev->device->block_size;

    serialprintf(COM1, "md_read(0x%08x, 0x%08x, %d)\n", address, data, len);

    memcpy(data, (uint8_t*)address, len);

    return BD_SUCCESS;
}

int32_t md_write(void* ptr, uint32_t block_number, uint8_t* data) {
    MemoryDevice_t* dev = (MemoryDevice_t*)ptr;

    uint32_t address = dev->address + (dev->device->block_size * block_number);
    uint32_t len = dev->device->block_size;


    serialprintf(COM1, "md_write(0x%08x, 0x%08x, %d)\n", address, data, len);

    memcpy((uint8_t*)address, data, len);
    
    return BD_SUCCESS;
}

BD_Callbacks_t md_callbacks = {
    .init = NULL,
    .free = NULL,
    .read = md_read,
    .write = md_write,
};

MemoryDevice_t* md_init(uint32_t address, uint32_t size, uint32_t block_size) {
    if (size == 0 || block_size == 0)
        goto _error;

    MemoryDevice_t* md_dev = (MemoryDevice_t*)kmalloc(sizeof(MemoryDevice_t));

    if (md_dev == NULL)
        goto _error;

    md_dev->address = address;
    md_dev->device = bd_init(size, block_size, (void*)md_dev, &md_callbacks);

    if (md_dev->device == NULL)
        goto _error;

    return md_dev;

_error:
    md_free(md_dev);
    return NULL;
}

void md_free(MemoryDevice_t* dev) {
    if (dev) {
        bd_free(dev->device);
        kfree(dev);
    }
}