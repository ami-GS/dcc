#include "parse.h"
#include "instruction.h"
#include "misc.h"
#include "preprocessor.h"
#include "emulate_cpu.h"
#include  <stdlib.h>

int leave_ifile = 0;   // -i
int show_assembly = 0; // -s
int show_movement = 0; // -m

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
	case 'i': leave_ifile = 1; break;
	case 's': show_assembly = 1; break;
	case 'm': show_movement = 1; break;
	}
      }
    }
  }

  for (i = 0; argv[i] != '\0'; i++) {
    for (j = 0; argv[i][j] != '\0'; j++) {}
    if (argv[i][j-2] == '.' && argv[i][j-1] == 'c') {
      char *fname_i = preprocess(argv[i]);
      compile(fname_i);
      if (!leave_ifile)
	remove(fname_i);
    }
  }
  if (show_assembly)
    code_dump();

  if (codes[0].opdata < 0)
    error("'main' function is missing");

  execute(codes, show_movement);
}
