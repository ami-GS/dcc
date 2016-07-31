#ifndef _DCC_PARSE_H_
#define _DCC_PARSE_H_

void compile(char *fname);
void set_dtype(TableEntry* ent, Token* t);
int set_name(TableEntry* ent, Token* t);
int set_array();
int declare_func();
int declare_var(TableEntry* ent, Token* t);


#endif // _DCC_PARSE_H_
