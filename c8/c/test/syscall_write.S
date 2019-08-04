section .data
str_c_lib: db "c libray truthofos",0xa
str_c_lib_len equ $-str_c_lib

str_sys_call: db "c system call truthofos",0xa
str_sys_call_len equ $-str_sys_call

section .text
global _start

_start:
	push str_c_lib_len
	push str_c_lib
	push 1
	
	call simu_write
	add esp, 12

	mov eax,4
	mov ebx,1
	mov ecx,str_sys_call
	mov edx,str_sys_call_len
	int 0x80
	mov eax,1
	int 0x80

simu_write:
	push ebp
	mov ebp,esp
	mov eax,4
	mov ebx,[ebp+8]
	mov ecx,[ebp+12]
	mov edx,[ebp+16]
	int 0x80
	pop ebp
	ret
