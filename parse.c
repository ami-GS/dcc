#include <stdlib.h>
#include <string.h>
#include "letter_analysis.h"
#include "parse_statement.h"
#include "parse.h"
#include "symbol_table.h"
#include "syntactic_analysis.h"
#include "malloc.h"

TableEntry* funcPtr = NULL;
#define is_main(p) (strcmp(p, "main")==0)

void compile(char *fname) {
  initKind();
  genCode2(CALL, -1); // for main
  genCode1(STOP);
  fOpen(fname);
  Token t = {NulKind, "", 0};

  nextToken(&t, 0);
  while (t.kind != EOF_token) {
    TableEntry entryTmp = {no_ID, "", NON_T, NO_LOCATION, 0, 0, 0};
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
      break;
    case Semicolon:
      nextToken(&t, 0);
      break;
    }
  }
  // TODO
  backpatch_calladdr();
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
  nextToken(t, 0);
  return;
}

int set_name(TableEntry* ent, Token* t) {
  if (t->kind != Ident) {
    // TODO57 : error
    return -1;
  }
  int len;
  ent->name = malloc(sizeof(char) * (t->intVal + 1)); // TODO : error check, and must free
  for (len = 0; len < t->intVal ; len++) {
    *(ent->name+len) = *(t->text+len);
  }
  *(ent->name+len) = '\0';
  nextToken(t, 0); // pint at ',', '[', '(', ';'
  return 1;
}

int set_array(TableEntry* ent, Token *t) {
  while (t->kind == Lbracket) {
    nextToken(t, 0);
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

    if (t->kind != Rbracket) {
      return -1; // TODO : no end bracket?
    }
    //nextToken(t, 0); // point at ']' <-
    if (t->kind == Rbracket) {
      nextToken(t, 0); // point at ',', ';' or '['
      return 1; // TODO : currently it doesn't support multi dimention
    }
  }
  return 1;
}

int set_address(TableEntry *te) {
  int i, size = INT_SIZE; // TODO : currently fixed size
  switch (te->kind) {
  case var_ID:
    if (te->arrLen != 0) {
      size *= te->arrLen; // TODO other type should be capable
    }
    if (te->level == GLOBAL) {
      te->code_addr = malloc_G(size);
      break;
    }
    te->code_addr = malloc_L(size); // ENHANCE : malloc_L & G can be unified?
    break;
  case func_ID:
    // TODO : need to study for seting func addr
    te->code_addr = code_ct;
    for (i = 1; i <= te->args; i++) {
      (te+i)->code_addr = malloc_L(size);
    }
    break;
  }
}

void set_main(TableEntry *ent) {
    if (ent->dType != INT_T || ent->args != 0) {
      return -1; // TODO : this is temporal, invalid main
    }
    backpatch(0, ent->code_addr); // set main func code addr
    return 1;
}

int declare_var(TableEntry* ent, Token* t) {
  ent->kind = var_ID;
  while (1) {
    set_array(ent, t);
    enter_table_item(ent);
    if (t->kind != Comma) {
      break; // TODO : suspicious
    }
    nextToken(t, 0); // next to ','
    set_name(ent, t);
  }
  return t->kind == Semicolon;
}

int set_func_process(TableEntry* ent, Token *t) {
  // TODO : if this is main(), then do special case
  if (is_main(ent->name))
    set_main(ent);
  begin_declare_func(ent);
  SymbolKind last_statement = block(t, ent);
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
  t_buf_open = 0;
  ent->kind = get_func_type();
  t_buf_open = 1;
  nextToken(t, 0); // point at ')' or arguments
  funcPtr = enter_table_item(ent); // TODO : funcPtr is not needed?
  // open local table
  open_local_table();

  // declare arguments
  TableEntry arg;
  switch (t->kind) {
  case Void:
    nextToken(t, 0); // point to ')'
    break;
  case Rparen:
    break;
  default:
    while (1) {
      set_dtype(&arg, t);
      set_name(&arg, t);
      enter_table_item(&arg); // to avoid multiple declaration in case of using declare_var
      (funcPtr->args)++;
      if (t->kind != Comma) {
	break;
      }
      nextToken(t, 0);
    }
  }
  if (t->kind != Rparen) {
    return -1; // TODO : no ')' error (Is this conducted in get_func_type?)
  }
  nextToken(t, 0);

  set_address(funcPtr);
  // TODO : put func chck

  switch(funcPtr->kind) {
  case func_ID:
    set_func_process(funcPtr, t); // enter to statement()
  case proto_ID:
    nextToken(t, 0); // point next to ';'
  }

  // close local table
  close_local_table(funcPtr);
  // TODO : delete duplication
  funcPtr = NULL; // finish function decleration

  return 1;
}

int begin_declare_func(TableEntry *func) {
  genCode2(ADBR, 0); // contents will be filled in end_declare_func()
  genCode(STO, LOCAL, 0);
  int i;
  for (i = func->args; i > 0; i--) { // store arguments
    genCode(STO, LOCAL, (func+i)->code_addr);
  }
}

int end_declare_func(TableEntry *func, SymbolKind last) {
  // TODO : study here
  backpatch(func->code_addr, -localAddress);
  if (last != Return) {
    // TODO : here
  }
  backpatch_return(funcPtr->code_addr);
  genCode(LOD, LOCAL, 0); // load return address to op_stack
  // TODO : study here
  genCode2(ADBR, localAddress); // release local frame
  genCode1(RET); // Return
}

SymbolKind block(Token *t, TableEntry *func) {
  nextToken(t, 0);
  blockNest_ct++;
  if (func != NULL) {
    // TODO : here is dcc specific declaration method in function block
    //        declaration is allowed only begining of func
    TableEntry tmp = {no_ID, "", NON_T, NO_LOCATION, 0, 0, 0};
    while (t->kind == Int) {
      set_dtype(&tmp, t);
      set_name(&tmp, t);
      declare_var(&tmp, t);
    }
  }

  Kind k = Others; // store last statement (for in case of return)
  while (t->kind != Rbrace) {
    k = t->kind; // TODO : here? I think later of statement is better?
    statement(t);
  }

  blockNest_ct--;
  if (blockNest_ct == 0) {
    return k; // TODO : this is workaround
  }
  nextToken(t, 0); // point to next to '}'
  return k;
}
