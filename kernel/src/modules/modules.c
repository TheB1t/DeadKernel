#include <modules/modules.h>

multiboot_mods_t* __modules = NULL;
uint32_t __modules_count = 0;

void initModules(multiboot_mods_t* modules, uint32_t count) {
    __modules = modules;
    __modules_count = count;
}

char* getModuleName(ELF32Obj_t* module) {
    ELF32SectionHeader_t* module_info_section = ELFLookupSectionByName(module, ".module_info");
    if (module_info_section == NULL)
        return NULL;

	ModuleInfo_t* module_info = (ModuleInfo_t*)((uint8_t*)module->header + module_info_section->offset);

    return (char*)&module_info->moduleName;
}

bool getModuleByName(char* name, ELF32Obj_t* module) {
    for (uint32_t i = 0; i < __modules_count; i++) {
        memset(module, 0, sizeof(ELF32Obj_t));

        if (!ELFLoad(__modules[i].mod_start, module))
            continue;

        char* module_name = getModuleName(module);

        if (module_name == NULL)
            continue;

        if (strcmp(name, module_name))
            continue;

        return true;
    }

    return false;
}