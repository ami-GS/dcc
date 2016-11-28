#include "instruction.h"
#include "opcode.h"
#include "letter_analysis.h"
#include "misc.h"

int code_ct = 0;

int genCode(OpCode op, int flag, int dat) {
  if (const_fold(op)) {
    return code_ct-1;
  }
  Instruction inst = {op, flag, dat};

  if (code_ct >= CODE_SIZ) {
    error("generated code size overflow");
  }
  codes[code_ct++] = inst;
  return code_ct-1;
}

int genCode_unary(Kind k) {
  OpCode op = NOP;
  switch (k) {
  case Add:
    return 1;
  case Sub:
    op = NEG;
    break;
  case Mul:
    codes[code_ct-1].opcode = LODV;
    return 1;
  case Band:
    codes[code_ct-1].opcode = LDA;
    return 1;
  case Not:
    op = NOT;
    break;
  case Incre:
    op = INCL;
    break;
  case Decre:
    op = DECL;
    break;
  case Bnot:
    op = BNOT;
    break;
  }

  genCode1(op);
  return 1;
}

int genCode_binary(Kind k) {
  OpCode op = NOP;
  switch (k) {
  case Add: case AddAss:
    op = ADDL;
    break;
  case Sub: case SubAss:
    op = SUBL;
    break;
  case Mul: case MulAss:
    op = MULL;
    break;
  case Div: case DivAss:
    op = DIVL;
    break;
  case Mod: case ModAss:
    op = MODL;
    break;
  case Equal:
    op = EQUL;
    break;
  case NotEq:
    op = NTEQL;
    break;
  case Less:
    op = LESSL;
    break;
  case LessEq: case EqLess:
    op = LSEQL;
    break;
  case Great:
    op = GRTL;
    break;
  case GreatEq: case EqGreat:
    op = GTEQL;
    break;
  case And:
    op = ANDL;
    break;
  case Or:
    op = ORL;
    break;
  case Band: case BandAss:
    op = BANDL;
    break;
  case Bor: case BorAss:
    op = BORL;
    break;
  case Bxor: case BxorAss:
    op = BXORL;
    break;
  case Lshift: case LsftAss:
    op = LSHIFTL;
    break;
  case Rshift: case RsftAss:
    op = RSHIFTL;
    break;
  }
  genCode1(op);
}

void backpatch(int c_ct, int addr) {
  codes[c_ct].opdata = addr;
}

void backpatch_break(int loop_top) {
  int i;
  for (i = code_ct-1; i >= loop_top; i--) {
    if (codes[i].opcode == JMP && codes[i].opdata == NO_FIX_BREAK_ADDR) {
      code_ct--;
    } else {
      break;
    }
  }
  for (i = code_ct-1; i >= loop_top; i--) {
    if (codes[i].opcode == JMP && codes[i].opdata == NO_FIX_BREAK_ADDR)
      codes[i].opdata = code_ct;
  }
}

void backpatch_return(int return_address) {
  int i;
  for (i = code_ct-1; i >= return_address; i--) {
    if (codes[i].opcode == JMP && codes[i].opdata == NO_FIX_RET_ADDR) {
      code_ct--;
    } else {
      break;
    }
  }
  for (i = code_ct-1; i >= return_address; i--) {
    if (codes[i].opcode == JMP && codes[i].opdata == NO_FIX_RET_ADDR)
      codes[i].opdata = code_ct;
  }
}

void backpatch_calladdr() {
  int i;
  for (i = 0; i < code_ct; i++) {
    if (codes[i].opcode == CALL && codes[i].opdata <= 0) {
      int addr = SymbolTable[-(codes[i].opdata)].var->code_addr;
      if (addr <= 0) {
	codes[i].opdata = SymbolTable[-addr].var->code_addr;
      } else {
	codes[i].opdata = addr; // workaround
      }
      if (codes[i].opdata < 0) {
	error("unknown function"); // is this true?
      }
    }
  }
}

