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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/vaddr.h>

enum {
  TK_NOTYPE = 256, TK_EQ = 255, TK_PLUS = 254, TK_SUB = 253, TK_MUL = 252, TK_DIV = 251, TK_LEF = 250, TK_RIG = 249, TK_NUM = 248,
	TK_MIN = 247, TK_UEQ = 246, TK_AND = 245, TK_OR = 243, TK_LE = 242, TK_ME = 241, TK_HEX = 240, TK_REG = 239, TK_POINT = 238
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},     // plus
  {"==", TK_EQ},        // equal
	{"-", TK_SUB},        // subtract
	{"\\*", TK_MUL},      // multiply
	{"\\/", TK_DIV},      // divide 
	{"\\(", TK_LEF},      // left parentheses
	{"\\)", TK_RIG},      // right parentheses
	{"\\$[$a-z0-9]+", TK_REG},
	{"!=", TK_UEQ},       
	{"&&", TK_AND},
	{"\\|\\|", TK_OR},
	{"<=", TK_LE},
	{">=", TK_ME},
	{"0x[0-9a-f]+", TK_HEX},
	{"[0-9]+", TK_NUM},

};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
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

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[3072] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;
				tokens[nr_token].str[substr_len] = '\0';
				if(substr_len > 32){
					assert(0);
				}

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array tokens. For certain types
         * of tokens, some extra actions should be performed.
         */

				if(rules[i].token_type != TK_NOTYPE ){
					tokens[nr_token].type = rules[i].token_type;        //record tokens'type
				}
				
        switch (rules[i].token_type) {
					case TK_HEX:
					case TK_NUM: 
						strncpy(tokens[nr_token].str, substr_start, substr_len); 
						nr_token++; 
						break;
					case TK_PLUS: 
					case TK_SUB: 
					case TK_MUL: 
					case TK_DIV:
					case TK_LEF: 
					case TK_RIG:
					case TK_AND:
					case TK_OR:
					case TK_LE:
					case TK_ME:
					case TK_EQ:
					case TK_UEQ: 
						nr_token++; 
						break;
					case TK_NOTYPE: 
						break;
					case TK_REG: 
						strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);;
						nr_token++;
						break;
          default: TODO();                                    //record the number of tokens
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

	/*compare the priority of operators*/
static int cp_pri(int n){
	switch(n){
	  case TK_MUL: return 1;
		case TK_DIV: return 1;
		case TK_PLUS: return 2;
		case TK_SUB: return 2;
		case TK_LE: return 3;
		case TK_ME: return 3;
		case TK_EQ: return 4;
		case TK_AND: return 5;
		case TK_OR: return 5;
		default: assert(0);
	}
}

	/*find dominant_operator*/
static int dominant_operator(int p, int q) {
	int pri = 0;
	int op = 0;
	for(int i = p; i < q; i++){
		switch (tokens[i].type) {
			case TK_HEX:
			case TK_NOTYPE:
			case TK_NUM: 
			case TK_MIN:
			case TK_REG:
			case TK_POINT:
				continue;
			case TK_LEF:
				int n1 = 1;
				int n2 = 0;
				do{
					i++;
					if(tokens[i].type == TK_LEF) n1++;
					else if(tokens[i].type == TK_RIG) n2++;
				}while(n1 != n2);
				continue;
			default:
				if(cp_pri(tokens[i].type) >= pri) {
					pri = cp_pri(tokens[i].type);
					op = i;
				}
		}
	}
 return op;
}

static bool check_parentheses(int p, int q) {
	int i = p;
	if(tokens[i].type != TK_LEF){
		return false;
	}
	int par = 0;
	for(i = p; i <= q; i++ ){
		if(tokens[i].type == TK_LEF){
			par++;
		}else if (tokens[i].type == TK_RIG){
			par--;
		}else{
			continue;
		}

		if(par == 0 && i < q){
			return false;
		}
	}
	if(par == 0){
		return true;
	}
	return false;
}


static uint32_t eval(int p, int q) {
	if(p > q) {
		return printf("Bad expression\n");
	}else if(p == q){
		uint32_t num = 0;
		switch(tokens[p].type){
			case TK_NUM:
				sscanf(tokens[p].str, "%u", &num);
				break;
			case TK_HEX:
				sscanf(tokens[p].str, "%x", &num );
				break;
			case TK_REG:
				bool success = true;
				num = isa_reg_str2val(tokens[p].str, &success);
				break;
			default: 
				assert(0);
		}
		return num;
	}else if(check_parentheses(p, q) == true){
		return eval(p + 1, q -1);
	}else if(p + 1 == q || check_parentheses(p + 1, q) == true){
		switch(tokens[p].type){
			case TK_MIN:
				return -eval(p + 1, q);
			case TK_POINT:
				vaddr_t addr = eval(p + 1, q);
				return vaddr_read(addr, 4);
				break;
			default:
				assert(0);
		}
	}else{
		int op = dominant_operator(p, q); 
		uint32_t val1 = eval(p, op - 1);
		uint32_t val2 = eval(op + 1, q);
 
		switch (tokens[op].type) {
			case TK_PLUS: return val1 + val2;
			case TK_SUB: return val1 - val2;
			case TK_MUL: return val1 * val2;
			case TK_DIV: return val1 / val2;
			case TK_EQ: return val1 == val2;
			case TK_UEQ: return val1 != val2;
			case TK_AND: return val1 && val2;
			case TK_OR: return val1 || val2;
			case TK_LE: return val1 <= val2;
			case TK_ME: return val1 >=val2;
			default: assert(0);
		}
	}
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
	*success = true;
	for(int i = 0; i < nr_token; i++){
		if(tokens[i].type == TK_SUB && (i == 0 || (tokens[i - 1].type != TK_NUM && tokens[i - 1].type != TK_RIG && tokens[i - 1].type != TK_HEX))) {
			tokens[i].type = TK_MIN;
		}else if(tokens[i].type == TK_MUL && (i == 0 || (tokens[i -1].type != TK_NUM && tokens[i - 1].type != TK_RIG && tokens[i - 1].type != TK_HEX))){
			tokens[i].type = TK_POINT;

		}
	}
	uint64_t values = eval(0, nr_token -1);
	printf("%lu\n", values);


  /* TODO: Insert codes to evaluate the expression. */
	// TODO();
  return values;
}
