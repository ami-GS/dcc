#include "preprocessor.h"
#include "letter_analysis.h"
#include <stdlib.h>
#include <string.h>

void writeWords(char *words) {
  int i;
  if (if_nest_ct > 0 && if_nest_table[if_nest_ct-1].ignore == 1)
    return;
  for (i = 0; *(words+i) != '\0'; i++)
    putc(*(words+i), streams[STREAM_SIZE-1]); // NOTICE : streams[1] must be .i file
}

int search_def(char *text) {
  int i;
  for (i = 0; i < def_table_ct; i++) {
    if (strcmp(text, define_table[i].n_bef) == 0) {
      return i;
    }
  }
  return -1;
}

int replace_def(Token *t, int save) {
  int i;
  char tmp[128];
  for (i = 0; t->text[i] != '\0'; i++) {
    tmp[i] = t->text[i];
  }
  tmp[i] = '\0';
  
  // search
  i = search_def(t->text);
  if (i != -1) {
      int j;
      if (define_table[i].argNum == 0) {
	// write define_table[i].n_af to i file
	if (save)
	  writeWords(define_table[i].n_af);
      } else {
	// func type
	wrapNext(t, 0); // -> '('
	if (t->kind != Lparen) {
	  error("this identifier should be function type");
	  return -1; // TODO : this definition is function type
	}
	char arg_table_buf[MAX_DEFINE_ARG_NUM][MAX_DEFINE_LENGTH];
	char af_buf[128];
	int af_idx = 0, aNum = 0, k;

	// save argumnets
	while (t->kind != Rparen) {
	  wrapNext(t, 0); // arg
	  if (t->kind != Comma && t->kind != Rparen &&
	      t->kind != Space && t->kind != Tab && t->kind != NewLine && t->kind != Blanks) {
	    for (k = 0; t->text[k] != '\0'; k++) 
	      arg_table_buf[aNum][k] = t->text[k];
	    arg_table_buf[aNum++][k] = '\0';
	  }
	}
	if (define_table[i].argNum != aNum) {
	  error("the number of argument mismatch");
	  return -1;
	}

	// replace template {N} by arguments and write it to af_buf
	for (k = 0; define_table[i].n_af[k] != '\0'; k++) {
	  if (define_table[i].n_af[k] == '{') {
	    int loc = 0;
	    k++; // arg
	    do {
	      loc = loc*10 + define_table[i].n_af[k] - '0';
	      k++; // correct index
	    } while (define_table[i].n_af[k] != '}');
	    int l;
	    for (l = 0; arg_table_buf[loc][l] != '\0'; l++)
	      af_buf[af_idx + l] = arg_table_buf[loc][l];
	    af_idx += l;
	  } else {
	    af_buf[af_idx++] = define_table[i].n_af[k];
	  }
	}
	af_buf[af_idx] = '\0';
	// write af_buf to i file
	if (save)
	  writeWords(af_buf);
      }
      return i; // 1 indicate replaced
    }

  if (save && i == -1) {
    writeWords(t->text);
    return -1; // 0 indicate not replaced
  }
  return -1;
}

int bef_line = 1;
int wrapNext(Token *t, int save) {
  int nl = 0;
  if (currentLines[streamNest-1] > bef_line) {
    nl = 1;
  }
  bef_line = currentLines[streamNest-1];
  nextToken(t, 0);
  if (save) {
    // write to file
    writeWords(t->text);
  }

  return nl; // return whtere new lined or not
}

