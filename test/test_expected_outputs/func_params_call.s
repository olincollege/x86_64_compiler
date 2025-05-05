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
        pop     rbp
        ret
