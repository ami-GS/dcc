#include "letter_analysis.h"
#include "parse.h"
#include "symbol_table.h"

void compile(char *fname) {
  fOpen(fname);
  initKind();
  Token t = {NulKind, "", 0};
  TableEntry entryTmp = {no_ID, "", NON_T, NO_LOCATION, 0, 0, 0};

  nextToken(&t);
  while (t.kind != EOF_token) {
    switch(t.kind) {
      // type, #, ;, }, )
    case Int: case Void:
      set_type(&entryTmp);
      set_name(&entryTmp);
    case Semicolon:
      nextToken(&t);
    }
  }

  
}
