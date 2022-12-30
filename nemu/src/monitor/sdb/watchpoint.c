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

/*typedef struct watchpoint {
  int NO;
	char exp[32];
	uint64_t value1;
  struct watchpoint *next;

 //  TODO: Add more members if necessary 

} WP;*/

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

bool wp_change(){
	bool success = false;
	uint64_t value2;
	WP *p = head;
	while(p != NULL){
		value2 = expr(p->exp, &success);
		if(p->value1 != value2){
			printf("Old value = %lu\n", p->value1);
			printf("New value = %lu\n", value2);
			printf("watchpoint %d:%s\n", p->NO, p->exp);
			p->value1 = value2;
			return true;
		}
		p = p->next;
	}
	return false;
}

/*遍历*/
WP* travel(WP *list){
	WP *p = list;
	while(p->next){
		p = p->next;
	}
	return p;
}

void insert_free(WP *free_, WP *wp){
	if(free_ != NULL){
		WP *p = travel(free_);
		p->next = wp;
    wp->next = NULL;
	}else{
		wp->next = NULL;
		free_ = wp;
	}
}

WP* new_wp(char *arg){
	assert(free_ != NULL);

	WP *temp = free_;
	bool success = false;
	strcpy(temp->exp, arg);
	temp->value1 = expr(arg, &success);
	free_ = free_->next;
	temp->next = NULL;

	if(head == NULL){
		head = temp;
	}else{
		WP *p = travel(head);
		p->next = temp;
	}
	/*WP *p = head;
	while(p != NULL){
		printf("new_head_addr=%p\n", p);
		p = p->next; 
	}*/
	return temp;
}

void free_wp(int n){
	 WP *wp = head;
	 while(wp->NO != n){
		 wp = wp->next;
	 }
	if(head == NULL){
		printf("Invaild Node\n");
		return ;
	}else if(wp == head){
		WP *buffer = head->next;
		insert_free(free_, wp);
		head = buffer;
	}else{
	  WP *buffer = head;
		WP *posnode = NULL;
		while(buffer != wp){
			posnode = buffer;
			buffer = buffer->next;
		}
		posnode->next = buffer->next;
		insert_free(free_, wp);
	}
	/*WP *point = head;
	while(point != NULL){
		printf("free_head_addr=%p\n", point);
		point = point->next;
	}*/
	printf("delete watchpoint %d\n", n);
}

void watchpoint_display(){
	WP *point = head;
	while(point != NULL){
		printf("wacthpoint %d: %s\n", point->NO, point->exp);
		point = point->next;
	}
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

