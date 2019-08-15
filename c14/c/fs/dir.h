#ifndef __FS_DIR_H
#define __FS_DIR_H

#define MAX_FILE_NAME_LEN 16

struct dir{
	struct inode* inode;
	uint32_t dir_pos;  //目录内偏移
	uint8_t dir_buf[512]; //目录的数据缓存
};

struct dir_entry{
	char filename[MAX_FILE_NAME_LEN];//文件名
	uint32_t i_no; //文件对应的inode号
	enum file_types f_type; //文件类型
};
#endif
