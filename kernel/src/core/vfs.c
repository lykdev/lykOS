#include "vfs.h"

#include <lib/log.h>
#include <lib/heap.h>
#include <lib/slock.h>
#include <lib/assert.h>
#include <lib/str.h>

list_t mountpoint_list = LIST_INIT;

vfs_mountpoint_t *vfs_mount(const char *dev, const char *fs_type, const char *path)
{
    vfs_mountpoint_t *mp = malloc(sizeof(vfs_mountpoint_t));

    strcpy(mp->dev, dev);
    strcpy(mp->fs_type, fs_type);
    strcpy(mp->mount_path, path);

    list_append(&mountpoint_list, &mp->list_element);

    return mp;
}

vfs_mountpoint_t *vfs_get_mountpoint(const char *path)
{
    int max_len = 0;
    vfs_mountpoint_t *ret;

    FOREACH (n, mountpoint_list)
    {
        vfs_mountpoint_t *mp = LIST_GET_CONTAINER(n, vfs_mountpoint_t, list_element);
        
        int match = strmatch(path, mp->mount_path);

        /**
         * This prevents paths like `/home/matei123/test` from being interpreted
         * as belonging to `/home/matei'.
         */
        if (path[match] != '/')
            continue;

        if (match > max_len)
        {
            max_len = match;
            ret = mp;
        }
    }

    return ret;
}

void vfs_init()
{
    vfs_mount("NONE", "NONE", "/");
    vfs_mount("NONE", "NONE", "/sys");
    vfs_mount("NONE", "NONE", "/home");
    vfs_mount("NONE", "NONE", "/home/matei");
    vfs_mount("NONE", "NONE", "/mnt");

    log("%s", vfs_get_mountpoint("/home/mateiu/test")->mount_path);
}

void vfs_debug_info()
{
    FOREACH (n, mountpoint_list)
    {
        vfs_mountpoint_t *mp = LIST_GET_CONTAINER(n, vfs_mountpoint_t, list_element);
        
        log("%s %s %s", mp->dev, mp->fs_type, mp->mount_path);
    }    
}
