#ifndef _DCC_OPCODE_H_
#define _DCC_OPCODE_H_


// these names can be refered in http://www.ibm.com/support/knowledgecenter/ja/SSLTBW_2.2.0/com.ibm.zos.v2r2.f54em00/ispem115.htm
typedef enum {
    NOP, INCL, DECL, INCSD, DECSD, NEG, NOT, ADDL, SUBL, MULL, DIVL, MODL, LESSL, LSEQL, GRTL, GTEQL, EQUL, NTEQL, ANDL, ORL, BANDL, BORL, BXORL, BNOT, LSHIFTL, RSHIFTL, ADDSD, SUBSD, MULSD, DIVSD, MODSD, LESSSD, LSEQSD, GRTSD, GTEQSD, EQUSD, NTEQSD, ANDSD, ORSD, BANDSD, BORSD, BXORSD, LSHIFTSD, RSHIFTSD, CALL, DEL, JMP, JPT, JPF, EQCMP, LOD, LODS, LODC, LODF, LODD, LODV, LDA, LDI, STO, STOS, STOC, STOF, STOD, ADBR, RET, ASS, ASSV, ASSS, ASVS, ASSF, ASVF, ASSC, ASVC, ASSD, ASVD, ASSP, ASVP, CPY, VAL, VALS, VALC, VALF, VALD, LIB, STOP,
} OpCode;

static char *OpCodeStr[] = {
    "NOP", "INCL", "DECL", "INCSD", "DECSD", "NEG", "NOT", "ADDL", "SUBL", "MULL", "DIVL", "MODL", "LESSL", "LSEQL", "GRTL", "GTEQL", "EQUL", "NTEQL", "ANDL", "ORL", "BANDL", "BORL", "BXORL", "BNOT", "LSHIFTL", "RSHIFTL", "ADDSD", "SUBSD", "MULSD", "DIVSD", "MODSD", "LESSSD", "LSEQSD", "GRTSD", "GTEQSD", "EQUSD", "NTEQSD", "ANDSD", "ORSD", "BANDSD", "BORSD", "BXORSD", "LSHIFTSD", "RSHIFTSD", "CALL", "DEL", "JMP", "JPT", "JPF", "EQCMP", "LOD", "LODS", "LODC", "LODF", "LODD", "LODV", "LDA", "LDI", "STO", "STOS", "STOC", "STOF", "STOD", "ADBR", "RET", "ASS", "ASSV", "ASSS", "ASVS", "ASSF", "ASVF", "ASSC", "ASVC", "ASSD", "ASVD", "ASSP", "ASVP", "CPY", "VAL", "VALS", "VALC", "VALF", "VALD", "LIB", "STOP",
};

#endif
