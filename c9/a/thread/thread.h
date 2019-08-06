#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H

#include "stdint.h"
typedef void thread_func(void*);

enum task_status{
	TASK_RUNNING,
	TASK_READY,
	TASK_BLOCKED,
	TASK_WAITING,
	TASK_HANGING,
	TASK_DIED
};

//中断栈
//中断发生时，保护线程或进程的上下文
struct intr_stack{
	uint32_t vec_no;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp_dummy;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;


	//下面是cpu从低特权级转到高特权级中断代码时，压入
	uint32_t erro_code;
	void (*eip) (void);
	uint32_t cs;
	uint32_t eflags;
	void* esp;
	uint32_t ss;
};
//线程栈，线程自己的栈
struct thread_stack{
	uint32_t ebp;
	uint32_t ebx;
	uint32_t edi;
	uint32_t esi;

	void (*eip) (thread_func* func, void* func_arg);

	void (*unused_retaddr);
	thread_func* function;
	void* func_arg;
};


//pcb  用于进程或线程
struct task_struct{
	uint32_t* self_kstack;
	enum task_status status;
	uint8_t priority;
	char name[16];
	uint32_t stack_magic;
};


void thread_create(struct task_struct*, thread_func, void*);
void init_thread(struct task_struct*, char*, int);
struct task_struct* thread_start(char*,int,thread_func,void*);
#endif
