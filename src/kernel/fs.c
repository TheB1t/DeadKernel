#include "fs.h"

FSNode_t *FSRoot = 0;

uint32_t readDS(FSNode_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->read != 0)
        return node->read(node, offset, size, buffer);
    else
        return 0;
}

uint32_t writeFS(FSNode_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->write != 0)
        return node->write(node, offset, size, buffer);
    else
        return 0;
}

void openFS(FSNode_t* node, uint8_t read, uint8_t write) {
    if (node->open != 0)
        return node->open(node);
}

void closeFS(FSNode_t *node) {
    if (node->close != 0)
        return node->close(node);
}

Dirent_t* readDirFS(FSNode_t* node, uint32_t index) {
    if ((node->flags & FS_DIRECTORY) && node->readDir != 0)
        return node->readDir(node, index);
    else
        return 0;
}

FSNode_t* findDirFS(FSNode_t* node, char* name) {
    if ((node->flags & FS_DIRECTORY) && node->findDir != 0)
        return node->findDir(node, name);
    else
        return 0;
}
