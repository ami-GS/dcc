#ifndef DCC_PARSE_STATEMENT_H_
#define DCC_PARSE_STATEMENT_H_

#define BREAK_LIST_SIZ 64
int break_list[BREAK_LIST_SIZ];
int break_list_ct = 0;

#define MAX_LOOP_NEST_SIZ 100
struct {
    Kind st_kind;
    int  loop_top;
    int  has_break;
} loopNest[MAX_LOOP_NEST_SIZ];
int loopNest_ct = 0;

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
void st_return(Token *t);
void st_build_in_void(Token *t);
void st_build_in(Token *t);
void st_ident(Token *t);
void st_inc_dec(Token *t);
void st_lbrace(Token *t);
void st_semicolon(Token *t);
void st_EOF(Token *t);

#endif //DCC_PARSE_STATEMENT_H_
