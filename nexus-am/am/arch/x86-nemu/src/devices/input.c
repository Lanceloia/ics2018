#include <am.h>
#include <x86.h>
#include <amdev.h>

#define I8042_DATA_PROT 0x60
#define KEYDOWN_MASK 0x8000

size_t input_read(uintptr_t reg, void *buf, size_t size) {
	switch (reg) {
		case _DEVREG_INPUT_KBD: 
			{
				_KbdReg *kbd = (_KbdReg *)buf;

				uint32_t k = inl(I8042_DATA_PROT);

				kbd->keydown = (k & KEYDOWN_MASK ? 1 : 0);
				kbd->keycode = k & ~KEYDOWN_MASK;

				return sizeof(_KbdReg);
			}
	}
	return 0;
}
