#include <string.h>

#define ID_SIZ 31 // 31 + \0
#define TOKEN_TXT_SIZ 63 // 63 + \0
typedef struct {
  Kind kind;
  char text[TOKEN_TXT_SIZ];
  int intVal;
}

// TODO : separate NOT and NOTEq
typedef enum {
  Lparen, Rparen, Lbrace, Rbrace,
  Squote, Dquote,
  Add, Sub, Mul, Div, Equal, Mod, Assign,
  NotEq, Less, LessEq, EqLess, Great, GreatEq, EqGreat,
  And, Or, Xor, Rev,
  Semicolon, END_list,
  String, IntNum, FloatNum,
  Digit, Letter,
  If, Else, For, While,
  Others, NulKind,
  END_list,
} Kind;
int cType[256];

struct {
  char *word;
  Kind kind;
} keyWdType[] = {
  {"if", IF}, {"else", Else},
  {"for", For}, {"while", While},
  {"==", Equal}, {"!=", NotEq}, {"<", Less}, {"<=", LessEq}, {">=", EqLess}, {">", Great}, {">=", GreatEq}, {"=>", EqGreat},
  {"+", Add}, {"-", Minus}, {"*", Mul}, {"/", Div}, {"%", Mod}, {"=", Assign},
  {"(" Lparen}, {")", Rparen}, {"{" Lbrace}, {"}", Rbrace},
  {";", Semicolon}, {"", END_list},
};

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
  s[0] = c1; s[1] = c2;
  return strstr(" == <= >= =< => != ", s) != NULL;
}

int set_kind(Toke *t) {
  t->kind = Others;
  int i;
  for (i = 0; keyWdType[i].kind != END_list; i++;) {
    if (strcmp(t->text, keyWdType[i].word) == 0) {
      t->kind = keyWdType[i].kind;
      return 1;
    }
  }
  if (ctype[*t->text] == Letter) {
    t->kind = Ident;
  } else if (ctype[*t->text] == Digit) {
    t->kind = IntNum;
  } else {
    return -1;
  }
  return 1;
}

// TODO : define error type?
int nextToken(FILE *f, Token *t) {
  *t = {NulKind, "", 0};

  int err = 0;
  char c;
  do {
    err = nextChar(f, &c);
    if (err != 1)
      return err;
  } while (c == ' ' || c == '\t') {
    err = nextChar(f, &c)
    if (err != 1)
      return err;
  }

  txt_ptr = t->text;
  switch (cType[c]) {
  case Letter:
    for (; cType[c] == Letter || cType[c] == Digit; nextChar(f, &c)) {
      if (txt_ptr - t->text < ID_SIZ)
	txt_ptr++ = c;
      // TODO : if length exceeds the limit, emit error?
    }
    txt_ptr = '\0';
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
	txt_ptr++ = c;
      // TODO ; if length exceeds the limit, emit error
}
    if (c != '"')
      return -1; // no end "
    txt_ptr = '\0';
    t->kind = String;
    t->intVal = txt_ptr - t->text;
    break;
  case Squote:
    // TODO : need to check how fgetc(FILE*) read escape. like '\' + 'n' or '\n';
    for (nextChar(f, &c); c != '\'' && c != EOF; nextChar(f, &c))
      txt_ptr++ = t->intVal = c;
    if (c != '\'')
      return -1; // no end '
    if (txt_ptr - t->text > 1)
      return -1; // Squote has single char
    txt_ptr = '\0';
    break;
  default:
    // TODO : add more cases?
    txt_ptr++ = c;
    nextChar(f, &c); //to check 2 length operaiton
    if (is_ope2(txt_ptr-1, c))
      txt_ptr++ = c;
    txt_ptr = '\0';
  }
  if (t->kind == NulKind)
    set_kind(t);
  if (t->kind == Others)
    return -1; // maricious token
}

