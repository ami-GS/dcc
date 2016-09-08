#include "parse.h"
#include "instruction.h"
#include "misc.h"

int main(int argc, char *argv[]) {
  if (argc == 1) {
    // TODO : show help
    return 0;
  }

  compile(argv[1]);
  code_dump();
  execute();
}
