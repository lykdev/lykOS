#pragma once

#define __MODULE_INFO(tag, info) \
    static const char __module_##tag[] __attribute__((section(".module_info"))) = info

#define MODULE_AUTHOR(name)      __MODULE_INFO(author, name)
#define MODULE_DESCRIPTION(desc) __MODULE_INFO(description, desc)
#define MODULE_LICENSE(lic)      __MODULE_INFO(license, lic)
#define MODULE_VERSION(ver)      __MODULE_INFO(version, ver)
#define MODULE_DEPENDS(deps)     __MODULE_INFO(depends, deps)

#define MODULE_INIT(fn) void *__module_init __attribute__((section(".module_info"))) = fn;
#define MODULE_EXIT(fn) void *__module_exit __attribute__((section(".module_info"))) = fn;