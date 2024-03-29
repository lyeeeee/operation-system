#ifndef __KERNEL_MEMONRY_H
#define __KERNEL_MEMNORY_H

#include "stdint.h"
#include "bitmap.h"

struct virtual_addr{
	struct bitmap vaddr_bitmap;
	uint32_t vaddr_start;
};

extern struct pool kernel_pool,user_pool;
void mem_init(void);
#endif
