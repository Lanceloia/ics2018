#include "proc.h"

#define DEFAULT_ENTRY 0x8048000

/* Now for PA3.3 */
	extern int fs_open(const char *pathname, int flags, int mode);
	extern size_t fs_filesz(int fd);
	extern size_t fs_read(int fd, void *buf, size_t len);
	extern size_t fs_close(int fd);
/* End for PA3.3 */


#define NO_Use (0)

static uintptr_t loader(PCB *pcb, const char *filename) {
	/* Now for PA4.2 */
	
	int fd = fs_open(filename, NO_Use, NO_Use); 
	int len = fs_filesz(fd);
	int pgsz = (len >> 12) + 1;

	//map
	void *va, *pa;

	//printf("&pcb->as: 0x%08x  ", (uint32_t)&(pcb->as));
	//printf("current->as: 0x%08x  ", (uint32_t)&(current->as));
	
	for(int i = 0; i < pgsz; i++){
		va = (void *)DEFAULT_ENTRY + (i << 12);
		pa = NULL;

		int ret = _map(&pcb->as, va, pa, NO_Use);
		assert(ret == 0);
	}

	printf("p->ptr 1, %x\n", (uint32_t)pcb->cp->prot->ptr);
	
	extern void get_cur_as(_Context *c);
	get_cur_as(pcb->cp);
	
	printf("p->ptr 2, %x\n", (uint32_t)pcb->cp->prot->ptr);
	
	extern void _switch(_Context *c);
	_switch(pcb->cp);
	
	printf("p->ptr 3, %x\n", (uint32_t)pcb->cp->prot->ptr);
	//

	fs_read(fd, (void *)DEFAULT_ENTRY, len);
	fs_close(fd);	 //Actually, have no use
	
	/* End for PA4.2 */
	return DEFAULT_ENTRY;
}

void naive_uload(PCB *pcb, const char *filename) {
	uintptr_t entry = loader(pcb, filename);
	((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
	_Area stack;
	stack.start = pcb->stack;
	stack.end = stack.start + sizeof(pcb->stack);

	pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
	
	extern int _protect(_Protect *);
	_protect(&pcb->as);
	
	uintptr_t entry = loader(pcb, filename);

	_Area stack;
	stack.start = pcb->stack;
	stack.end = stack.start + sizeof(pcb->stack);

	pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
