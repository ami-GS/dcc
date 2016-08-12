#include "instructoin.h"

int genCode(OpCode op, int flag, int dat) {
  Instruction inst = {op, flag, dat};
  if (codeCnt >= CODE_SIZ) {
    return -1; // TODO : code size overflow
  }
  codes[codeCnt++] = inst;
  return codeCnt;
}
