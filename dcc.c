#include "dcc.h"
#include "parse.h"
#include "instruction.h"
#include "misc.h"
#include "preprocessor.h"
#include "emulate_cpu.h"
#include  <stdlib.h>

debug_flag DEBUG_FLAG = 0;

int  main(int argc, char *argv[]) {
  if (argc == 1) {
    // TODO : show help
    return 0;
  }

  initKind();
  // TODO : temporally putting here
  genCode2(CALL, -1); // for main
  genCode1(STOP);
  int i, j;
  for (i = 0; argv[i] != '\0'; i++) {
    if (argv[i][0] == '-') {
      for (j = 1; argv[i][j] != '\0'; j++) {
	switch (argv[i][j]) {
	case 'i': DEBUG_FLAG |= LEAVE_IFILE; break;
	case 's': DEBUG_FLAG |= SHOW_ASSEMBLY; break;
	case 'r': DEBUG_FLAG |= RUN; break;
	case 'm': DEBUG_FLAG |= SHOW_MOVEMENT; break;
	case 't': DEBUG_FLAG |= SHOW_TREE; break;
	case 'o': DEBUG_FLAG |= OPTIMIZE; break;
	}
      }
    }
  }

  for (i = 0; argv[i] != '\0'; i++) {
    for (j = 0; argv[i][j] != '\0'; j++) {}
    if (argv[i][j-2] == '.' && argv[i][j-1] == 'c') {
      char *fname_i = preprocess(argv[i]);
      compile(fname_i);
      if (!(DEBUG_FLAG & LEAVE_IFILE))
	remove(fname_i);
    }
  }
  if (DEBUG_FLAG & SHOW_ASSEMBLY)
    code_dump();

  if (codes[0].opdata < 0)
    error("'main' function is missing");

  if (DEBUG_FLAG & RUN)
    execute(codes, (DEBUG_FLAG & SHOW_MOVEMENT) >> 2);
}
