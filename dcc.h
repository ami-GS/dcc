#ifndef DCC_DCC_H_
#define DCC_DCC_H_

typedef enum {
    LEAVE_IFILE, SHOW_ASSEMBLY, RUN, SHOW_MOVEMENT = 0x04, SHOW_TREE = 0x08,
} debug_flag;
extern debug_flag DEBUG_FLAG;

#endif // DCC_DCC_H_
