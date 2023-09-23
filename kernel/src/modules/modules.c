#include <modules/modules.h>

multiboot_mods_t* __modules = NULL;
uint32_t __modules_count = 0;

void initModules(multiboot_mods_t* modules, uint32_t count) {
    __modules = modules;
    __modules_count = count;
}

char* getModuleName(ELF32Header_t* module) {
    ELF32SectionHeader_t* module_info_section = ELFLookupSectionByName(module, ".module_info");
    if (module_info_section == NULL)
        return NULL;

	ModuleInfo_t* module_info = (ModuleInfo_t*)((uint8_t*)module + module_info_section->offset);
    // memPrint(module_info, sizeof(ModuleInfo_t));

    return (char*)&module_info->moduleName;
}

ELF32Header_t* getModuleByName(char* name) {
    for (uint32_t i = 0; i < __modules_count; i++) {
        ELF32Header_t* module = (ELF32Header_t*)__modules[i].mod_start;
        char* module_name = getModuleName((void*)module);

        if (module_name == NULL)
            continue;

        if (strcmp(name, module_name))
            continue;

        return module;
    }

    return NULL;
}