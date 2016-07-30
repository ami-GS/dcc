#ifndef DCC_LETTER_ANALYSIS_H_
#define DCC_LETTER_ANALYSIS_H_

#include <stdio.h>

#define ID_SIZ 31 // 31 + \0
#define TOKEN_TXT_SIZ 63 // 63 + \0
static FILE *fin;

// TODO : separate NOT and NOTEq
typedef enum {
  Lparen, Rparen, Lbrace, Rbrace,
  Squote, Dquote,
  Add, Sub, Mul, Div, Equal, Mod, Assign, Not,
  NotEq, Less, LessEq, EqLess, Great, GreatEq, EqGreat,
  And, Or, Xor, Rev,
  Int, Void,
  Semicolon,
  String, IntNum, FloatNum,
  Digit, Letter, VarName,
  If, Else, For, While,
  Others, NulKind, Ident,
  EOF_token,
  END_list,
} Kind;
int cType[256];

typedef struct {
  Kind kind;
  char text[TOKEN_TXT_SIZ];
  int intVal;
} Token;

typedef struct {
    char *word;
    Kind kind;
} KeyWordType;

KeyWordType keyWdType[] = {
  {"if", If}, {"else", Else},
  {"for", For}, {"while", While},
  {"==", Equal}, {"!=", NotEq}, {"<", Less}, {"<=", LessEq}, {">=", EqLess}, {">", Great}, {">=", GreatEq}, {"=>", EqGreat},
  {"+", Add}, {"-", Sub}, {"*", Mul}, {"/", Div}, {"%", Mod}, {"=", Assign},
  {"(", Lparen}, {")", Rparen}, {"{", Lbrace}, {"}", Rbrace},
  {";", Semicolon}, {"", END_list},
};

#define TOKEN_BUFFER_SIZ 10
Token t_buf[TOKEN_BUFFER_SIZ];
int t_buf_ptr;
// TODO : research about function name decisionning
int fOpen(char *fname);
void initKind();
int nextChar(char *c);
int is_ope2(const char *c1, const char *c2);
int set_kind(Token *t);
int nextToken(Token *t);
int checkNxtTokenKind(Kind k);

#endif // DCC_LETTER_ANALYSIS_H_
