#ifndef _DCC_INSTRUCTION_H_
#define _DCC_INSTRUCTION_H_

// TODO : should this be big as much as possible?
#define CODE_SIZ 65536

typedef enum {
    ADD, SUB, MUL, DIV,
    CALL,
} OpCode;


typedef struct {
    char opcode;
    char flat;
    int  opdata;
} Instruction;

Instruction codes[CODE_SIZ];
int codeCnt = 1; // 0 for 'main'

int genCode(OpCode op, int flag, int dat);

#endif // _DCC_INSTRUCTION_H_
