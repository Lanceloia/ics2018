#ifndef __SYSCALL_H__
#define __SYSCALL_H__

enum {
  SYS_exit,  //0
  SYS_yield, //1
  SYS_open,  //2
  SYS_read,  //3 
  SYS_write, //4
  SYS_kill,  
  SYS_getpid,
  SYS_close, //7
  SYS_lseek, //8
  SYS_brk,   //9
  SYS_fstat,
  SYS_time,
  SYS_signal,
  SYS_execve, //13
  SYS_fork,
  SYS_link,
  SYS_unlink,
  SYS_wait,
  SYS_times,
  SYS_gettimeofday
};

#endif
