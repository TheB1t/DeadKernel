#pragma once

#include <multiboot.h>
#include <utils/common.h>
#include <modules/modules_types.h>
#include <fs/elf/elf.h>

void            initModules(multiboot_mods_t* modules, uint32_t count);
char*           getModuleName(ELF32Header_t* module);
ELF32Header_t*  getModuleByName(char* name);