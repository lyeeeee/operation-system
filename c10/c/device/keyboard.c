#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"

#define KBD_BUF_PORT 0x60

static void intr_keyboard_handler(void){
	put_char('k');
	put_str("hello \n");
	inb(KBD_BUF_PORT);
	return;
}

void keyboard_init(void){
	put_str("keyboard init start\n");
	register_handler(0x21,intr_keyboard_handler);
	put_str("keyboard init done\n");
}
