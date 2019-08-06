#include  "list.h"
#include "stdint.h"
#include "print.h"
#include "interrupt.h"
#include "global.h"

void list_insert_before(struct list_elem* before, stuct list_elem* elem){
	enum intr_status old_status = intr_disable();

	before->prev->next = elem;
	elem->prev = before->prev;
	
	before->prev = elem;
	elem->next = before;

	intr_set_status(old_status);
}

void list_push(struct list* plist, struct list_elem* elem){
	list_insert_before(plist->head, elem);
}

void list_append(struct list* plist, struct list_elem* elem){
	list_insert_before(plist->tail,elem);
}
//开关中断
void list_remove(struct list_elem* pelem){
	enum intr_status old_status = intr_disable();

	pelem->prev->next = pelem->next;
	pelem->next->prev = pelem->prev;
	intr_set_status(old_status);
}

struct list_elem* list_pop(struct list* plist){
	struct list_elem* elem = plist->head.next;
	list_remove(elem);
}

bool elem_find(struct list* plist, struct list_elem* obj_elem){
	struct list_elem* elem = plist->head.next;
	while(elem != &plist->tail){
		if(elem == obj_elem)
			return true;
		elem = elem->next;
	}
	return false;
}


struct list_elem* list_traversal(struct list* plist, function func, int arg){
	if(list_empty(plist)){
		return NULL;
	}
	struct list_elem* elem = plist->head.next;
	while(elem != &plist->tail){
		if(func(elem,arg)){
			return elem;
		}
		elem = elem->next;
	}
	return NULL;
}

uint32_t list_len(struct list* plist){
	struct list_elem* elem = plist->head.next;
	uint32_t length = 0;
	while(elem != &plist->tail){
		length++;
		elem = elem->next;
	}
	return length;
}

bool list_empty(struct list* plist){
	return (plist->head.next == &plist->tail ? true:false);
}
