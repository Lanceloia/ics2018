#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);

make_EHelper(nemu_trap);

/* Lanceloia declaration begin */
#define make_EHelper_group(name1,name2,name3,name4) \
	make_EHelper(name1); \
	make_EHelper(name2); \
	make_EHelper(name3); \
	make_EHelper(name4)

make_EHelper_group(and,or,not,xor);
make_EHelper_group(add,sub,inc,dec);
make_EHelper_group(push,pop,call,ret);
make_EHelper_group(adc,sbb,mul,div);
make_EHelper_group(imul1,imul2,imul3,idiv);
make_EHelper_group(lea,nop,cltd,cwtl);
make_EHelper_group(leave,setcc,jmp,jcc);
make_EHelper_group(sar,shl,shr,neg);

make_EHelper(cmp);
make_EHelper(movzx);
make_EHelper(movsx);
make_EHelper(test);
make_EHelper(call_rm);
make_EHelper(jmp_rm);
make_EHelper(in);
make_EHelper(out);
make_EHelper(rol);
make_EHelper(ror);
make_EHelper(lidt);
make_EHelper(lgdt);
make_EHelper(int);
make_EHelper(pusha);
make_EHelper(popa);
make_EHelper(iret);

make_EHelper(mov_Rd2Cd);
make_EHelper(mov_Cd2Rd);

/* Lanceloia declaration end */
