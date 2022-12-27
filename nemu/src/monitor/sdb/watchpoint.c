/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
	char exp[32];
	uint64_t value1;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

WP* new_wp(char *arg){
	assert(free_ != NULL);

	WP *temp = free_;
	bool success = false;
	temp->next = NULL;
	strncpy(temp->exp, arg);
	temp->value1 = expr(arg, &success);
	free_ = free_->next;

	if(head == NULL){
		head = temp;
	}else{
		WP * p = head;
		while(p->next != NULL ){
			p = p->next;
		}
		p->next = temp;
	}
	return temp;
}

void free_wp(WP *wp){
	if(wp == NULL){
		printf("Invaild Node\n");
	}else if(wp == head){
		wp->next = free_->next;
		free_->next = wp;
	}else{
		WP *p = head;
		WP *posnode;
		while(p->next != wp){
			posnode = p;
			p = p->next;
		}
		posnode->next = p->next;
		wp->next = free_->next;
		free_->next = wp;
	}
	return 0;
}
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

