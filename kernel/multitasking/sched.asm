struc Thread
    .rsp resq 1
    ; ...
endstruc

; Thread* switch_context(Thread* old, Thread* new);
;     rdi: old
;     rsi: new
global switch_context
switch_context:
    ; Save old thread's rflags
    pushfq

    ; SYSV ABI saved regs
    push rbx
    push rbp
    push r15
    push r14
    push r13
    push r12

    ; Save rsp into old thread
    mov [rdi + Thread.rsp], rsp
    ; load rsp from new thread
    mov rsp, [rsi + Thread.rsp]

    ; Restore SYSV ABI saved regs
    pop r12
    pop r13
    pop r14
    pop r15
    pop rbp
    pop rbx

    ; pop new thread's rflags
    popfq

    ; Return the old thread
    mov rax, rdi
    ret

global enter_userspace
enter_userspace:

