#pragma once

#include <fs/vfs.h>
#include <sys/proc.h>

proc_t *exec_load(vnode_t *file);
