#include "common.h"
#include "syscall.h"
#include <unistd.h>

/* Lanceloia Create */
void do_syscall_SYS_open(_Context *c);
void do_syscall_SYS_read(_Context *c);
void do_syscall_SYS_write(_Context *c);
void do_syscall_SYS_close(_Context *c);
void do_syscall_SYS_lseek(_Context *c);
void do_syscall_SYS_brk(_Context *c);
void do_syscall_SYS_execve(_Context *c);

_Context* do_syscall(_Context *c) {
	uintptr_t a[4];
	a[0] = c->GPR1;

	//Log("do_syscall! GPR1,2,3,4: %x, %x, %x, %x", c->GPR1, c->GPR2, c->GPR3, c->GPR4);

	switch (a[0]) {
		//for show
		case SYS_exit: _halt(c->GPR2);break;
		//case SYS_exit: c->GPR2 = (intptr_t)"/bin/init"; do_syscall_SYS_execve(c);break;
		case SYS_yield: _yield(); c->GPRx=0; break;
		case SYS_open: do_syscall_SYS_open(c); break;		
		case SYS_read: do_syscall_SYS_read(c); break;
		case SYS_write: do_syscall_SYS_write(c); break;
		case SYS_execve: do_syscall_SYS_execve(c); break;
		case SYS_close: do_syscall_SYS_close(c); break;
		case SYS_lseek: do_syscall_SYS_lseek(c); break;
		case SYS_brk: do_syscall_SYS_brk(c); break;
		default: panic("Unhandled syscall ID = %d", a[0]);
	}

	return NULL;
}


extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, void *buf, size_t count);
extern size_t fs_write(int fd, void *buf, size_t count);
extern off_t fs_lseek(int fd, off_t offset, int whence);
extern int fs_close(int fd);
extern void naive_uload(intptr_t *pcb, const char *filename);
extern int mm_brk(uintptr_t new_brk);

void do_syscall_SYS_open(_Context *c){
	const char *pathname = (char *)c->GPR2;
	int flags = c->GPR3;
	int mode = c->GPR4;
	c->GPRx = (intptr_t)fs_open(pathname, flags, mode);
}

void do_syscall_SYS_read(_Context *c){
	int fd = c->GPR2;
	void *buf = (void *)c->GPR3;
	size_t count = c->GPR4;
	c->GPRx = (intptr_t)fs_read(fd, buf, count);
}

void do_syscall_SYS_write(_Context *c){
	int fd = c->GPR2;
	void *buf = (void *)c->GPR3;
	size_t count = c->GPR4;
	c->GPRx = (intptr_t)fs_write(fd, buf, count);
}

void do_syscall_SYS_close(_Context *c){
	int fd = c->GPR2;
	c->GPRx = (intptr_t)fs_close(fd);
}

void do_syscall_SYS_lseek(_Context *c){
	c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
}

void do_syscall_SYS_brk(_Context *c){
	c->GPRx = mm_brk(c->GPR2); //now handle it
}

void do_syscall_SYS_execve(_Context *c){
	char *fname = (char *)c->GPR2;
	//char *argv = c->GPR3;
	//char *envp = c->GPR4;
	naive_uload(NULL, fname);
	c->GPRx = 0; //always set 0, mean success
}
