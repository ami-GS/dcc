#include <string.h>
#include <stdio.h>
#include "syntactic_analysis.h"
#include "opcode.h"
#include "instruction.h"
#include "data_declare.h"
#include "symbol_table.h"

void expression(Token *t, DataType type) {
  // researve expression type
  Kind k;
  term(t, 2);
  switch (t->kind) {
    // TODO : need to study
  case Assign:
    to_left_val();
    nextToken(t, 0);
    expression(t, NON_T);
    break;
  case AddAss: case SubAss: case MulAss: case DivAss: case ModAss: // case NotEq:
  case BandAss: case BorAss: case BxorAss: case LsftAss: case RsftAss:
    k = t->kind;
    to_left_val();
    genCode1(CPY);
    genCode1(VAL);
    nextToken(t, 0);
    expression(t, NON_T);
    genCode_binary(k);
    break;
  }
  switch (type) {
  case INT_T:
    genCode1(ASSV); break;
  case DOUBLE_T:
    genCode1(ASVD); break;
  case FLOAT_T:
    genCode1(ASVF); break;
  case CHAR_T:
    genCode1(ASVC); break;
  default:
    if (type != 0 && type % 2 == 0)
      genCode1(ASVP);
    // TODO : generate code for each pointer type
    break;
  }
}

void term(Token *t, int n) {
  // TODO : need to study
  if (n == 12) {
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
  Kind op = t->kind;
  TableEntry *te_tmp = NULL;
  int find;

  switch (op) {
  case Add: case Sub: case Not: case Incre: case Decre: case Bnot: case Mul: case Band:
    // like, +1, -1, !0, ~1
    nextToken(t, 0);
    if (op == Mul && t->kind != Ident)
      error("'*' can be used for identifier");

    factor(t);
    // set inc dec preprocessing
    if (op== Incre || op == Decre) {
      to_left_val();
    }
    genCode_unary(op);
    break;
  case Ident:
    // TODO : search registered table item
    te_tmp = search(t->text);
    if (te_tmp == NULL) {
      error("undefined symbol");
    }
    switch (te_tmp->kind) {
    case var_ID: case arg_ID:
      if (te_tmp->arrLen == 0) {
	switch (te_tmp->dType) {
	case INT_T:
	  genCode(LOD, te_tmp->level, te_tmp->code_addr); break;
	case CHAR_T:
	  genCode(LODC, te_tmp->level, te_tmp->code_addr); break;
	default: // TODO : more type needed
	  if (te_tmp->dType % 2 == 0)
	    genCode(LODA, te_tmp->level, te_tmp->code_addr);
	  break;
	}
	nextToken(t, 0);
      } else {
	nextToken(t, 0);
	if (t->kind == '[') {
	  // TODO : currently only [] based addressing
	  genCode(LDA, te_tmp->level, te_tmp->code_addr); // load top start address of array
	  expr_with_check(t, '[', ']');
	  switch (te_tmp->dType) {
	  case INT_T:
	    genCode2(LDI, INT_SIZE); break;
	  case CHAR_T:
	    genCode2(LDI, CHAR_SIZE); break;
	  case DOUBLE_T:
	    genCode2(LDI, DOUBLE_SIZE); break;
	  default:
	    if (te_tmp->dType % 2 == 0)
	      genCode2(LDI, POINTER_SIZE);
	    break;
	  }
	  genCode1(MULL); // index * data size
	  genCode1(ADDL); // add it to tp->adr
	  // TODO : optimize here
	  switch (te_tmp->dType) {
	  case INT_T:
	    genCode1(VAL); break; // pick up the value from address
	  case CHAR_T:
	    genCode1(VALC); break; // pick up the value from address
	  case DOUBLE_T:
	    genCode1(VALD); break;
	  default:
	    if (te_tmp->dType % 2 == 0)
	      genCode2(LDI, POINTER_SIZE);
	    break;
	  }
	} else {
	  error("array requires index"); // TODO : this case is correct
	}
      }
      if (t->kind == Incre || t->kind == Decre) { // for A++
	to_left_val();
	// TODO : need to study below
	if (t->kind == Incre) {
	  genCode1(INCL);
	  genCode2(LDI, 1);
	  genCode1(SUBL);
	} else {
	  genCode1(DECL);
	  genCode2(LDI, 1);
	  genCode1(ADDL);
	}
	nextToken(t, 0);
      }
      break;
    case func_ID: case proto_ID:
      if (te_tmp->dType == VOID_T) {
	return -1; // TODO : void return function should not be in expression
      }
      callFunc(t, te_tmp);
      nextToken(t, 0); // workaround
      break;
    }
    break;
  case IntNum: case CharSymbol:
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
  if (l != 0) {
    if (t->text[0] != l) {
      fprintf(stderr, "expression is not starting with %s", l);
    }
    nextToken(t, 0);
  }
  expression(t, NON_T);
  if (r != 0) {
    if (t->text[0] != r) {
      fprintf(stderr, "expression is not ending with %s", r);
    }
    nextToken(t, 0);
  }
  return 1;
}

int is_const_expr() {
  Token t = {NulKind, Specific, "", 0};
  t_buf_open = 0;
  do {
    nextToken(&t, 1);
    t_buf_enqueue(t);
    // TODO : Can ! and - be allowed?
    if (!(t.kind == IntNum || is_ope1(t.kind) || t.kind == Rbracket)) {
      t_buf_open = 1;
      return -1; // TODO : invalid const expression
    }
  } while (t.kind != ']' && t.kind != ':'); // TODO : this might cause forever loop?
  t_buf_open = 1;
  return 1;
}


void callFunc(Token *t, TableEntry *te) {
  nextToken(t, 0); // point to '('
  nextToken(t, 0); // point to ')' or arguments
  int arg_cnt = 0;
  if (t->kind != ')') {
    while (1) {
      expression(t, NON_T);
      ++arg_cnt;
      if (t->kind != ',')
	break;
      nextToken(t, 0);
    }
  }
  if (t->kind != ')') {
    error("end ')' is missing");
  }

  if (arg_cnt != te->args) {
    error("few or many arguments for function");
  }
  genCode2(CALL, te->code_addr);
  return;
}

int opOder(Kind k) {
  // TODO : can operation kind itself have order integer?
  switch(k) {
  case Mul: case Div: case Mod:                                                return 11;
  case Add: case Sub:                                                          return 10;
  case Lshift: case Rshift:                                                    return 9;
  case Less: case LessEq: case EqLess: case Great: case GreatEq: case EqGreat: return 8;
  case Equal: case NotEq:                                                      return 7;
  case Band:                                                                   return 6;
  case Bxor:                                                                   return 5;
  case Bor:                                                                    return 4;
  case And:                                                                    return 3;
  case Or:                                                                     return 2;
  case Assign:                                                                 return 1;
  default:                                                                     return 0;
  }
}
