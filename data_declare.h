#ifndef _DCC_DATA_DECLARE_H_
#define _DCC_DATA_DECLARE_H_

#define INT_SIZE 4
#define FLOAT_SIZE 4
#define SHORT_SIZE 2
#define DOUBLE_SIZE 8
#define CHAR_SIZE 1
#define POINTER_SIZE INT_SIZE // TODO 8?. currently, this should be same as INT_SIZE

static int DATA_SIZE[] = {-1, -1, POINTER_SIZE, INT_SIZE, POINTER_SIZE,
                          SHORT_SIZE, POINTER_SIZE, CHAR_SIZE, POINTER_SIZE,
                          FLOAT_SIZE, POINTER_SIZE, DOUBLE_SIZE, POINTER_SIZE};

#endif //_DCC_DATA_DECLARE_H_
