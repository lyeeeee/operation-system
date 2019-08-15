#include "fs.h"
#include "stdint.h"
#include "global.h"
#include "super_block.h"
#include "string.h"
#include "inode.h"
#include "dir.h"
#include "ide.h"
#include "stdio-kernel.h"
#include "debug.h"

//格式化分区，初始化分区的元信息，创建文件系统
static void partition_format(struct partition* part){
	uint32_t boot_sector_sects = 1;
	uint32_t super_block_sects = 1;

	//inode 位图所占扇区数
	uint32_t inode_bitmap_sects = DIV_ROUND_UP(MAX_FILES_PER_PART, BITS_PER_SECTOR);
	
	//inode table所占扇区数
	uint32_t inode_table_sects = DIV_ROUND_UP((sizeof(struct inode) * MAX_FILES_PER_PART),SECTOR_SIZE);

	uint32_t used_sects = boot_sector_sects + super_block_sects + inode_bitmap_sects + inode_table_sects;

	uint32_t free_sects = part->sec_cnt - used_sects;

	//块位图所占扇区数
	uint32_t block_bitmap_sects;
	block_bitmap_sects = DIV_ROUND_UP(free_sects, BITS_PER_SECTOR);
	//block_bitmap_bit_len 是位图中位的长度，也是可用块的数量
	uint32_t block_bitmap_bit_len = free_sects - block_bitmap_sects;

	block_bitmap_sects = DIV_ROUND_UP(block_bitmap_bit_len, BITS_PER_SECTOR);

	//初始化超级块
	struct super_block sb;
	sb.magic = 0x19590318;
	sb.sec_cnt = part->sec_cnt;
	sb.inode_cnt = MAX_FILES_PER_PART;
	sb.part_lba_base = part->start_lba;
	
	sb.block_bitmap_lba = sb.part_lba_base + 2;
	sb.block_bitmap_sects = block_bitmap_sects;

	sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects;
	sb.inode_bitmap_sects = inode_bitmap_sects;

	sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
	sb.inode_table_sects = inode_table_sects;

	sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects;

	sb.root_inode_no = 0;
	sb.dir_entry_size = sizeof(struct dir_entry);

	printk("%s info:\n",part->name);
	printk("   magic:0x%x\n   part_lba_base:0x%x\n   all_sectors:0x%x\n   inode_cnt:0x%x\n   block_bitmap_lba:0x%x\n   block_bitmap_sectors:0x%x\n   inode_bitmap_lba:0x%x\n   inode_bitmap_sectors:0x%x\n   inode_table_lba:0x%x\n   inode_table_sectors:0x%x\n   data_start_lba:0x%x\n", sb.magic, sb.part_lba_base, sb.sec_cnt, sb.inode_cnt, sb.block_bitmap_lba, sb.block_bitmap_sects, sb.inode_bitmap_lba, sb.inode_bitmap_sects, sb.inode_table_lba, sb.inode_table_sects, sb.data_start_lba);

	struct disk* hd = part->my_disk;
	
	//超级快写到本分区的1扇区（lba）
	ide_write(hd, part->start_lba + 1, &sb, 1);
	printk("super block lba:0x%x\n",part->start_lba + 1);

	uint32_t buf_size = (sb.block_bitmap_sects >= sb.inode_bitmap_sects ? sb.block_bitmap_sects : sb.inode_bitmap_sects);

	buf_size = (buf_size >= sb.inode_table_sects ? buf_size:inode_table_sects)*SECTOR_SIZE;

	uint8_t* buf = (uint8_t*)sys_malloc(buf_size);//内存管理系统会帮助申请的内存清零

	//将位图块初始化写入扇区
	buf[0] |= 0x01;  //预留给根目录

	uint32_t block_bitmap_last_byte = block_bitmap_bit_len /8;
	uint8_t block_bitmap_last_bit = block_bitmap_bit_len % 8;
	uint32_t last_size = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE); //last_size是位图所在最后一个扇区中，不足扇区的其余部分
	
	memset(&buf[block_bitmap_last_byte], 0xff, last_size);

	uint8_t bit_idx = 0;
	while(bit_idx <= block_bitmap_last_bit){
		buf[block_bitmap_last_byte] &= ~(1 << bit_idx++);
	}
	
	//空闲块位图写到硬盘上
	ide_write(hd, sb.block_bitmap_lba, buf, sb.block_bitmap_sects);

	//初始化inode位图，并写入
	memset(buf, 0, buf_size);
	buf[0] |= 0x1;  //第0inode给了根目录
	ide_write(hd, sb.inode_bitmap_lba, buf, sb.inode_bitmap_sects);

	//初始化inode数组并写入 sb.inode_table_lba
	memset(buf, 0, buf_size);

	//根目录
	struct inode* i = (struct inode*)buf;
	i->i_size = sb.dir_entry_size*2;//  .和..
	i->i_no = 0;
	i->i_sectors[0] = sb.data_start_lba; //其他元素目前都是零
	
	ide_write(hd, sb.inode_table_lba, buf, sb.inode_table_sects);
	
	//根目录写入sb.data_start_lba
	memset(buf, 0 ,buf_size);
	struct dir_entry* p_de = (struct dir_entry*)buf;

	memcpy(p_de->filename, ".", 1);
	p_de->i_no = 0;
	p_de->f_type = FT_DIRECTORY;
	p_de++;
	memcpy(p_de->filename, "..",2);
	p_de->i_no = 0;
	p_de->f_type = FT_DIRECTORY;

	ide_write(hd, sb.data_start_lba, buf, 1);

	printk("	root_dir_lba:0x%x\n",sb.data_start_lba);
	printk("%s format done\n",part->name);
	sys_free(buf);
}

void filesys_init(){
	uint8_t channel_no = 0, dev_no, part_idx = 0;

	struct super_block* sb_buf = (struct super_block*)sys_malloc(SECTOR_SIZE);
	if(sb_buf == NULL){
		PANIC("alloc memory failed!");
	}

	printk("searching file system\n");
	while(channel_no < channel_cnt){
		dev_no = 0;
		while(dev_no < 2){
			if(dev_no == 0){
				dev_no++;
				continue;
			}
			struct disk* hd = &channels[channel_no].devices[dev_no];
			struct partition* part = hd->prim_parts;
			while(part_idx < 12){
				if(part_idx == 4){
					part = hd->logic_parts;
				}
				if(part->sec_cnt != 0){
					memset(sb_buf, 0 , SECTOR_SIZE);
					ide_read(hd,part->start_lba+1,sb_buf,1);
					if(sb_buf->magic == 0x19590318){
						printk("%s has filesystem\n",part->name);
					}else{
						printk("formatting %s's partition %s\n",hd->name,part->name);
						partition_format(part);
					}
				}
				part_idx++;
				part++;
			}
			dev_no++;
		}
		channel_no++;
	}
	sys_free(sb_buf);
}
