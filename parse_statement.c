#include "letter_analysis.h"
#include "syntactic_analysis.h"
#include "parse_statement.h"
#include "parse.h"
#include "opcode.h"
#include "instruction.h"
#include "misc.h"


void statement(Token *t) {
  Kind tk = t->kind;
  if (tk == While || tk == Do || tk == Switch) {
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
  case Ident: case IntNum: case Mul: // TODO : correct?
    expr_with_check(t, 0, ';');
    break;
  case Incre: case Decre:
    st_inc_dec(t);
    break;
  case Lbrace:
    block(t, NULL);
    break;
  case Semicolon:
    nextToken(t, 0);
    break;
  case Int: case Float: case Char: case Double:
  case IntP: case FloatP: case CharP: case DoubleP:// TODO : enhance here
    expr_with_check(t, 0, ';');
    break;
  case EOF_token:
    return; // TODO : unexpected error (occures when ';' is forgotten etc..)
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
    error("no corresponding statement for 'break'");
  }
  // set jump to loop end
  genCode2(JMP, NO_FIX_BREAK_ADDR);
  loopNest[loopNest_ct-1].has_break = 1;
  checkNxtTokenKind(';');
  nextToken(t, 0);
  return;
}

void st_continue(Token *t) {
  if (loopNest_ct == 0) {
    error("no corresponding statement for 'continue'");
  }
  // set jump to loop top
  GEN_JMP_TOP(get_loop_top());
  checkNxtTokenKind(';');
  nextToken(t, 0);
  return;
}

void st_switch(Token *t) {
  int table_addr, end;
  nextToken(t, 0);
  expr_with_check(t, '(', ')');
  // jump to condition table check (1)
  GEN_JMP_BUTTOM(table_addr); // TODO : need to change name
  begin_switch(); // initialize
  statement(t);
  // jump to end of switch (2)
  GEN_JMP_BUTTOM(end);
  // set label (1) for condition table
  backpatch(table_addr, code_ct);
  end_switch(); // apply gathered case list as table
  // set label (2)
  backpatch(end, code_ct);
  return;
}

void st_case(Token *t) {
  nextToken(t, 0);
  // pick up const value for condition check
  // TODO : suspicious
  if (!is_const_expr()) {
    error("switch condition must be const value");
  } else if (switchNest_ct == 0) {
    error("no corresponding switch for 'case'");
  }

  expr_with_check(t, 0 , ':');
  int val;
  if (codes[code_ct-1].opcode == LDI) {
    val = codes[code_ct-1].opdata; // TODO : suspicious
  }
  code_ct--; // remove LDI and its const
  int i;
  for (i = switchNest[switchNest_ct-1].case_list_st_addr; i < caseList_ct; i++) {
    if (caseList[i].value == val) {
      error("case condition is duplicating");
    }
  }
  if (caseList_ct >= MAX_CASE_SIZ) {
    error("too many cases");
  }
  caseList[caseList_ct].value = val;
  // caseList[i].address =
  caseList[caseList_ct++].address = code_ct;
  statement(t);
  return;
}

void st_default(Token *t) {
  if (switchNest_ct == 0) {
    error("no corresponding switch for 'default'");
  }
  // TODO : 0 is danger. -1 should be applied as initialization
  if (switchNest[switchNest_ct-1].default_addr != -1) {
    error("multiple 'default'");
  }
  // set address of switchNest[switchNest_ct].default_addr = ;
  switchNest[switchNest_ct-1].default_addr = code_ct;
  if (!checkNxtTokenKind(':')) {
    error("end ':' is missing");
  }
  nextToken(t, 0); nextToken(t, 0);
  statement(t);
}

void st_while(Token *t) {
  int loop_top, loop_buttom;
  nextToken(t, 0); // point at -> '('
  // set label to condition check. (1)
  LABEL_TOP(loop_top);
  expr_with_check(t, '(', ')');
  // jump to label (2) when condition is false.
  GEN_JPF_BUTTOM(loop_buttom);
  statement(t);
  // jump to label (1) to loop again
  GEN_JMP_TOP(loop_top);
  // set end label. (2)
  backpatch(loop_buttom, code_ct);
  return;
}

void st_if(Token *t) {
  int top, else_buttom, end_buttom;
  nextToken(t, 0);
  expr_with_check(t, '(', ')');
  // jump to label (1) when condition is false.
  GEN_JPF_BUTTOM(end_buttom);
  statement(t);
  if (t->kind != Else) {
    // set end label. (1)
    backpatch(end_buttom, code_ct);
    return;
  }
  // force jump to label (2) ???? suspicious
  GEN_JMP_BUTTOM(else_buttom);
  // set end label. (1)
  backpatch(end_buttom, code_ct);
  nextToken(t, 0); // point at -> 'if'?
  statement(t);
  // set end label. (2)
  backpatch(else_buttom, code_ct);
  return;  
}

