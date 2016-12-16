#ifndef DCC_LETTER_ANALYSIS_H_
#define DCC_LETTER_ANALYSIS_H_

#include <stdio.h>
#include "symbol_table.h"

#define ID_SIZ 31 // 31 + \0
#define TOKEN_TXT_SIZ 63 // 63 + \0

// TODO : separate NOT and NOTEq
typedef enum {
  Lparen = '(', Rparen = ')', Lbrace = '{', Rbrace = '}', Lbracket = '[', Rbracket = ']',
  Squote = '\'', Dquote = '\"', Comma = ',', Sharp = '#',
  Add = '+', Sub = '-', Mul = '*', Div = '/', Mod = '%', Assign = '=', Not = '!',
  Dot = '.', Band = '&', Bor = '|', Bxor = '^', Bnot = '~',
  Less = '<', Great = '>',
  Colon = ':', Semicolon = ';',
  Space = ' ', Tab = '\t', NewLine = '\n', Blanks,
  NulKind = 150, Others, Ident,
  Define, Include, Elif, Ifdef, Ifndef, Endif,                                            // 153 - 158
  Incre = 180, Decre, And, Or, Lshift, Rshift,                                            // 180 - 185
  Equal, NotEq, LessEq, EqLess, GreatEq, EqGreat, Arrow,
  AddAss, SubAss, MulAss, DivAss, ModAss,
  BandAss, BorAss, BxorAss, BnotAss, LsftAss, RsftAss,
  Int = 210, Short, Float, Double, Char, Void,                                                   // 210 - 214
  IntP, ShortP, FloatP, DoubleP, CharP, VoidP,
  Struct, Static, Const, Union, Typedef, Extern,
  String = 230, IntNum, FloatNum, CharSymbol,                                             // 230 - 233
  Digit, Letter, VarName,
  If = 250, Else, For, Do, While,                                                         // 250
  Switch, Case, Default,
  Break, Continue, Return,
  Printf, Exit, Input,                                                                    //     - 263
  LComment, MLCommS, MLCommE,
  EOF_token,
  END_list,
} Kind;
int cType[256];

typedef enum {
    Specific, Immediate, Function, Operator, CombOpe, Statement, LParens, RParens, Type, Modifier,
} HighLevelKind;

typedef struct {
  Kind kind;
  HighLevelKind hKind;
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
  {"void*", VoidP}, {"int*", IntP}, {"char*", CharP}, {"float*", FloatP}, {"double*", DoubleP},
  {"struct", Struct}, {"static", Static}, {"const", Const}, {"union", Union},  {"typedef", Typedef}, {"extern", Extern},
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
  {"&=", BandAss}, {"|=", BorAss}, {"^=", BxorAss}, {".", Dot}, {"->", Arrow},
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
