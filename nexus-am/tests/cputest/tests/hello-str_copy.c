#include "trap.h"

char buf[128];

int main() {
	
	sprintf(buf, "%s", "Hello world!\n");
	nemu_assert(strcmp(buf, "Hello world!\n") == 0);

	sprintf(buf, "%d + %d = %d\n", 1, 1, 2);
	nemu_assert(strcmp(buf, "1 + 1 = 2\n") == 0);

	sprintf(buf, "%d + %d = %d\n", 2, 10, 12);
	nemu_assert(strcmp(buf, "2 + 10 = 12\n") == 0);

	sprintf(buf, "%d + %d = %d\n", -2, 10, 8);
	nemu_assert(strcmp(buf, "-2 + 10 = 8\n") == 0);

	sprintf(buf, "%u + %u = %u\n", -2, 10, 8);
	printf("%s\n",buf);
	
	sprintf(buf, "0x%08x + 0x%08x = 0x%08x\n", -2, 10, 8);
	printf("%s\n",buf);
	
	return 0;
}
