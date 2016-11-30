#include <stdlib.h>
#include <string.h>
#include "letter_analysis.h"
#include "parse_statement.h"
#include "parse.h"
#include "symbol_table.h"
#include "syntactic_analysis.h"
#include "malloc.h"
#include "misc.h"

TableEntry* funcPtr = NULL;
#define is_main(p) (strcmp(p, "main")==0)

void compile(char *fname) {
  fOpen(fname, "r");
  Token t = {NulKind, Specific, "", 0};
  nextToken(&t, 0);
  while (t.kind != EOF_token) {
    VarElement varelem = {NON_T, "", NON_M, 0, 0};
    TableEntry entryTmp = {no_ID, &varelem, GLOBAL, 0};

    if (t.hKind == Type) {
      set_dtype(&entryTmp, &t);
      set_name(&entryTmp, &t);
      if (checkNxtTokenKind('(')) {
	declare_func(&entryTmp, &t);
      } else { // in case of ',' or '['
	//expression(&t, ';');
	declare_var(&entryTmp, &t);
      }
    } else if (t.hKind == Modifier) {
      statement(&t);
      // TBD
    } else {
      switch(t.kind) {
      case Semicolon:
	nextToken(&t, 0);
	break;
      case Ident:
	statement(&t);
	break;
      case Sharp:
	ignoreLine(&t);
	break;
      default:
	error("unknown token kind");
      }
    }
  }
  backpatch_calladdr();
  // TODO : memory ?
  fClose();
}


void ignoreLine(Token *t) {
  int l = currentLines[streamNest_ct-1];
  while (l == currentLines[streamNest_ct-1]) {
    nextToken(t, 0);
  }
  nextToken(t, 0);
}

DataType tkn2dType(Kind kind) {
  switch(kind) {
  case Int:     return INT_T;
  case IntP:    return INTP_T;
  case Short:   return SHORT_T;
  case ShortP:  return SHORTP_T;
  case Void:    return VOID_T;
  case VoidP:   return VOIDP_T;
  case Char:    return CHAR_T;
  case CharP:   return CHARP_T;
  case Float:   return FLOAT_T;
  case FloatP:  return FLOATP_T;
  case Double:  return DOUBLE_T;
  case DoubleP: return DOUBLEP_T;
  default:      return NON_T;
    // TODO : other types can be placed
  }
}

void set_dtype(TableEntry* ent, Token* t) {
  use_all_as_token = 1;
  int is_pointer = 0;
  Kind type = t->kind;
  nextToken(t, 0);
  if (t->kind == '*') {
    use_all_as_token = 0;
    is_pointer = 1;
    nextToken(t, 0);
  }

  switch(type) {
  case Int:
    ent->var->dType = INT_T + is_pointer;
    break;
  case Void:
    ent->var->dType = VOID_T + is_pointer;
    break;
  case Char:
    ent->var->dType = CHAR_T + is_pointer;
    break; // STRING_T is needed?
  case Float:
    ent->var->dType = FLOAT_T + is_pointer;
    break;
  case Double:
    ent->var->dType = DOUBLE_T + is_pointer;
    break;
  default:
    break;
    // TODO : other types can be placed
  }
  use_all_as_token = 0;
  return;
}

int set_name(TableEntry* ent, Token* t) {
  if (t->kind != Ident && t->kind != '*') {
    error("Token parse error");
  }
  if (t->kind == '*' && ent->var->dType % 2 == 1) {
    ent->var->dType += 1; // +1 can stand for pointer
    nextToken(t, 0);
  }

  int len;
  ent->var->name = malloc(sizeof(char) * (t->intVal + 1)); // TODO : error check, and must free
  for (len = 0; len < t->intVal ; len++) {
    *(ent->var->name+len) = *(t->text+len);
  }
  *(ent->var->name+len) = '\0';
  return 1;
}

void countInitialization(TableEntry *ent) {
  int count = 0;
  Token t = {NulKind, Specific, "", 0};
  do {
    nextToken(&t, 1);
    t_buf_enqueue(t);
    if (ent->var->dType == CHAR_T && t.kind == String) { // for char A = "1234abcs...";
      ent->var->arrLen = t.intVal;
      return;
    }
    count += (t.kind == ',');
  } while(t.kind != '}');
  ent->var->arrLen = count + 1;
}

