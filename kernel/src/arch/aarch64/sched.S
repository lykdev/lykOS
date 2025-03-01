.global arch_sched_context_switch
.extern sched_drop

arch_sched_context_switch:
    stp x0,  x1,  [sp, #-16]!  
    stp x2,  x3,  [sp, #-16]!  
    stp x4,  x5,  [sp, #-16]!  
    stp x6,  x7,  [sp, #-16]!  
    stp x8,  x9,  [sp, #-16]!  
    stp x10, x11, [sp, #-16]!  
    stp x12, x13, [sp, #-16]!  
    stp x14, x15, [sp, #-16]!  
    stp x16, x17, [sp, #-16]!  
    stp x18, x19, [sp, #-16]!  
    stp x20, x21, [sp, #-16]!  
    stp x22, x23, [sp, #-16]!  
    stp x24, x25, [sp, #-16]!  
    stp x26, x27, [sp, #-16]!  
    stp x28, x29, [sp, #-16]!  
    str x30,      [sp, #-16]!

    // Save current stack pointer in x0
    mov x2, sp  
    str x2, [x0]  

    stp x0, x1, [sp, #-16]!
    bl sched_drop
    ldp x0, x1, [sp], #16

    // Load new stack pointer from x1
    ldr x2, [x1]  
    mov sp, x2  

    ldr x30,      [sp], #16  
    ldp x28, x29, [sp], #16  
    ldp x26, x27, [sp], #16  
    ldp x24, x25, [sp], #16  
    ldp x22, x23, [sp], #16  
    ldp x20, x21, [sp], #16  
    ldp x18, x19, [sp], #16  
    ldp x16, x17, [sp], #16  
    ldp x14, x15, [sp], #16  
    ldp x12, x13, [sp], #16  
    ldp x10, x11, [sp], #16  
    ldp x8,  x9,  [sp], #16  
    ldp x6,  x7,  [sp], #16  
    ldp x4,  x5,  [sp], #16  
    ldp x2,  x3,  [sp], #16  
    ldp x0,  x1,  [sp], #16  

    ret
