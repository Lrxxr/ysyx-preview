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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
word_t expr(char *e, bool *success);
void cmd_p_test();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif
	//cmd_p_test();
	/* Start engine. */
	engine_start();

	return is_exit_status_bad();
}

void cmd_p_test(){
	char buffer[65535] = {};
	int cnt = 0;
	FILE *fp = fopen("./tools/gen-expr/input", "r");
	assert(fp != NULL);
	char *testbuf = fgets(buffer, ARRLEN(buffer), fp); 
	while(testbuf != NULL){
		testbuf[strlen(testbuf) - 1] = '\0';
		uint32_t inret = 0;
		bool success = true;
		char *arg = strtok(testbuf, " ");
		sscanf(arg, "%u", &inret);
		char *expression = testbuf + strlen(testbuf) + 1;
		uint32_t ret = expr(expression , &success);
		if (ret != inret){
			printf("result error, ret=%u, inret=%u, expression=%s\n", ret, inret, expression);
		}else{
			cnt++;
		}
		testbuf = fgets(buffer, ARRLEN(buffer), fp);
	}
	Log("一共通过了%d条测试用例",cnt);
	fclose(fp);

}
