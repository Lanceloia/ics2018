#include "cpu/exec.h"

make_EHelper(add) {
	//Addition: Add Left_op to Right_op

	rtl_add(&t0, &id_dest->val, &id_src->val);
	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);

	//two unsigned numbers add, but the result less than src, Carry
	rtl_li(&t1, (t0 < id_dest->val));
	rtl_set_CF(&t1);

	//two same signal numbers add, but the result is another signal, Overflow

	rtl_msb(&id_dest->val,&id_dest->val,id_dest->width);
	rtl_msb(&id_src->val, &id_src->val, id_dest->width);
	rtl_msb(&t0,&t0,id_dest->width);
	rtl_li(&t1, (id_dest->val == id_src->val && id_dest->val != t0));
	rtl_set_OF(&t1);

	print_asm_template2(add);
}

make_EHelper(sub) {
	//Subtration: Sub Left_op from Right_op

	rtl_sub(&t0, &id_dest->val, &id_src->val);
	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);

	//two unsigned num sub, but the result greater than src, Carry
	rtl_li(&t1, (t0 > id_dest->val));
	rtl_set_CF(&t1);

	//two same signal numbers sub, A - B == A + ~B + 1, Overflow
	rtl_not(&id_src->val, &id_src->val);
	rtl_msb(&id_dest->val,&id_dest->val,id_dest->width);
	rtl_msb(&id_src->val, &id_src->val, id_dest->width);
	rtl_msb(&t0,&t0,id_dest->width);
	rtl_li(&t1, (id_dest->val == id_src->val && id_dest->val != t0));
	rtl_set_OF(&t1);

	print_asm_template2(sub);
}

make_EHelper(cmp) {
	//Compare: Cmp Left_op to Right_op

	rtl_sub(&t0, &id_dest->val, &id_src->val);
	rtl_update_ZFSF(&t0, id_dest->width);

	//two unsigned num cmp, the same with sub, Carry
	rtl_li(&t1, (t0 > id_dest->val));
	rtl_set_CF(&t1);

	//two same signal numbers amp, the same with sub, Overflow
	rtl_not(&id_src->val, &id_src->val);
	rtl_msb(&id_dest->val,&id_dest->val,id_dest->width);
	rtl_msb(&id_src->val, &id_src->val, id_dest->width);
	rtl_msb(&t0,&t0,id_dest->width);
	rtl_li(&t1, (id_dest->val == id_src->val && id_dest->val != t0));
	rtl_set_OF(&t1);

	print_asm_template2(cmp);
}

make_EHelper(inc) {
	/* Before 10.18 */
	id_src->type = OP_TYPE_IMM;
	id_src->width = id_dest->width;
	id_src->val = 0x1;

	rtl_add(&t0, &id_dest->val, &id_src->val);
	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);
	/* */

	/* After 10.18 
		rtl_li(&t0, id_dest->val + 0x1);
		rtl_save_result(&t0,id_dest->width);
		rtl_update_ZFSF(&t0,id_dest->width);
	 */

	print_asm_template1(inc);
}

make_EHelper(dec) {
	/* Before 10.18 */
	id_src->type = OP_TYPE_IMM;
	id_src->width = id_dest->width;
	id_src->val = 0x1;

	rtl_sub(&t0, &id_dest->val, &id_src->val);
	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);
	/* */

	/* After 10.18 
		rtl_li(&t0, id_dest->val - 0x1);
		rtl_save_result(&t0,id_dest->width);
		rtl_update_ZFSF(&t0,id_dest->width);
	 */

	print_asm_template1(dec);
}

make_EHelper(neg) {
	rtl_li(&t0, id_dest->val != 0x0);
	rtl_set_CF(&t0);

	rtl_li(&t0, -id_dest->val);

	rtl_save_result(&t0,id_dest->width);
	rtl_update_ZFSF(&t0,id_dest->width);

	print_asm_template1(neg);
}