void pre_define(Token *t) {
  wrapNext(t, 0); wrapNext(t, 0); // skip space
  if (search_def(t->text) != -1) {
    error("duplicating define");
    return -1;
  }
  define_item item = define_table[def_table_ct]; // just copy, not reference. this is intentional
  int i;
  for (i = 0; t->text[i] != '\0'; i++)
    item.n_bef[i] = t->text[i];
  item.n_bef[i] = '\0';

  int af_idx = 0;
  int replaced_idx = -1;
  wrapNext(t, 0);
  if (t->kind == Lparen) {
    wrapNext(t, 0);
    // func type
    while (t->kind != Rparen) {
      if (t->kind == Ident) {
	for (i = 0; t->text[i] != '\0'; i++) 
	  item.arg_table[item.argNum][i] = t->text[i];
	item.arg_table[item.argNum++][i] = '\0';
      } else if (!(t->kind == Space || t->kind == Tab || t->kind == NewLine || t->kind == Blanks || t->kind == Comma)) {
	error("arguments of defined function should be identifier");
	return -1;
      }
      wrapNext(t, 0);
    }

    // until new line
    wrapNext(t, 0); // point at space
    while (t->kind != NewLine) {
      wrapNext(t, 0);
      if (t->kind == Ident) {
	for (i = 0; i < item.argNum; i++) {
	  if (strcmp(item.arg_table[i], t->text) == 0) {
	    int j;
	    for (j = 0; t->text[j] != '\0'; j++) {
	      item.n_af[af_idx++] = '{';
	      // TODO : currently10 arguments can be available
	      item.n_af[af_idx++] = (char)i + '0'; // 0 origin
	      item.n_af[af_idx++] = '}';
	    }
	    break;
	  }
	}
	// if any argument doesn't' match, try to replace by pre defined variable
	if (i == item.argNum) {
	  // self replace
	  replaced_idx = replace_def(t, 0);
	  if (replaced_idx != -1) {
	    for (i = 0; define_table[replaced_idx].n_af[i] != '\0'; i++)
	      item.n_af[af_idx++] = define_table[replaced_idx].n_af[i];
	  }
	}
      } else if (!(t->kind == Space || t->kind == Tab || t->kind == NewLine || t->kind == Blanks)) {
	for (i = 0; t->text[i] != '\0'; i++)
	  item.n_af[af_idx++] = t->text[i];
      }
    }
  } else if (t->kind == Space || t->kind == Tab) {
    // const type
    // until new line
    while (t->kind != NewLine) {
     wrapNext(t, 0);
      if (t->kind == Ident) {
	// self replace
	replaced_idx = replace_def(t, 0);
	if (replaced_idx != -1) {
	  for (i = 0; define_table[replaced_idx].n_af[i] != '\0'; i++)
	    item.n_af[af_idx++] = define_table[replaced_idx].n_af[i];
	}
      } else if (!(t->kind == Space || t->kind == Tab || t->kind == NewLine || t->kind == Blanks)) {
	for (i = 0; t->text[i] != '\0'; i++)
	  item.n_af[af_idx++] = t->text[i];
      }
    }
  } else if (t->kind == NewLine) {
    item.n_af[af_idx++] = ' '; // for like #define DCC_HEADER_H_
  } else {
    error("invalid pre define syntax");
  }
  item.n_af[af_idx] = '\0'; // end
  define_table[def_table_ct++] = item;
  return;
}

void pre_include(Token *t) {
  wrapNext(t, 0); wrapNext(t, 0);
  if (t->kind != String) {
    error("include needs \"FILENAME\"."); // TODO : <FILENAME> should be added
    return;
  }
  fOpen(t->text, "r");
  preprocess_sub();
  fClose();
}

int replace_com(Token *t) {
  if (t->kind == LComment) {
    while (!wrapNext(t, 0)) {}
  } else if (t->kind == MLCommS) {
    while (t->kind != MLCommE) {
      wrapNext(t, 0);
    }
    writeWords(" ");
  }
  return 0;
}

char *fOpen_i(char *fname) {
  int i;
  for (i = 0; fname[i] != '\0'; i++)
    fileNames[STREAM_SIZE-1][i] = fname[i];
  fileNames[STREAM_SIZE-1][i] = '\0';
  if (fname[i-2] == '.' && fname[i-1] == 'c') {
    fileNames[STREAM_SIZE-1][i-1] = 'i';
  } else {
    fprintf(stderr, "bad file format");
  }

  streamRW[STREAM_SIZE-1] = 'w';
  currentLines[STREAM_SIZE-1] = 1;
  if ((streams[STREAM_SIZE-1] = fopen(fileNames[STREAM_SIZE-1], "w")) == NULL) {
    error("file cannot be opened");
    return -1;
  }
  return fileNames[STREAM_SIZE-1];
}

