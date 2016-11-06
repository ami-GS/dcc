#include <string.h>
#include <stdio.h>
#include "letter_analysis.h"
#include "misc.h"

int t_buf_open = 0;
 // for creating '.i' file
int use_all_as_token = 1;
FILE *streams[STREAM_SIZE];
char fileNames[STREAM_SIZE][80];
char  streamRW[STREAM_SIZE];
int streamNest_ct = 0;
int currentLines[STREAM_SIZE];

int fOpen(char *fname, char *RW) {
  int i;
  for (i = 0; fname[i] != '\0'; i++)
    fileNames[streamNest_ct][i] = fname[i];
  fileNames[streamNest_ct][i] = '\0';
  streamRW[streamNest_ct] = RW[0];
  currentLines[streamNest_ct] = 1;
  if ((streams[streamNest_ct++] = fopen(fname, RW)) == NULL) {
    error("file cannot be opened");
    return -1;
  }
  return 1;
}

int fClose() {
  if (streamNest_ct < 1) {
    error("file was not opened");
    return -1;
  }
  fclose(streams[--streamNest_ct]);
}

void initKind() {
  int i;
  for (i = 0; i < 256; i++)
    cType[i] = Others;
  for (i = '0'; i <= '9'; i++)
    cType[i] = Digit;
  for (i = 'a'; i <= 'z'; i++)
    cType[i] = Letter;
  for (i = 'A'; i <= 'Z'; i++)
    cType[i] = Letter;
  cType['_'] = Letter;
  cType['('] = Lparen; cType[')'] = Rparen;
  cType['{'] = Lbrace; cType['}'] = Rbrace;
  cType['['] = Lbracket; cType[']'] = Rbracket;
  cType['+'] = Add; cType['-'] = Sub; cType['*'] = Mul; cType['/'] = Div; cType['%'] = Mod;
  cType['='] = Assign; cType['<'] = Less; cType['>'] = Great;
  cType[';'] = Semicolon; cType[','] = Comma; cType['.'] = Dot;
  cType['\''] = Squote; cType['"'] = Dquote;
  cType['#'] = Sharp;
  cType[' '] = Space; cType['\t'] = Tab; cType['\n'] = NewLine;
  cType['&'] = Band; cType['|'] = Bor; cType['~'] = Bnot; cType['^'] = Bxor;
}

void set_hKind(Token *t) {
  if (Lparen == t->kind || Rparen == t->kind || Lbrace == t->kind
      || Rbrace == t->kind || Lbracket == t->kind || Rbracket == t->kind) {
    t->hKind = Paren;
  } else if (Add == t->kind || Sub == t->kind || Mul == t->kind || Div == t->kind || Arrow == t->kind ||
	      Mod == t->kind || Not == t->kind || Band == t->kind || Bor == t->kind || Dot == t->kind ||
	      Bxor == t->kind || Bnot == t->kind || (Less <= t->kind && t->kind <= Great) ||
	     (Incre <= t->kind && t->kind <= EqGreat)) {
    t->hKind = Operator;
  } else if (Int <= t->kind && t->kind <= VoidP) {
    t->hKind = Type;
  } else if (String <= t->kind && t->kind <= CharSymbol) {
    t->hKind = Immediate;
  } else if (If <= t->kind && t->kind <= Input) {
    t->hKind = Statement;
  } else {
    t->hKind = Specific;
  }
}

int nextChar(char *c) {
  // TODO : here is my own implementation. suspicious
  if (prevC > 0) {
    *c = (char)prevC;
    prevC = -2;
    return 1;
  } else if (prevC == EOF) {
    *c = prevC;
    return *c;
  }
  *c = fgetc(streams[streamNest_ct-1]);
  if (*c == '\n') {
    currentLines[streamNest_ct-1]++; // TODO : this need to be made for each files
  }
  if (*c == EOF) {
    return *c;
  }
  return 1;
}

void notUseChar(char c) {
  if (!use_all_as_token && (c == ' ' || c == '\t' || c == '\n'))
    return;
  prevC = c;
}

int is_ope2(const char *c1, const char *c2) {
  if (*c2 == ' ' || *c2 == '\t' || *c2 == '\n') // TODO : redundant
    return 0;
  char s[2];
  s[0] = *c1; s[1] = *c2;
  // TODO : //, /* and */ are put here as workaround
  char ope2[24][2] = {"++", "--", "||", "&&", "==", "<=", ">=",
		      "=<", "=>", "!=", "+=", "-=", "*=", "/=",
		      "%=", "<<", ">>", "&=", "|=", "^=", "//",
		      "/*", "*/", "->"};
  int i;
  for (i = 0; i < 24; i++) {
    if (ope2[i][0] == s[0] && ope2[i][1] == s[1]) {
      return 1;
    }
  }
  return 0; // workaround
  //return strstr(" ++ -- || && == <= >= =< => != ", s) != NULL;
}

int is_ope1(Kind k) {
  return k == Add || k == Sub || k == Mul || k == Div || k == Mod;
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
  } else {
    error("unknown kind");
  }
  return 1;
}

