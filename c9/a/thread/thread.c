#include "stdint.h"
#include "thread.h"
#include "string.h"
#include "memory.h"
#include "global.h"

#define PG_SIZE 4096

static void kernel_thread(thread_func* func, void* arg){
	func(arg);
}

void thread_create(struct task_struct* pthread,thread_func func,void* arg){
	pthread->self_kstack -= sizeof(struct intr_stack);
	pthread->self_kstack -= sizeof(struct thread_stack);

	struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack;
	kthread_stack->ebp = kthread_stack->ebx = kthread_stack->edi = kthread_stack->esi = 0;
	kthread_stack->eip = kernel_thread;
	kthread_stack->function = func;
	kthread_stack->func_arg = arg;
}

void init_thread(struct task_struct* pthread, char* name, int prio){
	memset(pthread,0,sizeof(*pthread));
	strcpy(pthread->name,name);
	pthread->priority = prio;
	pthread->status = TASK_RUNNING;
	pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE);
	pthread->stack_magic = 0x20190806;
}
struct task_struct* thread_start(char* name,int prio, thread_func func, void* arg){
	struct task_struct* thread = get_kernel_pages(1);
	
	init_thread(thread,name,prio);

	thread_create(thread,func,arg);

	asm volatile ("movl %0, %%esp; \
		       pop %%ebp;      \
		       pop %%ebx;      \
		       pop %%edi;      \
		       pop %%esi;      \
		       ret"::"g"(thread->self_kstack):"memory");
	return thread;
}
