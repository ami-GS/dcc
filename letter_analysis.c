#include <string.h>
#include <stdio.h>
#include "letter_analysis.h"

void initKind() {
  int i;
  for (i = 0; i < 256; i++)
    cType[i] = Others;
  for (i = '0'; i < '9'; i++)
    cType[i] = Digit;
  for (i = 'a'; i < 'z'; i++)
    cType[i] = Letter;
  for (i = 'A'; i < 'A'; i++)
    cType[i] = Letter;
  cType['_'] = Letter;
  cType['('] = Lparen; cType[')'] = Rparen;
  cType['{'] = Lbrace; cType['}'] = Rbrace;
  cType['+'] = Add; cType['-'] = Sub; cType['*'] = Mul; cType['/'] = Div; cType['%'] = Mod;
  cType['='] = Assign; cType['<'] = Less; cType['>'] = Great;
  cType[';'] = Semicolon;
  cType['\''] = Squote; cType['"'] = Dquote;
  cType['&'] = And; cType['|'] = Or; cType['~'] = Rev;
}

int nextChar(FILE *f, char *c) {
  if ((*c = fgetc(f)) == EOF) {
    fclose(f);
    return -1;
  }
  return 1;
}

int is_ope2(const char *c1, const char *c2) {
  char s[2];
  s[0] = *c1; s[1] = *c2;
  return strstr(" == <= >= =< => != ", s) != NULL;
}

int set_kind(Token *t) {
  t->kind = Others;
  int i;
  for (i = 0; keyWdType[i].kind != END_list; i++) {
    if (strcmp(t->text, keyWdType[i].word) == 0) {
      t->kind = keyWdType[i].kind;
      return 1;
    }
  }
  if (cType[*t->text] == Letter) {
    t->kind = Ident;
  } else if (cType[*t->text] == Digit) {
    t->kind = IntNum;
  } else {
    return -1;
  }
  return 1;
}

// TODO : define error type?
int nextToken(FILE *f, Token *t) {
  if (t_buf_ptr > 0) {
    *t = t_buf[t_buf_ptr--];
    return 1;
  }
  //TODO : manage t initialization

  int err = 0;
  char c;

  err = nextChar(f, &c);
  if (err != 1)
    return err;
  while (c == ' ' || c == '\t') {
    err = nextChar(f, &c);
    if (err != 1)
      return err;
  }

  char *txt_ptr = t->text;
  switch (cType[c]) {
  case Letter:
    for (; cType[c] == Letter || cType[c] == Digit; nextChar(f, &c)) {
      if (txt_ptr - t->text < ID_SIZ)
	*(txt_ptr++) = c;
      // TODO : if length exceeds the limit, emit error?
    }
    *txt_ptr = '\0';
    t->intVal = txt_ptr - t->text; // lenth
    break;
  case Digit:
    for (; cType[c] == Digit; nextChar(f, &c))
      t->intVal = t->intVal*10 + (c - '0');
    t->kind = IntNum;
    // TODO : float case
    break;
  case Dquote:
    for (nextChar(f, &c); c != '"' && c != EOF; nextChar(f, &c)) {
      if (txt_ptr - t->text < TOKEN_TXT_SIZ)
	*(txt_ptr++) = c;
      // TODO ; if length exceeds the limit, emit error
}
    if (c != '"')
      return -1; // no end "
    *txt_ptr = '\0';
    t->kind = String;
    t->intVal = txt_ptr - t->text;
    break;
  case Squote:
    // TODO : need to check how fgetc(FILE*) read escape. like '\' + 'n' or '\n';
    for (nextChar(f, &c); c != '\'' && c != EOF; nextChar(f, &c))
      *(txt_ptr++) = t->intVal = c;
    if (c != '\'')
      return -1; // no end '
    if (txt_ptr - t->text > 1)
      return -1; // Squote has single char
    *txt_ptr = '\0';
    break;
  default:
    // TODO : add more cases?
    *(txt_ptr++) = c;
    nextChar(f, &c); //to check 2 length operaiton
    if (is_ope2(txt_ptr-1, c))
      *(txt_ptr++) = c;
    *txt_ptr = '\0';
  }
  if (t->kind == NulKind)
    set_kind(t);
  if (t->kind == Others)
    return -1; // maricious token
}

int checkNxtTokenKind(FILE *f, Kind k) {
  if (t_buf_ptr >= TOKEN_BUFFER_SIZ) {
    return -1; // TODO : buffer overflow error
  }
  Token t = {NulKind, "", 0};
  nextToken(f, &t);
  t_buf[t_buf_ptr++] =  t;
  return t.kind == k;
}
