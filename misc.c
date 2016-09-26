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
	case RET: case ASS: case ASSV: case ASSC: case ASVC: case NOT: case INC: case DEC:
	case NEG: case ADD: case SUB: case MUL: case DIV: case MOD: case LESS: case LSEQ:
	case GRT: case GTEQ: case EQU: case NTEQ: case AND: case OR: case VAL: case VALC:
	case VALD: case DEL: case NOP: case STOP: case CPY:
	  printf("%s\n", OpCodeStr[op]);
	  if (op == RET || op == STOP)
	    printf("\n");
	  break;
	case LOD: case LDA: case LDI: case STO: case STOC: case STOD: case CALL: case ADBR:
	case LIB: case JMP: case JPT: case JPF: case EQCMP:
	  printf("%s\t%d", OpCodeStr[op], codes[i].opdata);
	  if (codes[i].flag && (op == LOD || op == LDA || op == STO))
	    printf("[b]"); // TODO : ???
	  printf("\n");
	  break;
	default:
	  return -1; // TODO : invalid operand
	}
    }
}

void error(char *s) {
  fprintf(stderr, "file %s :  line %d\n", currentFile, currentLine);
  fprintf(stderr, "%s\n", s);
  return;
}
