#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "letter_analysis.h"
#include "parse.h"

TableEntry* get_table_entry(char *name) {
  // TODO : use quick, binary, hash search
  for (int i = 0; i < tblEntryCnt; i++) {
    for (int j = 0; *name != '\0'; j++) {
      if (*(name+j) != *(SymbolTable[i].name+j)) {
	return NULL;
      }
    }
    if (*(SymbolTable[i].name) == '\0') {
      return &SymbolTable[i];
    }
  }
  return NULL;
}

TableEntry *enter_table_item(TableEntry* ent) {
  // TODO : data validattion
  TableEntry* e = get_table_entry(ent->name);
  if (e != NULL && e->dType == ent->dType) {
    return NULL; // TODO : name duplication
  }
  if (tblEntryCnt >= TABLE_MAX) {
    return NULL; // TODO : table overflow error
  }

  if (ent->kind == arg_ID) {
    ent->level; // TODO : adjust args for function
  } else if (ent->kind == var_ID) {
    set_address(ent);
  } else if (ent->kind == func_ID) {
    // TODO : apply func setting
  }

  SymbolTable[tblEntryCnt] = *ent;
  return &SymbolTable[tblEntryCnt++];
}

// TODO : text book uses search and search_name separately. why?
TableEntry *search(char *text) {
  int i;
  for (i = tblEntryCnt-1; i >= LTBL_START; i--) {
    if (strcmp(SymbolTable[i].name, text) == 0) {
      return SymbolTable + i;
    }
  }
  for (; i >= GTBL_START; i--) {
    if (SymbolTable[i].kind != arg_ID && strcmp(SymbolTable[i].name, text) == 0) {
      return SymbolTable + i;
    }
  }
  return NULL;
}


void open_local_table() {
  LTBL_START = tblEntryCnt;
}


void close_local_table(TableEntry *ent) {
  tblEntryCnt = LTBL_START + ent->args; // to remain arguments
  LTBL_START = 0;
}
