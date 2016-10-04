#ifndef _DCC_EMULATE_CPU_H_
#define _DCC_EMULATE_CPU_H_

#include "instruction.h"

static int pc; // program counter
static int baseReg; // base register

#define OP_STACK_TOP 200
#define OP_STACK_BUTTOM 0
typedef union {
    char   sCHAR; //not used yet
    short  sSINT; //not used yet
    int    sINT;
    float  sFLT;  //not used yet
    double sDBL;  //not used yet
} stkData;
stkData op_stack[OP_STACK_TOP];

extern int stack_ptr;
#define PUSH(a) op_stack[stack_ptr++].sINT=a
#define POP()   op_stack[--stack_ptr].sINT // currently only int

#define MEM_MAX 65535 // TODO : more ?
char memory[MEM_MAX];
static int global_memory_addr = 0;

#define ZERO_CHK_DWORD() if(op_stack[stack_ptr-1].sINT == 0) return -1; // TODO : zero division
#define ZERO_CHK_QWORD() if(op_stack[stack_ptr-1].sDBL == 0) return -1; // TODO : zero division
#define MEMINT(n) (* (int *)(memory+n)) // TODO : more flexible to data size like DWORD or something
#define MEMDOUBLED(n) (* (double *)(memory+n)) // TODO : more flexible to data size like DWORD or something
#define UNI_OP_DWORD(op) op_stack[stack_ptr-1].sINT = op op_stack[stack_ptr-1].sINT
#define UNI_OP_QWORD(op) op_stack[stack_ptr-1].sDBL = op op_stack[stack_ptr-1].sDBL
#define BIN_OP_DWORD(op) op_stack[stack_ptr-2].sINT = op_stack[stack_ptr-2].sINT op op_stack[stack_ptr-1].sINT, stack_ptr--
#define BIN_OP_QWORD(op) op_stack[stack_ptr-2].sDBL = op_stack[stack_ptr-2].sDBL op op_stack[stack_ptr-1].sDBL, stack_ptr--
#define INCDEC_DWORD(num) MEMINT(op_stack[stack_ptr-1].sINT) += num, op_stack[stack_ptr-1].sINT = MEMINT(op_stack[stack_ptr-1].sINT)
#define INCDEC_QWORD(num) MEMINT(op_stack[stack_ptr-1].sDBL) += num, op_stack[stack_ptr-1].sDBL = MEMINT(op_stack[stack_ptr-1].sDBL)
#define ASSIGN(addr, dat) MEMINT(addr) = dat
#define ASSIGN_CHAR(addr, dat) *(memory+addr) = dat

int execute(Instruction *codes, int debug);

#endif //_DCC_EMULATE_CPU_H_
