#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
   //_yield();
	size_t ret;
	for(ret = 0; ret < len; ret ++)
		_putc(*(char *)buf++);
	return ret;
}

#define NAME(key) \
	[_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
	[_KEY_NONE] = "NONE",
	_KEYS(NAME)
};

extern int read_key();
extern uint32_t uptime();

size_t events_read(void *buf, size_t offset, size_t len) {
   //_yield();
	int key = read_key();
	if(key == _KEY_NONE){
		sprintf(buf, "t %d\n", uptime());
	}
	else{
		sprintf(buf, "%s%s\n", key&0x8000?"kd ":"ku ", keyname[key&0xff]);
	}
	//begin PA4.3
	extern int PROC_NO;
	if(strcmp(buf, "kd F1\n") == 0)
		PROC_NO = 1;
	else if(strcmp(buf, "kd F2\n") == 0)
		PROC_NO = 2;
	else if(strcmp(buf, "kd F3\n") == 0)
		PROC_NO = 3;
	//end PA4.3
	return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
	char *char_buf = (char *)buf;
	strcpy(char_buf, dispinfo+offset);
	return strlen(char_buf);
}

extern void draw_rect(uint32_t *pixels, int x, int y, int w, int h);

size_t fb_write(const void *buf, size_t offset, size_t len) {
	//_yield();
	draw_rect((uint32_t *)(buf), offset/4%400, offset/4/400, len/4, 1);
	return len;
}

void init_device() {
	Log("Initializing devices...");
	_ioe_init();

	// TODO: print the string to array `dispinfo` with the format
	// described in the Navy-apps convention

	// Lanceloia try to do it
	const int WIDTH = 400;
	const int HEIGHT = 300;
	sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d", WIDTH, HEIGHT);
}
