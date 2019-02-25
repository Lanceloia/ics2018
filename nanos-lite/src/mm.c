#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(size_t nr_page) {
	void *p = pf;
	pf += PGSIZE * nr_page;
	assert(pf < (void *)_heap.end);
	return p;
}

void free_page(void *p) {
	panic("not implement yet");
}

#include "klib.h"
#define DEFAULT_ENTRY 0x8048000

/* The brk() system call handler. */
int mm_brk(uintptr_t new_brk) {
	extern int _map(_Protect *p, void *va, void *pa, int mode);
	extern PCB *current;

	if(current->cur_brk == 0 && current->max_brk == 0){
		current->cur_brk = DEFAULT_ENTRY;
		current->max_brk = DEFAULT_ENTRY;
	}
	void *va=NULL, *pa=NULL;
	void *begin = (void *)((current->max_brk + 0xfff) & 0xfffff000);
	void *end = (void *)((new_brk + 0xfff) & 0xfffff000);

	if(current->max_brk <= new_brk){
		for(va = begin; va <= end; va += 0x1000){
			_map(&current->as, va, pa, 0);
		}

		current->cur_brk = (uint32_t)end;
		if(current->cur_brk > current->max_brk) {
			current->max_brk = current->cur_brk;
		}
	}
	else{
		//assert(0);
	}

	return 0;
}

void init_mm() {
	pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
	Log("free physical pages starting from %p", pf);

	_vme_init(new_page, free_page);
}
