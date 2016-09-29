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
    TableEntry entryTmp = {no_ID, "", NON_T, GLOBAL, 0, 0, 0};
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
    case Ident:
      statement(&t);
      break;
    default:
      error("unknown token kind");
    }
  }
  backpatch_calladdr();
  // TODO : memory ?
}


void set_dtype(TableEntry* ent, Token* t) {
  switch(t->kind) {
  case Int:
    ent->dType = INT_T;
    break;
  case Void:
    ent->dType = VOID_T;
    break;
  case Char:
    ent->dType = CHAR_T;
    break; // STRING_T is needed?
  case Float:
    ent->dType = FLOAT_T;
    break;
  case Double:
    ent->dType = DOUBLE_T;
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
    error("Token parse error");
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

void countInitialization(TableEntry *ent) {
  int count = 0;
  Token t = {NulKind, "", 0};
  do {
    nextToken(&t, 1);
    t_buf_enqueue(t);
    if (ent->dType == CHAR_T && t.kind == String) { // for char A = "1234abcs...";
      ent->arrLen = t.intVal;
      return;
    }
    count += (t.kind == Comma);
  } while(t.kind != Rbrace);
  ent->arrLen = count + 1;
}

int set_array(TableEntry* ent, Token *t) {
  while (t->kind == Lbracket) {
    if (!is_const_expr()) {
      error("array length must be const value");
    }
    nextToken(t, 0);
    if (t->kind == Rbracket) { // A[] = ...;
      nextToken(t, 0); // point to '=' or another
      countInitialization(ent);
      return 1; // no array length declaration
    }
    expr_with_check(t, 0, ']'); // TODO : validate error?
    // expr_with_check folds const expression.
    // the result assigned arrLen and it is not needed on codes
    ent->arrLen = codes[--code_ct].opdata; // TODO : suspicious, my implementation
    if (ent->arrLen <= 0) {
      error("invalid array length");
    }

    if (t->kind == Semicolon || t->kind == Comma) {
      return 1; // TODO : no end bracket?
    }
    // TODO : currently it doesn't support multi dimention
    if (t->kind == Lbracket) {
      nextToken(t, 0); // point at ',', ';' or '['
      return 1;
    }
  }
  return 1;
}

int set_address(TableEntry *te) {
  int i, size;
  switch (te->dType) {
  case INT_T: case FLOAT_T:
    size = INT_SIZE; break;
  case CHAR_T:
    size = CHAR_SIZE; break;
  default:
    break; // error
  }
  switch (te->kind) {
  case var_ID:
    if (te->arrLen != 0)
      size *= te->arrLen; // other type should be capable
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

void init_var(TableEntry *ent, Token *t) {
  int i = 0;
  nextToken(t, 0); // point to '{'
  if (ent->dType == CHAR_T) {
    //nextToken(t, 0); // point to "...."
    if (ent->arrLen == 0) {
      ent->arrLen = t->intVal;
    } else if (t->intVal > ent->arrLen) {
      error("string length exceeds limits");
    }
    do {
      genCode(LDA, ent->level, ent->code_addr);
      genCode2(LDI, CHAR_SIZE*i);
      genCode1(ADDL);
      genCode2(LDI, *(t->text+i)); // TODO : LDI?
      genCode1(ASSC);
      i++;
    } while (ent->arrLen > i);
    nextToken(t, 0); // point to ';'
  } else { // TODO : currently only for INT_T
    if (ent->arrLen == 0) {
      genCode(LDA, ent->level, ent->code_addr);
      expression(t, ent->dType);
      remove_op_stack_top();
    } else {
      do {
	nextToken(t, 0); // point to value
	if (t->kind != IntNum) { // TODO : more flexible
	  error("initialize type mismatch");
	}
	genCode(LDA, ent->level, ent->code_addr);
	genCode2(LDI, INT_SIZE*i);
	genCode1(ADDL);
	expression(t, ent->dType); // point to ',' or '}'
	remove_op_stack_top();
	++i;
	if (ent->arrLen != -1 && i > ent->arrLen) {
	  error("the number of init value exceeds that of predefined");
	}
      } while(t->kind == Comma);
      nextToken(t, 0);
    }
  }
}

int declare_var(TableEntry* ent, Token* t) {
  ent->kind = var_ID;
  while (1) {
    set_array(ent, t);
    TableEntry *tmp =  enter_table_item(ent);
    if (t->kind == Assign) {
      init_var(tmp, t);
    }

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
  localAddress = START_LOCAL_ADDRESS;
  TableEntry* entTmp = get_table_entry(ent->name);
  // TODO : prototype can be declared several times with warning
  if (entTmp != NULL && entTmp->kind == func_ID && entTmp->kind == proto_ID) {
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
  TableEntry arg = {arg_ID, "", NON_T, LOCAL, 0, 0, 0};
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
    error("end ')' is missing ");
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
  del_func_entry(entTmp, funcPtr); // delete duplication
  funcPtr = NULL; // finish function decleration

  return 1;
}

int begin_declare_func(TableEntry *func) {
  genCode2(ADBR, 0); // contents will be filled in end_declare_func()
  genCode(STO, LOCAL, 0); // store return address
  int i;
  for (i = func->args; i > 0; i--) { // store arguments
    genCode(STO, LOCAL, (func+i)->code_addr);
  }
}

int end_declare_func(TableEntry *func, SymbolKind last) {
  backpatch(func->code_addr, -localAddress); // researve local frame for function call
  if (last != Return) {
    // TODO : here
  }
  backpatch_return(funcPtr->code_addr);
  genCode(LOD, LOCAL, 0); // load return address to op_stack
  genCode2(ADBR, localAddress); // release local frame
  genCode1(RET); // Return
}

SymbolKind block(Token *t, TableEntry *func) {
  nextToken(t, 0); // point to next to brace '{'
  blockNest_ct++;

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
