#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536];

uint32_t choose(uint32_t n){
	assert(n!=0);
	uint32_t ret=rand()%n;
	return ret;
}

static int poi;

static inline void gen_rand_expr() {
	switch (choose(10)) {
		case 0:case 1:case 2:case 3:	//gen_rand_num
			switch (choose(4)){
				case 0:
					poi+=sprintf(buf+poi,"-");
				case 1:
					poi+=sprintf(buf+poi,"0x%x",rand()%1000);break;
				case 2:
					poi+=sprintf(buf+poi,"-");
				case 3:
					poi+=sprintf(buf+poi,"%d",rand()%1000);break;
			}
		break;
		case 4:case 5:case 6:case 7:	//gen_rand_parenthesis
			poi+=sprintf(buf+poi,"(");
			gen_rand_expr();
			poi+=sprintf(buf+poi,")");
		break;
		default:						//gen_rand_operator
			gen_rand_expr();
			char op;
			switch (choose(4)){
				case 0:op='+';break;
				case 1:op='-';break;
				case 2:op='*';break;
				case 3:op='/';break;
			}
			poi+=sprintf(buf+poi,"%c",op);
			gen_rand_expr();
	}
	return;
}


static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
	int seed = time(0);
	srand(seed);
	int loop = 1;
	if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
	}
	int i;
	for (i = 0; i < loop; i ++) {
	//Lanceloia Insert
	poi=0;
	//
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen(".code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc .code.c -o .expr");
    if (ret != 0) continue;

    fp = popen("./.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
	}
	return 0;
}
