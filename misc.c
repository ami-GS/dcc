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
	case RET: case ASS: case ASSV: case ASSC: case ASSF: case ASSS: case ASSP: case ASVC:
	case ASVF: case ASVS: case ASVP: case NOT: case INCL: case DECL: case NEG: case ADDL:
	case SUBL: case MULL: case DIVL: case MODL: case LESSL: case LSEQL: case GRTL: case GTEQL:
	case EQUL: case NTEQL: case ANDL: case ORL: case BANDL: case BORL: case BXORL: case BNOT:
	case LSHIFTL: case RSHIFTL: case VAL: case VALS: case VALC: case VALF: case VALD: case DEL:
	case NOP: case STOP: case CPY:
	  printf("%s\n", OpCodeStr[op]);
	  if (op == RET || op == STOP)
	    printf("\n");
	  break;
	case LOD: case LODC: case LODV: case LDA: case LDI: case LDIF: case STO:
	case STOC: case STOD: case STOS: case STOF: case CALL: case ADBR:
	case LIB: case JMP: case JPT: case JPF: case EQCMP:
	  printf("%s\t%d", OpCodeStr[op], codes[i].opdatai); // TODO : separate for each data type
	  if (codes[i].flag && (op == LOD || op == LODC || op == LDA || op == STO))
	    printf("[b]"); // TODO : ???
	  printf("\n");
	  break;
	default:
	  printf("invalid operand OP=%d, DAT=%d\n", op, codes[i].opdatai);
	}
    }
}

void error(char *s) {
  fprintf(stderr, "file %s :  line %d\n", fileNames[streamNest_ct-1], currentLines[streamNest_ct-1]);
  fprintf(stderr, "%s\n", s);
  return;
}

void add_Rparens(Token *t) {
  switch (t->kind) {
  case '{':
    t->text[1] = '}'; break;
  case '[':
    t->text[1] = ']'; break;
  case '(':
    t->text[1] = ')'; break; // TODO : currently () is not used
  }
  t->text[2] = '\0';
  t->intVal = 2;
}
