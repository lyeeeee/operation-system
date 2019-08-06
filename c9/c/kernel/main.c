#include "print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"

void k_thread_a(void* arg);
int main(void){
	print_str("present by yleeeee\na simple operation system kernel!\n");
	init_all();
	void* addr = get_kernel_pages(3);
	print_str("get_kernel_page, addr start:0x"); put_int((int)addr);print_str("\n");
	thread_start("kernel_thread_a",10,k_thread_a,"arg");
	while(1);
	return 0;
}

void k_thread_a(void* arg){
	char* para = arg;
	for(;;){
		print_str(para);
		print_str("\n");
	}
}
