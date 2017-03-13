#include "dcc.h"
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
#include <math.h>

int getLowestPriorityIdx(int st, int end) {
  if (st == end)
    return st;

  int lowest_pri = 128, pri = 129, idx = st;
  int i, nest = 0;
  for (i = st; i <= end; i++) {
    if (expr_tkns[i].hKind == LParens) {
      nest++;
      if (nest == 1) {
	pri = 14;
	if (!(pri == 1 && lowest_pri == 1) && lowest_pri >= pri) {
	  lowest_pri = pri;
	  idx = i;
	}
      }
    } else if (expr_tkns[i].hKind == RParens) {
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
      } else if (expr_tkns[i].kind == Ident || expr_tkns[i].hKind == Immediate) {
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

  int idx = getLowestPriorityIdx(st, end);
  if (expr_tkns[idx].hKind == LParens) {
    add_Rparens(&expr_tkns[idx]);
    end--;
  }
  root->tkn = &expr_tkns[idx];

  root->l = &nodes[node_used_ct++];
  root->l->depth = root->depth+1;
  root->l->loc = root->loc*2;
  makeTree(root->l, st, idx-1);
  if (root->l->tkn == NULL) {
    root->l = NULL;
    node_used_ct--;
  }

  root->r = &nodes[node_used_ct++];
  root->r->depth = root->depth+1;
  root->r->loc = root->loc*2+1;
  makeTree(root->r, idx+1, end);
  if (root->r->tkn == NULL) {
    root->r = NULL;
    node_used_ct--;
  }
  if (TreeMaxDepth < root->depth)
    TreeMaxDepth = root->depth;
  return;
}

void dumpRevPolishBFS(Node *root) {
  if (root == NULL)
    return;
  int qc = 0, cp = 0, befdepth = -1;
  bfsq[qc++] = root;
  while (qc-cp != 0) {
    Node *nxt = bfsq[cp++];
    double spaceN = pow(2.0, (double)(TreeMaxDepth-nxt->depth));

    if (befdepth != nxt->depth) {
      if (nxt->depth != 0)
	printf("\n");
      befdepth = nxt->depth;
      for (int i = 0; i < nxt->loc; i++) {
	for (int j = 0; j < spaceN ; j++) {
	  printf(" ");
	  if (i != 0)
	    printf(" ");
	}
      }
    }

    if (nxt->loc != 0) {
      spaceN *= 2;
    }
    for (int i = 0; i < spaceN ; i++) {
      printf(" ");
    }

    if (nxt->tkn)
      printf("%s", nxt->tkn->text);

    if (nxt->l != NULL)
      bfsq[qc++] = nxt->l;
    if (nxt->r != NULL)
      bfsq[qc++] = nxt->r;
  }
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
    if (*parse_flag & DEC_EMPTY_ARRAY) {
      te_tmp = search(left_val.var->name);
      te_tmp->var->arrLen = arrayCount;
      malloc_more(te_tmp, get_data_size(te_tmp) * (arrayCount-1)); // first address is already allocated
      *parse_flag &= ~DEC_EMPTY_ARRAY;
    }
    int i = 0;
    while (i < arrayCount) {
      genCode1(ASSV_TYPE[left_val.var->dType]);
      if (i != arrayCount-1)
	remove_op_stack_top();
      i++;
    }
  } else if (var_tmp){
    genCode1(ASSV_TYPE[var_tmp->dType]);
  } else {
    genCode1(ASSV_TYPE[left_val.var->dType]);
  }
}

void genCode_tree_addressing(int offset) {
  genCode2(LDI, get_data_size(&left_val));
  genCode2(LDI, offset);
  genCode_binary(Mul);
  if (*parse_flag & IS_DECLARE)
    genCode(LDA, left_val.level, left_val.var->code_addr);
  genCode_binary(Add);
}

void genCode_tree_dec(Node *root, Node *self) {
  int arrLen = 0;
  SymbolKind sKind = var_ID;
  // this stands for bracket addressing, remove LDI of stack top
  if (funcPtr != NULL && funcPtr->args == -1)
    sKind = arg_ID;
  set_entry_member(&left_val, sKind, self->tkn->text, self->tkn->intVal, scope, arrLen);
  left_val.var->dType += root->tkn->kind == '*';
  left_val.dataSize = get_data_size(&left_val);
  te_tmp = enter_table_item(&left_val);
}

void _genCode_tree_Ident(Node *root, Node *self) {
  switch (te_tmp->kind) { // for initialization
  case func_ID: case proto_ID:
    *parse_flag |= CALL_FUNC;
    te_func = *te_tmp;
    break;
  case var_ID: case arg_ID:
    if (*parse_flag & BRACKET_ACCESS) {
      if (!(*parse_flag & IS_DECLARE)) {
	genCode(LDA, te_tmp->level, te_tmp->var->code_addr);
	if (is_pointer(te_tmp->var->dType))
	  codes[code_ct-1].opcode = LOD;
      }
    } else {
      if (te_tmp->var->arrLen == 0) {
	if (!(*parse_flag & IS_DECLARE) && is_pointer(te_tmp->var->dType)) {
	  genCode(LOD, te_tmp->level, te_tmp->var->code_addr); // for loading pointer
	} else {
	  genCode(LOD_TYPE[te_tmp->var->dType], te_tmp->level, te_tmp->var->code_addr);
	}
      } else {
	genCode(LDA, te_tmp->level, te_tmp->var->code_addr); // for int A[] = {2,3}; A; and int *A = &B;
      }
    }
    if (left_most_assign >= 1 && (root->tkn->kind == Assign || root->tkn->hKind == CombOpe))
      to_left_val();
    break;
  }
}

void genCode_tree_Ident_memb_access(Node *root, Node *self) {
  int addr_acc = 0;
  if (tdef_tmp != NULL)
    var_tmp = &tdef_tmp->var;
  else
    var_tmp = te_tmp->var->nxtVar;
  for (; var_tmp != NULL; var_tmp = var_tmp->nxtVar) {
    tdef_tmp = searchTag(var_tmp->tagName);
    if (strcmp(var_tmp->name, self->tkn->text) == 0) {
      genCode2(LDI, addr_acc);
      return;
    }
    if (var_tmp->dType == STRUCT_T) {
      if (tdef_tmp == NULL)
	error("no such struct for member");
      addr_acc += tdef_tmp->dataSize;
    } else {
      addr_acc += DATA_SIZE[var_tmp->dType];
    }
  }
  error("no such a member in the struct");
}

void genCode_tree_Ident_struct_dec(Node *root, Node *self) {
  TypeDefEntry *tent = searchTag(self->tkn->text);
  if (*parse_flag & SET_MEMBER) { // tent != NULL?
    if (tagName_tmp != NULL && strcmp(tagName_tmp, self->tkn->text) == 0) { // self referencing
      if (root->tkn->kind != '*')
	error("this struct should be reference");
      left_val.var->dType = STRUCTP_T;// TODO : this should be te_tmp
      left_val.dataSize = POINTER_SIZE;
      left_val.var->tagName = tagName_tmp; // bit dangerous
    } else if (tent != NULL) { // struct in struct
      left_val.var->dType = STRUCT_T;
      left_val.var->tagName = (char *)malloc(self->tkn->intVal);
      memcpy(left_val.var->tagName, self->tkn->text, self->tkn->intVal);
      left_val.dataSize = tent->dataSize;
    } else {
      left_val.var->name = malloc(sizeof(char) * (self->tkn->intVal + 1)); // TODO : error check, and must free
      memcpy(left_val.var->name, self->tkn->text, self->tkn->intVal);
      VarElement *varp = &TypeDefTable[typedef_ent_ct].var;
      if (TypeDefTable[typedef_ent_ct].structEntCount > 0) { // == 0 is for first entry
	while (varp->nxtVar != NULL)
	  varp = varp->nxtVar;
	varp->nxtVar = (VarElement *)malloc(sizeof(VarElement)); // TODO : not cool
	varp = varp->nxtVar;
      }
      memcpy(varp, left_val.var, sizeof(VarElement));
      memcpy(varp->name, left_val.var->name, 10); // TODO : temporally
      TypeDefTable[typedef_ent_ct].dataSize += get_data_size(&left_val);
      TypeDefTable[typedef_ent_ct].structEntCount++;
    }
  } else if (tent == NULL && !(*parse_flag & IS_DECLARE)) {
    if (root->tkn->kind == '{') {
      *parse_flag |= SET_MEMBER;
      tagName_tmp = root->tkn->text; // save for self reference
      TypeDefTable[typedef_ent_ct].tagName = (char *)malloc(self->tkn->intVal);
      memcpy(TypeDefTable[typedef_ent_ct].tagName, self->tkn->text, self->tkn->intVal);
    }
  } else if (tent != NULL && self->l->tkn->kind == Struct) { // declare
    left_val.var->dType = STRUCT_T; // this should be te_tmp
    left_val.structEntCount = tent->structEntCount;
    left_val.dataSize = tent->dataSize;
    left_val.var->nxtVar = &tent->var;
    left_val.var->tagName = tent->tagName;
    *parse_flag |= IS_DECLARE;
  } else {
    genCode_tree_dec(root, self);
  }
}

void genCode_tree_Ident(Node *root, Node *self) {
  if (*parse_flag & IS_TYPEDEF) {
    TypeDefTable[typedef_ent_ct].newType = (char *)malloc(self->tkn->intVal);
    memcpy(TypeDefTable[typedef_ent_ct++].newType, self->tkn->text, self->tkn->intVal);
    return;
  }
  if (root->tkn->kind == '[' && root->l == self)
    *parse_flag |= BRACKET_ACCESS;
  else
    *parse_flag &= ~BRACKET_ACCESS;

  if (member_nest && root->r == self && (root->tkn->kind == Arrow || root->tkn->kind == Dot)) { // TODO : temporally conditin. member access, like . ->
    genCode_tree_Ident_memb_access(root, self);
    return;
  }

  if (*parse_flag & (SET_MEMBER | IS_STRUCT)) {
    genCode_tree_Ident_struct_dec(root, self);
    return;
  }

  DataType dtype = searchType(self->tkn->text);
  if (dtype) { // defined type by 'typedef'
    left_val.var->dType = dtype;
    *parse_flag |= IS_DECLARE;
    return;
  }

  te_tmp = search(self->tkn->text);
  if (te_tmp == NULL) {
    if (*parse_flag & IS_DECLARE) {
      genCode_tree_dec(root, self);
      if (((*parse_flag & (IS_DECLARE | BRACKET_ACCESS) == (IS_DECLARE | BRACKET_ACCESS)) && left_most_assign) || !left_most_assign)
	return;
    } else {
      error("unknown identifier");
    }
  } else if (left_val.kind == no_ID) {
    left_val = *te_tmp;
  }

  if (te_tmp != NULL)
    _genCode_tree_Ident(root, self); // actually generate codes
  if (left_most_assign == 2 && root->tkn->hKind == CombOpe)
    genCode(LOD_TYPE[left_val.var->dType], left_val.level, left_val.var->code_addr);
}

void genCode_tree_Immediate(Node *root, Node *self) {
  if (self->tkn->kind == FloatNum) {
    if (left_val.var->dType == DOUBLE_T) {
      genCode2(LDID, 0);
      codes[code_ct-1].opdatad = self->tkn->dVal;
    } else {
      genCode2(LDIF, 0); // TODO : workaround
      codes[code_ct-1].opdataf = self->tkn->dVal;
    }
  } else {
    genCode2(LDI, self->tkn->intVal);
  }

  if (left_most_assign == 2 && root->tkn->hKind == CombOpe)
    genCode(LOD_TYPE[left_val.var->dType], left_val.level, left_val.var->code_addr);
}

void genCode_tree_String(Token *tkn) {
  if (left_val.var->arrLen == 0)
    left_val.var->arrLen = tkn->intVal; // this is for A[]
  if (left_val.var->dType == CHAR_T || (*parse_flag & DEC_EMPTY_ARRAY)) {
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
  if ((self->l != NULL && self->l->tkn->hKind == Type) || (self->l != NULL && self->l->l != NULL && self->l->l->tkn->kind == Struct))
    return; // TODO : workaround for int *a like. and struct TAG *VAL;
  if (self->l != NULL && self->l->tkn->hKind != Statement && self->r != NULL) {
    genCode_binary(self->tkn->kind);
  } else {
    if (!(self->tkn->kind == '&' && te_tmp->var->arrLen)) { // TODO : workaround for b = &a[1];
      if (!(*parse_flag & IS_DECLARE))
	genCode_unary(self->tkn->kind);
    } else if (te_tmp->var->dType != STRUCT_T) // TODO : workaround for above when these are struct
      code_ct--;
  }
  if (left_most_assign >= 1)
    to_left_val();
  if (self->tkn->hKind == CombOpe) {
    genCode_tree_assign(root, self);
    remove_op_stack_top();
  }
}

void genCode_tree_type(Node *self) {
  if (*parse_flag & IS_TYPEDEF) {
    TypeDefTable[typedef_ent_ct].baseType = tkn2dType(self->tkn->kind);
    TypeDefTable[typedef_ent_ct].dataSize = DATA_SIZE[tkn2dType(self->tkn->kind)];
    return;
  }
  left_val.var->dType = tkn2dType(self->tkn->kind);
  *parse_flag |= IS_DECLARE; // TODO : need to consider CAST
  return;
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

void genCode_tree_Lbracket(Node *root, Node *self) {
  if (!(*parse_flag & IS_DECLARE)) {
    if (codes[code_ct-1].opcode == LDI)
      genCode_tree_addressing(codes[--code_ct].opdatai);
    else {
      te_tmp = &val_stack.s[--val_stack.idx];
      genCode2(LDI, get_data_size(te_tmp));
      genCode_binary(Mul);
      genCode_binary(Add);
    }
    if (!left_most_assign && te_tmp->var->dType != STRUCT_T)
      genCode1(VAL_TYPE[te_tmp->var->dType]);
  } else {
    if (self->r != NULL) {
      left_val.var->arrLen = codes[--code_ct].opdatai;
      malloc_more(te_tmp, get_data_size(te_tmp) * (left_val.var->arrLen-1));
      te_tmp->var->arrLen = left_val.var->arrLen;
    } else {
      *parse_flag |= DEC_EMPTY_ARRAY; // for int A[] = {1,2,..};
    }
    *parse_flag |= DEC_ARRAY;
  }

  if (root->tkn->kind == '=' && root->r->tkn->kind == '{') // initialize array
    *parse_flag |= WITH_INIT;
}

void go_left_node(Node *self, Node *root) {
  if (self->tkn->kind == Dot || self->tkn->kind == Arrow)
    member_nest++;
  if (left_most_assign == 0 && (self->tkn->kind == Assign || self->tkn->hKind == CombOpe))
    left_most_assign = (self->tkn->hKind == CombOpe) + 1; // the most left '='
  if (self->l != NULL)
    genCode_tree(self->l, self);
  if (left_most_assign >= 1 && (self->tkn->kind == Assign || self->tkn->hKind == CombOpe))
    left_most_assign = 0;
}

void go_right_node(Node *self, Node *root) {
  if (self->r != NULL)
    genCode_tree(self->r, self);
  if (self->tkn->kind == Dot || self->tkn->kind == Arrow)
    member_nest--;
}

void genCode_tree(Node *self, Node *root) {
  if (root->tkn->kind == Comma && self->tkn->kind != Comma && (*parse_flag & WITH_INIT)) {
    // for loading array initialization value like below
    // int A[4] = {1,2,3,4};
    if (arrayCount >= left_val.var->arrLen && !(*parse_flag & DEC_EMPTY_ARRAY))
      error("initialize length overflowing");
    genCode_tree_addressing(arrayCount++);
  }
  go_left_node(self, root);
  go_right_node(self, root);

  if (self->tkn != NULL) {
    switch (self->tkn->kind) {
    case Assign:
      genCode_tree_assign();
      if (root->tkn->kind == Comma || root == self) { // when int A[2] = {1, a=b}; cause error
	remove_op_stack_top();
      }
      break;
    case Incre: case Decre:
      genCode_tree_incdec(root, self);
      break;
    case Ident:
      genCode_tree_Ident(root, self);
      if (*parse_flag & BRACKET_ACCESS)
	val_stack.s[val_stack.idx++] = *te_tmp; // TODO : need limit check
      break;
    case CharSymbol:
      genCode_tree_Immediate(root, self);
      break;
    case String:
      genCode_tree_String(self->tkn);
      break;
    case Struct:
      *parse_flag |= IS_STRUCT;
      break;
    case Typedef:
      *parse_flag |= IS_TYPEDEF;
      break;
    case Const:
      left_val.var->modifier |= CONST_M;
      break;
    case Static:
      left_val.var->modifier |= STATIC_M;
      break;
    case Comma:
      left_val.var->dType -= root->tkn->kind == '*';
      break;
    case Dot: case Arrow:
      genCode_binary(Add);
      if ((!left_most_assign || member_nest) && var_tmp->dType != STRUCT_T)
	genCode1(VAL);
      break;
    case Lparen:
     if (*parse_flag & CALL_FUNC) {
	genCode2(CALL, te_func.var->code_addr); // TODO : te_func should be included in val_stack
	if (te_func.var->dType != VOID_T && root == self)
	  genCode1(DEL);
	*parse_flag &= ~CALL_FUNC;
      }
      break;
    case Lbrace:
      if (*parse_flag & WITH_INIT)
	*parse_flag &= ~WITH_INIT;
      if (*parse_flag & (IS_STRUCT | SET_MEMBER))
	typedef_ent_ct++;
      break;
    case Lbracket:
      genCode_tree_Lbracket(root, self);
      break;
    case Return:
      *parse_flag |= ST_RETURN;
      break;
    default:
      switch (self->tkn->hKind) {
      case Immediate:
	genCode_tree_Immediate(root, self); break;
      case Operator: case CombOpe:
	genCode_tree_operator(root, self); break;
      case Type:
	genCode_tree_type(self); break;
      }
      break;
    }
  }
}

int init_expr(Token *t, char endChar) {
  while (node_used_ct > 0) {
    nodes[--node_used_ct].l = NULL;
    nodes[node_used_ct].r = NULL;
    nodes[node_used_ct].tkn = NULL;
    nodes[node_used_ct].depth = 0;
    nodes[node_used_ct].loc = 0;
  }
  TreeMaxDepth = 0;

    int len, nest = 0;
  for (len = 0; !(t->kind == endChar && nest == 0); len++) { // TODO ',' should be considered
    expr_tkns[len] = *t;
    if (expr_tkns[len].hKind == LParens) {
      nest++;
    } else if (expr_tkns[len].hKind == RParens) {
      if (nest == 0)
	error("Invalid ')', ']', '}'");
      nest--;
    }
    nextToken(t, 0);
  }
  arrayCount = 0;
  te_tmp = NULL;
  var_tmp = NULL;
  tdef_tmp = NULL;
  memset(&left_val, 0, sizeof(TableEntry));
  left_val.var = (VarElement *)malloc(sizeof(VarElement));
  memset(left_val.var, 0, sizeof(VarElement));
  parse_flags.f[0] = 0;
  parse_flag = &parse_flags.f[0];
  val_stack.idx = 0;
  return len;
}

void genCode_last(Node *root) {
  if (*parse_flag & ST_RETURN) {
    if (root->tkn->kind == Return) {
      if (funcPtr->var->dType != VOID_T)
	error("Warnning : this function must return something");
    } else if (funcPtr->var->dType != VOID_T) {
      if (codes[code_ct-1].opcode == DEL)
	code_ct--;
    } else
      error("Warnning : this function returns void");
    genCode2(JMP, NO_FIX_RET_ADDR);
  }
}

void expression(Token *t, char endChar) {
  int len = init_expr(t, endChar);
  Node *root = &nodes[node_used_ct++];
  makeTree(root, 0, len-1);
  if (DEBUG_FLAG & SHOW_TREE) {
    printf("\n-------------tree-S(");
    dumpRevPolish(root);
    printf(")-------------\n");
    dumpRevPolishBFS(root);
  }
  if (root->tkn != NULL) {
    genCode_tree(root, root);
    genCode_last(root);
  }
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
