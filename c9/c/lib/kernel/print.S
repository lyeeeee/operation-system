TI_G equ 0
RPL0 equ 0

video_selector equ (0x0003 << 3) + TI_G + RPL0

[bits 32]
section .data
put_int_buffer dq 0
section .text

global put_char

global print_str

print_str:  ;实现打印字符串
	push ebx
	push ecx

	xor ecx,ecx
	mov ebx,[esp+12]
.go_on:	mov cl,[ebx]
	cmp cl,0
	jz .str_end
	push ecx
	call put_char
	add esp,4
	inc ebx
	loop .go_on	
.str_end:
	pop ecx
	pop ebx
	ret

;打印整数
global put_int
put_int:
	pushad
	mov ebp,esp
	mov eax,[ebp+4*9]
	mov edx,eax
	mov ebx,put_int_buffer
	mov edi,7
	mov ecx,8
	
.handle_for_each_4bits:
	and edx,0x0000000f
	cmp edx,9
	jg .atoz
	add edx,'0'
	jmp .store_4bits
.atoz:
	sub edx,10
	add edx,'A'
		
.store_4bits:
	mov [ebx+edi],dl
	dec edi
	shr eax,4
	mov edx,eax
	loop .handle_for_each_4bits

.go_to_print:
	inc edi
.skip_prefixzero:
	cmp edi,8
	je .all_zero
.go_on_skip:
	mov cl,[put_int_buffer+edi]
	inc edi
	cmp cl,'0'
	je .skip_prefixzero
	dec edi
	jmp .put_each_4bits
	
.all_zero:
	mov cl,'0'

.put_each_4bits:
	push ecx
	call put_char
	add esp,4
	inc edi
	mov cl,[put_int_buffer+edi]
	cmp edi,8
	jl .put_each_4bits
	popad
	ret
	
		
; 打印单个字符	
put_char:
	pushad ; 备份32位寄存器环境
	mov ax,video_selector
	mov gs,ax
	
	;获取当前光标位置
	mov dx,0x03d4
	mov al,0x0e
	out dx,al
	
	mov dx,0x03d5
	in al,dx
	mov ah,al

	mov dx,0x03d4
	mov al,0x0f
	out dx,al
	
	mov dx,0x03d5
	in al,dx

	mov bx,ax
	
	;在栈中获取待打印的字符
	mov ecx,[esp+36]
	
	cmp cl,0xd
	jz .is_carriage_return		;回车
	
	cmp cl,0xa
	jz .is_line_feed       		;换行

	cmp cl,0x8
	jz .is_backspace		;退格

	jmp .put_other


	
.is_backspace:	;处理退格逻辑
	dec bx
	shl bx,1
	mov byte [gs:bx],0x20
	inc bx
	mov byte [gs:bx],0x07
	shr bx,1
	jmp .set_cursor



.put_other: 	;处理正常字符
	shl bx,1
	mov byte [gs:bx],cl
	inc bx
	mov byte [gs:bx],0x07
	shr bx,1
	inc bx
	cmp bx,2000 ;光标值小于2000 说明没有超过屏幕范围
	jl .set_cursor
	

.is_line_feed:   ;处理换行逻辑

.is_carriage_return:   ;处理回车逻辑
		
	xor dx,dx
	mov ax,bx
	mov si,80
	div si
	sub bx,dx
	
.is_carrage_return_end:
	add bx,80
	cmp bx,2000
.is_line_feed_end:
	jl .set_cursor

.roll_screen:
	cld
	mov ecx,960
	
	mov esi,0xc00b80a0
	mov edi,0xc00b8000
	rep movsd
	
	mov ebx,3840
	mov ecx,80

.cls:
	mov word [gs:ebx],0x0720
	add ebx,2
	loop .cls
	
	mov bx,1920

	
.set_cursor:
	mov dx,0x03d4
	mov al,0x0e
	out dx,al

	mov dx,0x03d5
	mov al,bh
	out dx,al
	
	mov dx,0x03d4
	mov al,0x0f
	out dx,al
	
	mov dx,0x03d5
	mov al,bl
	out dx,al
.put_char_done:
	popad
	ret		
	
	

	
global set_cursor
set_cursor:
   pushad
   mov bx, [esp+36]
   ;;;;;;; 1 先设置高8位 ;;;;;;;;
   mov dx, 0x03d4			  ;索引寄存器
   mov al, 0x0e				  ;用于提供光标位置的高8位
   out dx, al
   mov dx, 0x03d5			  ;通过读写数据端口0x3d5来获得或设置光标位置 
   mov al, bh
   out dx, al
   ;;;;;;; 2 再设置低8位 ;;;;;;;;;
   mov dx, 0x03d4
   mov al, 0x0f
   out dx, al
   mov dx, 0x03d5 
   mov al, bl
   out dx, al
   popad
   ret
