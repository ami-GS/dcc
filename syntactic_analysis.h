#ifndef DCC_SYNTACTIC_ANALYSIS_H_
#define DCC_SYNTACTIC_ANALYSIS_H_

#include "letter_analysis.h"
#include "symbol_table.h"

typedef struct Node Node;

struct Node {
  Node *l, *r;
  Token *tkn;
};

#define MAX_NODES 64
static Node nodes[MAX_NODES];
static int node_used_ct = 0;
#define MAX_EXPR_TOKENS 256
static Token expr_tkns[MAX_EXPR_TOKENS];
static DataType expr_type = NON_T;
static VarElement left_varelem = {NON_T, "", NON_M, 0, 0};
static TableEntry left_val = {no_ID, &left_varelem, LOCAL, 0};
static int addressing = 0;
static int left_most_assign = 0;
#define MAX_DEFINE_SIZE
static char is_declare = 0;
static char is_typedef = 0;
static int arrayCount = 0;
static int empty_array = 0;
static int is_bracket_addressing = 0;


void expression(Token *t, char endChar);
int getLowestPriorityIdx(int st, int end);
void makeTree(Node *root, int st, int end);
void dumpRevPolish(Node *root);
void genCode_tree_assign();
void genCode_tree_addressing(int offset);
void genCode_tree_operator(Node *root, Node *self);
void genCode_tree_Ident(Node *root, Node *self);
void genCode_tree_IntNum(Node *root, Node *self);
void genCode_tree_CharSymbol(Node *root, Node *self);
void genCode_tree_String(Token *tkn);
void genCode_tree_incdec(Node *root, Node *self);
void genCode_tree(Node *self, Node * root);
void expr_with_check(Token *t, char l, char r);
void callFunc(Token *t, TableEntry *te);
// TODO : this is my own implementation. suspicious
int is_const_expr(); // for array length setting.

#endif // DCC_SYNTACTIC_ANALYSIS_H_
