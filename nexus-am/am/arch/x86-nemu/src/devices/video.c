#include <am.h>
#include <x86.h>
#include <amdev.h>
#include <klib.h>

#define VMEM 0x40000

/* Maybe Change */
#define SCREEN_H_PORT 0x100
#define SCREEN_W_PORT 0x102

static uint32_t* const fb __attribute__((used)) = (uint32_t *)0x40000;

size_t video_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_INFO: {
      _VideoInfoReg *info = (_VideoInfoReg *)buf;
		/* Const */	
      info->height = inw(SCREEN_H_PORT);
		info->width = inw(SCREEN_W_PORT);
      return sizeof(_VideoInfoReg);
    }
  }
  return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_FBCTL: {
      _FBCtlReg *ctl = (_FBCtlReg *)buf;

		/* Draw */
		int width = screen_width();
		int w = ctl -> w, h = ctl -> h;
		for(int i = 0; i < h; ++i)
			for(int j = 0; j < w; ++j)
				fb[(ctl->y + i ) * width + (ctl->x + j)] = ctl->pixels[i * ctl->w + j];
		/* End Draw */

      if (ctl->sync) {
        // do nothing, hardware syncs.
      }
      return sizeof(_FBCtlReg);
    }
  }
  return 0;
}

void vga_init() {

}