void pre_if(Token *t) {
  wrapNext(t, 0); wrapNext(t, 0);
  if (!(t->kind == Ident || t->kind == IntNum)) {
    error("#if needs value");
    return;
  }
  // TODO : below lines are not cool
  if (t->kind == Ident) {
    int idx = replace_def(t, 0);
    if (idx == -1 || strcmp(define_table[idx].n_af, "0") == 0) {
      if_nest_table[if_nest_ct].ignore = 1;
    } else {
      if_nest_table[if_nest_ct].has_true = 1;
    }
  } else {
    if (t->intVal == 0) { // TODO : is other value available?
      if_nest_table[if_nest_ct].ignore = 1;
    } else {
      if_nest_table[if_nest_ct].has_true = 1;
    }
  }
  if_nest_table[if_nest_ct].if_type = 1;
  ++if_nest_ct;
}

void pre_else(Token *t, int is_elif) {
  if (if_nest_ct < 1) {
    error("missing previous #if, #ifdef or #ifndef");
    return;
  }

  if (if_nest_table[if_nest_ct-1].has_true) {
    if_nest_table[if_nest_ct-1].ignore = 1;
  } else if (is_elif) {
    wrapNext(t, 0); wrapNext(t, 0);
    if (!(t->kind == Ident || t->kind == IntNum)) { // TODO : other type should be available
      error("#elif of #if needs value");
      return;
    }

    // TODO : below lines are not cool
    if (t->kind == Ident) {
      int idx = replace_def(t, 0);
      if (idx == -1 || strcmp(define_table[idx].n_af, "0") == 0) {
	if_nest_table[if_nest_ct-1].ignore = 1;
      } else {
	if_nest_table[if_nest_ct-1].ignore = 0;
	if_nest_table[if_nest_ct-1].has_true = 1;
      }
    } else if (t->kind == IntNum){
      if (t->intVal == 0) { // TODO : is other value available?
	if_nest_table[if_nest_ct-1].ignore = 1;
      } else {
	if_nest_table[if_nest_ct-1].ignore = 0;
	if_nest_table[if_nest_ct-1].has_true = 1;
      }
    }
  } else { // when #else and false on #if, #ifdef or #ifndef
    if_nest_table[if_nest_ct-1].ignore = 0;
  }
}

void pre_endif(Token *t) {
  if (if_nest_ct > 0 && if_nest_table[if_nest_ct-1].if_type > 0) {
    if_nest_table[--if_nest_ct].has_true = 0;
    if_nest_table[if_nest_ct].if_type = 0;
    if_nest_table[if_nest_ct].ignore = 0;
  } else {
    error("missing previous #if, #ifdef or #ifndef");
  }
}

void preprocess_sub() {
  Token t = {NulKind, "", 0, 0.0};
  while (t.kind != EOF_token) {
    wrapNext(&t, 0);
    if (t.kind == Sharp) {
      wrapNext(&t, 0);
      switch(t.kind) {
      case Define:
	if (!if_nest_table[if_nest_ct-1].ignore)
	  pre_define(&t);
	break;
      case If:
	// in preprocessor.c, this only validate the elif, else, endif pair
	// actual process after like #if is conducted in compiler()
	pre_if(&t);
	break;
      case Elif:
	pre_else(&t, 1);
	break;
      case Else:
	pre_else(&t, 0);
	break;
      case Endif:
	pre_endif(&t);
	break;
      case Include:
	if (!if_nest_table[if_nest_ct-1].ignore)
	  pre_include(&t);
	break;
	//case Pragma:
      default:
	error("undefined predefine");
      }
    } else {
      switch(t.kind) {
      case LComment: case MLCommS:
	replace_com(&t); break;
      case Ident:
	replace_def(&t, 1); break;
      case String:
	writeWords("\""); // workaround
	writeWords(t.text);
	writeWords("\""); break;
      case CharSymbol:
	writeWords("\'"); // workaround
	writeWords(t.text);
	writeWords("\'"); break;
      default:
	writeWords(t.text); break;
      }
    }
  }
}

char *preprocess(char *fname) {
  use_all_as_token = 1;
  fOpen(fname, "r");
  char *fname_i = fOpen_i(fname);
  preprocess_sub();
  fclose(streams[STREAM_SIZE-1]);
  fClose();
  use_all_as_token = 0;
  return fname_i; // success
}
