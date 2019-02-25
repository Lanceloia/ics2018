#ifndef __CPU_RTL_H__
#define __CPU_RTL_H__

#include "nemu.h"
#include "util/c_op.h"
#include "cpu/relop.h"
#include "cpu/rtl-wrapper.h"

#define rtl_save_result(RESULT,WIDTH){ \
	if(id_dest->type == OP_TYPE_REG){ \
		switch(WIDTH){ \
			case 4:	reg_l(id_dest->reg)=((*RESULT)); break; \
			case 2:	reg_w(id_dest->reg)=((*RESULT)&0x0000ffff); break; \
			case 1:	reg_b(id_dest->reg)=((*RESULT)&0x000000ff); break; \
			default: assert(0); break; \
		} \
	} \
	else if(id_dest->type == OP_TYPE_MEM) \
	vaddr_write(id_dest->addr,(*RESULT),id_dest->width); \
	else \
	assert(0); \
} \

extern rtlreg_t t0, t1, t2, t3, at;

void decoding_set_jmp(bool is_jmp);
bool interpret_relop(uint32_t relop, const rtlreg_t src1, const rtlreg_t src2);

/* RTL basic instructions */

static inline void interpret_rtl_li(rtlreg_t* dest, uint32_t imm) {
	*dest = imm;
}


static inline void interpret_rtl_mv(rtlreg_t* dest, const rtlreg_t *src1) {
	*dest = *src1;
}

#define make_rtl_arith_logic(name) \
	static inline void concat(interpret_rtl_, name) (rtlreg_t* dest, const rtlreg_t* src1, const rtlreg_t* src2) { \
		*dest = concat(c_, name) (*src1, *src2); \
	} \
	/* Actually those of imm version are pseudo rtl instructions,
	 * but we define them here in the same macro */ \
	static inline void concat(rtl_, name ## i) (rtlreg_t* dest, const rtlreg_t* src1, int imm) { \
		rtl_li(&at, imm); \
			rtl_ ## name (dest, src1, &at); \
	}

	make_rtl_arith_logic(add)
	make_rtl_arith_logic(sub)
	make_rtl_arith_logic(and)
	make_rtl_arith_logic(or)
	make_rtl_arith_logic(xor)
	make_rtl_arith_logic(shl)
	make_rtl_arith_logic(shr)
	make_rtl_arith_logic(sar)
	make_rtl_arith_logic(mul_lo)
	make_rtl_arith_logic(mul_hi)
	make_rtl_arith_logic(imul_lo)
	make_rtl_arith_logic(imul_hi)
	make_rtl_arith_logic(div_q)
	make_rtl_arith_logic(div_r)
	make_rtl_arith_logic(idiv_q)
make_rtl_arith_logic(idiv_r)

	static inline void interpret_rtl_div64_q(rtlreg_t* dest,
			const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
		uint64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
		uint32_t divisor = (*src2);
		*dest = dividend / divisor;
	}

static inline void interpret_rtl_div64_r(rtlreg_t* dest,
		const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
	uint64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
	uint32_t divisor = (*src2);
	*dest = dividend % divisor;
}

static inline void interpret_rtl_idiv64_q(rtlreg_t* dest,
		const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
	int64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
	int32_t divisor = (*src2);
	*dest = dividend / divisor;
}

static inline void interpret_rtl_idiv64_r(rtlreg_t* dest,
		const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
	int64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
	int32_t divisor = (*src2);
	*dest = dividend % divisor;
}

static inline void interpret_rtl_lm(rtlreg_t *dest, const rtlreg_t* addr, int len) {
	*dest = vaddr_read(*addr, len);
}

static inline void interpret_rtl_sm(const rtlreg_t* addr, const rtlreg_t* src1, int len) {
	vaddr_write(*addr, *src1, len);
}

static inline void interpret_rtl_host_lm(rtlreg_t* dest, const void *addr, int len) {
	switch (len) {
		case 4: *dest = *(uint32_t *)addr; return;
		case 1: *dest = *( uint8_t *)addr; return;
		case 2: *dest = *(uint16_t *)addr; return;
		default: assert(0);
	}
}

static inline void interpret_rtl_host_sm(void *addr, const rtlreg_t *src1, int len) {
	switch (len) {
		case 4: *(uint32_t *)addr = *src1; return;
		case 1: *( uint8_t *)addr = *src1; return;
		case 2: *(uint16_t *)addr = *src1; return;
		default: assert(0);
	}
}

static inline void interpret_rtl_setrelop(uint32_t relop, rtlreg_t *dest,
		const rtlreg_t *src1, const rtlreg_t *src2) {
	*dest = interpret_relop(relop, *src1, *src2);
}

static inline void interpret_rtl_j(vaddr_t target) {
	cpu.eip = target;
	decoding_set_jmp(true);
}

static inline void interpret_rtl_jr(rtlreg_t *target) {
	cpu.eip = *target;
	decoding_set_jmp(true);
}

static inline void interpret_rtl_jrelop(uint32_t relop,
		const rtlreg_t *src1, const rtlreg_t *src2, vaddr_t target) {
	bool is_jmp = interpret_relop(relop, *src1, *src2);
	if (is_jmp) cpu.eip = target;
	decoding_set_jmp(is_jmp);
}

void interpret_rtl_exit(int state);


/* RTL pseudo instructions */

