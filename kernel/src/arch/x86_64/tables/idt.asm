extern arch_int_handler
global arch_int_stub_table

%macro SWAPGS_CONDITIONAL 0
        test qword [rsp + 24], 3
        jz %%noswap
        swapgs
    %%noswap:
%endmacro

isr_stub:
    cld                                                     ; Clear direction flag

    SWAPGS_CONDITIONAL

    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    xor rbp, rbp
    mov rdi, rsp                                            ; RDI to be used as a pointer to the int frame
    call arch_int_handler                                   ; Call interrupt handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax

    SWAPGS_CONDITIONAL

    add rsp, 16                                            ; Discard interrupt number and error code
    iretq

%macro ISR 2
    global isr_%1
    isr_%1:
        %if %2 == 0
            push qword 0                                    ; Push 0 as error code for interrupt without error code
        %endif
        push qword %1                                       ; Push interrupt number
        jmp isr_stub
%endmacro

ISR  0, 0
ISR  1, 0
ISR  2, 0
ISR  3, 0
ISR  4, 0
ISR  5, 0
ISR  6, 0
ISR  7, 0
ISR  8, 1
ISR  9, 0
ISR 10, 1
ISR 11, 1
ISR 12, 1
ISR 13, 1
ISR 14, 1
ISR 15, 0
ISR 16, 0
ISR 17, 1
ISR 18, 0
ISR 19, 0
ISR 20, 0
ISR 21, 1
ISR 22, 0
ISR 23, 0
ISR 24, 1
ISR 25, 0
ISR 26, 0
ISR 27, 0
ISR 28, 1
ISR 29, 1
ISR 30, 1
ISR 31, 0

%assign i 32
%rep 224
    ISR i, 0
%assign i i+1
%endrep

section .data

arch_int_stub_table:
%assign i 0
%rep 256
    dq isr_%+i
%assign i i+1
%endrep
