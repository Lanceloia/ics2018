#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
static uint32_t wp_size;

void init_wp_pool() {
		int i;
		for (i = 0; i < NR_WP; i ++) {
				wp_pool[i].NO = i;
				wp_pool[i].next = &wp_pool[i + 1];
				wp_pool[i].wp_value = 0;
				wp_pool[i].wp_type = WP_CHANGE;
				wp_pool[i].wp_expr[0] = '\0';
		}
		wp_pool[NR_WP - 1].next = NULL;

		head = NULL;
		free_ = wp_pool;
		wp_size = 0;
}

/* TODO: Implement the functionality of watchpoint */

void insert_wp(WP *wp){

		if(wp==NULL) {
				return;
		}

		if(head==NULL) {
				head=wp;
				wp->next=NULL;
				return;
		}

		else {
				WP *it=head;
				while(it->next != NULL)
						it = it->next;
				it->next=wp;
				wp->next=NULL;
				return;
		}

}

void free_wp(WP *wp){

		if(head == NULL)
				return;

		WP *it=head;
		while(it->next != NULL && it != wp && it->next != wp)
				it = it->next;

		if(it == wp){								//wp is the head
				head = wp->next;

				wp->next = free_;				//free wp
				wp->wp_value = 0;
				wp->wp_type = WP_CHANGE;
				wp->wp_expr[0] = '\0';
				free_ = wp;
		}
		else if(it->next == wp) {		//wp isn't the head
				it->next = wp->next;	

				wp->next = free_;				//free wp
				wp->wp_value = 0;
				wp->wp_type = WP_CHANGE;
				wp->wp_expr[0] = '\0';
				free_=wp;
		}
		else {											//can't find wp
				printf("Lanceloia: free_wp failed\n");
		}
}

void show_wps(){

		WP *it = head;
		printf("| Num  \t| Value     \t| Type       | What                 |\n");
		while(it != NULL){
				printf("| %-5d\t| %-10d\t| %s |%-18.18s   |\n",
						it->NO,it->wp_value,it->wp_type==WP_CHANGE?"watchpoint":"breakpoint",it->wp_expr);
				it = it->next;
		}
		return;
}

WP *new_wp(char *args, int type){

		if(free_ == NULL)
				return NULL;
		bool success_flag = false;
		int init_value = expr(args, &success_flag);
		if(success_flag == false)
				return NULL;
		WP *wp = free_;
		free_ = free_->next;
		insert_wp(wp);
		wp->wp_value = init_value;
		wp->wp_type = type;
		strcpy(wp->wp_expr,args);	
		return wp;
}

bool dsb_wp(int Num){

		WP *it = head;
		while(it != NULL&& it->NO != Num)
				it = it ->next;
		if(it == NULL) {
				return false;
		}
		else {
				free_wp(it);
				return true;
		}
}

bool upd_wp(){

		bool change_flag = false;
		bool success_flag = false;
		WP *it = head;

		int old_v, new_v;
		while(it != NULL){
			switch(it->wp_type){
				case WP_CHANGE:
				old_v = it->wp_value;
				new_v = expr(it->wp_expr, &success_flag);
				if (new_v != old_v){
						printf("watchpoint %d: %s from %d to %d\n",it->NO,it->wp_expr,old_v,new_v);
						change_flag=true;
				}
				break;
				case WP_TRUE:
				new_v = expr(it->wp_expr,&success_flag);
				if(new_v == true){
						printf("breakpoint %d: %s\n",it->NO,it->wp_expr);
						change_flag=true;
				}
				break;
				default: assert(0);break;
			}
				it->wp_value = new_v;
				it = it->next;
			
		}

		return change_flag;
}