make_EHelper(adc) {
	rtl_add(&t2, &id_dest->val, &id_src->val);
	rtl_setrelop(RELOP_LTU, &t3, &t2, &id_dest->val);
	rtl_get_CF(&t1);
	rtl_add(&t2, &t2, &t1);
	operand_write(id_dest, &t2);

	rtl_update_ZFSF(&t2, id_dest->width);

	rtl_setrelop(RELOP_LTU, &t0, &t2, &t1);
	rtl_or(&t0, &t3, &t0);
	rtl_set_CF(&t0);

	rtl_xor(&t0, &id_dest->val, &id_src->val);
	rtl_not(&t0, &t0);
	rtl_xor(&t1, &id_dest->val, &t2);
	rtl_and(&t0, &t0, &t1);
	rtl_msb(&t0, &t0, id_dest->width);
	rtl_set_OF(&t0);

	print_asm_template2(adc);
}

make_EHelper(sbb) {
	rtl_sub(&t2, &id_dest->val, &id_src->val);
	rtl_setrelop(RELOP_LTU, &t3, &id_dest->val, &t2);
	rtl_get_CF(&t1);
	rtl_mv(&id_src2->val,&t2);
	rtl_sub(&t2, &t2, &t1);
	operand_write(id_dest, &t2);

	rtl_update_ZFSF(&t2, id_dest->width);

	rtl_setrelop(RELOP_LTU, &t0, &id_src2->val, &t2);
	rtl_or(&t0, &t3, &t0);
	rtl_set_CF(&t0);

	rtl_xor(&t0, &id_dest->val, &id_src->val);
	rtl_xor(&t1, &id_dest->val, &t2);
	rtl_and(&t0, &t0, &t1);
	rtl_msb(&t0, &t0, id_dest->width);
	rtl_set_OF(&t0);

	print_asm_template2(sbb);
}

make_EHelper(mul) {
	rtl_lr(&t0, R_EAX, id_dest->width);
	rtl_mul_lo(&t1, &id_dest->val, &t0);

	switch (id_dest->width) {
		case 1:
			rtl_sr(R_AX, &t1, 2);
			break;
		case 2:
			rtl_sr(R_AX, &t1, 2);
			rtl_shri(&t1, &t1, 16);
			rtl_sr(R_DX, &t1, 2);
			break;
		case 4:
			rtl_mul_hi(&t2, &id_dest->val, &t0);
			rtl_sr(R_EDX, &t2, 4);
			rtl_sr(R_EAX, &t1, 4);
			break;
		default: assert(0);
	}

	print_asm_template1(mul);
}

bool judgeCFOF(uint32_t high, uint32_t msb, int width){
	switch (width){
		case 1: 
			if((high==0xff&&msb==0x1) || (high==0x00&&msb==0x0))
				return false;
			else
				return true;
		case 2:
			if((high==0xffff&&msb==0x1) || (high==0x0000&&msb==0x0))
				return false;
			else
				return true;
		case 4:
			if((high==0xffffffff&&msb==0x1) || (high==0x00000000&&msb==0x0))
				return false;
			else
				return true;
		default:
			assert(0);
			return false;
	}
}

// imul with one operand
make_EHelper(imul1) {
	rtl_lr(&t0, R_EAX, id_dest->width);
	rtl_imul_lo(&t1, &id_dest->val, &t0);

	rtl_update_ZFSF(&t1, id_dest->width);

	switch (id_dest->width) {
		case 1:
			assert(0);
			rtl_sr(R_AX, &t1, 2);

			cpu.EFLAGS.CF = cpu.EFLAGS.OF =
				judgeCFOF(
						(R_EAX & 0x0000ff00) >> 8,
						(R_EAX & 0x00000080) >> 7,
						1
						);
			break;
		case 2:
			assert(0);
			rtl_sr(R_AX, &t1, 2);
			rtl_shri(&t1, &t1, 16);
			rtl_sr(R_DX, &t1, 2);

			cpu.EFLAGS.CF = cpu.EFLAGS.OF =
				judgeCFOF(
						(R_EDX & 0x0000ffff) >> 0,
						(R_EAX & 0x00008000) >> 15,
						2
						);

			break;
		case 4:
			rtl_imul_hi(&t2, &id_dest->val, &t0);
			rtl_sr(R_EDX, &t2, 4);
			rtl_sr(R_EAX, &t1, 4);

			cpu.EFLAGS.CF = cpu.EFLAGS.OF =
				judgeCFOF(
						(R_EDX & 0xffffffff) >> 0,
						(R_EAX & 0x80000000) >> 31,
						4
						);

			break;
		default: assert(0);
	}

	print_asm_template1(imul);
}

