

typedef enum {
  Lparen, Rparen, Lbrace, Rbrace
  Plus, Minus, Multi, Div, Equal, Mod,
  Not, NotEq, Less, LessEq, Great, GreatEq,
  And, Or, Xor, Rev,
  Semicolon,
  String, Int, 
  Digit, Letter,
  If, Else, For, While,
  Others,
} Kind;
int cType[256];

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
  cType['+'] = Plus; cType['-'] = Minus; cType['*'] = Multi; cType['/'] = Div; cType['%'] = Mod;
  cType['!'] = Not; cType['='] = Equal; cType['<'] = Less; cType['>'] = Great;
  cType[';'] = Semicolon;

  cType['&'] = And; cType['|'] = Or; cType['~'] = Rev;

    
}

