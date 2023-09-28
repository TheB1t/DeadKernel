#pragma once

#include <stdio.h>
#include <llist.h>
#include <fs/block_device.h>

#define VFS_OPS(val) ((val)->ops)
#define VFS_TYPE(val) ((val)->type)
#define VFS_SB(val) ((val)->sb)
#define VFS_BD(val) ((val)->bd)

#define VFS_MAX_NAME    128

#define VFS_INODE_TYPE_BAD   0x0
#define VFS_INODE_TYPE_FILE  0x1
#define VFS_INODE_TYPE_DIR   0x2

#define VFS_INODE_FLAG_BAD   0x0
#define VFS_INODE_FLAG_ROOT  0x1

struct inode {
    struct super_block*         sb;
    struct inode_operations*    ops;

    uint8_t     type;
    uint8_t     flags;
    void*       fs_info;

    struct list_head    child_nodes;
    struct list_head    parent_list;
    struct list_head    sb_list;
};

struct super_block {
    BlockDevice_t*                  bd;
    struct super_block_operations*  ops;
    struct filesystem_type*         type;

    void*       fs_info;
    
    struct list_head    child_nodes;
};

struct inode_operations {
    int32_t     (*get_name)(struct inode* node, const char* name);
    int32_t     (*set_name)(struct inode* node, const char* name);
};

struct super_block_operations {
    struct inode*   (*alloc_inode)(struct super_block* sb);
    int32_t         (*put_inode)(struct super_block* sb, struct inode* node);

    int32_t         (*load_super)(struct super_block* sb);
    int32_t         (*put_super)(struct super_block* sb);
};

struct filesystem_type {
    const char*     name;

    int32_t         (*mount)(struct super_block* sb);
    int32_t         (*umount)(struct super_block* sb);
    
    struct list_head list;
};

int32_t     vfs_init();
int32_t     vfs_register_fs(struct filesystem_type* type);

int32_t     vfs_mount(BlockDevice_t* bd, const char* fs_name, const char* path);
int32_t     vfs_umount(const char* path);

int32_t     vfs_mkdir(const char* path);
int32_t     vfs_rmdir(const char* path);

int32_t     vfs_open(const char* path);

struct inode*   sb_new_inode(struct super_block* sb);

int32_t     sb_load(struct super_block* sb);
int32_t     sb_put(struct super_block* sb);

int32_t     sb_mount(struct super_block* sb);
int32_t     sb_umount(struct super_block* sb);

int32_t     sb_bread(struct super_block* sb, uint32_t block, uint8_t* dbuf);
int32_t     sb_bwrite(struct super_block* sb, uint32_t block, uint8_t* dbuf);