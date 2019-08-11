#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "keyboard.h"
#include "ioqueue.h"
#include "process.h"
#include "syscall-init.h"
#include "syscall.h"

void k_thread_a(void*);
void k_thread_b(void*);
void u_prog_a(void);
void u_prog_b(void);
int prog_a_pid = 0,prog_b_pid = 0;
int main(void) {
   put_str("I am kernel\n");
   init_all();

   process_execute(u_prog_a, "user_prog_a");
   process_execute(u_prog_b, "user_prog_b");
   int i;
   for(i = 0;i < 10000;++i){
   
   }
   thread_start("cosumer_a", 31, k_thread_a, " A:");
   thread_start("cnsumer_b", 8, k_thread_b, " B:");
   

   intr_enable();	// 打开中断,使中断起作用
   while(1);
   return 0;
}

/* 在线程中运行的函数 */
void k_thread_a(void* arg) {     
/* 用void*来通用表示参数,被调用的函数知道自己需要什么类型的参数,自己转换再用 */
	console_put_str(" thread_a:");
        console_put_int(sys_getpid());
	console_put_str("\n");
	console_put_str(" prog_a_pid:");
        console_put_int(prog_a_pid);
	console_put_str("\n");
	while(1);
}

/* 在线程中运行的函数 */
void k_thread_b(void* arg) {     
/* 用void*来通用表示参数,被调用的函数知道自己需要什么类型的参数,自己转换再用 */
	console_put_str(" thread_b:");
        console_put_int(sys_getpid());
	console_put_str("\n");
	console_put_str(" prog_b_pid:");
        console_put_int(prog_b_pid);
	console_put_str("\n");
	while(1);
}

void u_prog_a(void){
	prog_a_pid = getpid();
	while(1);
}
void u_prog_b(void){
	prog_b_pid = getpid();
	while(1);
}
