#include <common/log.h>

u64 syscall_debug_log(const char *str) {
  log("DEBUG LOG: %s", str);

  return 0;
}
