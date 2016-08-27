#include <string.h>
#include <stdio.h>
#include "syntactic_analysis.h"
#include "opcode.h"
#include "instruction.h"
#include "data_declare.h"
#include "symbol_table.h"

void expression(Token *t) {
  if (t->kind == VarName || checkNxtTokenKind(Assign)) {
    int varName = t->text[0];
    nextToken(t); // skip '='
    nextToken(t);
    expression(t);
    variables[varName] = stack[stack_c];
  } else {
    or_exp(t);
  }
}

void or_exp(Token *t) {
  and_exp(t);
  while (t->kind == Or) {
    nextToken(t);
    and_exp(t);
    operate(Or);
  }
}

void and_exp(Token *t) {
  equ_exp(t);
  while (t->kind == And) {
    nextToken(t);
    equ_exp(t);
    operate(And);
  }
}

void equ_exp(Token *t) {
  Kind op;
  rel_exp(t);
  while (t->kind == Equal || t->kind == NotEq) {
    op = t->kind;
    nextToken(t);
    rel_exp(t);
    operate(op);
  }
}

void rel_exp(Token *t) {
  Kind op;
  add_sub_exp(t);
  while (strstr(" == <= >= =< => ", t->text) != NULL) {
    op = t->kind;
    nextToken(t);
    add_sub_exp(t);
    operate(op);
  }
}

void add_sub_exp(Token *t) {
  Kind op;
  mul_div_mod_exp(t);
  while (t->kind == Add || t->kind == Sub) {
    op = t->kind;
    nextToken(t);
    mul_div_mod_exp(t);
    operate(op);
  }
}

void mul_div_mod_exp(Token *t) {
  Kind op;
  factor(t);
  while (t->kind == Mul || t->kind == Div || t->kind == Mod) {
    op = t->kind;
    nextToken(t);
    factor(t);
    operate(op);
  }
}

int factor(Token *t) {
  Kind op;
  TableEntry *te_tmp;
  int find;

  switch (t->kind) {
  case Add: case Sub: case Not: case Incre: case Decre:
    // like, +1, -1, !0
    nextToken(t);
    factor(t);
    // set inc dec preprocessing
    if (t->kind == Incre || t->kind == Decre) {
      to_left_val();
    }
    genCode_unary(t->kind);
    break;
  case Ident:
    // TODO : search registered table item
    find = search(t->text, te_tmp);
    if (!find) {
      return -1; // TODO : do something
    }
    switch (te_tmp->kind) {
    case var_ID: case arg_ID:
      if (te_tmp->arrLen == 0) {
	genCode(LOD, te_tmp->level, te_tmp->addr);
	nextToken(t);
      } else {
	nextToken(t);
	if (t->kind == Lbrace) {
	  // TODO : currently only [] based addressing
	  nextToken(t);
	  genCode(LDA, te_tmp->level, te_tmp->addr); // TODO : unsure here
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
	nextToken(t);
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
    nextToken(t);
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
  nextToken(t);
}

void operate(Kind op) {
  int v2 = pop(), v1 = pop();
  if ((op == Div || op == Mod) && v2 == 0) {
    // TODO : zero division
    return;
  }

  switch (op) {
  case Add:
    push(v1+v2);
    break;
  case Sub:
    push(v1-v2);
    break;
  case Mul:
    push(v1*v2);
    break;
  case Div:
    push(v1/v2);
    break;
  case Mod:
    push(v1%v2);
    break;
  default:
    break;
  }
}

// TODO : consider the type pushed, Token might be good?
void push(int n) {
  if (stack_c >= STACK_SIZ) {
    return; // TODO : stack overflow
  }
  stack[stack_c++] = n;
}

//TODO : same as the above
int pop() {
  if (stack_c <= 0) {
    return -1; // TODO : stack underflow
  }
  return stack[--stack_c];
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

void callFunc(Token *t, TableEntry *te) {
  nextToken(t); // point to '('
  int arg_cnt = 0;
  if (!checkNxtTokenKind(Rparen)) {
    do {
      nextToken(t);
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
  genCode2(CALL, te->addr);
  return;
}
