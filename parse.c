#include <stdlib.h>
#include <string.h>
#include "letter_analysis.h"
#include "parse_statement.h"
#include "parse.h"
#include "symbol_table.h"
#include "syntactic_analysis.h"
#include "malloc.h"

#define is_main(p) (strcmp(p, "main")==0)

void compile(char *fname) {
  initKind();
  genCode2(CALL, -1); // for main
  genCode1(STOP);
  fOpen(fname);
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

    if (!is_const_expr()) {
      return -1; // TODO : array length must be const
    }
    expr_with_check(t, 0, ']'); // TODO : validate error?
    // expr_with_check folds const expression.
    // the result assigned arrLen and it is not needed on codes
    ent->arrLen = codes[--code_ct].opdata; // TODO : suspicious, my implementation
    if (ent->arrLen <= 0) {
      return -1; // TODO : invalid array length;
    }

    if (!checkNxtTokenKind(Rbracket)) {
      return -1; // TODO : no end bracket?
    }
    nextToken(t); // point at ']' <-
    nextToken(t); // point at ',', ';' or '['
    if (t->kind == Rbracket) {
      return 1; // TODO : currently it doesn't support multi dimention
    }
  }
  return 1;
}

int set_address(TableEntry *te) {
  int size;
  switch (te->kind) {
  case var_ID:
    if (te->arrLen != 0) {
      size = INT_SIZE * te->arrLen; // TODO other type should be capable
    }
    if (te->level == GLOBAL) {
      te->addr = malloc_G(size);
      break;
    }
    te->addr = malloc_L(size); // ENHANCE : malloc_L & G can be unified?
    break;
  case func_ID:
    // TODO : need to study for seting func addr
    break;
  }
}

void set_main(TableEntry *ent) {
    if (ent->dType != INT_T || ent->args != 0) {
      return -1; // TODO : this is temporal, invalid main
    }
    backpatch(0, ent->addr);
    return 1;
}

int declare_var(TableEntry* ent, Token* t) {
  while (1) {
    set_array(ent, t);
    enter_table_item(ent);
    if (t->kind != Comma) {
      break; // TODO : suspicious
    }
    nextToken(t); // next to ','
    set_name(ent, t);
  }
  return checkNxtTokenKind(Semicolon);
}

int set_func_process(TableEntry* ent, Token *t) {
  // TODO : if this is main(), then do special case
  if (is_main(ent->name))
    set_main(ent);
  begin_declare_func(ent);
  SymbolKind last_statement = block(t, 1);
  end_declare_func(ent, last_statement);
  return 1;
}


int declare_func(TableEntry* ent, Token* t) {
  TableEntry* entTmp = get_table_entry(ent->name);
  if (entTmp != NULL && entTmp->kind == func_ID) {
    // TODO : check all arguments for overload
    return -1;
  }
  int* argnum_ptr = &(ent->args);
  nextToken(t); // point at ')' or arguments
  ent->kind = get_func_type();
  enter_table_item(ent);
  funcPtr = ent; // TODO : funcPtr is not needed?
  // open local table
  open_local_table();

  // declare arguments
  TableEntry arg;
  switch (t->kind) {
  case Void:
    nextToken(t); // point to ')'
    break;
  case Rparen:
    break;
  default:
    while (1) {
      set_dtype(&arg, t);
      set_name(&arg, t);
      enter_table_item(&arg); // to avoid multiple declaration in case of using declare_var
      (ent->args)++;
      if (t->kind != Comma) {
	break;
      }
      nextToken(t);
    }
  }
  if (!checkNxtTokenKind(Rparen)) {
    return -1; // TODO : no ')' error (Is this conducted in get_func_type?)
  }

  set_address(ent);
  // TODO : put func chck

  switch(ent->kind) {
  case func_ID:
    set_func_process(ent, t); // enter to statement()
  case proto_ID:
    nextToken(t); // point next to ';'
  }

  // close local table
  close_local_table();
  // TODO : delete duplication
  funcPtr = NULL; // finish function decleration

  return 1;
}

int begin_declare_func(TableEntry *func) {
  genCode2(ADBR, 0); // contents will be filled in end_declare_func()
  genCode(STO, LOCAL, 0);
  int i;
  for (i = func->args; i > 0; i--) {
    genCode(STO, LOCAL, (func+i)->addr);
  }
}

int end_declare_func(TableEntry *func, SymbolKind last) {
  backpatch(func->addr, -1 /* TODO : temporally */);
  if (last != Return) {
    // TODO : here
  }
  genCode(LOD, LOCAL, 0); // load return address to op_stack
  genCode2(ADBR, 0 /* TODO : temporally */); // release local frame
  genCode1(RET); // Return
}

SymbolKind block(Token *t, int is_func) {
  nextToken(t);
  blockNest_ct++;
  if (is_func) {
    // TODO : here is dcc specific declaration method in function block
    TableEntry *tmp;
    while (t->kind == Int) {
      set_dtype(tmp, t);
      set_name(tmp, t);
      declare_var(tmp, t);
    }
  }

  Kind k = Others;
  while (t->kind != '}') {
    k = t->kind; // TODO : here? I think later of statement is better?
    statement(t);
  }

  blockNest_ct--;
  nextToken(t); // point to next to '}'
  return k;
}
