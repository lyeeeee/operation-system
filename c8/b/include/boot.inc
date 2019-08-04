;--------------loader 和  kernel---------------
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 0x2
LOADER_STACK_TOP equ LOADER_BASE_ADDR

KERNEL_BIN_BASE_ADDR equ 0x70000
KERNEL_START_SECTOR equ 0x9
KERNEL_ENTRY_POINT equ 0xc0001500

;-----------------一级页表物理起始地址---------
PAGE_DIR_TABLE_BASE equ 0x100000
;--------------GDT 描述符属性------------------

DESC_G_4K equ 1_000_0000_0000_0000_0000_0000b;粒度位
DESC_DB_32 equ 1_00_0000_0000_0000_0000_0000b;D/B位,有效地址及操作数大小
DESC_L equ 1_0_0000_0000_0000_0000_0000b;是否是64位代码段
DESC_AVL equ 0_0000_0000_0000_0000_0000b;avaliable 位
DESC_LIMIT_CODE2 equ 1111_0000_0000_0000_0000b;第二部分代码段界线
DESC_LIMIT_DATA2 equ 1111_0000_0000_0000_0000b;第二部分数据段界线
DESC_LIMIT_VIDEO2 equ 1111_0000_0000_0000_0000b;第二部分显存段界线
DESC_P equ 1_000_0000_0000_0000b;是否位于内存中
DESC_DPL equ 0_00_0000_0000_0000b;DPL 初始设为0，最高
DESC_S_CODE equ 1_0000_0000_0000b;非系统段
DESC_S_DATA equ DESC_S_CODE
DESC_S_SYS equ 0_0000_0000_0000b;系统段
DESC_TYPE_CODE equ 1000_0000_0000b;非系统段代码段  只执行
DESC_TYPE_DATA equ 0010_0000_0000b;非系统段数据段  可读可写

DESC_CODE_HIGH4 equ (0x00 <<24)+ DESC_G_4K + DESC_DB_32 + DESC_L  + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL + DESC_S_CODE + DESC_TYPE_CODE + 0x00
DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_DB_32 + DESC_L  + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL + DESC_S_DATA + DESC_TYPE_DATA + 0x00
DESC_VIDEO_HIGH4 equ (0x00<<24) + DESC_G_4K + DESC_DB_32 + DESC_L  + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL + DESC_S_DATA + DESC_TYPE_DATA + 0x0b

;------------------选择子属性-----------------------
RPL0 equ 00b
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b

TI_GDT equ 000b
TI_LDT equ 100b
;-----------------页表项和页目录项的相关属性--------
PG_P equ 01b
PG_RW_R equ 00b
PG_RW_W equ 10b
PG_USR_S equ 000b
PG_USR_U equ 100b
;-------------  program type 定义   --------------
PT_NULL equ 0
