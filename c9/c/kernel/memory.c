#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "bitmap.h"
#include "global.h"
#include "debug.h"
#include "string.h"

#define PAGE_SIZE 4096

//四个页框，支持表512M物理内存的位图管理
#define MEM_BITMAP_BASE 0xc009a000

//跨过低端1M的内存，低端1M的内存映射到物理地址的低端1M，内核存放此处
#define K_HEAP_START 0xc0100000

//用于获取页目录项和页表项的索引宏
#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

struct pool{
	struct bitmap pool_bitmap;
	uint32_t phy_add_start;
	uint32_t pool_size;
};

//内核和用户程序物理内存池
struct pool kernel_pool, user_pool;

//内核虚拟地址
struct virtual_addr kernel_vaddr;

//在对应的虚拟内存池中申请给定的虚拟内存页，成功返回起始地址，失败返回空
static void* vaddr_get(enum pool_flags pf, uint32_t pg_cnt){
	int vaddr_start = 0;
	int bit_idx_start = -1;
	uint32_t cnt = 0;
	if(pf == PF_KERNEL){
		bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
		if(bit_idx_start == -1)
			return NULL;
		while(cnt < pg_cnt){
			bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt, 1);
			cnt++;
		}
		vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start*PAGE_SIZE;
	}else{
	
	}
	return (void*)vaddr_start;
}

//得到虚拟地址对应的pte指针
uint32_t* pte_ptr(uint32_t vaddr){
	uint32_t* pte = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
	return pte;
}

//得到虚拟地址对应的pde指针
uint32_t* pde_ptr(uint32_t vaddr){
	uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr)*4);
	return pde;
}


static void* palloc(struct pool* m_pool){
	/**该处要保证原子性*/
	int bit_idx = bitmap_scan(&m_pool->pool_bitmap,1);
	if(bit_idx == -1){
		return NULL;
	}
	bitmap_set(&m_pool->pool_bitmap, bit_idx, 1);
	uint32_t page_phyaddr = ((bit_idx * PAGE_SIZE) + m_pool->phy_add_start);
	return (void*)page_phyaddr;
}


static void page_table_add(void* _vaddr, void* _page_phyaddr){
	uint32_t vaddr = (uint32_t)_vaddr;
	uint32_t page_phyaddr = (uint32_t)_page_phyaddr;

	uint32_t* pde = pde_ptr(vaddr);
	uint32_t* pte = pte_ptr(vaddr);

	if(*pde & 0x00000001){
		//页目录向存在
		ASSERT(!(*pte & 0x00000001));

		if(!(*pte & 0x00000001)){
			*pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
		}else{
			PANIC("pte repeat");
			*pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
		}
	}else{
		//页目录项不存在,要先创建一个页表，再将对应页目录项填入页目录表中
		//页表中用到的页框均从内核内存池中分配
		uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);
		*pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
		memset((void*)((int)pte & 0xfffff000) , 0, PAGE_SIZE);
		ASSERT(!(*pte & 0x00000001));
		*pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
	}
}


void* malloc_page(enum pool_flags pf, uint32_t pg_cnt){
	ASSERT(pg_cnt > 0 && pg_cnt < 3840);
	void* vaddr_start = vaddr_get(pf,pg_cnt);
	if(vaddr_start == NULL){
		return NULL;
	}

	uint32_t vaddr = (uint32_t)vaddr_start, cnt = pg_cnt;
	struct pool* m_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;

	while(cnt-- > 0){
		void* page_phyaddr = palloc(m_pool);
		if(page_phyaddr == NULL){
			return NULL;
		}

		page_table_add((void*)vaddr, page_phyaddr);
		vaddr += PAGE_SIZE;
	}
	return vaddr_start;
}


void* get_kernel_pages(uint32_t pg_cnt){
	void* vaddr = malloc_page(PF_KERNEL, pg_cnt);
	if(vaddr != NULL){
		memset(vaddr, 0 , pg_cnt*PAGE_SIZE);
	}
	return vaddr;
}


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
	//uint32_t total_mem_bytes = *((uint32_t*)(0xb00));
	uint32_t total_mem_bytes = (*(uint32_t*)(0xb00));
	print_str("total memonry:  0x"); put_int((int)total_mem_bytes);print_str("\n");
	mem_pool_init(total_mem_bytes);
	print_str("memonry init done\n");

}
