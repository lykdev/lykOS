.global arch_int_setup
.extern arch_int_handler 

arch_int_setup:
    mov x0, sp
    mov x1, #1
    msr spsel, x1
    mov sp, x0

    adr x0, arch_exception_vector
    msr vbar_el1, x0

    ret

int_stub:
    mrs x1, esr_el1
    mrs x2, elr_el1
    mrs x3, spsr_el1
    mrs x4, far_el1

    bl arch_int_handler

.align 11
arch_exception_vector:
.align  7
    mov x0, #0
    b int_stub
.align  7
    mov x0, #1
    b int_stub
.align  7
    mov x0, #2
    b int_stub
.align  7
    mov x0, #3
    b int_stub

.align  7
    mov x0, #4
    b int_stub
.align  7
    mov x0, #5
    b int_stub
.align  7
    mov x0, #6
    b int_stub
.align  7
    mov x0, #7
    b int_stub

.align  7
    mov x0, #8
    b int_stub
.align  7
    mov x0, #9
    b int_stub
.align  7
    mov x0, #10
    b int_stub
.align  7
    mov x0, #11
    b int_stub

.align  7
    mov x0, #12
    b int_stub
.align  7
    mov x0, #13
    b int_stub
.align  7
    mov x0, #14
    b int_stub
.align  7
    mov x0, #15
    b int_stub
