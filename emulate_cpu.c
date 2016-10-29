#include "emulate_cpu.h"
#include "instruction.h"
#include "opcode.h"
#include "misc.h"

int stack_ptr = OP_STACK_BUTTOM;

int execute(Instruction *codes, int debug) {
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
	printf("%d ", op_stack[k].sINT);
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
    case LODV:
      PUSH(MEMINT(MEMINT(addr))); break;
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
      ASSIGN_CHAR(op_stack[stack_ptr-2].sINT, op_stack[stack_ptr-1].sCHAR);
      stack_ptr -= 2; break;
    case ASSP:
      ASSIGN(op_stack[stack_ptr-2].sINT, op_stack[stack_ptr-1].sINT);
      stack_ptr -= 2; break;
    case ASSV:
      ASSIGN(op_stack[stack_ptr-2].sINT, op_stack[stack_ptr-1].sINT);
      op_stack[stack_ptr-2].sINT = op_stack[stack_ptr-1].sINT;
      stack_ptr--; break;
    case ASVC:
      ASSIGN_CHAR(op_stack[stack_ptr-2].sINT, op_stack[stack_ptr-1].sCHAR);
      op_stack[stack_ptr-2].sCHAR = op_stack[stack_ptr-1].sCHAR;
      stack_ptr--; break;
    case ASVP:
      ASSIGN_CHAR(op_stack[stack_ptr-2].sINT, op_stack[stack_ptr-1].sINT);
      op_stack[stack_ptr-2].sINT = op_stack[stack_ptr-1].sINT;
      stack_ptr--; break;
    case CPY:
      op_stack[stack_ptr].sINT = op_stack[stack_ptr-1].sINT;
      stack_ptr++; break;
    case VAL: // address to value conversion
      op_stack[stack_ptr-1].sINT = MEMINT(op_stack[stack_ptr-1].sINT); break;
    case VALC:
      op_stack[stack_ptr-1].sCHAR = memory[op_stack[stack_ptr-1].sCHAR]; break;
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
