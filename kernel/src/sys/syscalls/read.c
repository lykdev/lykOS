#include "syscall.h"

#include <common/log.h>
#include <fs/vfs.h>

int syscall_read(int fd, u64 count, void *buf) {
  log("READ");
  proc_t *proc = syscall_get_proc();

  vfs_node_t *node = resource_get(&proc->resource_table, fd)->node;
  if (node == NULL)
    return -1;

  node->ops->read(node, 0, count, buf);
  return 0;
}
