#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "letter_analysis.h"
#include "parse.h"

TableEntry SymbolTable[TABLE_MAX];
int tblEntryCnt = 0;

TableEntry* get_table_entry(char *name) {
  // TODO : use quick, binary, hash search
  int i, j;
  for (i = 0; i < tblEntryCnt; i++) {
    for (j = 0; *(name+j) != '\0'; j++) {
      if (*(name+j) != *(SymbolTable[i].name+j)) {
	break;
      }
    }
    if (*(SymbolTable[i].name+j) == '\0') {
      return &SymbolTable[i];
    }
  }
  return NULL;
}

TableEntry *enter_table_item(TableEntry* ent) {
  // TODO : data validattion
  /*
  TableEntry* e = get_table_entry(ent->name);
  if (e != NULL && e->kind == ent->kind) {
      // TODO : if both are prototype, then warning
      return NULL; // TODO : name duplication
  }
  */

  if (tblEntryCnt >= TABLE_MAX) {
    return NULL; // TODO : table overflow error
  }
  dupCheck(ent);

  if (ent->kind == arg_ID) {
    ent->level; // TODO : adjust args for function
  } else if (ent->kind == var_ID) {
    set_address(ent);
  } else if (ent->kind == func_ID ) {
    // apply func setting
    ent->code_addr = -tblEntryCnt;
    TableEntry* e = get_table_entry(ent->name);
  if (e != NULL && e->kind == proto_ID)
      e->code_addr = -tblEntryCnt;
  } else if (ent->kind == proto_ID) {
    ent->code_addr = -tblEntryCnt; // code.opdata -> SymbolTable[code] ->
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

void del_func_entry(TableEntry *f1, TableEntry *f2) {
  if (f1 == NULL) {
    return;
  }
  // TODO : NULL check
  if (f1->dType != f2->dType || f1->args != f2->args)
    return; // overload
  if (f1->kind == proto_ID && f2->kind == func_ID) {
    int i;
    for (i = 0; i < f2->args; i++) {
      *(f1+i) = *(f2+i); // copy to proto
    }
  }
  tblEntryCnt -= (f2->args);
  return;
}


void open_local_table() {
  LTBL_START = tblEntryCnt;
}


void close_local_table(TableEntry *ent) {
  tblEntryCnt = LTBL_START + ent->args; // to remain arguments
  LTBL_START = 0;
}

void dupCheck(TableEntry *ent) {
  if (ent->kind != arg_ID && ent->kind != var_ID)
    return;

  int level = blockNest_ct;
  TableEntry *p = search(ent->name);
  if (p == NULL) return;
  if (ent->kind == arg_ID) level++;
  if (p->level == level) {
    // TODO : duplicating
    return -1;
  }

}
