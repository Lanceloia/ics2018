#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*pgalloc_usr)(size_t);
static void (*pgfree_usr)(void*);

_Area segments[] = {      // Kernel memory mappings
	{
		.start = (void*)0,
		.end = (void*)PMEM_SIZE
	}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
	pgalloc_usr = pgalloc_f;
	pgfree_usr = pgfree_f;

	int i;

	// make all PDEs invalid
	for (i = 0; i < NR_PDE; i ++) {
		kpdirs[i] = 0;
	}

	PTE *ptab = kptabs;
	for (i = 0; i < NR_KSEG_MAP; i ++) {
		uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
		uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
		for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
			// fill PDE
			kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

			// fill PTE
			PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
			PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
			for (; pte < pte_end; pte += PGSIZE) {
				*ptab = pte;
				ptab ++;
			}
		}
	}

	set_cr3(kpdirs);
	set_cr0(get_cr0() | CR0_PG);

	return 0;
}

static _Protect *cur_as = NULL;

int _protect(_Protect *p) {
	PDE *updir = (PDE*)(pgalloc_usr(1));
	p->pgsize = 4096;
	p->ptr = updir;
	// map kernel space
	for (int i = 0; i < NR_PDE; i ++) {
		updir[i] = kpdirs[i];
	}

	p->area.start = (void*)0x8000000;
	p->area.end = (void*)0xc0000000;
	
	//hack
	cur_as = p;

	return 0;
}

void _unprotect(_Protect *p) {
}

void get_cur_as(_Context *c) {
	c->prot = cur_as;
}

void _switch(_Context *c) {
	set_cr3(c->prot->ptr);
	cur_as = c->prot;
}

int _map(_Protect *p, void *va, void *pa, int mode) {
	
	uint32_t DIR_offset = (((uint32_t)va >> 22) & 0x000003ff);
	uint32_t PAGE_offset = (((uint32_t)va >> 12) & 0x000003ff);

	PDE *updir = (PDE*)( (uint32_t)p->ptr & 0xfffff000 );
	PDE *uptab;
	
	if (updir[DIR_offset] & 0x1) {
		//exist
		uptab = (PDE*)( updir[DIR_offset] & 0xfffff000 );
	}
	else {
		//alloc
		uptab = (PDE*)(pgalloc_usr(1));
		updir[DIR_offset] = (uint32_t)uptab | PTE_P;
	}
	
	if(uptab[PAGE_offset] & 0x1){
		//remap
	}
	else{
		//newpage
		pa = pgalloc_usr(1);
		uptab[PAGE_offset] = (uint32_t) pa | PTE_P;
	}
	
	return 0;
}

_Context *_ucontext(_Protect *p, _Area ustack, _Area kstack, void *entry, void *args) {

   //Create context Do nothing
	_Context *scontext=(_Context *)(ustack.end - sizeof(_Context));
	scontext->eip = (intptr_t) NULL;

	//Create context
	_Context *ucontext=(_Context *)(ustack.end - 2*sizeof(_Context));
	ucontext->eip = (intptr_t) entry;
	ucontext->eflags = (intptr_t) 0x00000200; //set IF
	ucontext->prot = p;

   return ucontext;
}