// imul with two operands
make_EHelper(imul2) {
	rtl_sext(&t0, &id_src->val, id_src->width);
	rtl_sext(&t1, &id_dest->val, id_dest->width);

	rtl_imul_lo(&t2, &t1, &t0);
	rtl_imul_hi(&t3, &t1, &t0);
	operand_write(id_dest, &t2);

	rtl_update_ZFSF(&t2, id_dest->width);
	switch (id_dest->width){
		case 1:
			assert(0);
			break;
		case 2:
			//debugging
			//assert(0);
			cpu.EFLAGS.CF = cpu.EFLAGS.OF =
				judgeCFOF(
						(t3 & 0x0000ffff) >> 0,
						(t2 & 0x00008000) >> 15,
						2
						);
			break;
		case 4:
			cpu.EFLAGS.CF = cpu.EFLAGS.OF =
				judgeCFOF(
						(t3 & 0xffffffff) >> 0,
						(t2 & 0x80000000) >> 31,
						4
						);
			break;
		default:
			assert(0);
	}
	print_asm_template2(imul);
}

// imul with three operands
make_EHelper(imul3) {
	rtl_sext(&t0, &id_src->val, id_src->width);
	rtl_sext(&t1, &id_src2->val, id_src->width);
	rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

	rtl_imul_lo(&t2, &t1, &t0);
	operand_write(id_dest, &t2);

	print_asm_template3(imul);
}

make_EHelper(div) {
	switch (id_dest->width) {
		case 1:
			rtl_lr(&t0, R_AX, 2);
			rtl_div_q(&t2, &t0, &id_dest->val);
			rtl_div_r(&t3, &t0, &id_dest->val);
			rtl_sr(R_AL, &t2, 1);
			rtl_sr(R_AH, &t3, 1);
			break;
		case 2:
			rtl_lr(&t0, R_AX, 2);
			rtl_lr(&t1, R_DX, 2);
			rtl_shli(&t1, &t1, 16);
			rtl_or(&t0, &t0, &t1);
			rtl_div_q(&t2, &t0, &id_dest->val);
			rtl_div_r(&t3, &t0, &id_dest->val);
			rtl_sr(R_AX, &t2, 2);
			rtl_sr(R_DX, &t3, 2);
			break;
		case 4:
			rtl_lr(&t0, R_EAX, 4);
			rtl_lr(&t1, R_EDX, 4);
			rtl_div64_q(&cpu.eax, &t1, &t0, &id_dest->val);
			rtl_div64_r(&cpu.edx, &t1, &t0, &id_dest->val);
			break;
		default: assert(0);
	}

	print_asm_template1(div);
}

make_EHelper(idiv) {
	switch (id_dest->width) {
		case 1:
			rtl_lr(&t0, R_AX, 2);
			rtl_idiv_q(&t2, &t0, &id_dest->val);
			rtl_idiv_r(&t3, &t0, &id_dest->val);
			rtl_sr(R_AL, &t2, 1);
			rtl_sr(R_AH, &t3, 1);
			break;
		case 2:
			rtl_lr(&t0, R_AX, 2);
			rtl_lr(&t1, R_DX, 2);
			rtl_shli(&t1, &t1, 16);
			rtl_or(&t0, &t0, &t1);
			rtl_idiv_q(&t2, &t0, &id_dest->val);
			rtl_idiv_r(&t3, &t0, &id_dest->val);
			rtl_sr(R_AX, &t2, 2);
			rtl_sr(R_DX, &t3, 2);
			break;
		case 4:
			rtl_lr(&t0, R_EAX, 4);
			rtl_lr(&t1, R_EDX, 4);
			rtl_idiv64_q(&cpu.eax, &t1, &t0, &id_dest->val);
			rtl_idiv64_r(&cpu.edx, &t1, &t0, &id_dest->val);
			break;
		default: assert(0);
	}

	print_asm_template1(idiv);
}
