#ifndef DCC_SYNTACTIC_ANALYSIS_H_
#define DCC_SYNTACTIC_ANALYSIS_H_

#include "letter_analysis.h"
#include "symbol_table.h"

void expression(Token *t, DataType type);
void term(Token *t, int n);
int factor(Token *t);
int expr_with_check(Token *t, char l, char r);
void callFunc(Token *t, TableEntry *te);
// TODO : this is my own implementation. suspicious
int is_const_expr(); // for array length setting.
int opOder(Kind k);

#endif // DCC_SYNTACTIC_ANALYSIS_H_
