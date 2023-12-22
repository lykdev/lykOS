[extern isr_handler]
[extern irq_handler]
[global idt_stub_addr_table]

%macro PUSH_REGS 0
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro POP_REGS 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

%macro isr_no_err_stub 1
int_stub_%1:
    push 0
    push %1
    PUSH_REGS

    mov rdi, rsp
    call isr_handler

    POP_REGS

    add rsp, 16
    iretq
%endmacro

%macro isr_err_stub 1
int_stub_%1:
    push %1
    PUSH_REGS

    mov rdi, rsp
    call isr_handler

    POP_REGS

    add rsp, 16
    iretq
%endmacro

%macro irq_stub 1
int_stub_%1:
    push 0
    push %1
    PUSH_REGS

    mov rdi, rsp
    call irq_handler

    POP_REGS

    add rsp, 16
    iretq
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

irq_stub 32; IRQ 0

section .data

idt_stub_addr_table:
    %assign i 0
    %rep 33
        dq int_stub_%+i
        %assign i i+1
    %endrep