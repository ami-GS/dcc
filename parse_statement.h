#ifndef DCC_PARSE_STATEMENT_H_
#define DCC_PARSE_STATEMENT_H_

#include "instruction.h"
#include "opcode.h"

#define LABEL_TOP(addr) addr=code_ct

#define MAX_LOOP_NEST_SIZ 100
struct {
    Kind st_kind;
    int  loop_top;
    int  has_break;
} loopNest[MAX_LOOP_NEST_SIZ];
static int loopNest_ct = 0;


#define MAX_CASE_SIZ 100
struct {
    int value;
    int address;
} caseList[MAX_CASE_SIZ];
static int caseList_ct = 0; // TODO : this is not for each switch

#define MAX_SWITCH_NEST_SIZ 10
struct {
    int default_addr;
    int case_list_st_addr;
} switchNest[MAX_SWITCH_NEST_SIZ];
static int switchNest_ct = 0;

void statement(Token *t);
void st_break(Token *t);
void st_continue(Token *t);
void st_switch(Token *t);
void st_case(Token *t);
void st_default(Token *t);
void st_for(Token *t);
void st_do(Token *t);
void st_while(Token *t);
void st_if(Token *t);
void st_build_in_void(Token *t);
void st_build_in(Token *t);
void st_ident(Token *t);
void st_lbrace(Token *t);
void st_semicolon(Token *t);
void st_EOF(Token *t);
void begin_switch();
void end_switch();
void begin_continue_break();
void end_continue_break();
int get_loop_top();

#endif //DCC_PARSE_STATEMENT_H_
