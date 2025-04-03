#pragma once

#define __MODULE_INFO(tag, info) \
    static const char __module_##tag[] __attribute__((section(".module_info"))) = info

#define MODULE_NAME(n)           __MODULE_INFO(name, n)
#define MODULE_AUTHOR(auth)      __MODULE_INFO(author, auth)
#define MODULE_DESCRIPTION(desc) __MODULE_INFO(description, desc)
#define MODULE_LICENSE(lic)      __MODULE_INFO(license, lic)
#define MODULE_VERSION(ver)      __MODULE_INFO(version, ver)
#define MODULE_DEPENDS(deps)     __MODULE_INFO(depends, deps)

#define MODULE_INSTALL(fn) void *__module_install __attribute__((section(".module_info"))) = fn;
#define MODULE_DESTROY(fn) void *__module_destroy __attribute__((section(".module_info"))) = fn;
#define MODULE_PROBE(fn)   void *__module_probe __attribute__((section(".module_info"))) = fn;