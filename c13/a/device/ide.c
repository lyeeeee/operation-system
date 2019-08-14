#include "ide.h"
#include "sync.h"
#include "stdio-kernel.h"
#include "debug.h"
#include "stdint.h"
#include "list.h"
#include "stdio.h"
#include "io.h"
#include "timer.h"



//硬盘寄存器对应得端口号
#define reg_data(channel) (channel->port_base + 0)
#define reg_error(channel) (channel->port_base + 1)
#define reg_sect_cnt(channel) (channel->port_base + 2)
#define reg_lba_l(channel) (channel->port_base + 3)
#define reg_lba_m(channel) (channel->port_base + 4)
#define reg_lba_h(channel) (channel->port_base + 5)
#define reg_dev(channel) (channel->port_base + 6)
#define reg_status(channel) (channel->port_base + 7)
#define reg_cmd(channel) (reg_status(channel))
#define reg_alt_status(channel) (channel->port_base + 0x206)
#define reg_ctl(channel) (reg_alt_status(channel))

#define BIT_STAT_BSY 0x80  //硬盘忙
#define BIT_STAT_DRDY 0x40 //驱动准备好了
#define BIT_STAT_DRQ 0x8 //数据准备好了

#define BIT_DEV_MBS 0xa0 //device 寄存器得第7，5位固定是1
#define BIT_DEV_LBA 0x40 //lba 还是chs
#define BIT_DEV_DEV 0x10 //主盘还是从盘

#define CMD_IDENTIFY 0xec
#define CMD_READ_SECTOR 0x20
#define CMD_WRITE_SECTOR 0x30

#define max_lba ((80*1024*1024/512)-1)

uint8_t channel_cnt;
struct ide_channel channels[2];

int32_t ext_lba_base = 0; //总扩展分区的起始lba
uint8_t p_no = 0, l_no = 0;  //用来记录著主硬盘分区和逻辑分区的下标

struct list partition_list; //所有分区的链表
//16字节结构体，存放分区表项
struct partition_table_entry{
	uint8_t bootable; //是否可引导
	uint8_t start_head;//起始磁头号
	uint8_t start_sec;//起始扇区号
	uint8_t start_chs;//起始柱面号
	uint8_t fs_type; //分区类型
	uint8_t end_head;//结束磁头号
	uint8_t end_sec;//结束扇区号
	uint8_t end_chs;//结束柱面号
	uint32_t start_lba;// 该分区起始逻辑扇区号
	uint32_t sec_cnt; //一共多少扇区

};

//选择读写的硬盘
static void select_disk(struct disk* hd){
	uint8_t reg_device = BIT_DEV_MBS | BIT_DEV_LBA;
	if(hd->dev_no == 1){
		reg_device |= BIT_DEV_DEV;
	}
	outb(reg_dev(hd->my_channel), reg_device);
}

//向硬盘控制器写入起始扇区地址以及要读写的扇区数
static void select_sector(struct disk* hd, uint32_t lba, uint32_t sec_cnt){
	ASSERT(lba <= max_lba);
	struct ide_channel* channel = hd->my_channel;
	
	outb(reg_sect_cnt(channel),sec_cnt); //若sec_cnt 为0则表示操作256个扇区
	outb(reg_lba_l(channel),lba); //outb 汇编只会取低八位
	outb(reg_lba_m(channel),lba >> 8);
	outb(reg_lba_h(channel),lba >> 16);

	outb(reg_dev(channel) , BIT_DEV_MBS | BIT_DEV_LBA | lba >> 24);
}


//向通道channel发送命令
static void cmd_out(struct ide_channel* channel, uint8_t cmd){
	//向channel发送了命令，便把该标记标为true
	channel->expecting_intr = true;
	outb(reg_cmd(channel),cmd);
}

//读入sec_cnt 个扇区的数据到缓冲区
static void read_from_sector(struct disk* hd, void* buf, uint8_t sec_cnt){
	uint32_t size_in_byte;
	if(sec_cnt == 0){
		size_in_byte = 256*512;
	}else{
		size_in_byte = sec_cnt*512;
	}
	insw(reg_data(hd->my_channel),buf,size_in_byte/2);
}

