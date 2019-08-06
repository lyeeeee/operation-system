#include "timer.h"
#include "io.h"
#include "print.h"

#define COUNTER0_NO 0
#define COUNTER0_PORT 0x40
#define PIT_CONTROL_PORT 0x43
#define IRQ0_FREQUENCY 100
#define CLK_FREQUENCY 1193180
#define COUNTER0_INITIAL_VALUE CLK_FREQUENCY/IRQ0_FREQUENCY
#define COUNTER0_MODE 2
#define READ_WRITE_LATCH 3


static void frequency_set(uint8_t counter_no, uint8_t counter_port, uint8_t rwl, uint8_t counter_mode, uint16_t counter_initial_value){
	outb(PIT_CONTROL_PORT, (uint8_t) (counter_no << 6 | rwl << 4 | counter_mode << 1));
	outb(counter_port, (uint8_t)counter_initial_value);
	outb(counter_port, (uint8_t)(counter_initial_value >> 8));
}


void timer_init(){
	print_str("timer_init start\n");
	frequency_set(COUNTER0_NO, COUNTER0_PORT, COUNTER0_INITIAL_VALUE, COUNTER0_MODE, READ_WRITE_LATCH );
	print_str("timer init done\n");
}
