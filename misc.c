#include "misc.h"
#include "opcode.h"
#include "instruction.h"

void code_dump() {
    int i;
    OpCode op; 
    for (i = 0; i < code_ct; i++) {
        op = codes[i].opcode;
	printf("%d:\t", i);
        switch (op) {
	case RET: case ASS: case ASSV: case NOT: case INC: case DEC: case NEG: case ADD:
	case SUB: case MUL: case DIV: case MOD: case LESS: case LSEQ: case GRT: case GTEQ:
	case EQU: case NTEQ: case AND: case OR: case VAL: case DEL: case NOP: case STOP:
	  printf("%s\n", OpCodeStr[op]);
	  if (op == RET || op == STOP)
	    printf("\n");
	  break;
	case LOD: case LDA: case LDI: case STO: case CALL: case ADBR: case LIB: case JMP:
	case JPT: case JPF: case EQCMP:
	  printf("%s\t%d", OpCodeStr[op], codes[i].opdata);
	  if (codes[i].flag && (op == LOD || op == LDA || op == STO))
	    printf("[b]"); // TODO : ???
	  break;
	default:
	  return -1; // TODO : invalid operand
	}
    }
}
