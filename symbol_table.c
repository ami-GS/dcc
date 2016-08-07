#include "symbol_table.h"
#include <stdlib.h>

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

int enter_table_item(TableEntry* ent) {
  // TODO : data validattion
  TableEntry* e = get_table_entry(ent->name);
  if (e != NULL && e->dType == ent->dType) {
    return -1; // TODO : name duplication
  }
  if (tblEntryCnt >= TABLE_MAX) {
    return -1; // TODO : table overflow error
  }

  if (ent->kind == arg_ID) {
    ent->level; // TODO : adjust args for function
  } else if (ent->kind == var_ID) {
    set_address(ent);
  } else if (ent->kind == func_ID) {
    // TODO : apply func setting
  }

  SymbolTable[++tblEntryCnt] = *ent;
  
  return 1;
}
