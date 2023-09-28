#include <fs/fs.h>

struct list_head filesystem_list;
struct list_head root_node;

int32_t get_inode_by_name(struct list_head* root, const char* name, struct inode** node) {
    char _name[VFS_MAX_NAME];

    struct list_head* iter;
    list_for_each(iter, root) {
        struct inode* _node = list_entry(iter, struct inode, parent_list);
        memset(_name, 0, VFS_MAX_NAME);

        _node->ops->get_name(_node, name);
        if (strcmp(name, name) == 0) {
            *node = _node;
            return 0;
        }
    }

    *node = NULL;
    return -1;
}

int32_t get_inode_by_path(struct list_head* root, const char* path, struct inode** node) {
    if (strcmp(path, "/") != 0) {
        char* dir_name = strtok((char*)(path + 1), "/");

        struct inode* _node = NULL;
        while (true) {
            if (get_inode_by_name(root, dir_name, &_node) != 0)
                goto _error;
            
            root = &_node->child_nodes;

            dir_name = strtok(NULL, "/");
            if (dir_name == NULL)
                break;
        }

        *node = _node;
        return 0;
    }

_error:
    *node = NULL;
    return -1;
}

int32_t vfs_init() {

}

int32_t vfs_register_fs(struct filesystem_type* fs_type) {
    list_add_tail(LIST_GET_LIST(fs_type), &filesystem_list);

    return 0;
}

int32_t vfs_mount(BlockDevice_t* bd, const char* fs_name, const char* path) {
    struct filesystem_type* fs_type;
    struct inode* node;
    struct list_head *iter, *insert_node;
    list_for_each(iter, &filesystem_list) {
        fs_type = list_entry(iter, struct filesystem_type, list);

        if (strcmp(fs_type->name, fs_name) == 0)
            break;
    }

    if (iter == &filesystem_list)
        goto _error;

    insert_node = NULL;
    if (strcmp(path, "/") != 0) {
        node = NULL;

        if (get_inode_by_path(&root_node, path, &node) != 0)
            goto _error;

        if (node->type != VFS_INODE_TYPE_DIR)
            goto _error;

        insert_node = &node->child_nodes;
    } else {
        insert_node = &root_node;
    }

    struct super_block* sb = (struct super_block*)kmalloc(sizeof(struct super_block));

    if (sb == NULL)
        goto _error;

    INIT_LIST_HEAD(&sb->child_nodes);
    VFS_TYPE(sb) = fs_type;

    if (sb_mount(sb) != 0)
        goto _free_sb;

    sb_load(sb);

    return 0;

_free_sb:
    kfree(sb);
_error:
    return -1;
}

int32_t vfs_umount(const char* path) {
    struct inode* node;

    if (get_inode_by_path(&root_node, path, &node) != 0)
        goto _error;
    
    struct super_block* sb = VFS_SB(node);

    // TODO: Need to free all nodes and check mountpoints

    sb_put(sb);
    sb_umount(sb);
    
    kfree(sb);
_error:
    return -1;
}

int32_t vfs_mkdir(const char* path) {
    
}

int32_t vfs_rmdir(const char* path) {

}

int32_t vfs_open(const char* path) {

}