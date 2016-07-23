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

