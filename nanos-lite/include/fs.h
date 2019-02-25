#ifndef __FS_H__
#define __FS_H__

#include "common.h"

#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

size_t fs_filesz(int fd);
int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t count);
size_t fs_write(int fd, void *buf, size_t count);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);

#endif
