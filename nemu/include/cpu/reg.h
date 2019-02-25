#ifndef __REG_H__
#define __REG_H__

#include "common.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

typedef struct{
  /* Do NOT change the order of the GPRs' definitions. */
  
  /* In NEMU, rtlreg_t is exactly uint32_t. This makes RTL instructions
   * in PA2 able to directly access these registers.
   */
	union{
		union{
			uint8_t _8[2];
			uint16_t _16;
			uint32_t _32;
		}gpr[8];
		struct{
			rtlreg_t eax,ecx,edx,ebx;
			rtlreg_t esp,ebp,esi,edi;
		};
	};
	vaddr_t eip;
	union{
			struct{
			uint32_t CF:1 , UD0:1, PF:1 , UD1:1;
			uint32_t AF:1 , UD2:1, ZF:1 , SF:1 ;
			uint32_t TF:1 , IF:1 , DF:1 , OF:1 ;
			uint32_t IOPL:2      , NT:1 , UD3:1;
			uint32_t RF:1 , VM:1 , UD4:2       ;
			uint32_t UD5:4                     ;
			uint32_t UD6:4                     ;
			uint32_t UD7:4                     ;
			};
			rtlreg_t as_32bits_reg;
	}EFLAGS;

	rtlreg_t CS; //no use

	struct{
		uint16_t limit;
		uint32_t base;
	}IDTR;
	struct{
		uint16_t limit;
		uint32_t base;
	}GDTR;

	union{
		struct{
			uint32_t PE:1 , MP:1 , EM:1 , TS: 1;
			uint32_t ET:1 , N:1  , UD0:       2;
			uint32_t UD1:                     8;
			uint32_t WP:1 , UD2:              3;
			uint32_t UD3:                     8;
			uint32_t UD4:             3 , PG: 1;
		};
		rtlreg_t as_32bits_reg;	
	}CR0;
	rtlreg_t CR1;
	rtlreg_t CR2;
	union{
			struct{
			uint32_t P:1  , RW:1 , US:1 , UD0:1 ;
			uint32_t UD1:1, A:1  , D:1  , UD2:1 ;
			uint32_t UD3:1, AVAIL:3             ;
			uint32_t PAGE_FRAME_ADDRESS:      20;
			};
			rtlreg_t as_32bits_reg;
	}CR3;

	bool INTR;

} CPU_state;

extern CPU_state cpu;

static inline int check_reg_index(int index) {
  assert(index >= 0 && index < 8);
  return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];

static inline const char* reg_name(int index, int width) {
  assert(index >= 0 && index < 8);
  switch (width) {
    case 1: return regsb[index];
    case 2: return regsw[index];
    case 4: return regsl[index];
    default: assert(0);
  }
}

#endif
