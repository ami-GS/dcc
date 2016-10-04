#ifndef _DCC_INSTRUCTION_H_
#define _DCC_INSTRUCTION_H_

#include "opcode.h"
#include "letter_analysis.h"

#define NO_FIX_BREAK_ADDR -10001
#define NO_FIX_RET_ADDR -10002

typedef struct {
    char opcode;
    char flag;
    int  opdata;
} Instruction;

// TODO : should this be big as much as possible?
#define CODE_SIZ 65536
Instruction codes[CODE_SIZ];
extern int code_ct;

int genCode(OpCode op, int flag, int dat);
#define genCode1(op) genCode(op, 0, 0)
#define genCode2(op, dat) genCode(op, 0, dat)

int genCode_unary(Kind k);
int genCode_binary(Kind k);

void backpatch(int c_ct, int addr);
void backpatch_break(int loop_top);
void backpatch_return(int return_addr);
void backpatch_calladdr();
int binary_expr(OpCode op, int d1, int d2);
void to_left_val();
int const_fold(OpCode op);
void remove_op_stack_top();

#endif // _DCC_INSTRUCTION_H_
