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
	

	;-----------------------------------此处用来保存内存容量等数据------------------------
	total_mem dd 0
	gdt_ptr dw GDT_LIMIT
		dd GDT_BASE
	loadmsg db 'in_real_mode_ready_enter_pmode'
	
	;下面存放读取到的内存信息，读完之后统一处理
	;进行一下对齐顺便
	ards_buff times 211 db 0 ;用来存放返回的结构体
	ards_num dw 0 ;返回ards的数量
	


loader_start:
	;----------------------获取内存信息-----------------------------
	mov ebx,0
	mov edx,0x534d4150
	mov di,ards_buff
.e820_read_mem:
	mov eax,0x0000e820
	mov ecx,20
	int 0x15
	jc .e801_read_mem
	add di,cx
	inc word [ards_num]
	cmp ebx,0
	jnz .e820_read_mem
	;执行到此处说明e820子功能读取成功
	mov cx,[ards_num]
	mov ebx,ards_buff
	xor edx,edx ; 在edx中存放最大的内存段，即为要查找的内存总量
.find_max_mem:
	mov eax,[ebx]
	mov eax,[ebx+8]
	add ebx,20
	cmp edx,eax
	jge .next_ards_check
	mov edx,eax
.next_ards_check:
	loop .find_max_mem
	jmp .mem_find_ok

.e801_read_mem:
	;先获取15M以下的内存
	mov eax,0x0000e801
	int 0x15
	jc .88_read_mem
	mov cx,0x400
	mul cx
	shl edx,16
	and eax,0x0000ffff
	or edx,eax
	add edx,0x100000
	mov edi,edx
	;再获取16M以上的内存
	xor eax,eax
	mov ax,bx
	mov ecx,0x10000
	mul ecx
	add eax,edi ; 因为e801方法只能获取4gb以下内存，所以edx中高32位的内容肯定为0
	mov edx,eax
	jmp .mem_find_ok


.88_read_mem:
	mov ah,0x88
	int 0x15
	jc .find_mem_erro
	mov cx,0x400
	mul cx
	shl edx,16
	add eax,0x0000ffff
	or edx,eax
	add edx,0x100000
	jmp .mem_find_ok
	;----------------------获取内存信息结束-------------------------

.mem_find_ok:
	;获取内存完毕，内存大小存在edx中
	mov [total_mem],edx
	;----------------------打印开始进入保护模式字符串----------------
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

.find_mem_erro:; 获取内存失败就不开机了
	hlt
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
