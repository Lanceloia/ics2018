#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

enum{WP_CHANGE, WP_TRUE, WP_FALSE};

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	
	/* TODO: Add more members if necessary */
	int wp_value;
	int wp_type;
	char wp_expr[256];
} WP;

WP *new_wp();
bool dsb_wp(int);
bool upd_wp();
void insert_wp();
void free_wp();
void show_wps();
#endif
