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
    op = INC;
    break;
  case Decre:
    op = DEC;
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
    op = ADD;
    break;
  case Sub: case SubAss:
    op = SUB;
    break;
  case Mul: case MulAss:
    op = MUL;
    break;
  case Div: case DivAss:
    op = DIV;
    break;
  case Mod: case ModAss:
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
  case Band: case BandAss:
    op = BAND;
    break;
  case Bor: case BorAss:
    op = BOR;
    break;
  case Bxor: case BxorAss:
    op = BXOR;
    break;
  case Lshift: case LsftAss:
    op = LSHIFT;
    break;
  case Rshift: case RsftAss:
    op = RSHIFT;
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
  if ((op == DIV || op == MOD) && d2 == 0) {
    return -1; // TODO : zero division
  }
  // TODO : single '&' '|' '^' should be added. what is appropriate name
  switch (op) {
  case ADD:
    return d1 + d2;
  case SUB:
    return d1 - d2;
  case MUL:
    return d1 * d2;
  case DIV:
    return d1 / d2;
  case MOD:
    return d1 % d2;
  case EQU:
    return d1 == d2;
  case NTEQ:
    return d1 != d2;
  case LESS:
    return d1 < d2;
  case LSEQ:
    return d1 <= d2;
  case GRT:
    return d1 > d2;
  case GTEQ:
    return d1 >= d2;
  case AND:
    return d1 && d2;
  case OR:
    return d1 || d2;
  case BAND:
    return d1 & d2;
  case BOR:
    return d1 | d2;
  case BXOR:
    return d1 ^ d2;
  case LSHIFT:
    return d1 << d2;
  case RSHIFT:
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
    case LDA:
      PUSH(addr); break;
    case LDI:
      PUSH(dat); break;
    case STO:
      ASSIGN(addr, op_stack[stack_ptr-1]);
      stack_ptr--;
      break;
    case ADBR:  // research frame for calling func
      baseReg += dat;
      //TODO : boundary chech for stack overflow
      //if (baseReg < )
      break;

    case ASS:
      ASSIGN(op_stack[stack_ptr-2], op_stack[stack_ptr-1]);
      stack_ptr -= 2; break;
    case ASSC:
      ASSIGN_CHAR(op_stack[stack_ptr-2], op_stack[stack_ptr-1]);
      stack_ptr -= 2; break;
    case ASSV:
      ASSIGN(op_stack[stack_ptr-2], op_stack[stack_ptr-1]);
      op_stack[stack_ptr-2] = op_stack[stack_ptr-1];
      stack_ptr--; break;
    case ASVC:
      ASSIGN_CHAR(op_stack[stack_ptr-2], op_stack[stack_ptr-1]);
      op_stack[stack_ptr-2] = op_stack[stack_ptr-1];
      stack_ptr--; break;
    case CPY:
      op_stack[stack_ptr] = op_stack[stack_ptr-1];
      stack_ptr++; break;
    case VAL: // address to value conversion
      op_stack[stack_ptr-1] = MEMINT(op_stack[stack_ptr-1]); break;
    case VALC:
      op_stack[stack_ptr-1] = memory[op_stack[stack_ptr-1]]; break;
    case EQCMP:
      // TODO : suspicious
      if (dat == op_stack[stack_ptr-1]) {
	op_stack[stack_ptr-1] = 1; break;
      }
      PUSH(0); break;
    case CALL:
      PUSH(pc); pc = dat; break;
    case RET:
      pc = POP(); break;
    case INC:
      INCDEC(1); break;
    case DEC:
      INCDEC(-1); break;
    case NOT:
      UNI_OP(!); break;
    case BNOT:
      UNI_OP(~); break;
    case NEG:
      UNI_OP(-); break;
    case DIV:
      ZERO_CHK();
      BIN_OP(/); break;
    case MOD:
      ZERO_CHK();
      BIN_OP(%); break;
    case ADD:
      BIN_OP(+); break;
    case SUB:
      BIN_OP(-); break;
    case MUL:
      BIN_OP(*); break;
    case LESS:
      BIN_OP(<); break;
    case LSEQ:
      BIN_OP(<=); break;
    case GRT:
      BIN_OP(>); break;
    case GTEQ:
      BIN_OP(>=); break;
    case EQU:
      BIN_OP(==); break;
    case NTEQ:
      BIN_OP(!=); break;
    case AND:
      BIN_OP(&&); break;
    case OR:
      BIN_OP(||); break;
    case BAND:
      BIN_OP(&); break;
    case BOR:
      BIN_OP(|); break;
    case BXOR:
      BIN_OP(^); break;
    case LSHIFT:
      BIN_OP(<<); break;
    case RSHIFT:
      BIN_OP(>>); break;
    }
  }
}
