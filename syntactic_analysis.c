#include <string.h>
#include "syntactic_analysis.h"

void expression(FILE *f, Token *t) {
  if (t->kind == VarName || checkNxtTokenKind(f, Assign)) {
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
  Kind op:
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

void factor(Token *t) {
  Kind op;

  switch (t->kind) {
  case Add: case Sub: case Not:
    // like, +1, -1, !0
    op = t->kind;
    nextToken(t);
    factor();
    if (op == Sub) push(-pop());
    if (op == Not) push(!pop());
    return;
  case VarName:
    push(variables[t->text[0]]);
    break;
  case IntNum:
    // TODO : also every type. Using TokenStack is the besy way?
    push(t->intVal);
    break;
  case Lparen:
    nextToken(t);
    expression(t);
    if (!checkNxtTokenKind(f, Rparen)) {
      // it must be Rparen
      return -1; // TODO : ')' required
    }
    break;
  default:
    return; // TODO error
    }
  nextToken(t);
}

void operate(Kind op) {
  int v2 = pop(), v1 = pop();
  if (op == Div && v2 == 0) {
    // TODO : zero division
    return;
  }

  switch (op) {
  case Add:
    break;
  case Sub:
    break;
  case Mul:
    break;
  case Div:
    break;
  default:
    break

  }
}

// TODO : consider the type pushed, Token might be good?
void push(int n) {

}
//TODO : same as the above
int pop() {

}
