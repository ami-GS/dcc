#include "instruction.h"

int genCode(OpCode op, int flag, int dat) {
  Instruction inst = {op, flag, dat};
  if (codeCnt >= CODE_SIZ) {
    return -1; // TODO : code size overflow
  }
  codes[codeCnt++] = inst;
  return codeCnt;
}

int genCode_unary(Kind k) {
  OpCode op = NOP;
  switch (k) {
  case Add:
    return;
  case Minus:
    op = NEG;
    break;
  case Not:
    op = NOT;
    break;
  case Incre:
    op = INC;
    break;
  case Decre:
    op = DEC;
    break;
  }

  genCode1(op);
  return 1;
}

int genCode_binary(Kind k) {
  OpCode op = NOP;
  switch (k) {
  case Add:
    op = ADD;
    break;
  case Sub:
    op = SUB;
    break;
  case Mul:
    op = MUL;
    break;
  case Div:
    op = DIV;
    break;
  case Mod:
    op = MOD;
    break;
  case Equal:
    op = EQU;
    break;
  case NotEq:
    op = NTEQ;
    break;
  case Less:
    op = LESS;
    break;
  case LessEq: case EqLess:
    op = LSEQ;
    break;
  case Great:
    op = GRT;
    break;
  case GreatEq: case EqGreat:
    op = GTEQ;
    break;
  case And:
    op = AND;
    break;
  case Or:
    op = OR;
    break;
  }
  gencode1(op);

}
