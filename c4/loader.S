%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR

jmp loader_start

	GDT_BASE: dd 0x00000000
		  dd 0x00000000
	CODE_DESC: dd 0x0000ffff
		   dd DESC_CODE_HIGH4
	DATA_DESC: dd 0x0000ffff
		   dd DESC_DATA_HIGH4
	VIDEO_DESC:dd 0x80000007
		   dd DESC_VIDEO_HIGH4

	GDT_SIZE equ $ -GDT_BASE
	GDT_LIMIT equ GDT_SIZE-1
	times 60 dq 0
	SELECTOR_CODE equ (0x0001<<3)+RPL0 + TI_GDT
	SELECTOR_DATA equ (0x0002<<3)+RPL0 + TI_GDT
	SELECTOR_VIDEO equ (0x0003<<3)+RPL0 + TI_GDT
	
	gdt_ptr dw GDT_LIMIT
		dd GDT_BASE
	loadmsg db 'in_real_mode_ready_enter_pmode'





loader_start:
	mov sp,LOADER_BASE_ADDR
	mov bp,loadmsg
	mov cx,30
	mov ax,0x1301
	mov bx,0x001f
	mov dx,0x0100
	int 0x10 ;10h号bios中断打印字符串
	;------------------------------------------------进入保护模式-----------------------------
	;打开a20地址线	
	in al,0x92
	or al,0000_0010b
	out 0x92,al
	
	;在gdtr寄存器加载gdt的基地址和段界限
	lgdt[gdt_ptr]
	
	;cr0寄存器pe位置位1
	mov eax,cr0
	or eax,0x00000001
	mov cr0,eax
	;保护模式已经打开,需要刷新流水线，清除无效寄存器缓存

	jmp dword SELECTOR_CODE:protect_mode_start

	[bits 32]
protect_mode_start:
	mov ax,SELECTOR_DATA
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov esp,LOADER_STACK_TOP
	mov ax,SELECTOR_VIDEO 
	mov gs,ax
	
	mov byte [gs:240],'n'
	mov byte [gs:242],'o'
	mov byte [gs:244],'w'
	mov byte [gs:246],','
	mov byte [gs:248],'i'
	mov byte [gs:250],'n'
	mov byte [gs:252],' '
	mov byte [gs:254],'p'
	mov byte [gs:256],'r'
	mov byte [gs:258],'o'
	mov byte [gs:260],'t'
	mov byte [gs:262],'e'
	mov byte [gs:264],'c'
	mov byte [gs:266],'t'
	mov byte [gs:268],' '
	mov byte [gs:270],'m'
	mov byte [gs:272],'o'
	mov byte [gs:274],'d'
	mov byte [gs:276],'e'
	mov byte [gs:278],'!'
	jmp $
