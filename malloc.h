#ifndef _DCC_MALLOC_H_
#define _DCC_MALLOC_H_

#include "symbol_table.h"

extern int localAddress;
// I need to understancd why 1 * INT_SIZE is used.
// -> this is for storing return address
#define START_LOCAL_ADDRESS 1 * INT_SIZE;
static int globalAddress = 1 * INT_SIZE;

int malloc_G(int size);
int malloc_L(int size);
int malloc_more(TableEntry *te, int size);

#endif //_DCC_MALLOC_H_