static inline void rtl_lr(rtlreg_t* dest, int r, int width) {
	switch (width) {
		case 4: rtl_mv(dest, &reg_l(r)); return;
		case 1: rtl_host_lm(dest, &reg_b(r), 1); return;
		case 2: rtl_host_lm(dest, &reg_w(r), 2); return;
		default: assert(0);
	}
}

static inline void rtl_sr(int r, const rtlreg_t* src1, int width) {
	switch (width) {
		case 4: rtl_mv(&reg_l(r), src1); return;
		case 1: rtl_host_sm(&reg_b(r), src1, 1); return;
		case 2: rtl_host_sm(&reg_w(r), src1, 2); return;
		default: assert(0);
	}
}

static inline void rtl_not(rtlreg_t *dest, const rtlreg_t* src1) {
	// dest <- ~src1

	*dest = ~*src1;
}

static inline void rtl_sext(rtlreg_t* dest, const rtlreg_t* src1, int width) {
	// dest <- signext(src1[(width * 8 - 1) .. 0])
	assert(width == 1 || width == 2 || width == 4);

	if(width == 1){
		if((*src1 & 0x80) != 0x0)
			*dest = *src1 | 0xffffff00;
		else
			*dest = *src1 & 0x000000ff;
	}
	else if(width == 2){
		if((*src1 & 0x8000 ) != 0x0)
			*dest = *src1 | 0xffff0000;
		else
			*dest = *src1 & 0x0000ffff;
	}
	else{	//width == 4
		*dest = *src1;
	}
}

static inline void rtl_zext(rtlreg_t* dest, const rtlreg_t* src1, int width) {
	// dest <- signext(src1[(width * 8 - 1) .. 0])
	switch(width){
		case 1: *dest = *src1 & 0x000000ff;	break;
		case 2: *dest = *src1 & 0x0000ffff; break;
		case 4: *dest = *src1; break;
		assert(0);
	}
}

static inline void rtl_push(const rtlreg_t* src1) {
	// esp <- esp - 4
	// M[esp] <- src1

	cpu.esp = cpu.esp - 4;
	vaddr_write(cpu.esp, *src1, 4);
}

static inline void rtl_pop(rtlreg_t* dest) {
	// dest <- M[esp]
	// esp <- esp + 4

	*dest = vaddr_read(cpu.esp, 4);
	cpu.esp = cpu.esp + 4;
}

static inline void rtl_setrelopi(uint32_t relop, rtlreg_t *dest,
		const rtlreg_t *src1, int imm) {
	// dest <- (src1 relop imm ? 1 : 0)
	TODO();
}

static inline void rtl_msb(rtlreg_t* dest, const rtlreg_t* src1, int width) {
	// dest <- src1[width * 8 - 1]
	
	/* Before 10.18 
	rtl_li(&at, (*src1 & ((uint32_t)0x00000001 << ((width*8) - 1))) != 0x0 );
	rtl_li(dest, at);
	*/

	/* After 10.18 */
	switch(width){
		case 1:	rtl_li(&at, (*src1 & (0x00000080)) != 0x0 ); break;
		case 2:	rtl_li(&at, (*src1 & (0x00008000)) != 0x0 ); break;
		case 4:	rtl_li(&at, (*src1 & (0x80000000)) != 0x0 ); break;
		default: assert(0);
	}
	rtl_li(dest, at);
	/* */
}

#define make_rtl_setget_eflags(f) \
	static inline void concat(rtl_set_, f) (const rtlreg_t* src) { \
		cpu.EFLAGS.f = *src; \
	} \
	static inline void concat(rtl_get_, f) (rtlreg_t* dest) { \
		*dest = cpu.EFLAGS.f; \
	}

make_rtl_setget_eflags(CF);
make_rtl_setget_eflags(OF);
make_rtl_setget_eflags(ZF);
make_rtl_setget_eflags(SF);

static inline void rtl_update_ZF(const rtlreg_t* result, int width) {
	// eflags.ZF <- is_zero(result[width * 8 - 1 .. 0])
	
	/* Before 10.18
	rtl_li(&at, (*result & ((uint32_t)0xffffffff >> ((4-width) * 8))) == 0x0 );
	rtl_set_ZF(&at);
	*/

	/* After 10.18 */
	switch(width){
		case 1:	rtl_li(&at, (*result & (0x000000ff)) == 0x0 ); break;
		case 2:	rtl_li(&at, (*result & (0x0000ffff)) == 0x0 ); break;
		case 4:	rtl_li(&at, (*result) == 0x0 ); break;
		default: assert(0);
	}
	rtl_set_ZF(&at);
	/* */
}

static inline void rtl_update_SF(const rtlreg_t* result, int width) {
	// eflags.SF <- is_sign(result[width * 8 - 1 .. 0])

	/* Before 10.18 
	rtl_li(&at, (*result & ((uint32_t)0x00000001 << ((width*8) - 1))) != 0x0 );
	rtl_set_SF(&at);
	*/

	/* After 10.18 */
	switch(width){
		case 1:	rtl_li(&at, (*result & (0x00000080)) != 0x0 ); break;
		case 2:	rtl_li(&at, (*result & (0x00008000)) != 0x0 ); break;
		case 4:	rtl_li(&at, (*result & (0x80000000)) != 0x0 ); break;
		default: assert(0);
	}
	rtl_set_SF(&at);
}

static inline void rtl_update_ZFSF(const rtlreg_t* result, int width) {
	rtl_update_ZF(result, width);
	rtl_update_SF(result, width);
}

#endif
