#include <string.h>
#include "malloc.h"
#include "instruction.h"
#include "emulate_cpu.h"
#include "misc.h"

int localAddress;

int malloc_G(int size) {
  if (size <= 0)
    return globalAddress; // ?
  
  int align = INT_SIZE; // TODO : here should be more flexible
  globalAddress = (globalAddress + align-1) / align * align;
  if (MEM_MAX < (globalAddress + size)) {
    error("few memory to allocate");
  }
  memset(memory + globalAddress, 0, size); // zero init
  globalAddress += size;
  return globalAddress - size; // return head address
}

int malloc_L(int size) {
  if (size <= 0) {
    error("invalid memory size");
    size = 0;
  }
  localAddress += size;
  return localAddress - size; // start address
}

int malloc_more(TableEntry *te, int size) {
  // for variable, int A[] = {1,2,3};
  // size shall be, int A[] = {1,2} then 1
  te->var->arrLen = size + 1;
  if (te->level == GLOBAL)
    return malloc_G(size); // NOTICE : initial address is malloced already.
  return malloc_L(size);
}
