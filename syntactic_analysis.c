#include "letter_analysis.h"
#include "opcode.h"
#include "symbol_table.h"
#include "instruction.h"
#include "data_declare.h"
#include "syntactic_analysis.h"
#include "parse.h"
#include "misc.h"
#include "malloc.h"
#include <stdlib.h>

int getLowestPriorityIdx(int st, int end) {
  if (st == end)
    return st;

  int lowest_pri = 128, pri = 129, idx = st;
  int i, nest = 0;
  for (i = st; i <= end; i++) {
    if (nest == 0 && (expr_tkns[i].hKind == Operator || expr_tkns[i].kind == Comma || expr_tkns[i].hKind == Type || expr_tkns[i].kind == Ident)) {
      if (expr_tkns[i].hKind == Type) { // done is better than nothing
                                                                                   pri = 15;
      } else if (expr_tkns[i].kind == Ident) {
	                                                                           pri = 14;
      }
      switch (expr_tkns[i].kind) {
      case Comma:                                                                  pri = 0; break;
      case Assign:                                                                 pri = 1; break;
      case Or:                                                                     pri = 2; break;
      case And:
                                                                                   pri = 3;
										   if (i == st || expr_tkns[i-1].hKind == Operator)
										     pri = 13;
										   break;
      case Bor:                                                                    pri = 4; break;
      case Bxor:                                                                   pri = 5; break;
      case Band:                                                                   pri = 6; break;
      case Equal: case NotEq:                                                      pri = 7; break;
      case Less: case LessEq: case EqLess: case Great: case GreatEq: case EqGreat: pri = 8; break;
      case Lshift: case Rshift:                                                    pri = 9; break;
      case Not: case Bnot:
                                                                                   pri = 10; break;
      case Sub: case Add:
	                                                                           pri = 10;
                                                                                   if (i == st || expr_tkns[i-1].hKind == Operator)
										     pri = 13;
										   break;
      case Mod: case Div: case Mul:
                                                                                   pri = 11;
										   if (expr_tkns[i].kind == Mul && (i == st || expr_tkns[i-1].hKind == Operator))
										     pri = 13;
										   break;
      case Incre: case Decre:
                                                                                   pri = 14;
										   if (i == st || expr_tkns[i-1].hKind == Operator)
										     pri = 13;
										   break;
      default:
	break;
      }

      if (!(pri == 1 && lowest_pri == 1) && lowest_pri >= pri) {
	lowest_pri = pri;
	idx = i;
      }
    } else if (expr_tkns[i].kind == Lparen || expr_tkns[i].kind == Lbracket || expr_tkns[i].kind == Lbrace) {
      nest++;
    } else if (expr_tkns[i].kind == Rparen || expr_tkns[i].kind == Rbracket || expr_tkns[i].kind == Rbrace) {
      nest--;
    }
  }

  return idx;
}

void makeTree(Node *root, int st, int end) {
  if (st > end)
    return; // finish
  if (expr_tkns[st].kind == '(' && expr_tkns[end].kind == ')') {
    st++; end--;
  } else if (expr_tkns[st].kind == '[' && expr_tkns[end].kind == ']') {
    if (st + 1 == end) {
      expr_tkns[st].kind = IntNum;
      expr_tkns[st].hKind = Immediate;
      expr_tkns[st].intVal = 0;
      end--;
    } else {
      st++; end--;
    }
  } else if (expr_tkns[st].kind == '{' && expr_tkns[end].kind == '}') {
    st++; end--;
  }

  int idx = getLowestPriorityIdx(st, end);
  root->tkn = &expr_tkns[idx];

  root->l = &nodes[node_used_ct++];
  makeTree(root->l, st, idx-1);
  if (root->l->tkn == NULL) {
      root->l = NULL;
      node_used_ct--;
  }

  root->r = &nodes[node_used_ct++];
  makeTree(root->r, idx+1, end);
  if (root->r->tkn == NULL) {
    root->r = NULL;
    node_used_ct--;
  }
  return;
}

void dumpRevPolish(Node *root) {
  if (root->l != NULL)
    dumpRevPolish(root->l);
  if (root->r != NULL)
    dumpRevPolish(root->r);
  if (root->tkn != NULL)
    printf("%s", root->tkn->text);
}

