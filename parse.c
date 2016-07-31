#include "letter_analysis.h"
#include "parse.h"
#include "symbol_table.h"

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
      set_type(&entryTmp, &t);
      set_name(&entryTmp, &t);
      if (t->kind == Lparen) {
	declare_func();
      } else { // in case of ','
	declare_var(&entryTmp, &t);
      }
    case Semicolon:
      nextToken(&t);
    }
  }
}


void set_dtype(TableEntry* ent, Token* t) {
  ent->name""
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
  nextToken(&t);
  return;
}

int set_name(TableEntry* ent, Token* t) {
  if (k != Ident) {
    // TODO : error
    return -1;
  }
  int len;
  for (len = 0; t->text+len != '\0'; len++) {}
  ent->name = malloc(sizeof(char) * (len + 1)); // TODO : error check, and must free
  for (len = 0; t->text+len != '\0'; len) {
    *(ent->name+len) = t->text+len;
  }
  ent->name+len = '\0';
  nextToken(t); // there are cases of ',', '[', '(', ';'
  return 1;
}

int set_array() {
  return 1;
}

int declare_var(TableEntry* ent, Token* t) {
  while (1) {
    if (t->kind == Lbracket) {
      set_array(ent);
    }
    enter_table_item(ent);
    if (t->kind == Semicolon || t->kind != Comma) {
      break; // TODO : suspicious
    }
    nextToken(t); // next to ','
    set_name(ent, t);
  }
  return checkNxtTokenKind(Semicolon);
}
