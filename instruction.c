#include "instruction.h"
#include "opcode.h"
#include "letter_analysis.h"

int genCode(OpCode op, int flag, int dat) {
  if (const_fold(op)) {
    return code_ct-1;
  }
  Instruction inst = {op, flag, dat};

  if (code_ct >= CODE_SIZ) {
    return -1; // TODO : code size overflow
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
  genCode1(op);

}

void backpatch(int c_ct, int addr) {
  codes[c_ct].opdata = addr;
}

int binary_expr(OpCode op, int d1, int d2) {
  if ((op == DIV || op == MOD) && d2 == 0) {
    return -1; // TODO : zero division
  }
  switch (op) {
  case ADD:
    return d1 + d2;
  case SUB:
    return d1 - d2;
  case MUL:
    return d1 * d2;
  case Div:
    return d1 / d2;
  case Mod:
    return d1 % d2;
  case Equal:
    return d1 == d2;
  case NotEq:
    return d1 != d2;
  case Less:
    return d1 < d2;
  case LessEq: case EqLess:
    return d1 <= d2;
  case Great:
    return d1 > d2;
  case GreatEq: case EqGreat:
    return d1 >= d2;
  case And:
    return d1 && d2;
  case Or:
    return d1 || d2;
  }
}

void to_left_val() {
  // TODO : I need study more here
  switch (codes[code_ct].opcode) {
  case VAL:
    --code_ct;
    break;
  case LOD:
    codes[code_ct].opcode = LDA;
    break;
  default:
    return -1; // TODO : malicious left val

  }
}

int const_fold(OpCode op) {
  if (codes[code_ct-1].opcode == LDI) {
    if (op == NOT) {
      codes[code_ct-1].opdata = !codes[code_ct-1].opdata;
      return 1;
    } else if (op == NEG) {
      codes[code_ct-1].opdata = -codes[code_ct-1].opdata;
      return 1;
    } else if (is_binaryOP(op) && codes[code_ct-2].opcode == LDI) {
      codes[code_ct-2].opdata = binary_expr(op, codes[code_ct-2], codes[code_ct-1]);
      code_ct--;
      return 1;
    }
  }
  return -1;
}

int execute() {
  pc = 0; // proram counter

  int op, dat, addr;
  while (1) {
    if (pc < 0 || code_ct < pc) {
      return -1; // invalid operation
    }
    if (stack_ptr < OP_STACK_BUTTOM) {
      return -1; // TODO : operand stack underflow
    }
    if (stack_ptr > OP_STACK_TOP) {
      return -1; // TODO : operand stack overflow
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
      ASSIGN(addr, op_stack[stack_ptr--]); break;
    case ADBR:  // research frame for calling func
      baseReg += dat;
      //TODO : boundary chech for stack overflow
      //if (baseReg < )
      break;

    case ASS:
      ASSIGN(op_stack[stack_ptr-1], op_stack[stack_ptr]);
      stack_ptr -= 2; break;
    case ASSV:
      ASSIGN(op_stack[stack_ptr-1], op_stack[stack_ptr]);
      op_stack[stack_ptr-1] = op_stack[stack_ptr];
      stack_ptr--; break;
    case VAL: // address to value conversion
      op_stack[stack_ptr] = MEMINT(op_stack[stack_ptr]); break;
    case EQCMP:
      // TODO : suspicious
      if (dat == op_stack[stack_ptr]) {
	op_stack[stack_ptr] = 1; break;
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
    case NEG:
      UNI_OP(-); break;
    case DIV:
      ZERO_CHK();
      BIN_OP(/); break;
    case MOD:
      ZERO_CHK();
      BIN_OP(%); break;
    case ADD:
      BIN_OP(%); break;
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
    }
  }
}
