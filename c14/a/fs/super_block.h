#ifndef __FS_SUPER_BLOCK_H
#define __FS_SUPER_BLOCK_H

#include "stdint.h"
#include "global.h"
struct super_block{
	uint32_t magic;//魔数，标识文件系统类型

	uint32_t sec_cnt;  //该分区全部扇区数
	uint32_t inode_cnt; //本分区inode数量
	uint32_t part_lba_base; //分区起始的逻辑扇区号

	uint32_t block_bitmap_lba; //块位图本身起始逻辑扇区
	uint32_t block_bitmap_sects; //块位图占用的扇区数量
	
	uint32_t inode_bitmap_lba; //inode 位图起始逻辑扇区
	uint32_t inode_bitmap_sects; //inode 位图占用扇区数量

	uint32_t inode_table_lba; //inode 表起始扇区逻辑扇区
	uint32_t inode_table_sects; //inode 表占用扇区数量

	uint32_t data_start_lba; //数据区开始的逻辑扇区

	uint32_t root_inode_no; //根目录所在的inode 号

	uint32_t dir_entry_size; //目录项大小

	uint8_t pad[460]; //加上460字节，凑够512字节1扇区大小
}__attribute((packed));
#endif
