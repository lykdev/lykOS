#include <sys/module.h>

void __module_install()
{;
    //fs_type_register(&fs_type_ext2);
}

void __module_destroy()
{
    //fs_type_unregister(&fs_type_ext2);
}

MODULE_NAME("EXT2")
MODULE_VERSION("0.1")
MODULE_AUTHOR("Matei Lupu")
