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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

//this should be enough
static char buf[65536] = {};
static char outbuf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static uint32_t choose(uint32_t n) {
	return rand() % n;
}

static int gen_num() {
	char num_buf[200];
	uint32_t num = rand() % 100 + 1;
	sprintf(num_buf, "%du", num);
	//printf("num_buf=%s\n", num_buf);
	strcat(buf, num_buf);
}

static char gen(char str) {
	char str_buf[2] = {str, '\0'};
	strcat(buf, str_buf);
}

static char gen_rand_op() {
	switch (choose(4)){
		case 0:
			gen('+');
			return '+';
		case 1:
			gen('-');
			return '-';
		case 2:
			gen('*'); 
			return '*';
		case 3: 
			gen('/');
			return '/';
	}
}

static void gen_space() {
	switch(choose(6)) {
		case 0:
		case 1:
		case 2: break;
		case 3:
		case 5: strcat(buf, " ");	break;
	}
}

static void gen_rand_expr(){
	if(strlen(buf) >= 65535) {
		return ;
	}

	switch (choose(3)) {
		case 0: 
			gen_num(); 
			break;
		case 1: 
			gen('(');
			gen_space();
			gen_rand_expr();
			gen_space();
			gen(')');
			break;
		default:
			gen_rand_expr();
			gen_space();
			gen_rand_op();
			gen_space();
			gen_rand_expr();
			break;
	}
}

static void output_buf() {
	int j = 0;
	for(int i = 0; buf[i] != '\0'; i++){
		if(buf[i] != 'u'){
			outbuf[j++] = buf[i];
		}
	}
	outbuf[j] = '\0';
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
		buf[0] = '\0';
		gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr -Werror");
    if (ret != 0) {
			i--;
			continue;
		}

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    uint32_t result;
    int a = fscanf(fp, "%u", &result);
    pclose(fp);
		if (a != 1){
			i--;
			continue;
		}
		output_buf();

    printf("%u %s\n", result, outbuf);
  }
  return 0;
}
