#ifndef _DCC_SYMBOL_TABLE_H_
#define _DCC_SYMBOL_TABLE_H_

#define TABLE_MAX 65536

typedef enum {
    NO_LOCATION, GLOBAL, LOCAL, 
} Level; //TODO : the locathion should be devided by each scope, which means each function have each level
static int scopeCnt = 0;

typedef enum {
    NON_T, VOID_T, INT_T, // TODO : increase
} DataType;

typedef enum {
    no_ID, var_ID, func_ID, arg_ID, // TODO : increase
} SymbolKind;

typedef struct {
    SymbolKind  kind;
    char       *name;
    DataType   dType;
    Level      level;  // local, global etc..
    //int        scope; // TODO : after finish implementation, this can change to Level
    int         addr;

    // for variable
    int       arrLen;

    // for function
    int         args;
} TableEntry;

static TableEntry SymbolTable[TABLE_MAX];
static int tblEntryCnt = 0;

TableEntry* get_table_entry(char *name);
int enter_table_item(TableEntry* ent);

#endif // _DCC_SYMBOL_TABLE_H_
