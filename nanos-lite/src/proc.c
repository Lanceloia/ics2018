#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used));
static PCB pcb_boot;
PCB *current;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite for the %dth time!", j);
    j ++;
    _yield();
  }
}

//extern void naive_uload(PCB *pcb, const char *filename);
extern void context_kload(PCB *pcb, void *entry);
extern void context_uload(PCB *pcb, const char *filename);

void init_proc() {
	//context_kload(&pcb[0], (void *)hello_fun);
	//context_uload(&pcb[0], "/bin/init");
	context_uload(&pcb[0], "/bin/hello");
	context_uload(&pcb[1], "/bin/pal");
	context_uload(&pcb[2], "/bin/pal");
	context_uload(&pcb[3], "/bin/pal");
	switch_boot_pcb();
	
	Log("init_proc!");
	//naive_uload(NULL, "/bin/init");
}

static const int TIME_ALL = 50;
static const int TIME_P0 = 1;

static int SCHEDULE_CNT = 0;
int PROC_NO = 1;

_Context* schedule(_Context *prev) {
	//Log("schedule!");
	//return NULL;
	current->cp = prev;
	//current = &pcb[0];
	//current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
	//Log("current->cp: %x", current->cp);
	
	SCHEDULE_CNT = ((SCHEDULE_CNT + 1) % TIME_ALL);
	if(SCHEDULE_CNT < TIME_P0)
		current = &pcb[0];
	else
		current = &pcb[PROC_NO];
	
	return current->cp;
}
