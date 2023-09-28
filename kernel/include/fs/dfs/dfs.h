#pragma once

#include <stdio.h>
#include <fs/fs.h>

#define DFS_SIGNATURE   0x44465300
#define DFS_VERSION     0x01

#define DFS_MAX_AREAS   64
#define DFS_NAME_SIZE   32

#define DFS_AREA_FREE   0x0
#define DFS_AREA_DIR    0x1
#define DFS_AREA_FILE   0x2
#define DFS_AREA_BAD    0xF

#define DFS_INFO(val)   ((struct __dfs_info*)((val)->fs_info))
#define DFS_SB(val)     (DFS_INFO(val)->super)
#define DFS_AREAS(val)  (DFS_INFO(val)->areas)
#define DFS_NODE(val)   ((struct dfs_node*)((val)->fs_info))

struct dfs_data_area {
    uint32_t    start_block;
    uint32_t    size;
};

struct dfs_node {
    uint8_t     type;
    uint8_t     flags;
    uint16_t    areas[DFS_MAX_AREAS];

    const char  name[DFS_NAME_SIZE];
};

struct dfs_super_block {
    uint32_t    magic;
    uint8_t     version;
    uint8_t     block_size;

    uint32_t    volume_size;   
    uint32_t    data_area_size;
};

struct __dfs_info {
    struct dfs_super_block* super;
    struct dfs_data_area* areas;
    uint32_t entries;
};

int32_t init_dfs_fs();