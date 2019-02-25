#include <am.h>
#include <x86.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void vecsys();
void vectrap();
void irq0();
void vecnull();

#include "klib.h"
_Context* irq_handle(_Context *cp) {
	
	extern void get_cur_as(_Context *c);
	get_cur_as(cp);

	_Context *next = cp;
	
	if (user_handler) {
		_Event ev = {0};
		
		switch (cp->irq) {
			case 0x20: ev.event = _EVENT_IRQ_TIMER; break;
			case 0x80: ev.event = _EVENT_SYSCALL; break;
			case 0x81: ev.event = _EVENT_YIELD; break;
			default:
				ev.event = _EVENT_ERROR; break;
		}

		next = user_handler(ev, cp); // the user_handler is do_event
		if (next == NULL) {
			next = cp;
		}
	}

	extern void _switch(_Context *c);
	_switch(next);

	return next;
}

static GateDesc idt[NR_IRQ];

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
	// initialize IDT
	for (unsigned int i = 0; i < NR_IRQ; i ++) {
		idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
	}

	// -------------------- system call --------------------------
	idt[0x20] = GATE(STS_TG32, KSEL(SEG_KCODE), irq0, DPL_KERN);
	//idt[0x20] = GATE(STS_TG32, KSEL(SEG_KCODE), irq0, DPL_KERN);
	idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_KERN);
	idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), vectrap, DPL_KERN);

	set_idt(idt, sizeof(idt));

	// register event handler
	user_handler = handler;

	return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
	//Create context
	_Context *kcontext=(_Context *)(stack.end - sizeof(_Context));
	kcontext->eip = (intptr_t)entry;
	return kcontext;
}

void _yield() {
	asm volatile("int $0x81");
}

int _intr_read() {
	return 0;
}

void _intr_write(int enable) {
}
