.global x86_64_thread_userspace_init

x86_64_thread_userspace_init:
    pop %rcx

    cli
    swapgs

    pop %rax
    mov %rax, %rsp

    xor %rbp, %rbp
    xor %rax, %rax
    xor %rbx, %rbx
	xor %rdx, %rdx
	xor %rsi, %rsi
	xor %rdi, %rdi
	xor %r8, %r8
	xor %r9, %r9
	xor %r10, %r10
	xor %r12, %r12
	xor %r13, %r13
	xor %r14, %r14
	xor %r15, %r15

    mov $((1 << 9) | (1 << 1)), %r11
    sysretq