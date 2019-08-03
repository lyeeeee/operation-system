#include "debug.h"
#include "print.h"
#include "interrupt.h"

void panic_spin(char* filename, int line, const char* function, const char* condition){
	intr_disable();
	print_str("!!!!error!!!!\n");
	print_str("filename:");print_str(filename);print_str("\n");
	print_str("line:0x");put_int(line);print_str("\n");
	print_str("function:");print_str(function);print_str("\n");
	print_str("condition:");print_str(condition);print_str("\n");
	while(1);

}
