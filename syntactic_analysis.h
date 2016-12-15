#ifndef DCC_SYNTACTIC_ANALYSIS_H_
#define DCC_SYNTACTIC_ANALYSIS_H_

#include "letter_analysis.h"
#include "symbol_table.h"

typedef struct Node Node;

struct Node {
  Node *l, *r;
  Token *tkn;
};

typedef enum {
    IS_LEFT = 0x1, IS_DECLARE = 0x2, IS_STRUCT = 0x4, MEMBER_ACCESS = 0x8, BRACKET_ACCESS = 0x10, IS_TYPEDEF = 0x20, DEC_ARRAY = 0x40, DEC_EMPTY_ARRAY = 0x80,
    SET_MEMBER = 0x100,
} ParseFlag;
static ParseFlag parse_flag = 0;

#define MAX_NODES 64
static Node nodes[MAX_NODES];
static int node_used_ct = 0;
#define MAX_EXPR_TOKENS 256
static Token expr_tkns[MAX_EXPR_TOKENS];
static DataType expr_type = NON_T;
static VarElement left_varelem = {NON_T, "", NON_M, 0, 0};
static TableEntry left_val = {no_ID, &left_varelem, LOCAL, 0};
TableEntry *te_tmp;
VarElement *var_tmp; // for struct member addressing
TypeDefEntry *tdef_tmp;
char *tagName_tmp;
static int left_most_assign = 0;
#define MAX_DEFINE_SIZE
static int arrayCount = 0;
static int member_nest = 0;

void expression(Token *t, char endChar);
int getLowestPriorityIdx(int st, int end);
void makeTree(Node *root, int st, int end);
void dumpRevPolish(Node *root);
void genCode_tree_assign();
void genCode_tree_addressing(int offset);
void genCode_tree_operator(Node *root, Node *self);
void genCode_tree_Ident_memb_access(Node *root, Node *self);
int genCode_tree_Ident_struct_dec(Node *root, Node *self);
void genCode_tree_Ident(Node *root, Node *self);
void _genCode_tree_Ident(Node *root, Node *self);
void genCode_tree_IntNum(Node *root, Node *self);
void genCode_tree_CharSymbol(Node *root, Node *self);
void genCode_tree_String(Token *tkn);
void genCode_tree_incdec(Node *root, Node *self);
void genCode_tree(Node *self, Node * root);
void go_left_node(Node *self, Node *root);
void go_right_node(Node *self, Node *root);
void expr_with_check(Token *t, char l, char r);
void callFunc(Token *t, TableEntry *te);
// TODO : this is my own implementation. suspicious
int is_const_expr(); // for array length setting.

#endif // DCC_SYNTACTIC_ANALYSIS_H_
