#include "symbol_table.h"

TableEntry get_table_entry(char *name) {
  
}

int enter_table_item(TableEntry* ent) {
  // TODO : data validattion
  TableEntry e = get_table_entry(ent->name);
  if (e != NULL && e->dType == ent->dType) {
    return -1; // TODO : name duplication
  }
  if (tblEnyryCnt >= TABLE_MAX) {
    return -1; // TODO : table overflow error
  }

  if (ent->kind == arg_ID) {
    ent->level; // TODO : adjust args for function
  } else if (ent->kind == var_ID) {
    set_address(ent);
  } else if (ent->kind == func_ID) {
    // TODO : apply func setting
  }

  SymbolTable[++tblEnyryCnt] = *ent;
  
  return 1;
}
