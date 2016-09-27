#include "opcode.h"

int is_binaryOP(OpCode op) {
  return op == ADD || op == SUB || op == MUL || op == DIV || op == MOD || op == LESS || op == LSEQ ||
    op == GRT || op == GTEQ || op == EQU || op == NTEQ || op == AND || op == OR || op == BAND || op == BOR ||
    op == BXOR || op == LSHIFT || op == RSHIFT;
}
