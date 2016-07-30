#ifndef DCC_SYNTACTIC_ANALYSIS_H_
#define DCC_SYNTACTIC_ANALYSIS_H_

#include "letter_analysis.h"

#define STACK_SIZ 64
int stack[STACK_SIZ];
int stack_c = 0;
int variables[26]; // TODO : future, hash table?

void expression(Token *t);
void or_exp(Token *t);
void and_exp(Token *t);
void equ_exp(Token *t);
void rel_exp(Token *t);
void add_sub_exp(Token *t);
void mul_div_mod_exp(Token *t);
int factor(Token *t);
void operate(Kind op);
void push(int n);
int pop();


#endif // DCC_SYNTACTIC_ANALYSIS_H_
