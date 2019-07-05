%include "boot.inc"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; a.该部分代码创建的一级页表 位于物理内存0x100000处     ;
; 二级页表紧接着位于0x100000+0x1000 即位于一级页表      ;
; 之后                                                  ;
; b.然后将物理内存低端1mb的内存映射到了虚拟内存3gb之上，;
; 此处将来存放kernel                                    ;
; c.同时为为虚拟内存3gb之上的页表全部建立了页目录项     ;
; d.在此过程中第一个页目录项和虚拟内存3gb之后的第一个   ;
; 页目录项均指向第一个页表                              ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section loader vstart=LOADER_BASE_ADDR


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
	ards_buff times 214 db 0 ;用来存放返回的结构体
	ards_num dw 0 ;返回ards的数量
	


loader_start:
	;----------------------获取内存信息-----------------------------
	xor ebx,ebx
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
	add eax,[ebx+8]
	add ebx,20
	cmp edx,eax
	jge .next_ards_check
	mov edx,eax
.next_ards_check:
	loop .find_max_mem
	jmp .mem_find_ok

.e801_read_mem:
	;先获取15M以下的内存
	mov ax,0xe801
	int 0x15
	jc .88_read_mem
	mov cx,0x400
	mul cx
	shl edx,16
	and eax,0x0000ffff
	or edx,eax
	add edx,0x100000
	mov esi,edx
	;再获取16M以上的内存
	xor eax,eax
	mov ax,bx
	mov ecx,0x10000
	mul ecx
	add esi,eax ; 因为e801方法只能获取4gb以下内存，所以edx中高32位的内容肯定为0
	mov edx,esi
	jmp .mem_find_ok


.88_read_mem:
	mov ah,0x88
	int 0x15
	jc .find_mem_erro
	and eax,0x0000ffff
	mov cx,0x400
	mul cx
	shl edx,16
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

	; 加载kernel进内存0x70000处
	mov eax,KERNEL_START_SECTOR
	mov ecx,200
	mov ebx,KERNEL_BIN_BASE_ADDR	
	call read_disk_m32



	;创建部分页目录项和页表项，初始化页内存位图
	call init_page
	
	;更新gdt表的基地址，使其映射到高端内核空间
	sgdt [gdt_ptr]
	mov ebx,[gdt_ptr+2]
	or dword [ebx+0x18+4],0xc0000000; 将视频段映射到内核空间
	
	add dword [gdt_ptr+2],0xc0000000; 将gdt表映射到内核空间

	add dword esp,0xc0000000; 将栈指针映射到内核空间

	mov eax,PAGE_DIR_TABLE_BASE; 将页目录项基地址赋给cr3寄存器 
	mov cr3,eax
	mov eax,cr0
	or eax,0x80000000
	mov cr0,eax; 打开cr0寄存器的分页开关

	lgdt [gdt_ptr]; 重新加载gdt 到寄存器

	mov byte [gs:320],'V'
	
	
	jmp SELECTOR_CODE:kernel_entry_point

kernel_entry_point:
	call kernel_init
	mov byte [gs:322],'V'
	mov esp,0xc009f000
	mov byte [gs:324],'V'
	jmp KERNEL_ENTRY_POINT


;=============================将kernel.bin 中的各个segment拷贝到编译的地址============
kernel_init:
	xor eax,eax
	xor ebx,ebx
	xor ecx,ecx
	xor edx,edx
	
	mov dx,[KERNEL_BIN_BASE_ADDR + 42]; 程序头表项的大小
	mov ebx,[KERNEL_BIN_BASE_ADDR + 28]; 程序头表项的偏移量
	add ebx,KERNEL_BIN_BASE_ADDR
	mov cx,[KERNEL_BIN_BASE_ADDR + 44]; 表示有几个程序头表项
	
.for_each_header:
	cmp byte [ebx],PT_NULL
	je .NULL
	push dword [ebx+16]
	mov eax,[ebx+4]
	add eax,KERNEL_BIN_BASE_ADDR
	push eax
	push dword [ebx+8]
	call copy_program
	add esp,12
.NULL:
	add ebx,edx
	loop .for_each_header
	ret
	
;==================拷贝程序段的函数===============
copy_program:
	cld
	push ebp
	mov ebp,esp
	push ecx
	mov edi,[ebp+8]
	mov esi,[ebp+12]
	mov ecx,[ebp+16]
	rep movsb
	
	pop ecx
	pop ebp
	ret

init_page:  ; 初始化页目录项和页表项的函数

	mov ebx,PAGE_DIR_TABLE_BASE
	mov ecx,4096
	mov esi,0
.clear_page_dir_table:
	mov byte [ebx+esi],0x0
	inc esi
	loop .clear_page_dir_table
	
	mov eax,PAGE_DIR_TABLE_BASE
	add eax,0x1000
	mov ebx,eax
	
	or eax, PG_P | PG_RW_W | PG_USR_U
	mov [PAGE_DIR_TABLE_BASE],eax
	mov [PAGE_DIR_TABLE_BASE+0xc00],eax ; 第一个页表项和第768个页表项（3gb虚拟空间上1mb）内容均为第一个页表的物理地址
	
	sub eax,0x1000
	mov [PAGE_DIR_TABLE_BASE+4092],eax; 将页目录项的最后一项，指向一级页表的基地址
	
	;创建页表项,将低端1M的内存物理地址存入第一个页表中
	mov ecx,256
	mov esi,0
	mov edx,PG_P | PG_RW_W | PG_USR_U
.create_page_table_entry:
	mov [ebx+esi*4],edx
	inc esi
	add edx,0x1000
	loop .create_page_table_entry
	
	; 为内核空间的其他页表 创建其页目录项
	mov eax,PAGE_DIR_TABLE_BASE
	add eax,0x2000
	mov ebx,PAGE_DIR_TABLE_BASE
	or eax , PG_P | PG_RW_W | PG_USR_U
	mov ecx,254
	mov esi,769
.create_kernel_pde:
	mov [ebx+esi*4],eax
	add eax,0x1000
	inc esi
	loop .create_kernel_pde
	ret	


;读取磁盘内容进内存，32位
read_disk_m32:
	;eax是逻辑扇区号
	;ebx是要读入的内存起始地址
	;ecx是要读入的扇区号
	mov esi,eax
	mov di,cx	
	;设置要读取的扇区数
	mov dx,0x1f2
	mov al,cl
	out dx,al
	
	;将逻辑扇区号写入0x1f3-0x1f6端口
	mov dx,0x1f3
	mov eax,esi
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
	
	; 写入读命令
	mov dx,0x1f7
	mov al,0x20
	out dx,al
	
.not_ready:
	;0x1f7 端口的第4位为1表示准备好数据传输，即已经读完数据
	nop
	mov dx,0x1f7
	in al,dx
	and al,0x88
	cmp al,0x08
	jnz .not_ready

	;从0x1f0端口可以读数据了，一次读取16个字节
	mov ax,di
	mov dx,256
	mul dx
	mov cx,ax
	mov dx,0x1f0

.read_data:
	in ax,dx
	mov [ebx],ax
	add ebx,2
	loop .read_data
	ret 	
