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
      if (t.kind == Lparen) {
	declare_func();
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
  nextToken(&t);
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
  for (len = 0; t->text+len != '\0'; len) {
    *(ent->name+len) = t->text+len;
  }
  *(ent->name+len) = '\0';
  nextToken(t); // there are cases of ',', '[', '(', ';'
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
