#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "process.h"
#include "syscall-init.h"
#include "syscall.h"
#include "stdio.h"
#include "memory.h"

void k_thread_a(void*);
void k_thread_b(void*);
void u_prog_a(void);
void u_prog_b(void);

int main(void) {
   put_str("I am kernel\n");
   init_all();
   intr_enable();
   thread_start("k_thread_a", 31, k_thread_a, "argA ");
   thread_start("k_thread_b", 31, k_thread_b, "argB ");
   while(1);
   return 0;
}

/* 在线程中运行的函数 */
void k_thread_a(void* arg) {     
   char* para = arg;
   void* addr = sys_malloc(33);
   console_put_int((int)addr);
   console_put_str("\n");
   while(1);
}

/* 在线程中运行的函数 */
void k_thread_b(void* arg) {     
   char* para = arg;
   void* addr = sys_malloc(63);
   console_put_int((int)addr);
   console_put_str("\n");
   while(1);
}

/* 测试用户进程 */
void u_prog_a(void) {
   printf(" proga,name:%s,pid:0x%d%c","proga", getpid(),'\n');
   while(1);
}

/* 测试用户进程 */
void u_prog_b(void) {
   printf(" progb,name:%s,pid:0x%d%c","progb", getpid(),'\n');
   while(1);
}
