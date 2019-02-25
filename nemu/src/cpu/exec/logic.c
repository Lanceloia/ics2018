#include "cpu/exec.h"
#include "cpu/cc.h"

void difftest_skip_ref();
void difftest_skip_dut();

make_EHelper(test) {
	/* Before 10.18 
		rtl_li(&t0, 0x0);
		rtl_set_CF(&t0);
		rtl_set_OF(&t0);
	 */
	cpu.EFLAGS.CF = 0;
	cpu.EFLAGS.OF = 0;

	rtl_and(&t0,&id_src->val,&id_dest->val);
	rtl_update_ZFSF(&t0,id_dest->width);

	print_asm_template2(teist);
}

make_EHelper(and) {
	/* Before 10.18 
		rtl_li(&t0, 0x0);
		rtl_set_CF(&t0);
		rtl_set_OF(&t0);
	 */
	cpu.EFLAGS.CF = 0;
	cpu.EFLAGS.OF = 0;

	rtl_and(&t0,&id_src->val,&id_dest->val);
	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);

	print_asm_template2(and);
}

make_EHelper(xor) {
	/* Before 10.18 
		rtl_li(&t0, 0x0);
		rtl_set_CF(&t0);
		rtl_set_OF(&t0);
	 */
	cpu.EFLAGS.CF = 0;
	cpu.EFLAGS.OF = 0;

	rtl_xor(&t0,&id_src->val,&id_dest->val);
	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);

	print_asm_template2(xor);
}

make_EHelper(or) {
	/* Before 10.18 
		rtl_li(&t0, 0x0);
		rtl_set_CF(&t0);
		rtl_set_OF(&t0);
	 */
	cpu.EFLAGS.CF = 0;
	cpu.EFLAGS.OF = 0;

	rtl_or(&t0,&id_src->val,&id_dest->val);
	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);

	print_asm_template2(or);
}

make_EHelper(sar) {
	// unnecessary to update CF and OF in NEMU

	rtl_mv(&t0,&id_dest->val);
	rtl_sext(&t0,&t0,id_dest->width);
	rtl_sar(&t0,&t0,&id_src->val);
	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);

	print_asm_template2(sar);

#if defined(DIFF_TEST)
	difftest_skip_ref();
#endif
}

make_EHelper(shl) {
	// unnecessary to update CF and OF in NEMU
	
	rtl_mv(&t0,&id_dest->val);
	rtl_shl(&t0,&t0,&id_src->val);
	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);

	print_asm_template2(shl);

#if defined(DIFF_TEST)
	difftest_skip_ref();
#endif
}

make_EHelper(shr) {
	// unnecessary to update CF and OF in NEMU

	rtl_mv(&t0,&id_dest->val);
	rtl_zext(&t0,&t0,id_dest->width);
	rtl_shr(&t0,&t0,&id_src->val);
	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);

	print_asm_template2(shr);

#if defined(DIFF_TEST)
	difftest_skip_ref();
#endif
}

/* Lanceloia Insert */

make_EHelper(rol) {
	/* Lanceloia Create*/
	uint32_t temp = id_src->val;
	uint32_t width = id_dest->width;
	uint32_t result = id_dest->val;
	uint32_t tmpcf;
	while(temp != 0) {
		tmpcf = (result & (0x1 << ((width*8)-1))) != 0x0;
		result = result * 2 + tmpcf;
		temp = temp - 1;
	}
	/* No CF and OF Change */
	rtl_li(&t0,result);
	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);
	print_asm_template2(rol);
}

make_EHelper(ror) {
	/* Lanceloia Create */
	TODO();
	print_asm_template2(ror);
}


make_EHelper(setcc) {
	uint32_t cc = decoding.opcode & 0xf;

	rtl_setcc(&t2, cc);
	operand_write(id_dest, &t2);

	print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {
	rtl_not(&t0,&id_dest->val);
	rtl_save_result(&t0,id_src->width);

	print_asm_template1(not);
}
