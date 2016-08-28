#ifndef DCC_SYNTACTIC_ANALYSIS_H_
#define DCC_SYNTACTIC_ANALYSIS_H_

#include "letter_analysis.h"

void expression(Token *t);
void or_exp(Token *t);
void and_exp(Token *t);
void equ_exp(Token *t);
void rel_exp(Token *t);
void add_sub_exp(Token *t);
void mul_div_mod_exp(Token *t);
int factor(Token *t);
void operate(Kind op);
int expr_with_check(Token *t, char l, char r);
void callFunc(Token *t, TableEntry *te);
// TODO : this is my own implementation. suspicious
int is_const_expr(); // for array length setting.

#endif // DCC_SYNTACTIC_ANALYSIS_H_
