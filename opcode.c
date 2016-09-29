#include "opcode.h"

int is_binaryOP(OpCode op) {
  return op == ADDL || op == SUBL || op == MULL || op == DIVL || op == MODL || op == LESSL || op == LSEQL ||
    op == GRTL || op == GTEQL || op == EQUL || op == NTEQL || op == ANDL || op == ORL || op == BANDL || op == BORL ||
    op == BXORL || op == LSHIFTL || op == RSHIFTL ||
    op == ADDSD || op == SUBSD || op == MULSD || op == DIVSD || op == MODSD || op == LESSSD || op == LSEQSD ||
    op == GRTSD || op == GTEQSD || op == EQUSD || op == NTEQSD || op == ANDSD || op == ORSD || op == BANDSD || op == BORSD ||
    op == BXORSD || op == LSHIFTSD || op == RSHIFTSD;
}
