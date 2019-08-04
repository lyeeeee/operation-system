#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "bitmap.h"

#define PAGE_SIZE 4096

//四个页框，支持表512M物理内存的位图管理
#define MEM_BITMAP_BASE 0xc009a000

//跨过低端1M的内存，低端1M的内存映射到物理地址的低端1M，内核存放此处
#define K_HEAP_START 0xc0100000

struct pool{
	struct bitmap pool_bitmap;
	uint32_t phy_add_start;
	uint32_t pool_size;
};

//内核和用户程序物理内存池
struct pool kernel_pool, user_pool;

//内核虚拟地址
struct virtual_addr kernel_vaddr;

static void mem_pool_init(uint32_t all_mem){
	print_str("mem_pool_init start\n");
	//页表占用字节数
	uint32_t page_table_size = PAGE_SIZE*256;
	//使用内存 = 页表 + 低端1M
	uint32_t used_mem = page_table_size + 0x100000;
	//剩余内存
	uint32_t free_mem = all_mem - used_mem;
	//剩余页表数，余数不要了， 不足一页的内存不考虑了
	uint16_t all_free_pages = free_mem/PAGE_SIZE;
	
	uint16_t kernel_free_pages = all_free_pages/2;
	uint16_t user_free_pages = all_free_pages - kernel_free_pages;

	uint32_t kbm_length = kernel_free_pages / 8;
	uint32_t ubm_length = user_free_pages /8;

	uint32_t kp_start = used_mem;
	uint32_t up_start = kp_start + kernel_free_pages*PAGE_SIZE;

	kernel_pool.phy_add_start = kp_start;
	user_pool.phy_add_start = up_start;

	kernel_pool.pool_size = kernel_free_pages * PAGE_SIZE;
	user_pool.pool_size = user_free_pages * PAGE_SIZE;

	kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;
	user_pool.pool_bitmap.btmp_bytes_len = ubm_length;

	kernel_pool.pool_bitmap.bits = (void*)MEM_BITMAP_BASE;
	user_pool.pool_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length);

	print_str("kernel_pool_bitmap_start:  0x");put_int((int)kernel_pool.pool_bitmap.bits);print_str("\n");
	print_str("kernel_pool_bitmap_bytes_len:  0x");put_int((int)kernel_pool.pool_bitmap.btmp_bytes_len);print_str("\n");
	print_str("kernel_pool_phy_addr_start:  0x");put_int((int)kernel_pool.phy_add_start);print_str("\n");
	print_str("user_pool_bitmap_start:  0x");put_int((int)user_pool.pool_bitmap.bits);print_str("\n");
	print_str("user_pool_bitmap_bytes_len:  0x");put_int((int)user_pool.pool_bitmap.btmp_bytes_len);print_str("\n");
	print_str("user_pool_phy_addr_start:  0x");put_int((int)user_pool.phy_add_start);print_str("\n");

	bitmap_init(&kernel_pool.pool_bitmap);
	bitmap_init(&user_pool.pool_bitmap);

	kernel_vaddr.vaddr_start = K_HEAP_START; 
	kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length;
	kernel_vaddr.vaddr_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length + ubm_length);

	bitmap_init(&kernel_vaddr.vaddr_bitmap);

	
	print_str("kernel_virtual_pool_bitmap_start:  0x");put_int((int)kernel_vaddr.vaddr_bitmap.bits);print_str("\n");
	print_str("ksernel_virtual_pool_bitmap_bytes_len:  0x");put_int((int)kernel_vaddr.vaddr_bitmap.btmp_bytes_len);print_str("\n");
	print_str("kernel_virtual_pool_addr_start:  0x");put_int((int)kernel_vaddr.vaddr_start);print_str("\n");

	print_str("mem pool init done\n");

}

void mem_init(void){
	print_str("memonry init start\n");
	uint32_t total_mem_bytes = *((uint32_t*)(0xb00));
	print_str("total memonry:  0x"); put_int((int)total_mem_bytes);print_str("\n");
	mem_pool_init(total_mem_bytes);
	print_str("memonry init done\n");

}
