#include "cpu/exec.h"

make_EHelper(mov) {
	operand_write(id_dest, &id_src->val);
	print_asm_template2(mov);
}

make_EHelper(push) {

	switch(id_dest->type){
		case OP_TYPE_REG:
			rtl_mv(&t0,&reg_l(id_dest->reg));
			rtl_push(&t0);
			break;
		case OP_TYPE_MEM:
		case OP_TYPE_IMM:
			rtl_sext(&t0,&id_dest->val,id_dest->width);
			rtl_push(&t0);
			break;
		default: assert(0);
	}

	print_asm_template1(push);
}

make_EHelper(pop) {
	rtl_pop(&reg_l(id_dest->reg));

	print_asm_template1(pop);
}

make_EHelper(pusha) {
	if(decoding.is_operand_size_16)
		TODO();
	else{
		rtl_li(&t1, cpu.esp);
		rtl_push(&cpu.eax);
		rtl_push(&cpu.ecx);
		rtl_push(&cpu.edx);
		rtl_push(&cpu.ebx);
		rtl_push(&t1);
		rtl_push(&cpu.ebp);
		rtl_push(&cpu.esi);
		rtl_push(&cpu.edi);
	}
	print_asm("pusha");
}

make_EHelper(popa) {
	if(decoding.is_operand_size_16)
		TODO();
	else{
		rtl_pop(&cpu.edi);
		rtl_pop(&cpu.esi);
		rtl_pop(&cpu.ebp);
		rtl_pop(&t1);
		rtl_pop(&cpu.ebx);
		rtl_pop(&cpu.edx);
		rtl_pop(&cpu.ecx);
		rtl_pop(&cpu.eax);
	}
	print_asm("popa");
}

make_EHelper(leave) {
	rtl_mv(&cpu.esp,&cpu.ebp);
	rtl_pop(&cpu.ebp);
	print_asm("leave");
}

make_EHelper(cltd) {
	if (decoding.is_operand_size_16) {
		TODO();
	}
	else {
		if(reg_l(0) & 0x80000000)
			reg_l(2) |= 0xffffffff;
		else
			reg_l(2) &= 0x00000000;
	}

	print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
	if (decoding.is_operand_size_16) {
		TODO();
	}
	else {
		if(reg_l(0) & 0x00008000)
			reg_l(0) |= 0xffff0000;
		else
			reg_l(0) &= 0x0000ffff;
	}

	print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
	id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
	rtl_sext(&t0, &id_src->val, id_src->width);
	operand_write(id_dest, &t0);
	print_asm_template2(movsx);
}

make_EHelper(movzx) {
	id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
	rtl_zext(&t0, &id_src->val, id_src->width);
	operand_write(id_dest, &t0);
	print_asm_template2(movzx);
}

make_EHelper(lea) {
	operand_write(id_dest, &id_src->addr);
	print_asm_template2(lea);
}


make_EHelper(mov_Cd2Rd){
	switch(id_src->addr){
		case 0: cpu.gpr[id_dest->addr]._32 = cpu.CR0.as_32bits_reg; break;
		default: 
				  printf("id_src->addr==%d\n", id_src->addr);
				  assert(0); break;
	}
}

make_EHelper(mov_Rd2Cd){
	switch(id_dest->addr){
		case 0: cpu.CR0.as_32bits_reg = id_src->val; break;
		case 3: cpu.CR3.as_32bits_reg = id_src->val; break;
		default: assert(0); break;
	}
}