// TODO : define error type?
int nextToken(Token *t, int q_lock) {
  if (t_buf_head != t_buf_tail && !q_lock) {
    t_buf_dequeue(t);
    return 1;
  }
  // TODO : manage t initialization
  *(t->text) = '\0';
  t->kind = NulKind;
  t->intVal = 0;

  int err = 0;
  char c;
  err = nextChar(&c);
  if (!use_all_as_token) {
    while (c == ' ' || c == '\t' || c == '\n') {
      err = nextChar(&c);
    }
  }

  if (err == EOF) {
    t->kind = EOF_token;
    prevC = -2;
    return err;
  }

  float power;
  char *txt_ptr = t->text;
  switch (cType[c]) {
  case Letter:
    for (; cType[c] == Letter || cType[c] == Digit; nextChar(&c)) {
      if (txt_ptr - t->text < ID_SIZ)
	*(txt_ptr++) = c;
      // TODO : if length exceeds the limit, emit error?
    }
    notUseChar(c);
    *txt_ptr = '\0';
    t->intVal = txt_ptr - t->text; // lenth
    break;
  case Digit:
    for (; cType[c] == Digit; nextChar(&c)) {
      t->intVal = t->intVal*10 + (c - '0');
      *(txt_ptr++) = c;
    }
    if (c == '.') {
      *(txt_ptr++) = c;
      nextChar(&c);
      for (power = 1.0; cType[c] == Digit; nextChar(&c)) {
	t->intVal = t->intVal*10 + (c - '0');
	power *= 10.0;
	*(txt_ptr++) = c;
      }
      t->dVal = (double)t->intVal / power;
      t->kind = FloatNum;
    } else {
      t->kind = IntNum;
    }
    *txt_ptr = '\0';
    notUseChar(c);
    break;
  case Dquote:
    for (nextChar(&c); c != '"' && c != EOF; nextChar(&c)) {
      if (txt_ptr - t->text < TOKEN_TXT_SIZ)
	*(txt_ptr++) = c;
      // TODO ; if length exceeds the limit, emit error
    }
    if (c != '"')
      error("end \" is missing");
    *txt_ptr = '\0';
    t->kind = String;
    t->intVal = txt_ptr - t->text;
    break;
  case Squote:
    // TODO : need to check how fgetc(FILE*) read escape. like '\' + 'n' or '\n';
    for (nextChar(&c); c != '\'' && c != EOF; nextChar(&c))
      *(txt_ptr++) = t->intVal = c;
    if (c != '\'')
      error("end \' is missing");
    if (txt_ptr - t->text > 1)
      error("Single quote must have single char");
    t->kind = CharSymbol;
    *txt_ptr = '\0';
    break;
  case Space: case Tab:
    while (c == ' ' || c == '\t') {
      *(txt_ptr++) = c;
      nextChar(&c);
    }
    *txt_ptr = '\0';
    t->intVal = txt_ptr - t->text;
    if (t->intVal > 1) {
      t->kind = Blanks;
    }
    notUseChar(c);
    break;
  default:
    // TODO : add more cases?
    *(txt_ptr++) = c;
    nextChar(&c); //to check 2 length operaiton
    if (is_ope2(txt_ptr-1, &c)) {
      *(txt_ptr++) = c;
      nextChar(&c); //to check only <"<=" >">="
      if (is_ope2(txt_ptr-1, &c)) {
	*(txt_ptr++) = c;
      } else {
	notUseChar(c);
      }
    } else {
      notUseChar(c);
    }
    *txt_ptr = '\0';
  }
  if (t->kind == NulKind)
    set_kind(t);
  if (t->kind == Others) {
    error("unknown token kind");
  } else {
    set_hKind(t);
  }
  return 1;
}

int checkNxtTokenKind(Kind k) {
  int from_q = t_buf_head != t_buf_tail;
  Token t = {NulKind, Specific, "", 0, 0.0};
  nextToken(&t, 0);
  if (from_q) {
    t_buf_head -= from_q;
  } else {
    t_buf_enqueue(t);
  }
  return t.kind == k;
}

int t_buf_enqueue(Token t) {
  // TODO : not good, workaround here
  if ((t_buf_tail + 1)%TOKEN_BUFFER_SIZ == t_buf_head) {
    error("token buffer overflow");
  }
  t_buf[t_buf_tail++] = t;
  t_buf_tail %= TOKEN_BUFFER_SIZ;
  return 1;
}

int t_buf_dequeue(Token *t) {
  if (t_buf_head == t_buf_tail) {
    error("token buffer is empty");
  }
  *t = t_buf[t_buf_head++];
  t_buf_head %= TOKEN_BUFFER_SIZ;
  return 1;
}

SymbolKind get_func_type() {
  // TODO : this might use many memory of t_buf
  //        in case of there are many arguments
  do {
    Token tmp = {NulKind, Specific, "", 0, 0.0};
    nextToken(&tmp, 1);
    t_buf_enqueue(tmp);
    if (tmp.kind == ')') {
      nextToken(&tmp, 1); //suspicious
      t_buf_enqueue(tmp);
      switch (tmp.kind) {
      case ';': // prototype
	return proto_ID;
      case '{':
	return func_ID; // function
      default:
	return no_ID; // TODO : syntax error
      }
    }
  } while (1);
  return no_ID;
}
