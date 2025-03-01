#include "tss.h"

void x86_64_tss_set_rsp0(x86_64_tss_t *tss, uptr stack_pointer) {
  tss->rsp0_lower = (u32)(u64)stack_pointer;
  tss->rsp0_upper = (u32)((u64)stack_pointer >> 32);
}
