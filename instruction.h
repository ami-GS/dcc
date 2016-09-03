#ifndef _DCC_INSTRUCTION_H_
#define _DCC_INSTRUCTION_H_

#include "opcode.h"
#include "letter_analysis.h"

#define OP_STACK_TOP 200
#define OP_STACK_BUTTOM 0
int op_stack[OP_STACK_TOP];
static int stack_ptr = OP_STACK_BUTTOM;
#define PUSH(a) op_stack[stack_ptr++]=a
#define POP()   op_stack[stack_ptr--]

#define MEM_MAX 65535 // TODO : more ?
char memory[MEM_MAX];
static int global_memory_addr = 0;

#define NO_FIX_BREAK_ADDR -10001
#define NO_FIX_RET_ADDR -10002

static int pc; // program counter
static int baseReg; // base register

typedef struct {
    char opcode;
    char flag;
    int  opdata;
} Instruction;

// TODO : should this be big as much as possible?
#define CODE_SIZ 65536
Instruction codes[CODE_SIZ];
static int code_ct = 0; // 0 for 'main'

int genCode(OpCode op, int flag, int dat);
#define genCode1(op) genCode(op, 0, 0)
#define genCode2(op, dat) genCode(op, 0, dat)

int genCode_unary(Kind k);
int genCode_binary(Kind k);

void backpatch(int c_ct, int addr);
void backpatch_break(int loop_top);
int binary_expr(OpCode op, int d1, int d2);
void to_left_val();
int const_fold(OpCode op);
void remove_op_stack_top();

#define ZERO_CHK() if(op_stack[stack_ptr] == 0) return -1; // TODO : zero division
#define MEMINT(n) (* (int *)(memory+n)) // TODO : more flexible to data size like DWORD or something
#define UNI_OP(op) op_stack[stack_ptr] = op op_stack[stack_ptr]
#define BIN_OP(op) op_stack[stack_ptr-1] = op_stack[stack_ptr-1] op op_stack[stack_ptr], stack_ptr--
#define INCDEC(num) MEMINT(op_stack[stack_ptr]) += num, op_stack[stack_ptr] = MEMINT(op_stack[stack_ptr])
#define ASSIGN(addr, dat) MEMINT(addr) = dat

int execute();

#endif // _DCC_INSTRUCTION_H_
