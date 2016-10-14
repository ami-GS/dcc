#include "parse.h"
#include "instruction.h"
#include "misc.h"
#include "preprocessor.h"
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
  int i;
  char *fname;
  for (i = 0; argv[i] != '\0'; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'i': leave_ifile = 1; break;
      case 's': show_assembly = 1; break;
      case 'm': show_movement = 1; break;
      }
    } else {
      fname = argv[i]; // TODO : more validation needed
    }
  }

  char *fname_i = preprocess(fname);
  use_all_as_token = 0;
  compile(fname_i);
  if (!leave_ifile)
    remove(fname_i);
  if (show_assembly)
    code_dump();
  execute(codes, show_movement);
}
