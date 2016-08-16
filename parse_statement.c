#include "letter_analysis.h"
#include "syntactic_analysis.h"
#include "parse_statement.h"


void statement(Token *t) {
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
    st_semicolon(t);
    break;
  case EOF_token:
    st_EOF(t);
    break;
  default:
    // TODO : syntax error
    return; //?
  }
}

void st_break(Token *t) {
  if (break_list_ct > 0) {
    return -1; // TODO : invalid break, or ignore
  }
  // TODO : set jump to loop end
  loopNest[loopNest_ct].has_break = true;
  checkNxtTokenKind(Semicolon);
  return;
}

void st_continue(Token *t) {
  // TODO : set jump to loop top
  checkNxtTokenKind(Semicolon);
  return;
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


void begin_continue_break(Kind k, int top) {
  // TODO : "int top" is temporally
  if (loopNest_ct >= MAX_LOOP_NEXT_SIZ) {
    // TODO : error
  }
  loopNest[loopNest_ct].st_kind = k;
  loopNest[loopNest_ct].loop_top = top;
  loopNest[loopNest_ct++].has_break = false;
}

void end_continue_break() {
  if (loopNext[loopNext_ct].has_break) {
    // TODO : set loop top label ?
  }
  loopNext_ct--;
}
