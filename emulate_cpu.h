#ifndef _DCC_EMULATE_CPU_H_
#define _DCC_EMULATE_CPU_H_

#include "instruction.h"
#include "symbol_table.h"

static int pc; // program counter
static int baseReg; // base register

#define OP_STACK_TOP 200
#define OP_STACK_BUTTOM 0

typedef struct {
    union {
        char   sCHAR; //not used yet
        short  sSRT; //not used yet
        int    sINT;
        float  sFLT;  //not used yet
        double sDBL;  //not used yet
    };
    DataType type;
} stkData;
stkData op_stack[OP_STACK_TOP];

static int stack_ptr = OP_STACK_BUTTOM;
#define PUSHCHAR(a) op_stack[stack_ptr].sCHAR=a; op_stack[stack_ptr++].type=CHAR_T;
#define PUSHSRT(a) op_stack[stack_ptr].sSRT=a; op_stack[stack_ptr++].type=SHORT_T;
#define PUSHINT(a) op_stack[stack_ptr].sINT=a; op_stack[stack_ptr++].type=INT_T;
#define PUSHFLT(a) op_stack[stack_ptr].sFLT=a; op_stack[stack_ptr++].type=FLOAT_T;
#define PUSHDBL(a) op_stack[stack_ptr].sDBL=a; op_stack[stack_ptr++].type=DOUBLE_T;
#define POP()   op_stack[--stack_ptr].sINT // currently only int


#define MEM_MAX 65535 // TODO : more ?
char memory[MEM_MAX];
static int global_memory_addr = 0;

#define ZERO_CHK_DWORD() if(op_stack[stack_ptr-1].sINT == 0) return -1; // TODO : zero division
#define ZERO_CHK_QWORD() if(op_stack[stack_ptr-1].sDBL == 0) return -1; // TODO : zero division
#define MEMINT(n) (* (int *)(memory+n))
#define MEMFLOAT(n) (* (float *)(memory+n))
#define MEMDOUBLE(n) (* (double *)(memory+n))
#define MEMSHORT(n) (* (short *)(memory+n))
#define UNI_OP_DWORD(op) op_stack[stack_ptr-1].sINT = op op_stack[stack_ptr-1].sINT
#define UNI_OP_QWORD(op) op_stack[stack_ptr-1].sDBL = op op_stack[stack_ptr-1].sDBL
#define BIN_OP_BIT(op) op_stack[stack_ptr-2].sINT = op_stack[stack_ptr-2].sINT op op_stack[stack_ptr-1].sINT, stack_ptr--
// TODO : separate in each word size
#define BIN_OP_DWORD(op) {                                              \
        switch(op_stack[stack_ptr-2].type) {                            \
        case CHAR_T: op_stack[stack_ptr-2].sCHAR = op_stack[stack_ptr-2].sCHAR op op_stack[stack_ptr-1].sCHAR; stack_ptr--; break; \
        case SHORT_T: op_stack[stack_ptr-2].sSRT = op_stack[stack_ptr-2].sSRT op op_stack[stack_ptr-1].sSRT; stack_ptr--; break; \
        case INT_T: op_stack[stack_ptr-2].sINT = op_stack[stack_ptr-2].sINT op op_stack[stack_ptr-1].sINT; stack_ptr--; break; \
        case FLOAT_T: op_stack[stack_ptr-2].sFLT = op_stack[stack_ptr-2].sFLT op op_stack[stack_ptr-1].sFLT; stack_ptr--; break; \
        case DOUBLE_T: op_stack[stack_ptr-2].sDBL = op_stack[stack_ptr-2].sDBL op op_stack[stack_ptr-1].sDBL; stack_ptr--; break; \
        }                                                               \
    }
#define BIN_OP_QWORD(op) op_stack[stack_ptr-2].sDBL = op_stack[stack_ptr-2].sDBL op op_stack[stack_ptr-1].sDBL, stack_ptr--
#define INCDEC_DWORD(num) MEMINT(op_stack[stack_ptr-1].sINT) += num, op_stack[stack_ptr-1].sINT = MEMINT(op_stack[stack_ptr-1].sINT)
#define INCDEC_QWORD(num) MEMINT(op_stack[stack_ptr-1].sDBL) += num, op_stack[stack_ptr-1].sDBL = MEMINT(op_stack[stack_ptr-1].sDBL)
#define ASSIGN(addr, dat) MEMINT(addr) = dat
#define ASSIGN_CHAR(addr, dat) *(memory+addr) = dat
#define ASSIGN_SHORT(addr, dat) MEMSHORT(addr) = dat
#define ASSIGN_FLOAT(addr, dat) MEMFLOAT(addr) = dat
#define ASSIGN_DOUBLE(addr, dat) MEMDOUBLE(addr) = dat

int execute(Instruction *codes);
void debug_emulate(int pc, Instruction *code);

#endif //_DCC_EMULATE_CPU_H_
