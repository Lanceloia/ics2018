#include <dlfcn.h>

#include "nemu.h"
#include "monitor/monitor.h"
#include "diff-test.h"

static void (*ref_difftest_memcpy_from_dut)(paddr_t dest, void *src, size_t n);
static void (*ref_difftest_getregs)(void *c);
static void (*ref_difftest_setregs)(const void *c);
static void (*ref_difftest_exec)(uint64_t n);

static bool is_skip_ref;
static bool is_skip_dut;

void difftest_skip_ref() { is_skip_ref = true; }
void difftest_skip_dut() { is_skip_dut = true; }

void init_difftest(char *ref_so_file, long img_size) {
#ifndef DIFF_TEST
	return;
#endif
	assert(ref_so_file != NULL);

	void *handle;
	handle = dlopen(ref_so_file, RTLD_LAZY | RTLD_DEEPBIND);
	assert(handle);

	ref_difftest_memcpy_from_dut = dlsym(handle, "difftest_memcpy_from_dut");
	assert(ref_difftest_memcpy_from_dut);

	ref_difftest_getregs = dlsym(handle, "difftest_getregs");
	assert(ref_difftest_getregs);

	ref_difftest_setregs = dlsym(handle, "difftest_setregs");
	assert(ref_difftest_setregs);

	ref_difftest_exec = dlsym(handle, "difftest_exec");
	assert(ref_difftest_exec);

	void (*ref_difftest_init)(void) = dlsym(handle, "difftest_init");
	assert(ref_difftest_init);

	Log("Differential testing: \33[1;32m%s\33[0m", "ON");
	Log("The result of every instruction will be compared with %s. "
			"This will help you a lot for debugging, but also significantly reduce the performance. "
			"If it is not necessary, you can turn it off in include/common.h.", ref_so_file);

	ref_difftest_init();
	ref_difftest_memcpy_from_dut(ENTRY_START, guest_to_host(ENTRY_START), img_size);
	ref_difftest_setregs(&cpu);
}

bool Lanceloia_difftest_reg_check(CPU_state ref_r){
	bool ret = true;
	CPU_state *r = &ref_r;
	CPU_state *d = &cpu;
	ret &= d->eax == (r->eax);
	ret &= d->ecx == (r->ecx);
	ret &= d->edx == (r->edx);
	ret &= d->ebx == (r->ebx);
	ret &= d->esp == (r->esp);
	ret &= d->ebp == (r->ebp);
	ret &= d->esi == (r->esi);
	ret &= d->edi == (r->edi);
	ret &= d->eip == (r->eip);
	ret &= (d->EFLAGS.as_32bits_reg & 0x8c1) == (r->EFLAGS.as_32bits_reg & 0x8c1);
	if(!ret){
		printf("\n         EAX      ECX      EDX      EBX      ESP      EBP      ESI      EDI     CF ZF SF OF\n");
		printf("Ref: %08x %08x %08x %08x %08x %08x %08x %08x    %1x  %1x  %1x  %1x \n\n",r->eax,r->ecx,r->edx,r->ebx,r->esp,r->ebp,r->esi,r->edi,r->EFLAGS.CF,r->EFLAGS.ZF,r->EFLAGS.SF,r->EFLAGS.OF);
		printf("Dut: %08x %08x %08x %08x %08x %08x %08x %08x    %1x  %1x  %1x  %1x \n\n",d->eax,d->ecx,d->edx,d->ebx,d->esp,d->ebp,d->esi,d->edi,d->EFLAGS.CF,d->EFLAGS.ZF,d->EFLAGS.SF,d->EFLAGS.OF);
		printf("Dut's EIP stop at:  %08x\n",d->eip);
	}
	return ret;
}

#define TEST_SIZE 0x4000000

bool Lanceloia_difftest_mem_check(){
	//Don't check memory
	if(cpu.eip!=0x68)
		return true;

	bool ret = true;
	int i;
	for(i = 0; i < TEST_SIZE; i ++){
		ret = pmem[ENTRY_START + i] == (*(unsigned *)guest_to_host(ENTRY_START + i) & 0xff);
		if(!ret){
			printf("Addr %02x:   Ref: %02x    Dut: %x\n", ENTRY_START + i ,
					(*(unsigned *)guest_to_host(ENTRY_START + i) & 0xff), pmem[ ENTRY_START + i]);
			break;
		}
	}
	return ret;
}

bool Lanceloia_difftest_recover(){
	extern unsigned long int get_PMEM_SIZE();
	extern uint8_t* get_pmem();
	
	ref_difftest_memcpy_from_dut(0x0, (void *)(get_pmem()+0x0), 0x7c00);
	ref_difftest_memcpy_from_dut(0x100000, (void *)(get_pmem()+0x100000), get_PMEM_SIZE()-0x100000);
	ref_difftest_setregs(&cpu);

	return true;
}

void difftest_step(uint32_t eip) {
	CPU_state ref_r;

	if (is_skip_dut) {
		is_skip_dut = false;
		return;
	}

	if (is_skip_ref) {
		// to skip the checking of an instruction, just copy the reg state to reference design
		ref_difftest_setregs(&cpu);
		is_skip_ref = false;
		return;
	}

	ref_difftest_exec(1);
	ref_difftest_getregs(&ref_r);

	// TODO: Check the registers state with the reference design.
	// Set `nemu_state` to `NEMU_ABORT` if they are not the same.

	//check reg
	if(!Lanceloia_difftest_reg_check(ref_r)){
		nemu_state = NEMU_ABORT;
	}

	//check mem
	if(!Lanceloia_difftest_mem_check()){
		nemu_state = NEMU_ABORT;
	}
}

