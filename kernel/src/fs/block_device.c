#include <fs/block_device.h>

BlockDevice_t* bd_init(uint32_t size, uint32_t block_size, void* ptr, BD_Callbacks_t* callbacks) {
    if (size % block_size != 0 || ptr == NULL)
        return NULL;

    BlockDevice_t* device = (BlockDevice_t*)kmalloc(sizeof(BlockDevice_t));

    if (device == NULL)
        return NULL;

    device->buffer = (uint8_t*)kmalloc(block_size);
    device->ptr = ptr;
    device->size = size;
    device->block_size = block_size;
    device->block_count = size / block_size;
    device->callbacks = callbacks;

    if (device->callbacks->init)
        device->callbacks->init(device->ptr);

    return device;
}

void bd_free(BlockDevice_t* device) {
    if (device != NULL) {
        if (device->callbacks->free)
            device->callbacks->free(device->ptr);

        kfree(device->buffer);
        kfree(device);
    }
}

int32_t __bd_read(BlockDevice_t* device, uint32_t block_number) {
    serialprintf(COM1, "__bd_read(0x%08x, ", device);
    serialprintf(COM1, "0x%08x, ", device->callbacks);
    serialprintf(COM1, "0x%08x)\n", device->callbacks->read);

    if (device->callbacks->read == NULL)
        return BD_READ_FAILED;

    return device->callbacks->read(device->ptr, block_number, device->buffer);
}

int32_t __bd_write(BlockDevice_t* device, uint32_t block_number) {
    serialprintf(COM1, "__bd_write(0x%08x, ", device);
    serialprintf(COM1, "0x%08x, ", device->callbacks);
    serialprintf(COM1, "0x%08x)\n", device->callbacks->write);

    if (device->callbacks->write == NULL)
        return BD_WRITE_FAILED;

    return device->callbacks->write(device->ptr, block_number, device->buffer);
}

int32_t bd_read_block(BlockDevice_t* device, uint32_t block_number, uint8_t* buffer) {
    if (device == NULL || block_number >= device->block_count)
        return BD_INVALID_ARGUMENT;

    int32_t status = __bd_read(device, block_number);
    
    if (status != BD_SUCCESS)
        return status;

    memcpy(buffer, device->buffer, device->block_size);

    return status;
}

int32_t bd_write_block(BlockDevice_t* device, uint32_t block_number, uint8_t* data) {
    if (device == NULL || block_number >= device->block_count)
        return BD_INVALID_ARGUMENT;

    memcpy(device->buffer, data, device->block_size);

    return __bd_write(device, block_number);
}