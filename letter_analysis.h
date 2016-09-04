#ifndef DCC_LETTER_ANALYSIS_H_
#define DCC_LETTER_ANALYSIS_H_

#include <stdio.h>
#include "symbol_table.h"

#define ID_SIZ 31 // 31 + \0
#define TOKEN_TXT_SIZ 63 // 63 + \0
static FILE *fin;

// TODO : separate NOT and NOTEq
typedef enum {
  NulKind, Others, Ident,
  Lparen, Rparen, Lbrace, Rbrace, Lbracket, Rbracket,
  Squote, Dquote, Comma,
  Add, Sub, Mul, Div, Equal, Mod, Assign, Not,
  Incre, Decre,
  NotEq, Less, LessEq, EqLess, Great, GreatEq, EqGreat,
  And, Or, Xor, Rev,
  Int, Void,
  Semicolon,
  String, IntNum, FloatNum,
  Digit, Letter, VarName,
  If, Else, For, Do, While,
  Switch, Case, Default,
  Break, Continue, Return,
  Printf, Exit, Input,
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

static KeyWordType keyWdType[] = {
  {"void", Void}, {"int", Int},
  {"if", If}, {"else", Else},
  {"for", For}, {"while", While},
  {"return", Return},
  {"==", Equal}, {"!=", NotEq}, {"<", Less}, {"<=", LessEq}, {">=", EqLess}, {">", Great}, {">=", GreatEq}, {"=>", EqGreat},
  {"+", Add}, {"-", Sub}, {"*", Mul}, {"/", Div}, {"%", Mod}, {"=", Assign},
  {"(", Lparen}, {")", Rparen}, {"{", Lbrace}, {"}", Rbrace}, {"[", Lbracket}, {"]", Rbracket},
  {";", Semicolon}, {"", END_list},
};

#define TOKEN_BUFFER_SIZ 1000
Token t_buf[TOKEN_BUFFER_SIZ];
static int t_buf_head = 0, t_buf_tail = 0;
static int t_buf_open = 0; // TODO : workaround
int t_buf_enqueue(Token t);
int t_buf_dequeue(Token *t);

// TODO : research about function name decisionning
int fOpen(char *fname);
void initKind();
int nextChar(char *c);
int is_ope2(const char *c1, const char *c2);
int is_binaryOP(Kind k);
int set_kind(Token *t);
int nextToken(Token *t);
int checkNxtTokenKind(Kind k);
SymbolKind get_func_type();
static int prevC = -1;
void notUseChar(char c);

#endif // DCC_LETTER_ANALYSIS_H_
