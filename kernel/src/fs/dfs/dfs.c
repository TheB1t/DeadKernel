#include <fs/dfs/dfs_inode.h>
#include <fs/dfs/dfs_super.h>
#include <fs/dfs/dfs.h>


int32_t dfs_read_inode(struct inode* node) {
    struct super_block* sb = VFS_SB(node);
}

int32_t dfs_mount(struct super_block* sb) {
    sb->ops = &dfs_sb_ops;
    
    sb->fs_info = (void*)kmalloc(sizeof(struct __dfs_info));
    DFS_SB(sb) = (struct dfs_super_block*)kmalloc(VFS_BD(sb)->block_size);

    return 0;
}

int32_t dfs_umount(struct super_block* sb) {
    
    kfree(DFS_SB(sb));
    kfree(sb->fs_info);

    return 0;
}

struct filesystem_type dfs_fs_type = {
    .name = "dfs",
    .mount = dfs_mount,
    .umount = dfs_umount,
};

int32_t init_dfs_fs() {
    return vfs_register_fs(&dfs_fs_type);
}
