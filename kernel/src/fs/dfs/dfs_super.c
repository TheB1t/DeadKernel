#include <fs/dfs/dfs_super.h>

struct inode* dfs_alloc_inode(struct super_block* sb) {
    return (struct inode*)kmalloc(sizeof(struct inode));
}

struct dfs_data_area* dfs_get_area_by_index(struct super_block* sb, uint32_t index) {
    if (index > DFS_INFO(sb)->entries)
        goto _error;

    return &DFS_AREAS(sb)[index];
_error:
    return NULL;
}

int32_t dfs_load_super(struct super_block* sb) {
    sb_bread(sb, 0, (uint8_t*)DFS_SB(sb));

    struct dfs_super_block* dfs_sb = DFS_SB(sb);

    if (dfs_sb->magic != DFS_SIGNATURE)
        goto _error;

    struct inode* node = sb_new_inode(sb);

    VFS_SB(node) = sb;
    node->flags |= VFS_INODE_TYPE_DIR;
    DFS_AREAS(sb) = (void*)kmalloc(VFS_BD(sb)->block_size * dfs_sb->data_area_size);
    DFS_INFO(sb)->entries = (VFS_BD(sb)->block_size / sizeof(struct dfs_data_area)) * dfs_sb->data_area_size;

    struct dfs_node* dfs_node = DFS_NODE(node);

    // dfs_node = 
    return 0;

_error:
    return -1;
}

int32_t dfs_put_super(struct super_block* sb) {
    kfree(DFS_AREAS(sb));
}

struct super_block_operations dfs_sb_ops = {
    .alloc_inode = dfs_alloc_inode,
    .put_inode = NULL,

    .load_super = dfs_load_super,
    .put_super = dfs_put_super,
};