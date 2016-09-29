#include "instruction.h"
#include "opcode.h"
#include "letter_analysis.h"

int code_ct = 0;
int stack_ptr = OP_STACK_BUTTOM;

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
  if (codes[0].opdata < 0)
    error("'main' function is missing");
  int i;
  for (i = 0; i < code_ct; i++) {
    if (codes[i].opcode == CALL && codes[i].opdata <= 0) {
      int addr = SymbolTable[-(codes[i].opdata)].code_addr;
      if (addr <= 0) {
	codes[i].opdata = SymbolTable[-addr].code_addr;
      } else {
	codes[i].opdata = addr; // workaround
      }
      if (codes[i].opdata < 0) {
	return -1; // TODO : unknow function //is this true?
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
  case LOD:
    codes[code_ct-1].opcode = LDA;
    break;
  default:
    return -1; // TODO : malicious left val

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

int execute(int debug) {
  if (debug) {
    printf("****instructions****\n");
    printf("program counter\t\topcode\t\topdata\t\tstackvalue\n");
  }
  pc = 0; // proram counter
  baseReg = MEM_MAX-1;
  stack_ptr = 0;

  int op, dat, addr;
  while (1) {
    if (pc < 0 || code_ct < pc) {
      return -1; // invalid operation
    }
    if (stack_ptr < OP_STACK_BUTTOM) {
      error("operand stack underflow");
    }
    if (stack_ptr > OP_STACK_TOP) {
      error("operand stack overflow");
    }
    /*
      preprocessing
    */
    op = codes[pc].opcode;
    dat = codes[pc].opdata;
    if ( (codes[pc].flag & 0x01) ) {
      addr = baseReg + dat; // reative addr
    } else {
      addr = dat; // absolute addr
    }
    if (debug) {
      printf("%d:\t\t\t %s\t\t %d\t\t", pc, OpCodeStr[op], dat);
      int k;
      for (k = stack_ptr-1; k >= 0; k--) {
	printf("%d ", op_stack[k]);
      }
      printf("\n");
    }
    pc++;

    switch (op) {
    case DEL:
      --stack_ptr; break;
    case STOP:
      // TODO : study here
      if (stack_ptr > 0) {
	return POP();
      }
      return 0;
    case JMP:
      pc = dat; break;
    case JPT:
      if (POP())
	pc = dat;
      break;
    case JPF:
      if (!POP())
	pc = dat;
      break;
      /* TODO : study here
    case LIB:
      */
    case LOD:
      PUSH(MEMINT(addr)); break;
    case LODC:
      PUSH(*(memory+addr)); break;
    case LDA:
      PUSH(addr); break;
    case LDI:
      PUSH(dat); break;
    case STO:
      ASSIGN(addr, op_stack[stack_ptr-1].sINT);
      stack_ptr--;
      break;
    case STOC:
      ASSIGN_CHAR(addr, op_stack[stack_ptr-1].sINT);
      stack_ptr--;
      break;
    case ADBR:  // research frame for calling func
      baseReg += dat;
      //TODO : boundary chech for stack overflow
      //if (baseReg < )
      break;

    case ASS:
      ASSIGN(op_stack[stack_ptr-2].sINT, op_stack[stack_ptr-1].sINT);
      stack_ptr -= 2; break;
    case ASSC:
      ASSIGN_CHAR(op_stack[stack_ptr-2].sINT, op_stack[stack_ptr-1].sINT);
      stack_ptr -= 2; break;
    case ASSV:
      ASSIGN(op_stack[stack_ptr-2].sINT, op_stack[stack_ptr-1].sINT);
      op_stack[stack_ptr-2].sINT = op_stack[stack_ptr-1].sINT;
      stack_ptr--; break;
    case ASVC:
      ASSIGN_CHAR(op_stack[stack_ptr-2].sINT, op_stack[stack_ptr-1].sINT);
      op_stack[stack_ptr-2].sINT = op_stack[stack_ptr-1].sINT;
      stack_ptr--; break;
    case CPY:
      op_stack[stack_ptr].sINT = op_stack[stack_ptr-1].sINT;
      stack_ptr++; break;
    case VAL: // address to value conversion
      op_stack[stack_ptr-1].sINT = MEMINT(op_stack[stack_ptr-1].sINT); break;
    case VALC:
      op_stack[stack_ptr-1].sINT = memory[op_stack[stack_ptr-1].sINT]; break;
    case EQCMP:
      // TODO : suspicious
      if (dat == op_stack[stack_ptr-1].sINT) {
	op_stack[stack_ptr-1].sINT = 1; break;
      }
      PUSH(0); break;
    case CALL:
      PUSH(pc); pc = dat; break;
    case RET:
      pc = POP(); break;
    case INCL:
      INCDEC_DWORD(1); break;
    case DECL:
      INCDEC_DWORD(-1); break;
    case NOT:
      UNI_OP_DWORD(!); break;
    case BNOT:
      UNI_OP_DWORD(~); break;
    case NEG:
      UNI_OP_DWORD(-); break;
    case DIVL:
      ZERO_CHK_DWORD();
      BIN_OP_DWORD(/); break;
    case MODL:
      ZERO_CHK_DWORD();
      BIN_OP_DWORD(%); break;
    case ADDL:
      BIN_OP_DWORD(+); break;
    case SUBL:
      BIN_OP_DWORD(-); break;
    case MULL:
      BIN_OP_DWORD(*); break;
    case LESSL:
      BIN_OP_DWORD(<); break;
    case LSEQL:
      BIN_OP_DWORD(<=); break;
    case GRTL:
      BIN_OP_DWORD(>); break;
    case GTEQL:
      BIN_OP_DWORD(>=); break;
    case EQUL:
      BIN_OP_DWORD(==); break;
    case NTEQL:
      BIN_OP_DWORD(!=); break;
    case ANDL:
      BIN_OP_DWORD(&&); break;
    case ORL:
      BIN_OP_DWORD(||); break;
    case BANDL:
      BIN_OP_DWORD(&); break;
    case BORL:
      BIN_OP_DWORD(|); break;
    case BXORL:
      BIN_OP_DWORD(^); break;
    case LSHIFTL:
      BIN_OP_DWORD(<<); break;
    case RSHIFTL:
      BIN_OP_DWORD(>>); break;
    }
  }
}
