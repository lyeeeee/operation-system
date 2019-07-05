TI_G equ 0
RPL0 equ 0

video_selector equ (0x0003 << 3) + TI_G + RPL0

[bits 32]
section .text

global put_char

put_char:
	pushad ; 备份32位寄存器环境
	mov ax,video_selector
	mov gs,ax
	
	;获取当前光标位置
	mov dx,0x3d4
	mov al,0x0e
	out dx,al
	
	mov dx,0x3d5
	in al,dx
	mov ah,al

	mov dx,0x3d4
	mov al,0x0f
	out dx,al
	
	mov dx,0x3d5
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
	mov byte [gs:bx],0x70
	shr bx,1
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
	jl .set_cursor
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
	
	

	
		
