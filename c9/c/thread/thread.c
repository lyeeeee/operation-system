#include "stdint.h"
#include "thread.h"
#include "string.h"
#include "memory.h"
#include "global.h"
#include "interrupt.h"
#include "list.h"
#include "debug.h"
#include "print.h"

#define PG_SIZE 4096

struct task_struct* main_thread;     //主线程PCB指针
struct list thread_ready_list;       //就绪任务队列
struct list thread_all_list;         //所有任务的队列
static struct list_elem* thread_tag; //保存队列中的线程结点

extern void switch_to(struct task_struct* cur, struct task_struct* next);


//获取当前线程的pcb指针
struct task_struct* running_thread(void){
	uint32_t esp;
	asm volatile("mov %%esp, %0":"=g"(esp));
	return (struct task_struct*)(esp&0xfffff000);
}

static void kernel_thread(thread_func* func, void* arg){
	intr_enable();
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

	if(pthread == main_thread){
		pthread->status = TASK_RUNNING;
	}else{
		pthread->status = TASK_READY;
	}
	pthread->priority = prio;
	pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE);
	pthread->stack_magic = 0x20190806;
	pthread->ticks = prio;
	pthread->elapsed_ticks = 0;
	pthread->pgdir = NULL;
}
struct task_struct* thread_start(char* name,int prio, thread_func func, void* arg){
	print_str("thread start begin\n");
	struct task_struct* thread = get_kernel_pages(1);
	
	init_thread(thread,name,prio);

	thread_create(thread,func,arg);

	ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
	list_append(&thread_ready_list, &thread->general_tag);

	ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));
	list_append(&thread_all_list, &thread->all_list_tag);

	/**asm volatile ("movl %0, %%esp; \
		       pop %%ebp;      \
		       pop %%ebx;      \
		       pop %%edi;      \
		       pop %%esi;      \
		       ret"::"g"(thread->self_kstack):"memory");
	*/
	print_str("thread start end\n");
	return thread;
}


static void make_main_thread(void){
	//主线程pcb地址为0xc009e000
	main_thread = running_thread();
	ASSERT((uint32_t)main_thread == 0xc009e000);
	init_thread(main_thread,"main",31);
	ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
	list_append(&thread_all_list, &main_thread->all_list_tag);
}

void schedule(void){
	ASSERT(intr_get_status() == INTR_OFF);

	struct task_struct* cur = running_thread();
	if(cur->status == TASK_RUNNING){
		ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
		list_append(&thread_ready_list, &cur->general_tag);
		cur->ticks = cur->priority;
		cur->status = TASK_READY;
	}else{
		
	}

	ASSERT(!list_empty(&thread_ready_list));
	thread_tag = NULL;
	thread_tag = list_pop(&thread_ready_list);
	struct task_struct* next = elem2entry(struct task_struct, general_tag, thread_tag);
	next->status = TASK_RUNNING;
	switch_to(cur,next);

}	

void thread_init(void){
	print_str("thread init start\n");	
	list_init(&thread_ready_list);
	list_init(&thread_all_list);
	make_main_thread();
	print_str("thread init done\n");
}
