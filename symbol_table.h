#ifndef _DCC_SYMBOL_TABLE_H_
#define _DCC_SYMBOL_TABLE_H_

typedef enum {
    NO_LOCATION, GLOBAL, LOCAL, 
} Level; //TODO : the locathion should be devided by each scope, which means each function have each level
static int scopeCnt = 0;


typedef enum {
    NON_T, VOID_T, INT_T, // TODO : increase
} DataType;

typedef enum {
    no_ID, var_ID, func_ID, // TODO : increase
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

#endif // _DCC_SYMBOL_TABLE_H_
