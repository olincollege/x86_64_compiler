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
        mov     edx, 3
        mov     eax, 7
        imul    eax, edx
        pop     rbp
        ret
