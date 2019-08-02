#include "init.h"
#include "print.h"
#include "interrupt.h"

void init_all(){
	print_str("init_all \n");
	idt_init();
}
