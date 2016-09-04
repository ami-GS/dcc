#include "opcode.h"

int is_binaryOP(OpCode op) {
  return op == ADD || op == SUB || op == MUL || op == DIV || op == MOD;
}
