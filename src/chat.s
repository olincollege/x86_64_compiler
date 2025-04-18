.intel_syntax noprefix
.global _start
.text
_start:
    call main
    mov rdi, rax       # syscall: exit
    mov rax, 60        # exit code 0
    syscall

test:
        push    rbp
        mov     rbp, rsp
        mov     DWORD PTR [rbp-4], edi
        mov     DWORD PTR [rbp-8], esi
        mov     eax, 12
        mov     DWORD PTR [rbp-12], eax
        mov     eax, DWORD PTR [rbp-12]
        pop     rbp
        ret
main:
        push    rbp
        mov     rbp, rsp
        mov     eax, 0
        mov     DWORD PTR [rbp-4], eax
        mov     eax, 1
        mov     edi, eax
        mov     eax, 2
        mov     esi, eax
        call    test
        mov     DWORD PTR [rbp-8], eax
        mov     eax, 1
        mov     edi, eax
        mov     eax, 2
        mov     esi, eax
        call    test
        pop     rbp
        ret
        mov     eax, DWORD PTR [rbp-8]
        pop     rbp
        ret
