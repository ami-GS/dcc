#ifndef DCC_LETTER_ANALYSIS_H_
#define DCC_LETTER_ANALYSIS_H_

#include <stdio.h>
#include "symbol_table.h"

#define ID_SIZ 31 // 31 + \0
#define TOKEN_TXT_SIZ 63 // 63 + \0

// TODO : separate NOT and NOTEq
typedef enum {
  NulKind, Others, Ident,
  Lparen, Rparen, Lbrace, Rbrace, Lbracket, Rbracket,
  Squote, Dquote, Comma, Sharp,
  Define, Include, Elif, Ifdef, Ifndef, Endif,
  Add, Sub, Mul, Div, Equal, Mod, Assign, Not,
  Incre, Decre,
  AddAss, SubAss, MulAss, DivAss, ModAss,
  BandAss, BorAss, BxorAss, BnotAss, LsftAss, RsftAss,
  NotEq, Less, LessEq, EqLess, Great, GreatEq, EqGreat,
  And, Or, Rev,
  Band, Bor, Bxor, Bnot, Lshift, Rshift,
  Int, Float, Double, Char, Void,
  Colon, Semicolon,
  String, IntNum, FloatNum, CharSymbol,
  Digit, Letter, VarName,
  If, Else, For, Do, While,
  Switch, Case, Default,
  Break, Continue, Return,
  Printf, Exit, Input,
  LComment, MLCommS, MLCommE,
  Space, Tab, NewLine, Blanks,
  EOF_token,
  END_list,
} Kind;
int cType[256];

typedef struct {
  Kind kind;
  char text[TOKEN_TXT_SIZ];
  int intVal;
  double dVal;
} Token;

typedef struct {
    char *word;
    Kind kind;
} KeyWordType;

static KeyWordType keyWdType[] = {
  {"void", Void}, {"int", Int}, {"char", Char}, {"float", Float}, {"double", Double},
  {"if", If}, {"else", Else},
  {"for", For}, {"while", While}, {"do", Do},
  {"switch", Switch}, {"case", Case}, {"continue", Continue},
  {"default", Default}, {"break", Break},
  {"define", Define}, {"include", Include}, {"elif", Elif}, {"ifdef", Ifdef}, {"ifndef", Ifndef}, {"endif", Endif},
  {"return", Return},
  {"//", LComment}, {"/*", MLCommS}, {"*/", MLCommE},
  {"<<", Lshift}, {">>", Rshift}, {"<<=", LsftAss}, {">>=", RsftAss},
  {"==", Equal}, {"!=", NotEq}, {"<", Less}, {"<=", LessEq}, {"=<", EqLess}, {">", Great}, {">=", GreatEq}, {"=>", EqGreat},
  {"+=", AddAss}, {"-=", SubAss}, {"*=", MulAss}, {"/=", DivAss}, {"%=", ModAss},
  {"++", Incre}, {"--", Decre}, {"&&", And}, {"||", Or},
  {"+", Add}, {"-", Sub}, {"*", Mul}, {"/", Div}, {"%", Mod}, {"=", Assign},
  {"&", Band}, {"|", Bor}, {"^", Bxor}, {"~", Bnot}, {"#", Sharp},
  {"&=", BandAss}, {"|=", BorAss}, {"^=", BxorAss},
  {"(", Lparen}, {")", Rparen}, {"{", Lbrace}, {"}", Rbrace}, {"[", Lbracket}, {"]", Rbracket},
  {",", Comma}, {":", Colon}, {";", Semicolon},
  {" ", Space}, {"\t", Tab}, {"\n", NewLine},  {"", END_list},
};

#define TOKEN_BUFFER_SIZ 1000
Token t_buf[TOKEN_BUFFER_SIZ];
static int t_buf_head = 0, t_buf_tail = 0;
extern int t_buf_open; // TODO : workaround
int t_buf_enqueue(Token t);
int t_buf_dequeue(Token *t);
extern int use_all_as_token;
#define STREAM_SIZE 8
extern FILE *streams[STREAM_SIZE];
extern char fileNames[STREAM_SIZE][80];
extern char  streamRW[STREAM_SIZE];
extern int streamNest_ct;
extern int currentLines[STREAM_SIZE];

// TODO : research about function name decisionning
int fOpen(char *fname, char *RW);
int fClose();
void initKind();
int nextChar(char *c);
int is_ope2(const char *c1, const char *c2);
int is_ope1(Kind k);
int set_kind(Token *t);
int nextToken(Token *t, int q_lock);
int checkNxtTokenKind(Kind k);
SymbolKind get_func_type();
static int prevC = -2;
void notUseChar(char c);

#endif // DCC_LETTER_ANALYSIS_H_
