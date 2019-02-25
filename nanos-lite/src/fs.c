#include <unistd.h>
#include "fs.h"

typedef size_t (*ReadFn) (void *buf, off_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, off_t offset, size_t len);

typedef struct {
	char *name;
	size_t size;
	off_t disk_offset;
	off_t open_offset;
	ReadFn read;
	WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, off_t offset, size_t len) {
	panic("should not reach here");
	return 0;
}

size_t invalid_write(const void *buf, off_t offset, size_t len) {
	panic("should not reach here");
	return 0;
}

extern size_t dispinfo_read(void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, off_t offset, size_t len);
extern size_t serial_write(const void *buf, off_t offset, size_t len);
extern size_t fb_write(const void *buf, off_t offset, size_t len);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
	{"stdin", 0, 0, 0, invalid_read, invalid_write},
	{"stdout", 0, 0, 0, invalid_read, serial_write},
	{"stderr", 0, 0, 0, invalid_read, serial_write},
	{"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
	{"/dev/fb", 0, 0, 0, invalid_read, fb_write},
	{"/dev/events", 0, 0, 0, events_read, invalid_write},
	{"/dev/tty", 0, 0, 0, invalid_read, serial_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(void *buf, size_t offset, size_t len);

size_t fs_filesz(int fd){
	return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, int mode){
	//flags and mode have no use
	int fd;
	for(fd = 0; fd < NR_FILES; fd ++)
		if(strcmp(pathname, file_table[fd].name)==0) break;
	if(fd >= NR_FILES){
		panic("fs_open: \"%s\" no found", pathname);
		assert(0);
	}
	file_table[fd].open_offset = 0;
	return fd;
}

size_t fs_read(int fd, void *buf, size_t count){
	Finfo *fp = &file_table[fd];
	size_t read_sz;
	if(fp->read)
		read_sz = fp->read(buf, fp->open_offset, count);
	else{
		//printf("buf at: 0x%08x\n", (uint32_t)buf);
		//printf("count : 0x%08x\n", (uint32_t)count);
		read_sz = fp->size - fp->open_offset;
		count = read_sz < count ? read_sz : count;
		read_sz = ramdisk_read(buf, fp->disk_offset + fp->open_offset, count
);
	}
	fs_lseek(fd, (off_t)read_sz, SEEK_CUR);
	return read_sz;
}

size_t fs_write(int fd, void *buf, size_t count){
	size_t write_sz;
	if(file_table[fd].write)
		write_sz = file_table[fd].write(buf, file_table[fd].open_offset, count);
	else{
		write_sz = ramdisk_write(buf,file_table[fd].disk_offset+file_table[fd].open_offset,count);
	}
	fs_lseek(fd, (off_t)write_sz, SEEK_CUR);
	return write_sz;
}

off_t fs_lseek(int fd, off_t offset, int whence){
	if(fd == 0 || fd == 1 || fd == 2) return 0;

	switch (whence){
		case SEEK_SET: file_table[fd].open_offset = offset; break;
		case SEEK_CUR: file_table[fd].open_offset = file_table[fd].open_offset + offset; break;
		case SEEK_END: file_table[fd].open_offset = file_table[fd].size + offset; break;
		default: assert(0); break;
	}
	return file_table[fd].open_offset;
}

int fs_close(int fd){
	//Lanceloia do nothing
	return 0;
}

void init_fs() {
	// TODO: initialize the size of /dev/fb
	
	//Lanceloia do nothing
	Log("init_fs!");
}
