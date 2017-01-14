#ifndef _DCC_SYMBOL_TABLE_H_
#define _DCC_SYMBOL_TABLE_H_

#include "opcode.h"

#define INT_SIZE 4
#define FLOAT_SIZE INT_SIZE
#define SHORT_SIZE 2
#define DOUBLE_SIZE 8
#define CHAR_SIZE 1
#define POINTER_SIZE INT_SIZE // TODO 8?. currently, this should be same as INT_SIZE

typedef enum {
    GLOBAL, LOCAL, NO_LOCATION,
} Level; //TODO : the locathion should be devided by each scope, which means each function have each level
static int scopeCnt = 0;

typedef enum {
    NON_T, VOID_T, VOIDP_T, INT_T, INTP_T, SHORT_T, SHORTP_T, CHAR_T, CHARP_T, FLOAT_T, FLOATP_T, DOUBLE_T, DOUBLEP_T, STRUCT_T, STRUCTP_T, // TODO : increase
} DataType;

typedef enum {
    NON_M = 0, STRUCT_M = 1, STATIC_M = 2, CONST_M = 4, UNION_M = 8, EXTERN_M = 16,
} ModifierType;

static const int DATA_SIZE[] = {-1, -1, POINTER_SIZE, INT_SIZE, POINTER_SIZE,
                          SHORT_SIZE, POINTER_SIZE, CHAR_SIZE, POINTER_SIZE,
                          FLOAT_SIZE, POINTER_SIZE, DOUBLE_SIZE, POINTER_SIZE,
                          -1, POINTER_SIZE};
static const int ASSV_TYPE[] = {-1, -1, -1, ASSV, ASVP, ASVS, ASVP, ASVC, ASVP, ASVF, ASVP, ASVD, ASVP, -1, ASVP};
static const int LOD_TYPE[] = {-1, -1, -1, LOD, LDA, LODS, LDA, LODC, LDA, LODF, LDA, LODD, LDA, LDA, -1};
static const int VAL_TYPE[] = {-1, -1, -1, VAL, VAL, VALS, VAL, VALC, VAL, VALF, VAL, VALD, VAL, -1, -1};
static const int STO_TYPE[] = {-1, -1, -1, STO, STO, STOS, STO, STOC, STO, STOF, STO, STOD, STO, -1, -1};

typedef enum {
    no_ID, var_ID, func_ID, proto_ID, arg_ID, // TODO : increase
} SymbolKind;

typedef struct VarElement VarElement;

struct VarElement {
    DataType dType;
    char *name;
    ModifierType modifier;
    int code_addr;
    // for variable
    int       arrLen;
    VarElement *nxtVar;
    char *tagName; //used for type of struct
};

typedef struct {
    SymbolKind  kind;
    VarElement  *var; // will be list when this is struct
    Level      level;  // local, global etc..
    // for function
    int         args;
    // for struct
    int     structEntCount; // means struct
    int      dataSize;
} TableEntry;

typedef struct {
    DataType baseType; // TODO : this should be recursive
    char     *newType;

    int      structEntCount; // if 1 <= then struct
    char     *tagName; // if struct
    VarElement  var;   // if struct
    int      dataSize;
} TypeDefEntry;

#define TABLE_MAX 65536
extern TableEntry SymbolTable[TABLE_MAX];
extern int table_ent_ct;
#define TYPEDEF_MAX 1024
extern TypeDefEntry TypeDefTable[TYPEDEF_MAX];
extern int typedef_ent_ct;
#define GTBL_START 0
static int LTBL_START = 0;
#define LTBL_EMPTY TABLE_MAX+1

TableEntry *get_table_entry(char *name);
TableEntry *enter_table_item(TableEntry* ent);
TableEntry *search(char *text);
TypeDefEntry *searchTag(char *text);
DataType searchType(char *);
void set_entry_member(TableEntry *e, SymbolKind k, char *name, int len, Level l, int arrLen); // TODO temporally limited arguments
void del_func_entry(TableEntry *f1, TableEntry *f2);
void open_local_table();
void close_local_table();
void dupCheck(TableEntry *ent);
int get_data_size(TableEntry *ent);

#endif // _DCC_SYMBOL_TABLE_H_
