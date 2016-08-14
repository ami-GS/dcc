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
  return;
}

void st_while(Token *t) {
  nextToken(t); // point at -> '('
  // TODO : set label to condition check. (1)
  expr_with_paren_check(t);
  // TODO : jump to label (2) when condition is false.
  statement(t);
  // TODO : jump to label (1) to loop again
  // TODO : set end label. (2)
  return;
}

void st_If(Token *t) {
  nextToken(t);
  expr_with_paren_check(t);
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
    expr_with_paren_check(t);
    checkNxtTokenKind(Semicolon);
    // TODO : jump to label (1) when condition is True.
  } else {
    // error when While is not existing
    return -1;
  }
  return 1;
}
