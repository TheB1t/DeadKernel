#pragma once

#include <multiboot.h>
#include <utils/common.h>
#include <memory_managment/kheap.h>
#include <modules/modules_types.h>
#include <fs/elf/elf.h>

void            initModules(multiboot_mods_t* modules, uint32_t count);
char*           getModuleName(ELF32Obj_t* module);
bool            getModuleByName(char* name, ELF32Obj_t* module);