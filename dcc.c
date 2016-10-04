#include "parse.h"
#include "instruction.h"
#include "misc.h"
#include "preprocessor.h"
#include  <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc == 1) {
    // TODO : show help
    return 0;
  }

  initKind();
  char *fname_i = preprocess(argv[1]);
  compile(fname_i);
  free(fname_i);
  code_dump();
  execute(codes, 1);
}
