#ifndef _DCC_OPCODE_H_
#define _DCC_OPCODE_H_


// these names can be refered in http://www.ibm.com/support/knowledgecenter/ja/SSLTBW_2.2.0/com.ibm.zos.v2r2.f54em00/ispem115.htm
typedef enum {
    NOP, INC, DEC, NEG, NOT, ADD, SUB, MUL, DIV, MOD, LESS, LSEQ, GRT, GTEQ, EQU, NTEQ, AND, OR, CALL, DEL, JMP, JPT, JPF, EQCMP, LOD, LDA, LDI, STO, STOC, STOD, ADBR, RET, ASS, ASSV, ASSC, ASVC, ASSD, ASVD, VAL, VALC, VALD, LIB, STOP,
} OpCode;

static char *OpCodeStr[] = {
    "NOP", "INC", "DEC", "NEG", "NOT", "ADD", "SUB", "MUL", "DIV", "MOD", "LESS", "LSEQ", "GRT", "GTEQ", "EQU", "NTEQ", "AND", "OR", "CALL", "DEL", "JMP", "JPT", "JPF", "EQCMP", "LOD", "LDA", "LDI", "STO", "STOC", "STOD", "ADBR", "RET", "ASS", "ASSV", "ASSC", "ASVC", "ASSD", "ASVD" "VAL", "VALC", "VALD", "LIB", "STOP",
};

int is_binaryOP(OpCode op);

#endif
