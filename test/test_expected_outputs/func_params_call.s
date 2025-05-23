.intel_syntax noprefix
.global _start
.text
_start:
    call main
    mov rdi, rax       # syscall: exit
    mov rax, 60        # exit code 0
    syscall
foo:
        push    rbp
        mov     rbp, rsp
        mov     DWORD PTR [rbp-4], edi
        mov     DWORD PTR [rbp-8], esi
        mov     edx, DWORD PTR [rbp-8]
        mov     eax, DWORD PTR [rbp-4]
        add     eax, edx
        mov     DWORD PTR [rbp-12], eax
        mov     eax, DWORD PTR [rbp-12]
        pop     rbp
        ret
main:
        push    rbp
        mov     rbp, rsp
        mov     eax, 2
        mov     edi, eax
        mov     eax, 3
        mov     esi, eax
        call    foo
        mov     DWORD PTR [rbp-4], eax
        mov     eax, DWORD PTR [rbp-4]
        pop     rbp
        ret
