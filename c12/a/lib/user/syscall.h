#ifndef __LIB_USER_SYSTEMCALL_H
#define __LIB_USER_SYSTEMCALL_H
#include "stdint.h"

enum SYSCALL_NR{
	SYS_GETPID
};
uint32_t getpid(void);
#endif
