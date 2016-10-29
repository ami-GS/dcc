#ifndef _DCC_SYMBOL_TABLE_H_
#define _DCC_SYMBOL_TABLE_H_

typedef enum {
    GLOBAL, LOCAL, NO_LOCATION,
} Level; //TODO : the locathion should be devided by each scope, which means each function have each level
static int scopeCnt = 0;

typedef enum {
    NON_T, VOID_T, VOIDP_T, INT_T, INTP_T, SHORT_T, SHORTP_T, CHAR_T, CHARP_T, FLOAT_T, FLOATP_T, DOUBLE_T, DOUBLEP_T, // TODO : increase
} DataType;

typedef enum {
    no_ID, var_ID, func_ID, proto_ID, arg_ID, // TODO : increase
} SymbolKind;

typedef struct {
    SymbolKind  kind;
    char       *name;
    DataType   dType;
    Level      level;  // local, global etc..
    //int        scope; // TODO : after finish implementation, this can change to Level
    int         code_addr;

    // for variable
    int       arrLen;

    // for function
    int         args;
} TableEntry;

#define TABLE_MAX 65536
extern TableEntry SymbolTable[TABLE_MAX];
static int table_ent_ct = 0;
#define GTBL_START 0
static int LTBL_START = 0;
#define LTBL_EMPTY TABLE_MAX+1

TableEntry *get_table_entry(char *name);
TableEntry *enter_table_item(TableEntry* ent);
TableEntry *search(char *text);
void del_func_entry(TableEntry *f1, TableEntry *f2);
void open_local_table();
void close_local_table();
void dupCheck(TableEntry *ent);

#endif // _DCC_SYMBOL_TABLE_H_
