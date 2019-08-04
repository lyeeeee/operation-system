#include "bitmap.h"
#include "stdint.h"
#include "debug.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"


void bitmap_init(struct bit_map* btmp){
	memset(btmp->bits, 0, btmp->btmp_bytes_len);
}
bool bitmap_scan_test(struct bit_map* btmp, uint32_t bit_idx){
	uint32_t byte_idx = bit_idx/8;
	uint32_t bit_odd = byte_idx%8;
	return (btmp->bits[byte_idx] & (BITMAP_MASK << bit_odd));
}
int bitmap_scan(struct bit_map* btmp, uint32_t cnt){
	uint32_t byte_idx = 0;
	while((0xff == btmp->bits[byte_idx]) && byte_idx < btmp->btmp_bytes_len)
		byte_idx++;
	ASSERT(byte_idx <= btmp->btmp_bytes_len);
	if(byte_idx == btmp->btmp_bytes_len)
		return -1;

	int bit_idx = 0;
	while(! ((uint8_t)(BITMAP_MASK << bit_idx) & btmp->bits[byte_idx])){
		bit_idx++;
	}

	int bit_index_start = byte_idx*8+bit__idx;
	if(cnt == 1)
		return bit_index_start;
	//这里不知道该不该减一，暂时写上	
	uint32_t bit_left = (btmp->btmp_bytes_len*8-bit_idx_start -1);

	uint32_t next_bit = bit_index_start + 1;
	uint32_t count = 1;
	bit_index_start = -1;
	while(bit_left-- > 0){
		if(!bitmap_scan_test(btmp,next_bit)){
			count++;	
		}else{
			count = 0;
		}
		if(count == cnt){
			bit_idx_start = next_bit - cnt +1;
		}
		next_bit++;
	}
	return bit_idx_start;
}
void bitmap_set(struct bit_map* btmp, uint32_t bit_idx, int8_t value){
	ASSERT(value == 0 || value == 1);
	uint32_t byte_idx = bit_idx/8;
	uint32_t idx_odd = bit_idx%8;
	if(value){
		btmp->bits[byte_idx] |= (BITMAP_MASK << idx_odd);
	}else{
		btmp->bits[byte_idx] &= ~(BITMAP_MASK << idx_odd);
	}
}
