.intel_syntax noprefix
.global _start
.text
_start:
    call main
    mov rdi, rax       # syscall: exit
    mov rax, 60        # exit code 0
    syscall
main:
        push    rbp
        mov     rbp, rsp
        mov     eax, 3
        pop     rbp
        ret
