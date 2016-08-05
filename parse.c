#include <stdlib.h>
#include "letter_analysis.h"
#include "parse.h"
#include "symbol_table.h"
#include "syntactic_analysis.c"

void compile(char *fname) {
  fOpen(fname);
  initKind();
  Token t = {NulKind, "", 0};
  TableEntry entryTmp = {no_ID, "", NON_T, NO_LOCATION, 0, 0, 0};

  nextToken(&t);
  while (t.kind != EOF_token) {
    switch(t.kind) {
      // type, #, ;, }, )
    case Int: case Void:
      set_dtype(&entryTmp, &t);
      set_name(&entryTmp, &t);
      if (t.kind == Lparen) {
	declare_func(&entryTmp, &t);
      } else { // in case of ',' or '['
	declare_var(&entryTmp, &t);
      }
    case Semicolon:
      nextToken(&t);
    }
  }
}


void set_dtype(TableEntry* ent, Token* t) {
  switch(t->kind) {
  case Int:
    ent->dType = INT_T;
    break;
  case Void:
    ent->dType = VOID_T;
    break;
  default:
    break;
    // TODO : other types can be placed
  }
  nextToken(t);
  return;
}

int set_name(TableEntry* ent, Token* t) {
  if (t->kind != Ident) {
    // TODO57 : error
    return -1;
  }
  int len;
  for (len = 0; t->text+len != '\0'; len++) {}
  ent->name = malloc(sizeof(char) * (len + 1)); // TODO : error check, and must free
  for (len = 0; t->text+len != '\0'; len++) {
    *(ent->name+len) = *(t->text+len);
  }
  *(ent->name+len) = '\0';
  nextToken(t); // pint at ',', '[', '(', ';'
  return 1;
}

int set_array(TableEntry* ent, Token *t) {
  while (t->kind == Lbracket) {
    nextToken(t);
    if (t->kind == Rbracket) {
      return -1; // TODO : ']' this case can be ok
    }

    expression(t); // TODO : validate error?
    ent->arrLen = pop(); // TODO : this is not cool, expression should return value
    if (!checkNxtTokenKind(Rbracket)) {
      return -1; // TODO : no end bracket?
    }
    nextToken(t); // point at ']' <-
    nextToken(t); // point at ',', ';' or '['
    if (t->kind == Rbracket) {
      return -1; // TODO : currently it doesn't support multi dimention
    }
  }
  return 1;
}

int declare_var(TableEntry* ent, Token* t) {
  while (1) {
    set_array(ent, t);
    enter_table_item(ent);
    if (t->kind == Semicolon || t->kind != Comma) {
      break; // TODO : suspicious
    }
    nextToken(t); // next to ','
    set_name(ent, t);
  }
  return checkNxtTokenKind(Semicolon);
}

int declare_func(TableEntry* ent, Token* t) {
  TableEntry entTmp = get_table_entry(ent->name);
  if (&entTmp != NULL && entTmp.kind == func_ID) {
    // TODO : check all arguments for overload
    return -1;
  }
  int* argnum_ptr = &(ent->args);
  enter_table_item(ent); // TODO : why here?
  nextToken(t); // point at ')' or arguments
  switch(t->kind) {
  case Void:
    nextToken(t);
    break;
  case Rparen:
    break;
  default:
    // TODO : make function of 'set_arguments'
    while (1) {
      set_dtype(ent, t);
      set_name(ent, t);
      enter_table_item(ent); // to avoid multiple declaration in case of using declare_var
      if (t->kind != Comma) {
	break;
      }
      nextToken(t);
    }
  }
  if (!checkNxtTokenKind(Rparen)) {
    return -1; // TODO : no ')' error
  }
  nextToken(t);
  if (t->kind == Semicolon) {
    // TODO : prototype declaration
  }

  // TODO : make function of set_func_process
  switch (t->kind) {
  case Semicolon:
    break;
  case Lbrace:
    // TODO : if this is main(), then do special case

    // TODO : write for the func contents
    break;
  default:
    return -1; // TODO no '{' error
  }
}
