#ifndef DCC_SYNTACTIC_ANALYSIS_H_
#define DCC_SYNTACTIC_ANALYSIS_H_

#define STACK_SIZ 64;
int stack[STACK_SIZ];
int stack_c = 0;
int variables[26]; // TODO : future, hash table?

void expression(FILE *f, Token *t);
void or_exp(FILE *f, Token *t);
void and_exp(FILE *f, Token *t);
void equ_exp(FILE *f, Token *t);
void rel_exp(FILE *f, Token *t);
void add_sub_exp(FILE *f, Token *t);
void mul_div_mod_exp(FILE *f, Token *t);
void factor(FILE *f, Token *t);
void operate(Kind op);
void push(int n);
int pop();


#endif // DCC_SYNTACTIC_ANALYSIS_H_
