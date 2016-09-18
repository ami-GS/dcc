#include <string.h>
#include "malloc.h"
#include "instruction.h"

int localAddress;

int malloc_G(int size) {
  if (size <= 0)
    return globalAddress; // ?
  
  int align = INT_SIZE; // TODO : here should be more flexible
  globalAddress = (globalAddress + align-1) / align * align;
  if (MEM_MAX < (globalAddress + size)) {
    return -1; // TODO : memory cannot be alloced
  }
  memset(memory + globalAddress, 0, size); // zero init
  globalAddress += size;
  return globalAddress - size; // return head address
}

int malloc_L(int size) {
  if (size <= 0) {
    size = 0;
    return -1; // TODO : invalid memory size
  }
  localAddress += size;
  return localAddress - size; // TODO : need to study
}