int binary_expr(OpCode op, int d1, int d2) {
  if ((op == DIVL || op == MODL || DIVSD == 0 || MODSD == 0) && d2 == 0) {
    return -1; // TODO : zero division
  }
  // TODO : single '&' '|' '^' should be added. what is appropriate name
  switch (op) {
  case ADDL: case ADDSD:
    return d1 + d2;
  case SUBL: case SUBSD:
    return d1 - d2;
  case MULL: case MULSD:
    return d1 * d2;
  case DIVL: case DIVSD:
    return d1 / d2;
  case MODL: case MODSD:
    return d1 % d2;
  case EQUL: case EQUSD:
    return d1 == d2;
  case NTEQL: case NTEQSD:
    return d1 != d2;
  case LESSL: case LESSSD:
    return d1 < d2;
  case LSEQL: case LSEQSD:
    return d1 <= d2;
  case GRTL: case GRTSD:
    return d1 > d2;
  case GTEQL: case GTEQSD:
    return d1 >= d2;
  case ANDL: case ANDSD:
    return d1 && d2;
  case ORL: case ORSD:
    return d1 || d2;
  case BANDL: case BANDSD:
    return d1 & d2;
  case BORL: case BORSD:
    return d1 | d2;
  case BXORL: case BXORSD:
    return d1 ^ d2;
  case LSHIFTL: case LSHIFTSD:
    return d1 << d2;
  case RSHIFTL: case RSHIFTSD:
    return d1 >> d2;
  }
}

void to_left_val() {
  // TODO : I need study more here
  switch (codes[code_ct-1].opcode) {
  case VAL: case VALC: case VALD:
    --code_ct;
    break;
  case LOD: case LODC:
    codes[code_ct-1].opcode = LDA;
    break;
  case LODV:
    codes[code_ct-1].opcode = LOD; // LOAD Value Reference
  default:
    break;
  }
}

void remove_op_stack_top() {
  switch (codes[code_ct-1].opcode) {
  case ASSV:
    codes[code_ct-1].opcode = ASS; break;
  case ASVC:
    codes[code_ct-1].opcode = ASSC; break;
  case ASVD:
    codes[code_ct-1].opcode = ASSD; break;
  case ASVP:
    codes[code_ct-1].opcode = ASSP; break;
  default:
    genCode1(DEL);
  }
}

int const_fold(OpCode op) {
  if (code_ct >= 1 && codes[code_ct-1].opcode == LDI) {
    if (op == NOT) {
      codes[code_ct-1].opdata = !codes[code_ct-1].opdata;
      return 1;
    } else if (op == NEG) {
      codes[code_ct-1].opdata = -codes[code_ct-1].opdata;
      return 1;
    } else if (op == BNOT) {
      codes[code_ct-1].opdata = ~codes[code_ct-1].opdata;
      return 1;
    } else if (is_binaryOP(op) && codes[code_ct-2].opcode == LDI) {
      codes[code_ct-2].opdata = binary_expr(op, codes[code_ct-2].opdata, codes[code_ct-1].opdata);
      code_ct--;
      return 1;
    }
  }
  return 0;
}

int is_binaryOP(OpCode op) {
  return op == ADDL || op == SUBL || op == MULL || op == DIVL || op == MODL || op == LESSL || op == LSEQL ||
    op == GRTL || op == GTEQL || op == EQUL || op == NTEQL || op == ANDL || op == ORL || op == BANDL || op == BORL ||
    op == BXORL || op == LSHIFTL || op == RSHIFTL ||
    op == ADDSD || op == SUBSD || op == MULSD || op == DIVSD || op == MODSD || op == LESSSD || op == LSEQSD ||
    op == GRTSD || op == GTEQSD || op == EQUSD || op == NTEQSD || op == ANDSD || op == ORSD || op == BANDSD || op == BORSD ||
    op == BXORSD || op == LSHIFTSD || op == RSHIFTSD;
}
