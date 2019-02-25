#include "common.h"
#include "cpu/decode.h"
#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
	/* TODO: Trigger an interrupt/exception with ``NO''.
	 * That is, use ``NO'' to index the IDT.
	 */
	rtl_push(&cpu.EFLAGS.as_32bits_reg);
	rtl_push(&cpu.CS);
	rtl_li(&t1, ret_addr);
	rtl_push(&t1);

	cpu.EFLAGS.IF = 0;
	
	uint32_t idt_base = cpu.IDTR.base;
	uint32_t GD_lo32bits = vaddr_read(idt_base + (NO * 8), 4);
	uint32_t GD_hi32bits = vaddr_read(idt_base + (NO * 8) + 4, 4);

	uint32_t jump_dest_addr = (GD_lo32bits & 0x0000ffff) 
		| (GD_hi32bits & 0xffff0000);

	rtl_j(jump_dest_addr);
}

/* Lanceloia Create */
void end_raise_intr(){
	rtl_pop(&t1);
	rtl_pop(&cpu.CS);
	rtl_pop(&cpu.EFLAGS.as_32bits_reg);
	
	rtl_jr(&t1);
}

void dev_raise_intr() {
	cpu.INTR = true;
}