int set_array(TableEntry* ent, Token *t) {
  nextToken(t, 0);
  while (t->kind == '[') {
    if (!is_const_expr()) {
      error("array length must be const value");
    }
    if (checkNxtTokenKind(']')) { // A[] = ...;
      nextToken(t, 0); nextToken(t, 0); // point to '=' or another
      countInitialization(ent);
      return 1; // no array length declaration
    }
    // TODO : workaround
    expr_with_check(t, '[', ']');
    // expr_with_check folds const expression.
    // the result assigned var->arrLen and it is not needed on codes
    ent->var->arrLen = codes[--code_ct].opdata; // TODO : suspicious, my implementation
    if (ent->var->arrLen <= 0) {
      error("invalid array length");
    }

    if (t->kind == ';' || t->kind == ',') {
      return 1; // TODO : no end bracket?
    }
    // TODO : currently it doesn't support multi dimention
    if (t->kind == '[') {
      nextToken(t, 0); // point at ',', ';' or '['
      return 1;
    }
  }
  return 1;
}

int set_address(TableEntry *te) {
  int i = 0, size;
  VarElement *var = te->var;
  switch (te->kind) {
  case var_ID: case arg_ID:
    size = DATA_SIZE[var->dType];
    do {
      if (te->structEntCount) {
	var = var->nxtVar;
	size = DATA_SIZE[var->dType];
      }
      if (var->arrLen != 0)
	size *= var->arrLen; // other type should be capable
      if (te->level == GLOBAL) {
	var->code_addr = malloc_G(size);
	break;
      }
      var->code_addr = malloc_L(size); // ENHANCE : malloc_L & G can be unified?
      i++;
    } while (i < te->structEntCount);
    if (te->structEntCount) {
      te->var->code_addr = te->var->nxtVar->code_addr;
      te->var->dType = te->var->nxtVar->dType;
    }
    break;
  case func_ID:
    // TODO : need to study for seting func addr
    te->var->code_addr = code_ct;
    for (i = 1; i <= te->args; i++) {
      size = DATA_SIZE[(te+i)->var->dType];
      (te+i)->var->code_addr = malloc_L(size);
    }
    break;
  }
}

void set_main(TableEntry *ent) {
  backpatch(0, ent->var->code_addr); // set main func code addr
}

int declare_var(TableEntry* ent, Token* t) {
  ent->kind = var_ID;
  while (t->kind != ';') {
    if (checkNxtTokenKind('[')) {
      set_array(ent, t);
    } else if (checkNxtTokenKind('=')) {
      expression(t, ';');
    } else if (checkNxtTokenKind(';') || checkNxtTokenKind(',')) {
      nextToken(t, 0);
    }
    enter_table_item(ent);

    if (t->kind == ',') {
      nextToken(t, 0);// next to ','
      if (t->kind != Ident && t->kind != '*') {
	error("after ',' should be Identifier");
	return -1;
      }
      set_name(ent, t);
    }
  }
  return 1;
}

int set_func_process(TableEntry* ent, Token *t) {
  // TODO : if this is main(), then do special case
  if (is_main(ent->var->name))
    set_main(ent);
  begin_declare_func(ent);
  SymbolKind last_statement = block(t, ent);
  end_declare_func(ent, last_statement);
  return 1;
}


int declare_func(TableEntry* ent, Token* t) {
  localAddress = START_LOCAL_ADDRESS;
  TableEntry* entTmp = get_table_entry(ent->var->name);
  // TODO : prototype can be declared several times with warning
  if (entTmp != NULL && entTmp->kind == func_ID && entTmp->kind == proto_ID) {
    // TODO : check all arguments for overload
    return -1;
  }

  int* argnum_ptr = &(ent->args);
  t_buf_open = 0;
  ent->kind = get_func_type();
  t_buf_open = 1;
  nextToken(t, 0); // point at '(' or arguments
  funcPtr = enter_table_item(ent); // TODO : funcPtr is not needed?
  // open local table
  open_local_table();

  int tmp = table_ent_ct;
  funcPtr->args = -1;
  expr_with_check(t, '(', ')');
  funcPtr->args = table_ent_ct - tmp;
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
    genCode(STO_TYPE[(func+i)->var->dType], LOCAL, (func+i)->var->code_addr);
  }
}

int end_declare_func(TableEntry *func, SymbolKind last) {
  backpatch(func->var->code_addr, -localAddress); // researve local frame for function call
  if (last != Return) {
    // TODO : here
  }
  backpatch_return(funcPtr->var->code_addr);
  genCode(LOD, LOCAL, 0); // load return address to op_stack
  genCode2(ADBR, localAddress); // release local frame
  genCode1(RET); // Return
}

SymbolKind block(Token *t, TableEntry *func) {
  nextToken(t, 0); // point to next to brace '{'
  blockNest_ct++;

  Kind k = Others; // store last statement (for in case of return)
  while (t->kind != '}') {
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
