#include <string.h>
#include "syntactic_analysis.h"

void expression(FILE *f, Token *t) {
  if (t->kind == VarName || checkNxtTokenKind(f, Assign)) {
    int varName = t->text[0];
    nextToken(f, t); // skip '='
    nextToken(f, t);
    expression(f, t);
    variables[varName] = stack[stack_c];
  } else {
    or_exp(f, t);
  }
}

void or_exp(FILE *f, Token *t) {
  and_exp(f, t);
  while (t->kind == Or) {
    nextToken(f, t);
    and_exp(f, t);
    operate(Or);
  }
}

void and_exp(FILE *f, Token *t) {
  equ_exp(f, t);
  while (t->kind == And) {
    nextToken(f, t);
    equ_exp(f, t);
    operate(And);
  }
}

void equ_exp(FILE *f, Token *t) {
  Kind op;
  rel_exp(f, t);
  while (t->kind == Equal || t->kind == NotEq) {
    op = t->kind;
    nextToken(f, t);
    rel_exp(f, t);
    operate(op);
  }
}

void rel_exp(FILE *f, Token *t) {
  Kind op;
  add_sub_exp(f, t);
  while (strstr(" == <= >= =< => ", t->text) != NULL) {
    op = t->kind;
    nextToken(f, t);
    add_sub_exp(f, t);
    operate(op);
  }
}

void add_sub_exp(FILE *f, Token *t) {
  Kind op;
  mul_div_mod_exp(f, t);
  while (t->kind == Add || t->kind == Sub) {
    op = t->kind;
    nextToken(f, t);
    mul_div_mod_exp(f, t);
    operate(op);
  }
}

void mul_div_mod_exp(FILE *f, Token *t) {
  Kind op;
  factor(f, t);
  while (t->kind == Mul || t->kind == Div || t->kind == Mod) {
    op = t->kind;
    nextToken(f, t);
    factor(f, t);
    operate(op);
  }
}

void factor(FILE *f, Token *t) {
  Kind op;

  switch (t->kind) {
  case Add: case Sub: case Not:
    // like, +1, -1, !0
    op = t->kind;
    nextToken(f, t);
    factor(f, t);
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
    nextToken(f, t);
    expression(f, t);
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
  default:
    break

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
