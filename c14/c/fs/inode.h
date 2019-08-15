#ifndef __FS_INODE_H
#define __FS_INODE_H

#include "stdint.h"
#include "list.h"
#include "global.h"
#include "ide.h"
struct inode{
	uint32_t i_no; //inode号
	uint32_t i_size; //inode是文件时指文件大小，inode是目录时指的是目录项之和大小
	uint32_t i_open_cnts; //文件被打开的次数
	bool write_deny; //写文件不能并行
	uint32_t i_sectors[13]; //0-11是直接块，12用来存一级间接指针
	struct list_elem inode_tag;
};

void inode_sync(struct partition* part, struct inode* inode, void* io_buf);
struct inode* inode_open(struct partition* part, uint32_t inode_no);
void inode_close(struct inode* inode);
void inode_init(uint32_t inode_no, struct inode* new_inode);
#endif