void genCode_tree(Node *root) {
  if (root->tkn->hKind == Type)
    declare_type += tkn2dType(root->tkn->kind); // ';' can reset this?
  if (declare_type > NON_T && root->tkn->kind == '*') // TODO : consider int* a, b; case
    declare_type++; // indicate pointer

  if (gen_left == 0 && root->tkn->kind == Assign)
    gen_left = 1; // the most left '='
  if (root->l != NULL)
    genCode_tree(root->l);
  if (gen_left == 1 && root->tkn->kind == Assign)
    gen_left = 0;
  if (root->r != NULL)
    genCode_tree(root->r);

  int i=0;
  int arrLen = 0;
  TableEntry *te_tmp;
  if (root->tkn != NULL) {
    switch (root->tkn->kind) {
    case Assign:
      if (arrayCount > 0) {
	if (empty_array) {
	  te_tmp = search(left_val.name);
	  malloc_more(te_tmp, arrayCount-1); // first address is already allocated
	  empty_array = 0;
	}
	while (i < arrayCount) {
	  genCode1(ASSV_TYPE[left_val.dType]);
	  if (i != arrayCount-1)
	    remove_op_stack_top();
	  i++;
	}
      } else {
	genCode1(ASSV_TYPE[left_val.dType]);
      }
      break;
    case Add: case Sub: case Mul: case Div: case Mod: case Band:
      if (root->l != NULL && root->r != NULL) {
	genCode_binary(root->tkn->kind);
	if (codes[code_ct-1].opcode == ADDL && root->tkn->text[0] == '[') // TODO : workaround for [] addressing
	  genCode1(VAL);
      } else {
	genCode_unary(root->tkn->kind);
      }
      if (declare_type > NON_T)
	code_ct--; // for int A[5];, int A[5] = {1,2...}
      if (gen_left == 1)
	to_left_val();
      break;
    case Incre: case Decre:
      if (codes[code_ct-1].opcode == LOD)
	codes[code_ct-1].opcode = LDA;
      genCode_unary(root->tkn->kind);
      break;
    case Ident:
      te_tmp = search(root->tkn->text);
      if (te_tmp != NULL && gen_left && left_val.kind == no_ID) {
	left_val = *te_tmp;
      } else if (te_tmp == NULL && declare_type > NON_T) {
	if (codes[code_ct-1].opcode == LDI)
	  arrLen = codes[--code_ct].opdata; // TODO : suspicious
	set_entry_member(&left_val, var_ID, root->tkn->text, root->tkn->intVal, declare_type, LOCAL, arrLen);
	enter_table_item(&left_val);
	if (gen_left) {
	  te_tmp = &left_val;
	} else {
	  break;
	}
      }
      switch (te_tmp->kind) {
      case func_ID: case proto_ID:
	genCode2(CALL, te_tmp->code_addr);
	break;
      case var_ID: case arg_ID:
	if (te_tmp->arrLen == 0 && !empty_array) {
	  genCode(LOD_TYPE[te_tmp->dType], te_tmp->level, te_tmp->code_addr);
	} else if (declare_type == NON_T) { // array
	  genCode2(LDI, DATA_SIZE[te_tmp->dType]);
	  genCode_binary(Mul);
	  genCode(LDA, te_tmp->level, te_tmp->code_addr);
	  genCode_binary(Add);
	  genCode1(VAL);
	}
	if (gen_left == 1)
	  to_left_val();
	// incre decre ?
	break;
      }
      //}
      break;
    case IntNum:
      if (left_val.kind != no_ID && (left_val.arrLen > 0 || empty_array) && declare_type > 0) {
	if (arrayCount < left_val.arrLen || empty_array) {
	  genCode2(LDI, DATA_SIZE[left_val.dType]);
	  genCode2(LDI, arrayCount++);
	  genCode_binary(Mul);
	  genCode(LDA, left_val.level, left_val.code_addr);
	  genCode_binary(Add);
	} else {
	  error("initialize length overflowing");
	}
      }
      if (root->tkn->text[0] == '[')
	empty_array = 1;
      genCode2(LDI, root->tkn->intVal);
      break;
    case CharSymbol:
      genCode2(LDI, root->tkn->intVal);
      break;
    case String:
      if (left_val.arrLen == 0)
	left_val.arrLen = root->tkn->intVal; // this is for A[]
      do {
	genCode(LDA, left_val.level, left_val.code_addr);
	genCode2(LDI, CHAR_SIZE*i);
	genCode1(ADDL);
	genCode2(LDI, *(root->tkn->text+i)); // TODO : LDI?
	genCode1(ASSC);
	i++;
      } while (left_val.arrLen > i);
      break;
    case Comma:
      break; // ignore?
    }
  }
}

void expression(Token *t, char endChar) {
  int i;
  for (i = 0; t->kind != endChar; i++) { // TODO ',' should be considered
    expr_tkns[i] = *t;
    nextToken(t, 0);
  }
  Node root = nodes[node_used_ct++];
  left_val.kind = no_ID;
  arrayCount = 0;
  declare_type = NON_T;
  makeTree(&root, 0, i-1);
  dumpRevPolish(&root);
  printf("\n");
  genCode_tree(&root);
  if (root.tkn->kind == Assign)
    remove_op_stack_top();
}

void expr_with_check(Token *t, char l, char r) {
  if (l != 0) {
    if (t->text[0] != l) {
      fprintf(stderr, "expression is not starting with '%c'", l);
    }
    nextToken(t, 0);
  }
  expression(t, r);
  if (r != 0) {
    if (t->text[0] != r) {
      fprintf(stderr, "expression is not ending with '%c'", r);
    }
    nextToken(t, 0);
  }
  return;
}

int is_const_expr() {
  Token t = {NulKind, Specific, "", 0};
  t_buf_open = 0;
  do {
    nextToken(&t, 1);
    t_buf_enqueue(t);
    // TODO : Can ! and - be allowed?
    if (!(t.kind == IntNum || is_ope1(t.kind) || t.kind == Rbracket)) {
      t_buf_open = 1;
      return -1; // TODO : invalid const expression
    }
  } while (t.kind != ']' && t.kind != ':'); // TODO : this might cause forever loop?
  t_buf_open = 1;
  return 1;
}
