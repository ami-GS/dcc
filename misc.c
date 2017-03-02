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
	case RET: case ASS: case ASSV: case ASSC: case ASSF: case ASSD: case ASSS: case ASSP: case ASVC:
	case ASVF: case ASVS: case ASVP: case NOT: case INCL: case DECL: case NEG: case ADDL:
	case SUBL: case MULL: case DIVL: case MODL: case LESSL: case LSEQL: case GRTL: case GTEQL:
	case EQUL: case NTEQL: case ANDL: case ORL: case BANDL: case BORL: case BXORL: case BNOT:
	case LSHIFTL: case RSHIFTL: case VAL: case VALS: case VALC: case VALF: case VALD: case DEL:
	case NOP: case STOP: case CPY:
	  printf("%s\n", OpCodeStr[op]);
	  if (op == RET || op == STOP)
	    printf("\n");
	  break;
	case LOD: case LODC: case LODV: case LDA: case LDI: case LDIF: case STO: case LDIS:
	case STOC: case STOD: case STOS: case STOF: case CALL: case ADBR: case LDID: case LDIC:
	case LIB: case JMP: case JPT: case JPF: case EQCMP: case LODF: case LODD: case LODS:
	  printf("%s\t", OpCodeStr[op]); // TODO : separate for each data type
	  if (LDI <= op && op <= LDID) {
	    switch (codes[i].type) {
	    case CHAR_T: printf("%c", codes[i].opdatac); break;
	    case SHORT_T: printf("%d", codes[i].opdatas); break;
	    case INT_T: printf("%d", codes[i].opdatai); break;
	    case FLOAT_T: printf("%f", codes[i].opdataf); break;
	    case DOUBLE_T: printf("%lf", codes[i].opdatad); break;
	    }
	  } else {
	    printf("%d", codes[i].opdatai);
	  }
	  if (codes[i].flag && (op == LOD || op == LODC || op == LODF || op == LODS
				|| op == LODD || op == LDA || op == STO))
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
