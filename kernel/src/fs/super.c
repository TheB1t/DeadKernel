#include <fs/fs.h>

void sb_add_inode(struct super_block* sb, struct inode* node) {
    list_add(&node->parent_list, &sb->child_nodes);
}

struct inode* sb_new_inode(struct super_block* sb) {
    if (VFS_OPS(sb)->alloc_inode)
        goto _error;

    struct inode* node = VFS_OPS(sb)->alloc_inode(sb);

    if (node) {
        INIT_LIST_HEAD(&node->child_nodes);
        sb_add_inode(sb, node);
    }

    return node;

_error:
    return NULL;
}

int32_t sb_load(struct super_block* sb) {
    if (VFS_OPS(sb)->load_super == NULL)
        goto _error;

    return VFS_OPS(sb)->load_super(sb);

_error:
    return -1;
}

int32_t sb_put(struct super_block* sb) {
    if (VFS_OPS(sb)->put_super == NULL)
        goto _error;

    return VFS_OPS(sb)->put_super(sb);

_error:
    return -1;
}


int32_t sb_mount(struct super_block* sb) {
    if (VFS_TYPE(sb)->mount == NULL)
        goto _error;

    return VFS_TYPE(sb)->mount(sb);

_error:
    return -1;
}

int32_t sb_umount(struct super_block* sb) {
    if (VFS_TYPE(sb)->umount == NULL)
        goto _error;

    return VFS_TYPE(sb)->umount(sb);

_error:
    return -1;
}

int32_t sb_bread(struct super_block* sb, uint32_t block, uint8_t* dbuf) {
    return bd_read_block(VFS_BD(sb), block, dbuf);
}

int32_t sb_bwrite(struct super_block* sb, uint32_t block, uint8_t* dbuf) {
    return bd_write_block(VFS_BD(sb), block, dbuf);
}