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
    if (expr_tkns[i].hKind == LParens) {
      nest++;
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
    if (parse_flag & DEC_EMPTY_ARRAY) {
      te_tmp = search(left_val.var->name);
      te_tmp->var->arrLen = arrayCount;
      malloc_more(te_tmp, DATA_SIZE[te_tmp->var->dType] * (arrayCount-1)); // first address is already allocated
      parse_flag &= ~DEC_EMPTY_ARRAY;
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
  genCode2(LDI, DATA_SIZE[left_val.var->dType]);
  genCode2(LDI, offset);
  genCode_binary(Mul);
  genCode(LDA, left_val.level, left_val.var->code_addr);
  genCode_binary(Add);
}

void genCode_tree_dec(Node *root, Node *self) {
  int arrLen = 0;
  SymbolKind sKind = var_ID;
      // this stands for bracket addressing, remove LDI of stack top
  if (parse_flag & DEC_ARRAY)
	arrLen = codes[--code_ct].opdata;
  if (funcPtr != NULL && funcPtr->args == -1)
    sKind = arg_ID;
  set_entry_member(&left_val, sKind, self->tkn->text, self->tkn->intVal, LOCAL, arrLen);
  left_val.var->dType += root->tkn->kind == '*';
  enter_table_item(&left_val);
  left_val.var->dType -= root->tkn->kind == '*';
}

void _genCode_tree_Ident(Node *root, Node *self) {
  switch (te_tmp->kind) { // for initialization
  case func_ID: case proto_ID:
    genCode2(CALL, te_tmp->var->code_addr);
    if (te_tmp->var->dType != VOID_T && root == self)
      genCode1(DEL);
    break;
  case var_ID: case arg_ID:
    if ((parse_flag & BRACKET_ACCESS) && !(parse_flag & IS_DECLARE)) {
      if (te_tmp->var->dType == STRUCT_T)
	genCode2(LDI, te_tmp->dataSize);
      else
	genCode2(LDI, DATA_SIZE[te_tmp->var->dType]);
      genCode_binary(Mul);
      genCode(LDA, te_tmp->level, te_tmp->var->code_addr);
      if (te_tmp->var->dType%2 == 0)
	codes[code_ct-1].opcode = LOD;
      genCode_binary(Add);
      if (te_tmp->var->dType != STRUCT_T)
	genCode1(VAL_TYPE[te_tmp->var->dType]);
    } else if (!(parse_flag & BRACKET_ACCESS)) {
      if (te_tmp->var->arrLen == 0 && te_tmp->var->dType%2 == 0) {
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

void genCode_tree_Ident_memb_access(Node *root, Node *self) {
  int addr_acc = 0;
  if (tdef_tmp != NULL)
    var_tmp = &tdef_tmp->var;
  else
    var_tmp = te_tmp->var->nxtVar;
  for (; var_tmp != NULL; var_tmp = var_tmp->nxtVar) {
    if (strcmp(var_tmp->name, self->tkn->text) == 0) {
      tdef_tmp = searchTag(var_tmp->tagName);
      if (codes[code_ct-1].opcode == LDA && root->tkn->kind == Arrow) // TODO : workaround
	genCode1(VAL);
      if (root->tkn->kind != Arrow && left_most_assign)
	to_left_val();
      genCode2(LDI, addr_acc);
      genCode2(ADDL, addr_acc);
      return;
    }
    if (var_tmp->dType == STRUCT_T) {
      tdef_tmp = searchTag(var_tmp->tagName);
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
  if (parse_flag & SET_MEMBER) {
    if (root != self && tagName_tmp != NULL && strcmp(tagName_tmp, self->tkn->text) == 0) {
      left_val.var->dType = STRUCT_T + (root->tkn->kind == '*'); // TODO : this should be te_tmp
      left_val.dataSize = POINTER_SIZE;
      left_val.var->tagName = tagName_tmp;
      parse_flag |= IS_DECLARE;
    } else if (root == self) { // declare member when define
      TypeDefTable[typedef_ent_ct].tagName = (char *)malloc(self->tkn->intVal);
      memcpy(TypeDefTable[typedef_ent_ct++].tagName, self->tkn->text, self->tkn->intVal);
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
      if (varp->dType == STRUCT_T)
	TypeDefTable[typedef_ent_ct].dataSize += left_val.dataSize;
      else
	TypeDefTable[typedef_ent_ct].dataSize += DATA_SIZE[varp->dType];
      TypeDefTable[typedef_ent_ct].structEntCount++;
    }
  } else if (tent != NULL && self->l->tkn->kind == Struct) { // declare
    left_val.var->dType = STRUCT_T; // this should be te_tmp
    left_val.structEntCount = tent->structEntCount;
    left_val.dataSize = tent->dataSize;
    left_val.var->nxtVar = &tent->var;
    left_val.var->tagName = tent->tagName;
    parse_flag |= IS_DECLARE;
  } else {
    genCode_tree_dec(root, self);
  }
}

void genCode_tree_Ident(Node *root, Node *self) {
  if (self->r != NULL && (self->r->tkn->kind == Ident || self->r->tkn->kind == IntNum || self->r->tkn->hKind == Operator))
    parse_flag |= BRACKET_ACCESS;
  else
    parse_flag &= ~BRACKET_ACCESS;

  if (member_nest && root->r == self) { // TODO : temporally conditin. member access, like . ->
    genCode_tree_Ident_memb_access(root, self);
    return;
  }

  if (parse_flag & SET_MEMBER || parse_flag & IS_STRUCT) {
    genCode_tree_Ident_struct_dec(root, self);
    return;
  }

  te_tmp = search(self->tkn->text);
  if (te_tmp == NULL) {
    if (parse_flag & IS_DECLARE) {
      genCode_tree_dec(root, self);
      if (left_most_assign)
	te_tmp = &left_val;
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

void genCode_tree_IntNum(Node *root, Node *self) {
  if (parse_flag & IS_DECLARE) {
    if (root->tkn->kind == Ident)
      parse_flag |= DEC_ARRAY;
    if (self->tkn->text[0] == '[') // for int A[] = {1,2,..};
     parse_flag |= DEC_EMPTY_ARRAY;
  }
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
  if (left_val.var->dType == CHAR_T || parse_flag & DEC_EMPTY_ARRAY) {
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
  if (self->l != NULL && self->r != NULL) {
    genCode_binary(self->tkn->kind);
  } else {
    if (!(self->tkn->kind == '&' && parse_flag & BRACKET_ACCESS)) // TODO : workaround for b = &a[1];
      genCode_unary(self->tkn->kind);
    else if (te_tmp->var->dType != STRUCT_T) // TODO : workaround for above when these are struct
      code_ct--;
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

void go_left_node(Node *self, Node *root) {
  if (self->tkn->kind == Dot || self->tkn->kind == Arrow)
    member_nest++;
  if (left_most_assign == 0 && (self->tkn->kind == Assign || self->tkn->hKind == CombOpe)) {
    left_most_assign = (self->tkn->hKind == CombOpe) + 1; // the most left '='
  }
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
    if (root->tkn->kind == Comma && self->tkn->kind != Comma && (parse_flag & IS_DECLARE) && (left_val.var->arrLen > 0 || parse_flag & DEC_EMPTY_ARRAY)) {
      if (arrayCount >= left_val.var->arrLen && !(parse_flag & DEC_EMPTY_ARRAY))
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
      parse_flag |= IS_TYPEDEF;
      parse_flag |= IS_STRUCT;
      if (root->r != NULL && root->r->tkn->kind == ';') {
	tagName_tmp = root->tkn->text; // save for self reference
	parse_flag |= SET_MEMBER;
      }
      break;
    case Comma:
      break; // ignore?
    case Dot: case Arrow:
      if (!member_nest && !left_most_assign && var_tmp->dType != STRUCTP_T) // TODO : workaround
	genCode1(VAL_TYPE[var_tmp->dType]);
      break;
    default:
      switch (self->tkn->hKind) {
      case Operator: case CombOpe:
	genCode_tree_operator(root, self);
	break;
      case Type:
        left_val.var->dType = tkn2dType(self->tkn->kind);
	parse_flag |= IS_DECLARE; // TODO : need to consider CAST
	break;
      }
      break;
    }
  }

  if (root->tkn->kind == Semicolon && self->tkn->kind != Semicolon) {
    if (!(parse_flag & IS_TYPEDEF))
      remove_op_stack_top();
    left_val.var->dType = NON_T;
    parse_flag &= ~IS_DECLARE;
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
    if (expr_tkns[i].hKind == LParens) {
      nest++;
    } else if (expr_tkns[i].hKind == RParens) {
      if (nest == 0)
	error("Invalid ')', ']', '}'");
      nest--;
    }
    nextToken(t, 0);
  }
  Node root = nodes[node_used_ct++];
  arrayCount = 0;
  te_tmp = NULL;
  var_tmp = NULL;
  tdef_tmp = NULL;
  memset(&left_val, 0, sizeof(TableEntry));
  left_val.var = (VarElement *)malloc(sizeof(VarElement));
  memset(left_val.var, 0, sizeof(VarElement));
  parse_flag = 0;
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
