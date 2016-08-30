#ifndef _DCC_MALLOC_H_
#define _DCC_MALLOC_H_

#include "data_declare.h"

static int localAddress = 0;
// TODO : I need to understand why 1 * INT_SIZE is used.
static int globalAddress = 1 * INT_SIZE;

int malloc_G(int size);
int malloc_L(int size);

#endif //_DCC_MALLOC_H_
