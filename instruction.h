#ifndef _DCC_INSTRUCTION_H_
#define _DCC_INSTRUCTION_H_

#include "opcode.h"
#include "letter_analysis.h"

#define OP_STACK_TOP 200
#define OP_STACK_BUTTOM 0
int op_stack[OP_STACK_TOP];
int stack_ptr = OP_STACK_BUTTOM;
#define PUSH(a) op_stack[stack_ptr++]=a
#define POP()   op_stack[stack_ptr--]
int pc; // program counter
int baseReg; // base register

typedef struct {
    char opcode;
    char flat;
    int  opdata;
} Instruction;

// TODO : should this be big as much as possible?
#define CODE_SIZ 65536
Instruction codes[CODE_SIZ];
int code_ct = 1; // 0 for 'main'

int genCode(OpCode op, int flag, int dat);
#define genCode1(op) genCode(op, 0, 0)
#define genCode2(op, dat) genCode(op, 0, dat)

int genCode_unary(Kind k);
int genCode_binary(Kind k);

void backpatch(int c_ct, int addr);
int binary_expr(OpCode op, int d1, int d2);
void to_left_val();


#endif // _DCC_INSTRUCTION_H_
