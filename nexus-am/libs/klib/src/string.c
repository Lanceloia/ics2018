#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
		size_t len = 0;
		while(*s != '\0'){
				len++;
				s++;
		}
		return len;
}

char* strcpy(char* dst,const char* src) {
		char *dst_copy = dst;
		while(*src != '\0'){
				*dst = *src;
				dst++, src++;
		}
		*dst = '\0';
		return dst_copy;
}

char* strncpy(char* dst, const char* src, size_t n) {
		char *dst_copy = dst;
		while(n > 0 && *src != '\0'){
				*dst = *src;
				dst++, src++, n--;
		}
		if(n > 0)
				*dst = '\0';
		return dst_copy;
}

char* strcat(char* dst, const char* src) {
		char *dst_copy = dst;
		while(*dst != '\0'){
				dst++;
		}
		while(*src != '\0'){
				*dst = *src;
				dst++, src++;
		}
		*dst = '\0';
		return dst_copy;
}

int strcmp(const char* s1, const char* s2) {
		int ret = 0;
		while(*s1 == *s2 && *s1 != '\0'){
				s1++, s2++;
		}
		if(*s1 < *s2)
				ret = -1;
		else if(*s1 > *s2)
				ret = 1;
		return ret;
}

int strncmp(const char* s1, const char* s2, size_t n) {
		int ret = 0;
		while(n > 0 && *s1 == *s2 && *s1 != '\0'){
				s1++, s2++, n--;
		}
		if(n == 0)
				ret = 0;
		else if(*s1 < *s2)
				ret = -1;
		else if(*s1 > *s2)
				ret = 1;
		return ret;
}

void* memset(void* v,int c,size_t n) {
		char *tv = (char *)v;
		char tc = c & 0xff;
		while(n > 0){
				*tv = tc;
				tv++, n--;
		}
		return v;
}

void* memcpy(void* out, const void* in, size_t n) {
		void *out_copy = out;
		while(n > 0){
				*(char *)out = *(char *)in;
				out++, in++, n--;
		}
		return out_copy;
}

int memcmp(const void* s1, const void* s2, size_t n){
		int ret = 0;
		char *ts1 = (char *)s1;
		char *ts2 = (char *)s2;
		while(n > 0 && *ts1 == *ts2){
				ts1++, ts2++, n--;
		}
		if(n == 0)
				ret = 0;
		else if(*ts1 < *ts2)
				ret = -1;
		else if(*ts1 > *ts2)
				ret = 1;
		return ret;
}

#endif
