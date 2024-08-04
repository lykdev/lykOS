[global arch_context_switch]

[extern sched_queue_add_task]

; rdi - current task
; rsi - new task
arch_context_switch:
;   Save old task state.
    push rax
    push rcx
    push rdx
    push rbx
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

    mov qword [rdi + 8], rsp

;   Load new task state.
    mov rsp, qword [rsi + 8]

;   Add the old task back to the scheduler's queue.
;   We do this after loading the new stack.
;   rdi - still holds the old task as the first argument
    call sched_queue_add_task 

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
    pop rbx
    pop rdx
    pop rcx
    pop rax

    ret
