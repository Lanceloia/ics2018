#include "cpu/exec.h"
#include "device/port-io.h"

void difftest_skip_ref();
void difftest_skip_dut();

make_EHelper(lgdt) {
	printf("Now is \"lgdt\"\n");
	TODO();

	print_asm_template1(lgdt);
}

make_EHelper(lidt) {
	cpu.IDTR.limit=vaddr_read(id_dest->addr,2);
	cpu.IDTR.base=vaddr_read(id_dest->addr+2,4);

	print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
	TODO();

	print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
	TODO();

	print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#if defined(DIFF_TEST)
	difftest_skip_ref();
#endif
}

extern void raise_intr(uint8_t NO, vaddr_t ret_addr); //extern function
extern void end_raise_intr(); //extern function

make_EHelper(int) {
	//Lancleoia
	//rtl_push(&cpu.EFLAGS.as_32bits_reg);
	//rtl_push(&cpu.CS);
	//rtl_li(&t1, *eip);
	//rtl_push(&t1);
	extern void update_eip();
	update_eip();
	raise_intr((uint8_t)id_dest->val, cpu.eip);

	print_asm("int %s", id_dest->str);

#if defined(DIFF_TEST) && defined(DIFF_TEST_QEMU)
	difftest_skip_dut();
#endif
}

make_EHelper(iret) {
	//Lanceloia
	//rtl_pop(&t1);
	//rtl_pop(&cpu.CS);
	//rtl_pop(&cpu.EFLAGS.as_32bits_reg);
	//rtl_jr(&t1);
	end_raise_intr();

	print_asm("iret");
}

make_EHelper(in) {
	switch(id_src->type) {
		case OP_TYPE_REG:
			switch(id_dest->width){
				case 4: rtl_li(&t0, pio_read_l(id_src->val)); break;
				case 2: rtl_li(&t0, pio_read_w(id_src->val)); break;
				case 1: rtl_li(&t0, pio_read_b(id_src->val)); break;
				default: assert(0);
			}
			break;
		default: assert(0);
	}
	operand_write(id_dest, &t0);

	print_asm_template2(in);

#if defined(DIFF_TEST)
	difftest_skip_ref();
#endif
}

make_EHelper(out) {
	rtl_li(&t0, id_src->val);
	switch(id_dest->width){
		case 1:pio_write_b(id_dest->val, t0); break;
		default: assert(0);
	}

	print_asm_template2(out);

#if defined(DIFF_TEST)
	difftest_skip_ref();
#endif
}
