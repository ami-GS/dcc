#include <string.h>
#include <stdio.h>
#include "syntactic_analysis.h"
#include "opcode.h"
#include "instruction.h"
#include "data_declare.h"
#include "symbol_table.h"

void expression(Token *t) {
  term(t, 2);
  if (t->kind == Assign) {
    // TODO : need to study
    to_left_val();
    nextToken(t, 0);
    expression(t);
    genCode1(ASSV);
  }
}

void term(Token *t, int n) {
  // TODO : need to study
  if (n == 8) {
    factor(t);
    return;
  }
  term(t, n+1);

  Kind k;
  while (n == opOder(t->kind)) {
    k = t->kind;
    nextToken(t, 0);
    term(t, n+1);
    genCode_binary(k);
  }
}

int factor(Token *t) {
  Kind op;
  TableEntry *te_tmp = NULL;
  int find;

  switch (t->kind) {
  case Add: case Sub: case Not: case Incre: case Decre:
    // like, +1, -1, !0
    nextToken(t, 0);
    factor(t);
    // set inc dec preprocessing
    if (t->kind == Incre || t->kind == Decre) {
      to_left_val();
    }
    genCode_unary(t->kind);
    break;
  case Ident:
    // TODO : search registered table item
    te_tmp = search(t->text);
    if (te_tmp == NULL) {
      return -1; // TODO : do something
    }
    switch (te_tmp->kind) {
    case var_ID: case arg_ID:
      if (te_tmp->arrLen == 0) {
	genCode(LOD, te_tmp->level, te_tmp->code_addr);
	nextToken(t, 0);
      } else {
	nextToken(t, 0);
	if (t->kind == Lbrace) {
	  // TODO : currently only [] based addressing
	  nextToken(t, 0);
	  genCode(LDA, te_tmp->level, te_tmp->code_addr); // TODO : unsure here
	  expr_with_check(t, '[', ']');
	  genCode2(LDI, INT_SIZE);
	  genCode1(MUL); // index * data size
	  genCode1(Add); // add it to tp->adr
	  genCode1(VAL); // pick up the value
	} else {
	  return -1; // TODO : no index
	}
      }
      if (t->kind == Incre || t->kind == Decre) {
	to_left_val();
	// TODO : need to study below
	if (t->kind == Incre) {
	  genCode1(INC);
	  genCode2(LDI, 1);
	  genCode1(SUB);
	} else {
	  genCode1(DEC);
	  genCode2(LDI, 1);
	  genCode1(ADD);
	}
	nextToken(t, 0);
      }
      break;
    case func_ID: case proto_ID:
      if (te_tmp->dType == VOID_T) {
	return -1; // TODO : void return function should not be in expression
      }
      callFunc(t, te_tmp);
      break;
    }
    break;
  case IntNum:
    // TODO : also every type. Using TokenStack is the besy way?
    genCode2(LDI, t->intVal);
    nextToken(t, 0);
    break;
  case Lparen:
    if (expr_with_check(t, '(', ')')) {
      return -1; // TODO : error
    }
    break;
  //case Printf: case Input: case Exit: // TODO : not yet implemented

  default:
    return -1; // TODO error
    }
  //nextToken(t, 0); // TODO : no need?
}

int expr_with_check(Token *t, char l, char r) {
  if (l != 0 && t->text[0] != l) {
    return -1;
  }
  expression(t);
  if (r != 0 && t->text[0] != r) {
    return -1;
  }
  return 1;
}

int is_const_expr() {
  Token t = {NulKind, "", 0};
  t_buf_open = 0;
  do {
    nextToken(&t, 0);
    t_buf_enqueue(t);
    // TODO : Can ! and - be allowed?
    if (!(t.kind == Int || is_ope1(t.kind) || t.kind == Rbracket)) {
      t_buf_open = 1;
      return -1; // TODO : invalid const expression
    }
  } while (t.kind != Rbracket); // TODO : this might cause forever loop?
  t_buf_open = 1;
  return 1;
}


void callFunc(Token *t, TableEntry *te) {
  nextToken(t, 0); // point to '('
  int arg_cnt = 0;
  if (!checkNxtTokenKind(Rparen)) {
    do {
      nextToken(t, 0);
      expression(t);
      ++arg_cnt;
    } while (t->kind != Comma);
  }
  if (!checkNxtTokenKind(Rparen)) {
    return -1; // TODO : no end paren
  }

  if (arg_cnt != te->args) {
    return -1; // TODO : few or more arguments
  }
  genCode2(CALL, te->code_addr);
  return;
}

int opOder(Kind k) {
  // TODO : can operation kind itself have order integer?
  switch(k) {
  case Mul: case Div: case Mod:                                                return 7;
  case Add: case Sub:                                                          return 6;
  case Less: case LessEq: case EqLess: case Great: case GreatEq: case EqGreat: return 5;
  case Equal: case NotEq:                                                      return 4;
  case And:                                                                    return 3;
  case Or:                                                                     return 2;
  case Assign:                                                                 return 1;
  default:                                                                     return 0;
  }
}
