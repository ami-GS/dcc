#ifndef _DCC_SYMBOL_TABLE_H_
#define _DCC_SYMBOL_TABLE_H_

typedef enum {
    NO_LOCATION, GLOBAL, LOCAL, 
} Level;

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
    int         addr;

    // for variable
    int       arrLen;

    // for function
    int         args;
} TableEntry;

#endif // _DCC_SYMBOL_TABLE_H_
