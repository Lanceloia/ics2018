#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

extern void cpu_exec(uint32_t);

extern WP *new_wp(char *args, int type);
extern bool dsb_wp(int);
extern void show_wps();

bool is_skip_difftest;

/* We use the `readline' library to provide more flexibility to read from stdin. */

#include "cmd.h"

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Step [N] forward", cmd_si },
	{ "info", "Print register[r] or watcher[w]", cmd_info },
	{ "p", "Print an [EXPR]", cmd_p },
	{ "x", "Print [N] bytes from the [EXPR]", cmd_x },
	{ "w", "Set a watchpoint about the [EXPR]", cmd_w },
	{ "d", "Disable a watchpoint and set it free", cmd_d},
	{ "b", "Set a breakpoint at [EXPR]", cmd_b},
	{ "detach", "Quit DiffTest mode", cmd_detach},
	{ "attach", "Enter DiffTest mode", cmd_attach},
	{ "save", "Save to [path]", cmd_save},
	{ "load", "Load from [path]", cmd_load}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if (arg == NULL) {
		/* no argument given */
		for (i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for (i = 0; i < NR_CMD; i ++) {
			if (strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;

}

static int cmd_si(char *args) {
	uint32_t n=1;
	if(args) n=atoi(args);
	if(n==0) printf("Lanceloia: Function cmd_si error\n");
	cpu_exec(n);
	return 0;
}

static int cmd_info(char *args) {
	if(strcmp(args,"r")==0){
		printf("------------------------------------\n");
		printf(" EAX  0x%08X |        OF    %d\n",cpu.eax,cpu.EFLAGS.OF);
		printf(" ECX  0x%08X |        SF    %d\n",cpu.ecx,cpu.EFLAGS.SF);
		printf(" EDX  0x%08X |        ZF    %d\n",cpu.edx,cpu.EFLAGS.ZF);
		printf(" EBX  0x%08X |        AF    %d\n",cpu.ebx,cpu.EFLAGS.AF);
		printf(" ESP  0x%08X |        PF    - \n",cpu.esp);
		printf(" EBP  0x%08X |        CF    %d\n",cpu.ebp,cpu.EFLAGS.CF);
		printf(" ESI  0x%08X |\n",cpu.esi);
		printf(" EDI  0x%08X | EIP  0x%08X\n",cpu.edi,cpu.eip);
		printf("------------------------------------\n");
	}
	else if(strcmp(args,"w")==0){
		printf("------------------------------------------------\n");
		show_wps();
		printf("------------------------------------------------\n");
	}
	return 0;
}

static int cmd_p(char *args){
	bool success_flag=false;
	int value=expr(args,&success_flag);
	printf("value = %d\t(0x%08X)\n",value,value);
	return 0;
}

static int cmd_x(char *args){
	bool success_flag=false;
	uint32_t N=0,M=0;
	int substrlen=0;
	for(;args[substrlen]!=' '&&args[substrlen]!='\0';substrlen++);
	N=to_number(args,substrlen);
	args+=substrlen+1;
	M=expr(args,&success_flag);
	if(N&&success_flag){
		printf("----------------------------------\n");
		for(int i=0;i<N;i++){
			printf("  %X: ",M+4*i);
			printf("%02X %02X %02X %02X",
					vaddr_read(M+4*i,1),
					vaddr_read(M+4*i+1,1),
					vaddr_read(M+4*i+2,1),
					vaddr_read(M+4*i+3,1));
			printf("  0x%08X  \n",vaddr_read(M+4*i,4));
		}
		printf("----------------------------------\n");
	}
	else
		printf("Lanceloia: cmd_x failed\n");
	return 0;
}

static int cmd_w(char *args){
	WP *wp = new_wp(args,WP_CHANGE);
	if(wp)
		printf("set watchpoint %d: %s\n", wp->NO, wp->wp_expr);
	else
		printf("set watchpoint failed\n");
	return 0;
}

static int cmd_d(char *args){
	int Num = to_number(args,strlen(args));
	if(dsb_wp(Num))
		printf("disable watchpoint %d\n", Num);
	else
		printf("disable watchpoint %d failed\n", Num);
	return 0;
}

static int cmd_b(char *args){
	char buf[512]={"$eip=="};
	strcat(buf,args);
	WP *wp = new_wp(buf,WP_TRUE);
	if(wp)
		printf("set breakpoint %d: %s\n", wp->NO, wp->wp_expr);
	else
		printf("set breakpoint failed\n");
	return 0;
}

static int cmd_detach(char *args){
	is_skip_difftest = true;
	printf("Quit Difftest mode succeed\n");
	return 0;
}

static int cmd_attach(char *args){
	extern bool Lanceloia_difftest_recover();
	if(Lanceloia_difftest_recover()){
		is_skip_difftest = false;
		printf("Enter Difftest mode succeed\n");
	}
	else{
		printf("Enter Difftest mode failed\n");
	}
	return 0;
}

static int cmd_save(char *args){
	
	char path[512]="/home/ics/ics2018/bak_folder/";
	strcat(path,args);

	FILE *fp=fopen(path, "wb");

	if(!fp){
		printf("Failed: in save function, fopen(%s, \"wb\");\n", path);
	}
	else{
		extern unsigned long int get_PMEM_SIZE();
		extern uint8_t* get_pmem();

		fwrite(&cpu, sizeof(cpu), 1, fp);
		fwrite(get_pmem(), get_PMEM_SIZE(), 1, fp);
		printf("Succeed: save %s at \"%s\"\n", args, path);
		fclose(fp);
	}
	return 0;
}

static int cmd_load(char *args){
	char path[512]="/home/ics/ics2018/bak_folder/";
	strcat(path,args);

	FILE *fp=fopen(path, "rb");

	if(!fp){
		printf("Failed: in load function, fopen(%s, \"rb\");\n", path);
	}
	else{
		extern unsigned long int get_PMEM_SIZE();
		extern uint8_t* get_pmem();

		fread(&cpu, sizeof(cpu), 1, fp);
		fread(get_pmem(), get_PMEM_SIZE(), 1, fp);
		printf("Succeed: load %s at \"%s\"\n", args, path);
		fclose(fp);
	}
	return 0;
}


void ui_mainloop(int is_batch_mode) {

	if (is_batch_mode) {
		//		cmd_c(NULL);
		//		return;
	}

	is_skip_difftest = true;

	while (1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if (cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if (args >= str_end) {
			args = NULL;
		}

#ifdef HAS_IOE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for (i = 0; i < NR_CMD; i ++) {
			if (strcmp(cmd, cmd_table[i].name) == 0) {
				if (cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
