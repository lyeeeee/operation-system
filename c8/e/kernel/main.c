#include "print.h"
#include "init.h"
#include "debug.h"
#include "memory.h"
int main(void){
print_str("hello world!\n  present by yleeeee\na simple operation system kernel!\n");
init_all();
void* addr = get_kernel_pages(3);
print_str("get_kernel_page, addr start:0x"); put_int((int)addr);print_str("\n");
while(1);
return 0;
}

