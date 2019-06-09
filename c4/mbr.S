%include "boot.inc"
;boot program
;--------------------------------------
SECTION MBR vstart=0x7c00
	mov ax,cs
	mov ss,ax
	mov es,ax
	mov ds,ax
	mov fs,ax
	mov sp,0x7c00
	mov ax,0xb800
	mov gs,ax
	
	mov ax,0x600
	mov bx,0x700
	mov cx,0x00
	mov dx,0x184f
	int 0x10	
	
	;输出字符串
	mov byte [gs:0x00],'1'
	mov byte [gs:0x01],0xA4
	mov byte [gs:0x02],'M'
	mov byte [gs:0x03],0xA4
	mov byte [gs:0x04],'B'
	mov byte [gs:0x05],0xA4
	mov byte [gs:0x06],'R'
	mov byte [gs:0x07],0xA4
	
	;读硬盘
	mov cx,4
	mov eax,LOADER_START_SECTOR 
	mov bx,LOADER_BASE_ADDR
	call rd_disk_m_16
	jmp LOADER_BASE_ADDR
	;功能：读取n个扇区
rd_disk_m_16:
	;eax加载的起始扇区号	
	;cx 要读取的扇区总数
	;bx 加载到内存的位置
	mov esi,eax
	mov di,cx
	;设置读取的扇区数
	mov	al,cl 
	mov dx,0x1f2
	out dx,al

	;设置起始扇区号
	mov eax,esi

	mov dx,0x1f3
	out dx,al

	mov dx,0x1f4
	mov cl,8
	shr eax,cl
	out dx,al

	mov dx,0x1f5
	shr eax,cl
	out dx,al

	mov dx,0x1f6
	shr eax,cl
	and al,0x0f
	or al,0xe0
	out dx,al
	
	mov dx,0x1f7
	mov al,0x20
	out dx,al

.not_reday:
	nop
	in al,dx
	and al,0x88
	cmp al,0x08
	jnz .not_reday

	mov ax,di
	mov dx,256
	mul dx
	mov cx,ax
	mov dx,0x1f0

.go_on_read:
	in ax,dx
	mov [bx],ax
	add bx,2
	loop .go_on_read
	ret

	times 510-($-$$) db 0
	db 0x55,0xaa
