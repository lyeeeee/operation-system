#include "timer.h"
#include "io.h"
#include "print.h"
#include "thread.h"
#include "interrupt.h"
#include "debug.h"

#define COUNTER0_NO 0
#define COUNTER0_PORT 0x40
#define PIT_CONTROL_PORT 0x43
#define IRQ0_FREQUENCY 100
#define CLK_FREQUENCY 1193180
#define COUNTER0_INITIAL_VALUE CLK_FREQUENCY/IRQ0_FREQUENCY
#define COUNTER0_MODE 2
#define READ_WRITE_LATCH 3


uint32_t ticks; //自中断开启后  总共的滴答数

static void intr_timer_handler(void){
	struct task_struct* cur_thread = running_thread();
	ASSERT(cur_thread->stack_magic == 0x20190806);
	cur_thread->elapsed_ticks++;
	ticks++;

	if(cur_thread->ticks == 0){
		schedule();
	}else{
		cur_thread->ticks--;
	}
}

static void frequency_set(uint8_t counter_no, uint8_t counter_port, uint8_t rwl, uint8_t counter_mode, uint16_t counter_initial_value){
	outb(PIT_CONTROL_PORT, (uint8_t) (counter_no << 6 | rwl << 4 | counter_mode << 1));
	outb(counter_port, (uint8_t)counter_initial_value);
	outb(counter_port, (uint8_t)counter_initial_value >> 8);
}


void timer_init(){
	print_str("timer_init start\n");
	frequency_set(COUNTER0_NO, COUNTER0_PORT, COUNTER0_INITIAL_VALUE, COUNTER0_MODE, READ_WRITE_LATCH );
	register_handler(0x20, intr_timer_handler);
	print_str("timer init done\n");
}
