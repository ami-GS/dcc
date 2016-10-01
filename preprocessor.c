#include "preprocessor.h"
#include "letter_analysis.h"
#include <stdlib.h>
#include <string.h>

void writeWords(char *words) {
  int i;
  for (i = 0; *(words+i) != '\0'; i++) {
    putc(*(words+i), i_file);
  }
  putc(' ', i_file); // TODO : differenciate when ' ' is put or not
}

int replace_def(Token *t) {
  int i;
  char tmp[128];
  for (i = 0; t->text[i] != '\0'; i++) {
    tmp[i] = t->text[i];
  }
  tmp[i] = '\0';
  
  // search
  for (i = 0; i < def_table_ct; i++) {
    if (strcmp(t->text, define_table[i].n_bef) == 0) {
      int j;
      if (define_table[i].argNum == 0) {
	// write define_table[i].n_af to i file
	writeWords(define_table[i].n_af);
	return 1; // 1 indicates replaced
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
	  if (t->kind != Comma && t->kind != Rparen) {
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
	writeWords(af_buf);
      }
      return 1; // 1 indicate replaced
    }
  }
  
  if (i == def_table_ct) {
    writeWords(t->text);
    return 0; // 0 indicate not replaced
  }
  return 0;
}

int bef_line = 1;
int wrapNext(Token *t, int save) {
  int nl = 0;
  if (currentLine > bef_line) {
    nl = 1;
    putc('\n', i_file);
  }
  bef_line = currentLine;
  nextToken(t, 0);

  if (save) {
    // write to file
    writeWords(t->text);
  }

  return nl; // return whtere new lined or not
}

void pre_define(Token *t) {
  define_item item = define_table[def_table_ct]; // just copy, not reference. this is intentional
  wrapNext(t, 1);
  int i;
  for (i = 0; t->text[i] != '\0'; i++)
    item.n_bef[i] = t->text[i];
  item.n_bef[i] = '\0';

  int af_idx = 0;
  wrapNext(t, 0);
  if (t->kind == Lparen) {
    writeWords(t->text);
    wrapNext(t, 1);
    // func type
    while (t->kind != Rparen) {
      if (t->kind == Ident) {
	for (i = 0; t->text[i] != '\0'; i++) 
	  item.arg_table[item.argNum][i] = t->text[i];
	item.arg_table[item.argNum++][i] = '\0';
	wrapNext(t, 1);
	if (t->kind != Comma)
	  break;
	wrapNext(t, 1);
      } else {
	  error("arguments of defined function should be identifier");
	  return -1;
	}
    }
    
    // until new line
    while (!wrapNext(t, 0)) {
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
	  } /*else if () {
	      TODO : self replace
          }*/
	}
	// if any argument doesn't' match, try to replace by pre defined variable
	if (i == item.argNum) {
	  replace_def(t); // self replace
	}
      } else {
	for (i = 0; t->text[i] != '\0'; i++)
	  item.n_af[af_idx++] = t->text[i];
      }
      writeWords(t->text);
    }
  } else {
    // const type
    // until new line
    do {
      if (t->kind == Ident) {
	replace_def(t); // self replace
	continue;
      }
      for (i = 0; t->text[i] != '\0'; i++)
	item.n_af[af_idx++] = t->text[i];
      writeWords(t->text);
      // write to i file
    } while (!wrapNext(t, 0));
  }
  writeWords(t->text);
  item.n_af[af_idx] = '\0'; // end
  define_table[def_table_ct++] = item;
  return;
}

int replace_com(Token *t) {
  if (t->kind == LComment) {
    while (!wrapNext(t, 0)) {}
    writeWords(t->text);
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
  for (i = 0; fname[i] != '\0'; i++) {}
  char *fname_i;
  fname_i = (char *)malloc(sizeof(char) * i);
  memcpy(fname_i, fname, i);
  if (fname_i[i-2] == '.' && fname_i[i-1] == 'c') {
    fname_i[i-1] = 'i';
  } else {
    fprintf(stderr, "bad file format");
  }
  if ((i_file = fopen(fname_i, "w")) == NULL)
    fprintf(stderr, "file cannnot be opened");
  return fname_i;
}

char *preprocess(char *fname) {
  fOpen(fname);
  char *fname_i = fOpen_i(fname);
  Token t = {NulKind, "", 0, 0.0};
  while (t.kind != EOF_token) {
    wrapNext(&t, 0);
    if (t.kind == Sharp) {
      writeWords(t.text);
      wrapNext(&t, 1);
      switch(t.kind) {
      case Define:
	pre_define(&t);
	break;
      case Include:
	break;
	//case Pragma:
      default:
	error("undefined predefine");
      }
    } else if (t.kind == LComment || t.kind == MLCommS) {
      replace_com(&t);
    } else if (t.kind == Ident) {
      replace_def(&t);
    } else {
      writeWords(t.text);
    }
  }
  fclose(i_file);
  return fname_i; // success
}
