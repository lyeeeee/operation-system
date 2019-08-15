#ifndef __FS_FILE_H
#define __FS_FILE_H

#include "stdint.h"
#include "ide.h"

struct file{
	uint32_t fd_ops; //记录当前文件操作的偏移地址
	uint32_t fd_flag;  //文件操作标识
	struct inode* fd_inode;
};

enum std_fd{
	stdin_no,
	stdout_no,
	stderr_no
};

enum bitmap_type{
	INODE_BITMAP,
	BLOCK_BITMAP
};

#define MAX_FILE_OPEN 32 //系统可打开的最大文件次数

int32_t get_free_slot_in_global(void);
 int32_t pcb_fd_install(int32_t global_fd_idx);
 int32_t inode_bitmap_alloc(struct partition* part);
 int32_t block_bitmap_alloc(struct partition* part);
 void bitmap_sync(struct partition* part, uint32_t bit_idx, uint8_t btmp);
#endif
