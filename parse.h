#ifndef _DCC_PARSE_H_
#define _DCC_PARSE_H_

#include "symbol_table.h"
#include "letter_analysis.h"

static int blockNest_ct = 0;
extern TableEntry* funcPtr;

void compile(char *fname);
void set_dtype(TableEntry* ent, Token* t);
int set_name(TableEntry* ent, Token* t);
int set_array();
int set_address(TableEntry *te);
int declare_func();
int declare_var(TableEntry* ent, Token* t);
int begin_declare_func(TableEntry *func);
int end_declare_func(TableEntry *func, SymbolKind last);
SymbolKind block(Token *t, TableEntry* funcPtr);
int set_func_process(TableEntry *ent, Token *t);
int countInitialization();


#endif // _DCC_PARSE_H_
