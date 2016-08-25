#include "letter_analysis.h"
#include "syntactic_analysis.h"
#include "parse_statement.h"


void statement(Token *t) {
  Kind tk = t->kind;
  if (tk == While || tk == Do || tk == switch) {
    begin_continue_break(tk);
  }
  switch (t->kind) {
  case Break:
    st_break(t);
    break;
  case Continue:
    st_continue(t);
    break;
  case Switch:
    st_switch(t);
    break;
  case Case:
    st_case(t);
    break;
  case Default:
    st_default(t);
    break;
  case For:
    st_for(t);
    break;
  case Do:
    st_do(t);
    break;
  case While:
    st_while(t);
    break;
  case If:
    st_if(t);
    break;
  case Return:
    st_return(t);
    break;

  case Printf: case Exit:
    // TODO : Built-in fucntion, no returns
    st_build_in_void(t);
    break;
  case Input:
    // TODO : Build-in function, non-void returns
    st_build_in(t);
    break;
  case Ident:
    st_ident(t);
    break;
  case Incre: case Decre:
    st_inc_dec(t);
    break;
  case Lbrace:
    st_lbrace(t);
    break;
  case Semicolon:
    nextToken(t);
    break;
  case EOF_token:
    st_EOF(t);
    break;
  default:
    // TODO : syntax error
    return; //?
  }
  if (tk == While || tk == Do || tk == Switch || tk == For) {
    end_continue_break();
  }
}

void st_break(Token *t) {
  if (loopNest_ct == 0) {
    return -1; // TODO : invalid break, or ignore
  }
  // TODO : set jump to loop end
  loopNest[loopNest_ct].has_break = 1;
  checkNxtTokenKind(Semicolon);
  return;
}

void st_continue(Token *t) {
  // TODO : set jump to loop top
  checkNxtTokenKind(Semicolon);
  return;
}

void st_switch(Token *t) {
  nextToken(t);
  expr_with_check(t, '(', ')');
  // TODO : jump to condition table check (1)
  begin_switch(); // initialize
  statement(t);
  // TODO : jump to end of switch (2)
  // TODO : set label (1) for condition table
  end_switch(); // apply gathered case list as table
  // TODO : set label (2)
  return;
}

void st_case(Token *t) {
  nextToken(t);
  int val;
  // TODO : pick up const value for condition check
  expr_with_check(t, 0 , ':');
  if (switchNest_ct == 0) {
    return -1; // TODO : no switch associating
  }
  int i;
  for (i = switchNest[switchNest_ct].case_list_st_addr; i <= caseList_ct; i++) {
    if (caseList[i].value == val) {
      return -1; // TODO : case duplication
    }
  }
  if (caseList_ct >= MAX_CASE_SIZ) {
    return -1; // TODO : case size overflow
  }
  caseList[i].value = val;
  // TODO : caseList[i].address =
  statement(t);
  return;
}

void st_default(Token *t) {
  if (switchNest_ct == 0) {
    return -1; // TODO : no switch association
  }
  if (switchNest[switchNest_ct].default_addr != -1) {
    return -1; // TODO : duplication of default
  }
  // TODO : set address of switchNest[switchNest_ct].default_addr = ;
  statement(t);

}

void st_while(Token *t) {
  nextToken(t); // point at -> '('
  // TODO : set label to condition check. (1)
  expr_with_check(t, '(', ')');
  // TODO : jump to label (2) when condition is false.
  statement(t);
  // TODO : jump to label (1) to loop again
  // TODO : set end label. (2)
  return;
}

void st_If(Token *t) {
  nextToken(t);
  expr_with_check(t, '(', ')');
  // TODO : jump to label (1) when condition is false.
  statement(t);
  if (t->kind != Else) {
    // TODO : set end label. (1)
    return;
  }
  // TODO : force jump to label (2) ???? suspicious
  // TODO : set end label. (1)
  nextToken(t); // point at -> 'if'?
  statement(t);
  // TODO : set end label. (2)
  return;  
}

void st_Do(Token *t) {
  nextToken(t); // point at -> '{'
  // TODO : set label to loop. (1)
  statement(t);
  //nextToken(t); //-> while
  if (t->kind == While) {
    nextToken(t); //-> condition
    expr_with_check(t, '(', ')');
    checkNxtTokenKind(Semicolon);
    // TODO : jump to label (1) when condition is True.
  } else {
    // error when While is not existing
    return -1;
  }
  return 1;
}

void st_For(Token *t) {
  nextToken(t);
  if (t->kind == Semicolon) {
    // TODO : no expr 1
  } else {
    expr_with_check(t, 0, ';');
    // TODO : remove result;
  }

  // TODO : set label to loop. (1)
  if (t->kind == Semicolon) {
    // TODO : no expr 2
  } else {
    expr_with_check(t, 0, ';');
  }
  // TODO : jump to end if false (3)
  // TODO : jump to statement (2)

  // TODO : set label to increment (4)
  begin_continue_break();
  if (t->kind == Rparen) {
    // TODO : no expr 3
  } else {
    expr_with_check(t, 0, ')');
    // TODO : remove result;
  }
  // TODO : jump to label (1)

  // TODO : set label (2)
  statement(t);
  // TODO : jump to label (4)
  // TODO : set label end (3)
  return;
}

void st_return(Token *t) {
  nextToken(t);
  if (t->kind == Semicolon) {
    // TODO : check func return type, if not void, then error
  } else {
    expression(t);
    // TODO : return type comparison t->kind, func type
  }
  checkNxtTokenKind(Semicolon);
  // TODO : jump to caller
  return;
}

void begin_switch() {
  if (switchNest_ct >= MAX_SWITCH_NEST_SIZ) {
    return -1; // TODO : switch nest over limitation
  }
  switchNest[switchNest_ct].default_addr = -1;
  switchNest[switchNest_ct++].case_list_st_addr = caseList_ct+1;
}

void end_switch() {
  int i, st = switchNest[switchNest_ct].case_list_st_addr;
  for (i = st; i <= caseList_ct; i++) {
    // TODO : compare caseList[i].value and stack top?
    // TODO : jump to caseList[i].addr if true
  }
  // TODO : remove top comparison result?
  if (switchNest[switchNest_ct].default_addr != -1) {
    // when this nest has default
    // TODO : jump to switch_nest[switchNest_ct].default_addr
  }
  caseList_ct = st - 1;
  switchNest_ct--;
}

void begin_continue_break(Kind k, int top) {
  // TODO : "int top" is temporally
  if (loopNest_ct >= MAX_LOOP_NEST_SIZ) {
    // TODO : error
  }
  loopNest[loopNest_ct].st_kind = k;
  loopNest[loopNest_ct].loop_top = top;
  loopNest[loopNest_ct++].has_break = 0;
}

void end_continue_break() {
  if (loopNest[loopNest_ct].has_break) {
    // TODO : set loop top label ?
  }
  loopNest_ct--;
}

int get_loop_top() {
  int i;
  // TODO : check textbook is using i > 0
  if (i = loopNest_ct; i >=0 ; i--) {
    if (loopNest[i].st_kind != switch)
      return loopNest[i].loop_top;
  }
  return -1; // TODO : no loop associating
}
