#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "io.h"


#define IDT_DESC_CNT 0x21

#define PIC_M_CTRL 0x20
#define PIC_S_CTRL 0xa0
#define PIC_M_DATA 0x21
#define PIC_S_DATA 0xa1

#define EFLAGS_IF 0x00000200
#define GET_EFLAGS(EFLAG_VAR) asm volatile ("pushfl; popl %0": "=g"(EFLAG_VAR));


struct gate_desc{
	uint16_t func_offset_low_word;
	uint16_t selector;
	uint8_t dcount;
	uint8_t attribute;
	uint16_t func_offset_high_word;
};

static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);
static struct gate_desc idt[IDT_DESC_CNT];


char* intr_name[IDT_DESC_CNT];
intr_handler idt_table[IDT_DESC_CNT];
extern intr_handler intr_entry_table[IDT_DESC_CNT];

static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function){
	p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000ffff;
	p_gdesc->selector = SELECTOR_K_CODE;
	p_gdesc->dcount = 0;
	p_gdesc->attribute = attr;
	p_gdesc->func_offset_high_word = ((uint32_t)function & 0xffff0000) >> 16;
}

static void idt_desc_init(void){
	int i;
	for(i = 0;i < IDT_DESC_CNT;++i){
		make_idt_desc(&idt[i],IDT_DESC_ATTR_DPL0,intr_entry_table[i]);
	}
	print_str("  idt_desc_init done\n");
}

static void pic_init(void){
	outb(PIC_M_CTRL,0x11);
	outb(PIC_M_DATA,0x20);
	outb(PIC_M_DATA,0x04);
	outb(PIC_M_DATA,0x01);

	outb(PIC_S_CTRL,0x11);
	outb(PIC_S_DATA,0x28);
	outb(PIC_S_DATA,0x02);
	outb(PIC_S_DATA,0x01);

	outb(PIC_M_DATA,0xfe);
	outb(PIC_S_DATA,0xff);
	print_str("pic_init done\n");	
}

static void general_intr_handler(uint8_t vec_nr){
	if(vec_nr == 0x27 || vec_nr == 0x2f){
		return;
	}
	print_str("int vector: 0x");
	put_int(vec_nr);
	put_char('\n');
}
static void exception_init(){
	int i = 0;
	for(i = 0;i < IDT_DESC_CNT;++i){
		idt_table[i] = general_intr_handler;
		intr_name[i] = "unknown";
	}
	intr_name[0] = "#DE Divide Error";
	intr_name[1] = "#DB Debug Exception";
	intr_name[2] = "NMI Interrupt";
	intr_name[3] = "#BP Breakpoint Exception";
	intr_name[4] = "#OF Overflow Exception";
	intr_name[5] = "#BR BOUND Range Exceeded Exception";
	intr_name[6] = "#UD Invalid Opcode Exception";
	intr_name[7] = "#NM Device Not Available Exception";
	intr_name[8] = "#DF Double Fault Exception";
	intr_name[9] = "Coprocessor Segment Overrun";
	intr_name[10] = "#TS Invalid TSS Exception";
	intr_name[11] = "#NP Segment Not Present";
	intr_name[12] = "#SS Stack Fault Exception";
	intr_name[13] = "#GP General Protection Exception";
	intr_name[14] = "#PF Page-Fault Exception";
	//intr_name[15] 第15项是intel保留项，未使用
	intr_name[16] = "#MF x87 FPU Floating-Point Error";
	intr_name[17] = "#AC Alignment Check Exception";
	intr_name[18] = "#MC Machine-Check Exception";
	intr_name[19] = "#XF SIMD Floating-Point Exception";
}


enum intr_status intr_get_status(void){
	uint32_t eflags = 0;
	GET_EFLAGS(eflags);
	return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}

enum intr_status intr_set_status(enum intr_status status){
	return status & INTR_ON ? intr_enable():intr_disable();
}


enum intr_status intr_enable(void){
	enum intr_status old_status;
	if(INTR_ON == intr_get_status()){
		old_status = INTR_ON;
		return old_status;
	}else{
		old_status = INTR_OFF;
		asm volatile ("sti");
		return old_status;
	}
}


enum intr_status intr_disable(void){
	enum intr_status old_status;
	if(INTR_OFF == intr_get_status()){
		old_status = INTR_OFF;
		return old_status;
	}else{
		asm volatile("cli":::"memory");
		old_status = INTR_ON;
		return old_status;
	}
}



void idt_init(){
	print_str("idt_init begin\n");
	idt_desc_init();
	exception_init();
	pic_init();
	
	uint64_t idt_operand = ((sizeof(idt)-1) | (  (uint64_t)(uint32_t)idt   << 16));
	asm volatile ("lidt %0"::"m"(idt_operand));	
	print_str("idt_init done\n");
}
