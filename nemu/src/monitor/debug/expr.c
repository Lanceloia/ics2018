#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#define MAX_TOKENS_SIZE 128

typedef struct token {
	int type;
	char str[32];
}Token;

Token tokens[MAX_TOKENS_SIZE];
int nr_token;

enum {
	TK_NOTYPE = 256, TK_EQ, TK_NEQ, 
	TK_LST, TK_GRT, TK_NLT, TK_NGT, TK_AND, TK_OR, 
	TK_NUM, TK_ADD, TK_SUB, TK_MUL, TK_DIV, 
	TK_EAX, TK_ECX, TK_EDX, TK_EBX,
	TK_ESP, TK_EBP, TK_ESI, TK_EDI, 
	TK_EIP, 
	TK_NEG, TK_DRF,TK_LPT, TK_RPT,
	TK_CR3
};

static struct rule{
	char *regex;
	int token_type;
	} rules[] = {
	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{" +", TK_NOTYPE},							// spaces
	{"0x(([0-9])|([a-f])|([A-F]))+", TK_NUM}, 	// hexdecimal number
	{"0b[0-1]+", TK_NUM},						// binary num
	{"0[0-7]+", TK_NUM},						// octonary number
	{"[0-9]+", TK_NUM},							// decimal number
	{"\\+", TK_ADD},	// addition
	{"-", TK_SUB},		// subtraction|negative operator
	{"\\*", TK_MUL},	// multiplication|dereference operator
	{"/", TK_DIV},		// divition
	{"==", TK_EQ},		// equal
	{"!=", TK_NEQ},		// not equal
	{"<=", TK_NGT},		// not greater than
	{"<", TK_LST},		// less than
	{">=", TK_NLT},		// not less than
	{">", TK_GRT},		// greater than		
	{"&&", TK_AND},		// AND
	{"\\|\\|", TK_OR},	// OR
	{"\\(", TK_LPT},						// left parenthesis
	{"\\)", TK_RPT},						// right parenthesis
	{"\\$((eax)|(EAX))", TK_EAX},			// reg eax
	{"\\$((ecx)|(ECX))", TK_ECX},			// reg ecx
	{"\\$((edx)|(EDX))", TK_EDX},			// reg edx
	{"\\$((ebx)|(EBX))", TK_EBX},			// reg ebx
	{"\\$((esp)|(ESP))", TK_ESP},			// reg esp
	{"\\$((ebp)|(EBP))", TK_EBP},			// reg ebp
	{"\\$((esi)|(ESI))", TK_ESI},			// reg esi
	{"\\$((edi)|(EDI))", TK_EDI},			// reg edi
	{"\\$((eip)|(EIP))", TK_EIP},			// reg eip

	{"\\$((cr3)|(CR3))", TK_CR3},			// control reg cr3
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */

void init_regex(){
	int i;
	char error_msg[128];
	int ret;
	for (i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0) {
		regerror(ret, &re[i], error_msg, 128);
		panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

uint32_t to_number(char *s,uint32_t length){	
	bool success_flag=true;

	if(length==0)
		length=32;
	uint32_t base=10;
	if(*s=='0'){
		base=8;
		s++;
		if(*s=='x')base=16,s++;
		else if(*s=='b')base=2,s++;
	}

	if(base==10&&!(0<=*s&&*s<='9')) success_flag=false;

	uint32_t num=0;
	switch(base){
		case 2:
			for(int i=2;i<length;i++)//to skip the 0b
				if('0'<=*s&&*s<='1')num=num*2+(*s-'0'),s++;
				else success_flag=false;
			break;
		case 8:
			for(int i=1;i<length;i++)//to skip the 0
				if('0'<=*s&&*s<='7')num=num*8+(*s-'0'),s++;
				else success_flag=false;
			break;
		case 10:
			for(int i=0;i<length;i++)//noting to skip
				if('0'<=*s&&*s<='9')num=num*10+(*s-'0'),s++;
				else success_flag=false;
			break;
		case 16:
			for(int i=2;i<length;i++)//to skip the 0x
				if('0'<=*s&&*s<='9')num=num*16+(*s-'0'),s++;
				else if('A'<=*s&&*s<='F')num=num*16+(*s-'A'+10),s++;
				else if('a'<=*s&&*s<='f')num=num*16+(*s-'a'+10),s++;
				else success_flag=false;
			break;
		default:
			break;
	}
	if(success_flag)
		return num;
	else
		{printf("Lanceloia: to_number failed, return 0\n");return 0;}
}

#define ADD_TOKEN(token_type)\
strncpy(tokens[nr_token].str,substr_start,substr_len);\
tokens[nr_token].str[substr_len]='\0';\
tokens[nr_token++].type = (token_type)

bool isFormerNumber(){
	if(nr_token==0)
		return false;
	else
		switch(tokens[nr_token-1].type){
			case TK_NUM:
			case TK_RPT: case TK_EIP:
			case TK_EAX: case TK_ECX: case TK_EDX: case TK_EBX:
			case TK_ESP: case TK_EBP: case TK_ESI: case TK_EDI:
			case TK_CR3:
				return true;
			default:
				return false;}
}

static bool make_token(char *e) {
	int i,position = 0;
	regmatch_t pmatch;
	nr_token = 0;
	while (e[position] != '\0') {
	/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i ++) {
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				//Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
				//  i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */
		
				switch(rules[i].token_type){
					
					case TK_SUB:if(isFormerNumber()){ADD_TOKEN(TK_SUB);}else{ADD_TOKEN(TK_NEG);}break;
					case TK_MUL:if(isFormerNumber()){ADD_TOKEN(TK_MUL);}else{ADD_TOKEN(TK_DRF);}break;
					
					case TK_NOTYPE:break;
					default:ADD_TOKEN(rules[i].token_type);break;
				}
				break;
			}
		}
		if (i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
	return true;
}

uint32_t expr(char *e, bool *success) {
	if (!make_token(e)) {
	*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	int value = eval(0,nr_token-1);
	*success = true;
	return value;
}

uint32_t oprt_prrt(int token_type){
	//operator_priority
	switch(token_type){
		case TK_OR:  return 12;
		case TK_AND: return 11;
		case TK_EQ:  case TK_NEQ: return 6;
		case TK_GRT: case TK_LST: 
		case TK_NGT: case TK_NLT: return 5;
		case TK_ADD: case TK_SUB: return 4;
		case TK_MUL: case TK_DIV: return 3;
		case TK_NEG: case TK_DRF: return 2;
	}
	return 0;
}

bool check_parentheses(int p,int q){
	int left_parenthesis_cnt=0;
	int right_parenthesis_cnt=0;
	
	if(tokens[p].type!=TK_LPT||tokens[q].type!=TK_RPT)
		return false;

	for(int r=p;r<=q;r++){

	if(tokens[r].type==TK_LPT){left_parenthesis_cnt++;}
	else if(tokens[r].type==TK_RPT){right_parenthesis_cnt++;}

	if(right_parenthesis_cnt>left_parenthesis_cnt){
		printf("Lanceloia: illegal parentheses match: R>L\n");
		return false;}
	else if(right_parenthesis_cnt==left_parenthesis_cnt)
		if(r!=q) return false;
	}
	if(right_parenthesis_cnt<left_parenthesis_cnt){
		printf("Lanceloia: illegal parentheses match: L>R\n");
		return false;}
	return true;
}

#define i_type (tokens[i].type)

int eval(int p,int q){
	if(p>q){
		printf("Lanceloia: eval p>q\n");
		//assert(0);
	}
	else if(p==q){
		switch (tokens[p].type){
			case TK_EAX: return cpu.eax;
			case TK_ECX: return cpu.ecx;
			case TK_EDX: return cpu.edx;
			case TK_EBX: return cpu.ebx;
			case TK_ESP: return cpu.esp;
			case TK_EBP: return cpu.ebp;
			case TK_ESI: return cpu.esi;
			case TK_EDI: return cpu.edi;
			case TK_EIP: return cpu.eip;
			case TK_CR3: return cpu.CR3.as_32bits_reg;
			case TK_NUM: return to_number(tokens[p].str,strlen(tokens[p].str));
			default: printf("Lanceloia: unknow type\n");
		}
	}
	else if(check_parentheses(p,q)==true){
		return eval(p+1,q-1);
	}
	else {
		int pop=-1;	//priority_operation_position;
		for(int i=p;i<=q;i++){
			if(i_type==TK_LPT){
				int L_cnt=1;i++;
				while(L_cnt){
					if(i_type==TK_LPT)		L_cnt++;
					else if(i_type==TK_RPT)	L_cnt--;
					i++;}
				if(!(i<=q))break;
			}
			if(pop==-1&&oprt_prrt(i_type))pop=i;
			else if(oprt_prrt(i_type)> oprt_prrt(tokens[pop].type))pop=i;
			else if(oprt_prrt(i_type)==oprt_prrt(tokens[pop].type)){
				if(tokens[pop].type!=TK_NEG&&tokens[pop].type!=TK_DRF)pop=i;	//From left to right
				else;}															//From right to left
		}
		switch(tokens[pop].type){
			case TK_ADD: return eval(p,pop-1)+eval(pop+1,q);
			case TK_SUB: return eval(p,pop-1)-eval(pop+1,q);
			case TK_MUL: return eval(p,pop-1)*eval(pop+1,q);
			case TK_DIV: if(eval(pop+1,q))	return eval(p,pop-1)/eval(pop+1,q);
						 else return printf("Lanceloia: Divide by 0, Error\n")&0x0;

			case TK_EQ:  return (eval(p,pop-1)==eval(pop+1,q))&0x1;
			case TK_NEQ: return (eval(p,pop-1)!=eval(pop+1,q))&0x1;
			case TK_LST: return (eval(p,pop-1)<eval(pop+1,q))&0x1;
			case TK_NGT: return (eval(p,pop-1)<=eval(pop+1,q))&0x1;
			case TK_GRT: return (eval(p,pop-1)>eval(pop+1,q))&0x1;
			case TK_NLT: return (eval(p,pop-1)>=eval(pop+1,q))&0x1;

			case TK_AND: return (eval(p,pop-1)&&eval(pop+1,q))&0x1;
			case TK_OR:  return (eval(p,pop-1)||eval(pop+1,q))&0x1;	
			
			case TK_NEG: return -eval(pop+1,q);
			case TK_DRF: return vaddr_read(eval(pop+1,q),8);

			default:printf("Lanceloia: Cann't match\n");return 0x7fffffff;
		}
	}
	return 0;
}
