#ifndef __LIB_KERNEL_BITMAP_H
#define __LIB_KERNEL_BITMAP_H

#include "global.h"
#include "stdint.h"
#define BITMAP_MASK 1

struct bit_map{
	uint32_t btmp_bytes_len;
	uint8_t* bits;
};

void bitmap_init(struct bit_map* btmp);
void bitmap_scan_test(struct bit_map* btmp, uint32_t bit_idx);
void bitmap_scan(struct bit_map* btmp, uint32_t cnt);
void bitmap_set(struct bit_map* btmp, uint32_t bit_idx, int8_t value);
#endif
