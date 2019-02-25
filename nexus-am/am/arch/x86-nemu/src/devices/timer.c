#include <am.h>
#include <x86.h>
#include <amdev.h>


#include <klib.h>
#define RTC_PORT 0x48

static uint32_t ref_time_usec;
static uint32_t UPTIME_min;

size_t timer_read(uintptr_t reg, void *buf, size_t size) {
	switch (reg) {
		case _DEVREG_TIMER_UPTIME: 
			{
				_UptimeReg *uptime = (_UptimeReg *)buf;
				
				uint32_t now_time_usec = inl(RTC_PORT);
				uint32_t diff_time_usec = ((now_time_usec - ref_time_usec) + 0x00010000) & 0x0000ffff;
				
				uptime->hi = 0;
				uptime->lo = UPTIME_min * 60 * 1000 + diff_time_usec;
				if (diff_time_usec >= 60 * 1000){
					UPTIME_min++;
					ref_time_usec = now_time_usec;
				}

				return sizeof(_UptimeReg);
			}
		case _DEVREG_TIMER_DATE: 
			{
				_RTCReg *rtc = (_RTCReg *)buf;
				rtc->second = 0;
				rtc->minute = 0;
				rtc->hour   = 0;
				rtc->day    = 0;
				rtc->month  = 0;
				rtc->year   = 2018;
				return sizeof(_RTCReg);
			}
	}

	return 0;
}

void timer_init() {
	ref_time_usec = inl(RTC_PORT);
	UPTIME_min = 0;
}
