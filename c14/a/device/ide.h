#ifndef __DEVICE_IDE_H
#define __DEVICE_IDE_H

#include "stdint.h"
#include "bitmap.h"
#include "list.h"
#include "sync.h"
struct partition{
	uint32_t start_lba;
	uint32_t sec_cnt;
	struct disk* my_disk; //属于哪块硬盘
	struct list_elem part_tag; //队列标记
	char name[8];
	struct super_block* sb;
	struct bitmap block_bitmap;
	struct bitmap inode_bitmap;
	struct list open_inodes; //该分区打开得inode队列
};

struct disk{
	char name[8];
	struct ide_channel* my_channel; //属于哪个通道
	uint8_t dev_no; //主盘还是从盘 ,0是主盘，1是从盘
	struct partition prim_parts[4];
	struct partition logic_parts[8];
};

struct ide_channel{
	char name[8];
	uint16_t port_base; //通道起始端口号
	uint8_t irq_no;  //通道所用中断号
	struct lock lock;
	bool expecting_intr;
	struct semaphore disk_done;  //阻塞和唤醒驱动程序
	struct disk devices[2];//主从硬盘
};

void ide_init(void);
void ide_read(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);
void ide_write(struct disk* hd,uint32_t lab, void* buf, uint32_t sec_cnt);
void intr_hd_handler(uint8_t irq_no);

extern uint8_t channel_cnt;
extern struct ide_channel channels[2];
#endif
