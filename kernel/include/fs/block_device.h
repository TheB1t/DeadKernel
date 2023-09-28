#pragma once

#include <stdlib.h>
#include <memory_managment/kheap.h>

#define BD_ERROR(code)          -(code)
#define BD_SUCCESS              0
#define BD_FAILED               BD_ERROR(0x1)
#define BD_INVALID_ARGUMENT     BD_ERROR(0x2)
#define BD_READ_FAILED          BD_ERROR(0x3)
#define BD_WRITE_FAILED         BD_ERROR(0x4)

typedef int32_t (*BD_Init_t)(void* ptr);
typedef int32_t (*BD_Read_t)(void* ptr, uint32_t block_number, uint8_t* data);
typedef int32_t (*BD_Write_t)(void* ptr, uint32_t block_number, uint8_t* data);
typedef int32_t (*BD_Free_t)(void* ptr);

typedef struct {
    BD_Init_t init;
    BD_Read_t read;
    BD_Write_t write;
    BD_Free_t free;
} BD_Callbacks_t;

typedef struct {
    uint32_t    size;
    uint32_t    block_size;
    uint32_t    block_count;
    void*       ptr;
    uint8_t*    buffer;
    BD_Callbacks_t* callbacks;
} BlockDevice_t;

BlockDevice_t*  bd_init(uint32_t size, uint32_t block_size, void* ptr, BD_Callbacks_t* callbacks);
void            bd_free(BlockDevice_t* device);
int32_t         bd_read_block(BlockDevice_t* device, uint32_t block_number, uint8_t* buffer);
int32_t         bd_write_block(BlockDevice_t* device, uint32_t block_number, uint8_t* data);