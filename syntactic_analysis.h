#ifndef DCC_SYNTACTIC_ANALYSIS_H_
#define DCC_SYNTACTIC_ANALYSIS_H_

#include "letter_analysis.h"
#include "symbol_table.h"

typedef struct Node Node;

struct Node {
  Node *l, *r;
  Token *tkn;
};

#define MAX_NODES 128
static Node nodes[MAX_NODES];
static int node_used_ct = 0;
#define MAX_EXPR_TOKENS 256
static Token expr_tkns[MAX_EXPR_TOKENS];
static DataType expr_type = NON_T;
static TableEntry left_val = {no_ID, "", NON_T, LOCAL, 0, 0, 0};;
static int addressing = 0;
static int gen_left = 0; // detect only first '='
#define MAX_DEFINE_SIZE
static DataType declare_type = NON_T;


void expression(Token *t, char endChar);
int getLowestPriorityIdx(int st, int end);
void makeTree(Node *root, int st, int end);
void dumpRevPolish(Node *root);
void genCode_tree(Node *root);
void expr_with_check(Token *t, char l, char r);
void callFunc(Token *t, TableEntry *te);
// TODO : this is my own implementation. suspicious
int is_const_expr(); // for array length setting.

#endif // DCC_SYNTACTIC_ANALYSIS_H_