void st_do(Token *t) {
  int loop_top, loop_buttom;
  nextToken(t, 0); // point at -> '{'
  // set label to loop. (1)
  LABEL_TOP(loop_top);
  statement(t);
  if (t->kind == While) {
    nextToken(t, 0); //-> condition
    expr_with_check(t, '(', ')');
    checkNxtTokenKind(Semicolon);
    nextToken(t, 0);
    // jump to label (1) when condition is True.
    GEN_JPT_BUTTOM(loop_top);
  } else {
    error("'do' statement needs 'while'");
  }
}

void st_for(Token *t) {
  int loop_top, loop_buttom, inst_top, exp_label;
  nextToken(t, 0); nextToken(t, 0);
  if (t->kind == ';') {
    // no expr 1
    nextToken(t, 0);
  } else {
    expr_with_check(t, 0, ';');
  }

  // set label to loop. (1)
  LABEL_TOP(loop_top);
  if (t->kind == ';') {
    // no expr 2
    genCode2(LDI, 1); // true
    nextToken(t, 0);
  } else {
    expr_with_check(t, 0, ';');
  }
  // jump to end if false (3)
  GEN_JPF_BUTTOM(loop_buttom);
  // jump to statement (2)
  GEN_JMP_BUTTOM(inst_top);

  // set label to increment (4)
  begin_continue_break();
  LABEL_TOP(exp_label);
  if (t->kind == ')') {
    // no expr 3
    nextToken(t, 0);
  } else {
    expr_with_check(t, 0, ')');
    // remove result;
    if (codes[code_ct-1].opcode == INCL || codes[code_ct-1].opcode == DECL)
      remove_op_stack_top();
  }
  // jump to label (1)
  GEN_JMP_TOP(loop_top);

  // set label (2)
  backpatch(inst_top, code_ct);
  statement(t);
  // jump to label (4)
  GEN_JMP_TOP(exp_label);
  // set label end (3)
  backpatch(loop_buttom, code_ct);
  return;
}

void st_return(Token *t) {
  nextToken(t, 0);
  if (t->kind == ';') {
    // check func return type, if not void, then error
    if (funcPtr->dType != VOID_T)
      error("Warnning : this function must return void");
  } else {
    expr_with_check(t, 0, ';');
    // return type comparison t->kind, func type
    if (funcPtr->dType == VOID_T)
      error("Warnning : this function must not return anything");
  }
  // jump to caller
  genCode2(JMP, NO_FIX_RET_ADDR);
}

void st_build_in_void(Token *t) {
  return;
}

void st_build_in(Token *t) {
  return;
}

void st_inc_dec(Token *t) {
  expr_with_check(t, 0, ';');
  remove_op_stack_top();
  return;
}

void st_declare(Token *t) {
  while (t->kind == Int || t->kind == Char || t->kind == Float) { // TODO : not only Int
    TableEntry tmp = {no_ID, "", NON_T, LOCAL, 0, 0, 0};
    set_dtype(&tmp, t);
    set_name(&tmp, t);
    declare_var(&tmp, t);
    nextToken(t, 0);
  }
  return;
}

void begin_switch() {
  if (switchNest_ct >= MAX_SWITCH_NEST_SIZ) {
    error("switch is nesting over limitation");
  }
  switchNest[switchNest_ct].default_addr = -1;
  switchNest[switchNest_ct++].case_list_st_addr = caseList_ct;
}

void end_switch() {
  int i, st = switchNest[switchNest_ct-1].case_list_st_addr;
  for (i = st; i < caseList_ct; i++) {
    // compare caseList[i].value and stack top?
    genCode2(EQCMP, caseList[i].value);
    // jump to caseList[i].addr if true
    genCode2(JPT, caseList[i].address);
  }
  // TODO : remove top comparison result?
  genCode1(DEL);
  if (switchNest[switchNest_ct-1].default_addr != -1) {
    // when this nest has default
    // jump to switch_nest[switchNest_ct].default_addr
    genCode2(JMP, switchNest[switchNest_ct-1].default_addr);
  }
  caseList_ct = st;
  switchNest_ct--;
}

void begin_continue_break(Kind k) {
  if (loopNest_ct >= MAX_LOOP_NEST_SIZ) {
    // TODO : error
  }
  loopNest[loopNest_ct].st_kind = k;
  loopNest[loopNest_ct].loop_top = code_ct;
  loopNest[loopNest_ct++].has_break = 0;
}

void end_continue_break() {
  if (loopNest[loopNest_ct-1].has_break)
    // set loop top label
    backpatch_break(loopNest[loopNest_ct-1].loop_top);
  loopNest_ct--;
}

int get_loop_top() {
  int i;
  // TODO : check textbook is using i > 0
  for (i = loopNest_ct-1; i >=0 ; i--) {
    if (loopNest[i].st_kind != Switch)
      return loopNest[i].loop_top;
  }
  error("no associating loop");
}
