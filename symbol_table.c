#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "letter_analysis.h"
#include "parse.h"
#include "misc.h"

TableEntry SymbolTable[TABLE_MAX];
int table_ent_ct = 0;
TypeDefEntry TypeDefTable[TYPEDEF_MAX];
int typedef_ent_ct = 0;


TableEntry* get_table_entry(char *name) {
  // TODO : use quick, binary, hash search
  int i, j;
  for (i = 0; i < table_ent_ct; i++) {
    for (j = 0; *(name+j) != '\0'; j++) {
      if (*(name+j) != *(SymbolTable[i].var->name+j)) {
	break;
      }
    }
    if (*(SymbolTable[i].var->name+j) == '\0') {
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

  if (table_ent_ct >= TABLE_MAX) {
    error("symbol table overflow");
  }
  dupCheck(ent);

  if (ent->kind == arg_ID) {
    //ent->level++; // TODO : this is depends on declare_func of parse.c 'arg'
  } else if (ent->kind == var_ID) {
    set_address(ent);
  } else if (ent->kind == func_ID ) {
    // apply func setting
    ent->var->code_addr = -table_ent_ct;
    TableEntry* e = get_table_entry(ent->var->name);
    if (e != NULL && e->kind == proto_ID)
      e->var->code_addr = -table_ent_ct;
  } else if (ent->kind == proto_ID) {
    ent->var->code_addr = -table_ent_ct; // code.opdata -> SymbolTable[code] ->
  }

  SymbolTable[table_ent_ct] = *ent;
  SymbolTable[table_ent_ct].var = (VarElement *)malloc(sizeof(VarElement));
  memcpy(SymbolTable[table_ent_ct].var, ent->var, sizeof(VarElement));

  return &SymbolTable[table_ent_ct++];
}

// TODO : text book uses search and search_name separately. why?
TableEntry *search(char *text) {
  int i;
  for (i = table_ent_ct-1; i >= LTBL_START; i--) {
    if (strcmp(SymbolTable[i].var->name, text) == 0) {
      return SymbolTable + i;
    }
  }
  for (; i >= GTBL_START; i--) {
    if (SymbolTable[i].kind != arg_ID && strcmp(SymbolTable[i].var->name, text) == 0) {
      return SymbolTable + i;
    }
  }
  return NULL;
}

TypeDefEntry *searchTag(char *text) {
  if (text == NULL)
    return NULL;
  int i;
  for (i = 0; i < typedef_ent_ct; i++) {
    if (strcmp(TypeDefTable[i].tagName, text) == 0)
      return TypeDefTable + i;
  }
  return NULL;
}

DataType searchType(char *name) {
  if (name == NULL)
    return NON_T;
  int i;
  for (i = 0; i < typedef_ent_ct; i++) {
    if (TypeDefTable[i].newType != NULL && strcmp(TypeDefTable[i].newType, name) == 0)
      return TypeDefTable[i].baseType;
  }
  return NON_T;
}

void del_func_entry(TableEntry *f1, TableEntry *f2) {
  if (f1 == NULL) {
    return;
  }
  // TODO : NULL check
  if (f1->var->dType != f2->var->dType || f1->args != f2->args)
    return; // overload
  if (f1->kind == proto_ID && f2->kind == func_ID) {
    int i;
    for (i = 0; i < f2->args; i++) {
      *(f1+i) = *(f2+i); // copy to proto
    }
  }
  table_ent_ct -= (f2->args);
  return;
}



void open_local_table() {
  LTBL_START = table_ent_ct;
}


void close_local_table(TableEntry *ent) {
  table_ent_ct = LTBL_START + ent->args; // to remain arguments
  LTBL_START = 0;
}

void dupCheck(TableEntry *ent) {
  if (ent->kind != arg_ID && ent->kind != var_ID)
    return;

  int level = blockNest_ct;
  TableEntry *p = search(ent->var->name);
  if (p == NULL) return;
  if (ent->kind == arg_ID) level++;
  if (p->level == level)
    error("duplicated declaration");
}

void set_entry_member(TableEntry *e, SymbolKind k, char *name, int len, Level l, int arrLen) { // TODO temporally limited arguments
  int i;
  e->kind = k;
  e->var->name = malloc(sizeof(char) * (len + 1)); // TODO : error check, and must free
  memcpy(e->var->name, name, len+1);
  e->level = l;
  e->var->code_addr = 0;
  e->var->arrLen = arrLen;
  e->args = 0;
}

int get_data_size(TableEntry *ent) {
  if (ent->var->dType == STRUCT_T)
    return ent->dataSize;
  else
    return DATA_SIZE[ent->var->dType];
}

int is_pointer(DataType dtype) {
  if (dtype == NON_T) {
    error("NON_T is comming");
    return -1;
  }
  if (dtype%2 == 0)
    return 1;
  return 0;
}