//将buf中 sec_cnt 扇区的数据写入硬盘
static void write2sector(struct disk* hd, void* buf, uint8_t sec_cnt){
	uint32_t size_in_byte;
	if(sec_cnt == 0){
		size_in_byte = 256*512;
	}else{
		size_in_byte = sec_cnt*512;
	}
	outsw(reg_data(hd->my_channel),buf,size_in_byte/2);
}

//等待30秒
static bool busy_wait(struct disk* hd){
	struct ide_channel* channel = hd->my_channel;
	uint16_t time_limit = 30*1000;
	while(time_limit -= 10 >= 0){
		if(!(inb(reg_status(channel)) & BIT_STAT_BSY)){
			return (inb(reg_status(channel)) & BIT_STAT_DRQ);
		}else{
			mtime_sleep(10);
		}	
	}
	return false;
}

void ide_read(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt){
	ASSERT(lba <= max_lba);
	ASSERT(sec_cnt > 0);
	lock_acquire(&hd->my_channel->lock);

	select_disk(hd);
	uint32_t secs_op; //每次操作扇区数
	uint32_t secs_done = 0; //已操作完成扇区数
	while(secs_done < sec_cnt){
		if((secs_done + 256) <= sec_cnt)
			secs_op = 256;
		else
			secs_op = sec_cnt - secs_done;	
		
		select_sector(hd, lba + secs_done, secs_op);
		cmd_out(hd->my_channel, CMD_READ_SECTOR);
		//将自己阻塞，等待硬盘读完的中断处理程序唤醒自己
		sema_down(&hd->my_channel->disk_done);

		if(!busy_wait(hd)){
			char error[64];
			sprintf(error, "%s read sector %d failed !\n", hd->name,lba);
			PANIC(error);
		}

		read_from_sector(hd, (void*)((uint32_t)buf + secs_done*512), secs_op);
		secs_done += secs_op;
	}
	lock_release(&hd->my_channel->lock);
}

void ide_write(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt){
	ASSERT(lba <= max_lba);
	ASSERT(sec_cnt > 0);
	lock_acquire(&hd->my_channel->lock);

	select_disk(hd);
	uint32_t secs_op;
	uint32_t secs_done = 0;
	while(secs_done < sec_cnt){
		if((secs_done + 256) <= sec_cnt){
			secs_op = 256;
		}else{
			secs_op = sec_cnt - secs_done;
		}	
		select_sector(hd, lba+secs_done, secs_op);

		cmd_out(hd->my_channel, CMD_WRITE_SECTOR);

		if(!busy_wait(hd)){
			char error[64];
			sprintf(error, "%s write sector %d failed !\n", hd->name,lba);
			PANIC(error);
		}

		write2sector(hd,(void*)((uint32_t)buf + secs_done*512), secs_op);
		sema_down(&hd->my_channel->disk_done);
		secs_done += secs_op;
	}
	lock_release(&hd->my_channel->lock);
}

//硬盘中断处理程序
void intr_hd_handler(uint8_t irq_no){
	ASSERT(irq_no == 0x2e || irq_no == 0x2f);
	uint8_t ch_no = irq_no - 0x2e;
	struct ide_channel* channel = &channels[ch_no];

	ASSERT(channel->irq_no == irq_no);

	//每次读硬盘都会保证加锁，所以中断一定是对应于上次对硬盘的操作
	if(channel->expecting_intr){
		channel->expecting_intr = false;
		sema_up(&channel->disk_done);

		//读取状态寄存器数据，让硬盘控制器知道此次中断被处理
		inb(reg_status(channel));
	}
}
void ide_init(void){
	printk("ide_init start\n");
	uint8_t hd_cnt = *((uint8_t*)0x475);
	ASSERT(hd_cnt > 0);
	channel_cnt = DIV_ROUND_UP(hd_cnt,2);

	struct ide_channel* channel;
	uint8_t channel_no = 0;
	
	while(channel_no < channel_cnt){
		channel = &channels[channel_no];
		sprintf(channel->name,"ide%d",channel_no);
		switch(channel_no){
			case 0:
				channel->port_base = 0x1f0;
				channel->irq_no = 0x20 + 14;
				break;
			case 1:
				channel->port_base = 0x170;
				channel->irq_no = 0x20 + 15;
				break;
		}
		channel->expecting_intr = false;
		lock_init(&channel->lock);
		//线程请求数据后，会阻塞，直到中断处理程序执行sema_up唤醒线程
		sema_init(&channel->disk_done,0);
		channel_no++;
	}
	printk("ide_init done\n");
}

