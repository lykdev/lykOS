[global arch_context_switch]

TASK_STRUCT_RSP_OFFSET equ 17

arch_context_switch:
    ; rdi - old task
    ; rsi - new task

    ; Most regs are already pushed/popped automatically according to the Sys V ABI.

    ; Save the old task regs.
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    mov qword [rdi + TASK_STRUCT_RSP_OFFSET], rsp

    ; Load the new task regs
    mov rsp, qword [rsi + TASK_STRUCT_RSP_OFFSET]
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx

    ; Return the old task.
    mov rax, rdi
    ret