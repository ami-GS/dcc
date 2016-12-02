#include "letter_analysis.h"
#include "opcode.h"
#include "symbol_table.h"
#include "instruction.h"
#include "syntactic_analysis.h"
#include "parse.h"
#include "misc.h"
#include "malloc.h"
#include <stdlib.h>
#include <string.h>

int getLowestPriorityIdx(int st, int end) {
  if (st == end)
    return st;

  int lowest_pri = 128, pri = 129, idx = st;
  int i, nest = 0;
  for (i = st; i <= end; i++) {
    if (expr_tkns[i].kind == Lparen || expr_tkns[i].kind == Lbracket || expr_tkns[i].kind == Lbrace) {
      nest++;
    } else if (expr_tkns[i].kind == Rparen || expr_tkns[i].kind == Rbracket || expr_tkns[i].kind == Rbrace) {
      if (nest == 0)
	error("Invalid ')', ']', '}'");
      nest--;
    } else if (nest == 0) {
      if (expr_tkns[i].kind == Semicolon) {
	                            pri = -1;
      } else if (expr_tkns[i].kind == Comma) {
	                            pri = 0;
      } else if (expr_tkns[i].hKind == CombOpe) {
	                            pri = 1;
      } else if (expr_tkns[i].hKind == Operator) {
	switch (expr_tkns[i].kind) {
	case Assign:                pri = 1; break;
	case Or:                    pri = 2; break;
	case And:
	                            pri = 3;
				    if (i == st || expr_tkns[i-1].hKind == Operator)
				      pri = 13;
				    break;
	case Bor:                   pri = 4; break;
	case Bxor:                  pri = 5; break;
	case Band:                  pri = 6; break;
	case Equal: case NotEq:     pri = 7; break;
	case Less: case LessEq:
	case EqLess: case Great:
	case GreatEq: case EqGreat: pri = 8; break;
	case Lshift: case Rshift:   pri = 9; break;
	case Not: case Bnot:        pri = 10; break;
	case Sub: case Add:
                                    pri = 10;
				    if (i == st || expr_tkns[i-1].hKind == Operator)
				      pri = 13;
				    break;
	case Mod: case Div:	    pri = 11; break;
	case Mul:
                                    pri = 11;
				    if (i == st || expr_tkns[i-1].hKind == Operator)
				      pri = 13;
				    break;
	case Incre: case Decre:
	case Dot: case Arrow:
                                    pri = 14;
				    if (i == st || expr_tkns[i-1].hKind == Operator)
				      pri = 13;
				    break;
	default:
	  continue;
	}
      } else if (expr_tkns[i].kind == Ident) {
	pri = 15;
      } else if (expr_tkns[i].hKind == Type || expr_tkns[i].hKind == Modifier) {
	pri = 16;
      } else {
	continue;
      }
      if (!(pri == 1 && lowest_pri == 1) && lowest_pri >= pri) {
	lowest_pri = pri;
	idx = i;
      }
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
    printf("%s ", root->tkn->text);
}

void genCode_tree_assign() {
  if (arrayCount > 0) {
    if (empty_array) { // for int A[] = {1,2...}; initialization
      TableEntry *te_tmp = search(left_val.var->name);
      te_tmp->var->arrLen = arrayCount;
      malloc_more(te_tmp, DATA_SIZE[te_tmp->var->dType] * (arrayCount-1)); // first address is already allocated
      empty_array = 0;
    }
    int i = 0;
    while (i < arrayCount) {
      genCode1(ASSV_TYPE[left_val.var->dType]);
      if (i != arrayCount-1)
	remove_op_stack_top();
      i++;
    }
  } else {
    genCode1(ASSV_TYPE[left_val.var->dType]);
  }
}

void genCode_tree_addressing(int offset) {
  genCode2(LDI, DATA_SIZE[left_val.var->dType]);
  genCode2(LDI, offset);
  genCode_binary(Mul);
  genCode(LDA, left_val.level, left_val.var->code_addr);
  genCode_binary(Add);
}

void define_type(Node *root, Node *self) {
  int i, j;
  for (i = 0; i < typedef_ent_ct; i++) {
    if (memcmp(TypeDefTable[i].tagName, self->tkn->text, self->tkn->intVal) == 0) {
      is_declare = 1;
      VarElement *varr = TypeDefTable[i].var;
      VarElement *varl = left_val.var;
      left_val.structEntCount = TypeDefTable[i].structEntCount;
      for (j = 0; j < left_val.structEntCount; j++) {
	varl->nxtVar = (VarElement *)malloc(sizeof(VarElement));
	varl = varl->nxtVar;
	memcpy(varl, varr, sizeof(VarElement));
	varr = varr->nxtVar;
      }
      return;
    }
  }

  if (self->l->tkn->kind == Struct) { // means tag of struct \
    is_typedef = 0;
    TypeDefTable[typedef_ent_ct].tagName = (char *)malloc(self->tkn->intVal);
    memcpy(TypeDefTable[typedef_ent_ct++].tagName, self->tkn->text, self->tkn->intVal);
    return;
  }
  if (TypeDefTable[typedef_ent_ct].var == NULL)
    TypeDefTable[typedef_ent_ct].var = (VarElement *)malloc(sizeof(VarElement)); // TODO : not cool
  VarElement *varp = TypeDefTable[typedef_ent_ct].var;
  for (i = 0; i < TypeDefTable[typedef_ent_ct].structEntCount; i++) {
    if (varp->nxtVar == NULL)
      varp->nxtVar = (VarElement *)malloc(sizeof(VarElement)); // TODO : free
    varp = varp->nxtVar;
  }
  memcpy(varp, left_val.var, sizeof(VarElement));
  memcpy(varp->name, left_val.var->name, 10); // TODO : temporally
  TypeDefTable[typedef_ent_ct].structEntCount++;
}

void genCode_tree_Ident(Node *root, Node *self) {
  is_bracket_addressing = self->r != NULL;
  if (is_dot_arrow_addressing && root->r == self) {
    int i, match_flag = 0, addr_acc = 0;
    VarElement *var = left_val.var->nxtVar;
    for (i = 0; i < left_val.structEntCount; i++) {
      if (strcmp(var->name, self->tkn->text) == 0) {
	match_flag = 1;
	break;
      }
      addr_acc += DATA_SIZE[var->dType];
      var = var->nxtVar;
    }
    if (match_flag) {
      to_left_val();
      genCode2(LDI, addr_acc);
      genCode2(ADDL, addr_acc);
      left_val.var = var;
      return;
    } else {
      error("no such a member in the struct");
    }
  }
  TableEntry *te_tmp = search(self->tkn->text);
  if (te_tmp == NULL && (left_val.var->dType > NON_T || is_declare)) {
    int arrLen = 0;
    SymbolKind sKind = var_ID;
    // this stands for bracket addressing, remove LDI of stack top
    if (is_bracket_addressing)
      arrLen = codes[--code_ct].opdata;
    if (funcPtr != NULL && funcPtr->args == -1)
      sKind = arg_ID;
    set_entry_member(&left_val, sKind, self->tkn->text, self->tkn->intVal, LOCAL, arrLen);
    if (is_typedef && !left_val.structEntCount) {
      define_type(root, self); // not cool
      return;
    }
    enter_table_item(&left_val);
    if (left_most_assign)
      te_tmp = &left_val;
  } else if (is_typedef) { // means tag of struct
    define_type(root, self); // not cool
    return;
  } else if (te_tmp == NULL) {
    error("unknown identifier");
  } else if (left_val.kind == no_ID) {
    left_val = *te_tmp;
  }

  if (te_tmp != NULL) {
    switch (te_tmp->kind) { // for initialization
    case func_ID: case proto_ID:
      genCode2(CALL, te_tmp->var->code_addr);
      if (te_tmp->var->dType != VOID_T && root == self)
	genCode1(DEL);
      break;
    case var_ID: case arg_ID:
      if (is_bracket_addressing && !is_declare) {
	genCode2(LDI, DATA_SIZE[te_tmp->var->dType]);
	genCode_binary(Mul);
	genCode(LDA, te_tmp->level, te_tmp->var->code_addr);
	if (te_tmp->var->dType%2 == 0)
	  codes[code_ct-1].opcode = LOD;
	genCode_binary(Add);
	genCode1(VAL_TYPE[te_tmp->var->dType]);
      } else if (!is_bracket_addressing) {
	if (te_tmp->var->arrLen == 0 && (te_tmp->var->dType != STRUCT_T &&te_tmp->var->dType%2 == 0)) {
	  genCode(LOD, te_tmp->level, te_tmp->var->code_addr); // for loading pointer
	} else if (te_tmp->var->arrLen == 0) {
	  genCode(LOD_TYPE[te_tmp->var->dType], te_tmp->level, te_tmp->var->code_addr);
	} else {
	  genCode(LDA, te_tmp->level, te_tmp->var->code_addr); // for int A[] = {2,3}; A;
	}
      }
      if (left_most_assign >= 1 && (root->tkn->kind == Assign || root->tkn->hKind == CombOpe))
	to_left_val();
      break;
    }
  }
  if (left_most_assign == 2 && root->tkn->hKind == CombOpe)
    genCode(LOD_TYPE[left_val.var->dType], left_val.level, left_val.var->code_addr);
}

void genCode_tree_IntNum(Node *root, Node *self) {
  if (self->tkn->text[0] == '[') // for int A[] = {1,2,..};
    empty_array = 1;
  genCode2(LDI, self->tkn->intVal);
  if (left_most_assign == 2 && root->tkn->hKind == CombOpe)
    genCode(LOD_TYPE[left_val.var->dType], left_val.level, left_val.var->code_addr);
}

void genCode_tree_CharSymbol(Node *root, Node *self) {
  genCode2(LDI, self->tkn->intVal);
  if (left_most_assign == 2 && root->tkn->hKind == CombOpe)
    genCode(LOD_TYPE[left_val.var->dType], left_val.level, left_val.var->code_addr);
}

void genCode_tree_String(Token *tkn) {
  if (left_val.var->arrLen == 0)
    left_val.var->arrLen = tkn->intVal; // this is for A[]
  if (left_val.var->dType == CHAR_T || empty_array) {
    do {
      genCode_tree_addressing(arrayCount);
      genCode2(LDI, *(tkn->text+arrayCount++)); // TODO : LDI?
    } while (left_val.var->arrLen > arrayCount);
    if (tkn->intVal > left_val.var->arrLen)
      error("initialize length overflowing");
  } else {
    error("string can be assign to char array");
  }
}

void genCode_tree_operator(Node *root, Node *self) {
  if (self->l != NULL && self->l->tkn->hKind == Type)
    return; // TODO : workaround for int *a like.
  if (self->l != NULL && self->r != NULL) {
    genCode_binary(self->tkn->kind);
  } else {
    genCode_unary(self->tkn->kind);
  }
  if (left_most_assign >= 1)
    to_left_val();
  if (self->tkn->hKind == CombOpe) {
    genCode_tree_assign(root, self);
    remove_op_stack_top();
  }
}

void genCode_tree_incdec(Node *root, Node *self) {
  if (codes[code_ct-1].opcode == LOD) {
    if (self->r != NULL) { // for ++A;
      codes[code_ct-1].opcode = LDA;
      genCode_unary(self->tkn->kind);
    } else if (self->l != NULL) { // for A++
      codes[code_ct] = codes[code_ct-1];
      codes[code_ct++].opcode = LDA;
      genCode_unary(self->tkn->kind);
      genCode1(DEL);
    } else {
      error("no variable for inc/decrement");
    }
  } else {
    error("an error"); // TODO : what error here?
  }
  if (root == self)
    genCode1(DEL); // for A++; or ++A;
}

void genCode_tree(Node *self, Node *root) {
  if (root->tkn->kind == Comma && self->tkn->kind != Comma && is_declare && (left_val.var->arrLen > 0 || empty_array)) {
    if (arrayCount >= left_val.var->arrLen && !empty_array)
      error("initialize length overflowing");
    genCode_tree_addressing(arrayCount++);
  }

  if (self->tkn->kind == Dot || self->tkn->kind == Arrow)
    is_dot_arrow_addressing = 1;
  if (left_most_assign == 0 && (self->tkn->kind == Assign || self->tkn->hKind == CombOpe)) {
    left_most_assign = (self->tkn->hKind == CombOpe) + 1; // the most left '='
  }
  if (self->l != NULL)
    genCode_tree(self->l, self);
  if (left_most_assign >= 1 && (self->tkn->kind == Assign || self->tkn->hKind == CombOpe))
    left_most_assign = 0;

  if (self->r != NULL)
    genCode_tree(self->r, self);
  if (self->tkn->kind == Dot || self->tkn->kind == Arrow)
    is_dot_arrow_addressing = 0;

  int i=0;
  if (self->tkn != NULL) {
    switch (self->tkn->kind) {
    case Assign:
      genCode_tree_assign();
      if (root->tkn->kind == Comma || root == self) { // when int A[2] = {1, a=b}; cause error
	arrayCount = 0;
	left_val.var->arrLen = 0;
	remove_op_stack_top();
      }
      break;
    case Incre: case Decre:
      genCode_tree_incdec(root, self);
      break;
    case Ident:
      genCode_tree_Ident(root, self);
      break;
    case IntNum:
      genCode_tree_IntNum(root, self);
      break;
    case CharSymbol:
      genCode_tree_CharSymbol(root, self);
      break;
    case String:
      genCode_tree_String(self->tkn);
      break;
    case Struct:
      is_typedef = 1;
      break;
    case Comma:
      break; // ignore?
    case Dot:
      if (!left_most_assign)
	genCode1(VAL_TYPE[left_val.var->dType]);
      break;
    default:
      switch (self->tkn->hKind) {
      case Operator: case CombOpe:
	genCode_tree_operator(root, self);
	break;
      case Type:
        left_val.var->dType = tkn2dType(self->tkn->kind) + (root->tkn->kind == '*');
	is_declare = 1;
	break;
      }
      break;
    }
  }

  if (root->tkn->kind == Semicolon && self->tkn->kind != Semicolon) {
    if (!is_typedef)
      remove_op_stack_top();
    left_val.var->dType = NON_T;
    is_declare = 0;
    arrayCount = 0;
    left_val.kind = no_ID;
  }
}

void expression(Token *t, char endChar) {
  while (node_used_ct > 0) {
    nodes[--node_used_ct].l = NULL;
    nodes[node_used_ct].r = NULL;
    nodes[node_used_ct].tkn = NULL;
  }
  int i, nest = 0;
  for (i = 0; !(t->kind == endChar && nest == 0); i++) { // TODO ',' should be considered
    expr_tkns[i] = *t;
    if (expr_tkns[i].kind == Lparen || expr_tkns[i].kind == Lbracket || expr_tkns[i].kind == Lbrace) {
      nest++;
    } else if (expr_tkns[i].kind == Rparen || expr_tkns[i].kind == Rbracket || expr_tkns[i].kind == Rbrace) {
      if (nest == 0)
	error("Invalid ')', ']', '}'");
      nest--;
    }
    nextToken(t, 0);
    }
  Node root = nodes[node_used_ct++];
  left_val.kind = no_ID;
  arrayCount = 0;
  left_val.var = (VarElement *)malloc(sizeof(VarElement));
  memset(left_val.var, 0, sizeof(VarElement));
  left_val.structEntCount = 0;
  //left_val.var->dType = NON_T;
  is_declare = 0;
  is_typedef = 0;
  makeTree(&root, 0, i-1);
  dumpRevPolish(&root);
  printf("\n");
  if (root.tkn != NULL)
    genCode_tree(&root, &root);
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
