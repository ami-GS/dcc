#ifndef _DCC_PREPROCESSOR_H_
#define _DCC_PREPROCESSOR_H_

#include <stdio.h>
#include "letter_analysis.h"

#define LINE_LENGTH 512
#define MAX_DEFINE_LENGTH 128
#define MAX_DEFINE_ARG_NUM 16
#define MAX_DEFINE_ARG_LEN 14

typedef enum {
    INCLUDE,
    DEFINE,
} DIRECTIVE;

typedef struct {
    char n_bef[MAX_DEFINE_LENGTH]; // #define "DEF_NAME"(a, b, c) a+b+c
    char n_af[MAX_DEFINE_LENGTH];  // #define DEF_NAME(a, b, c) "a+b+c" => "{0}+{1}+{2}"
    char arg_table[MAX_DEFINE_ARG_NUM][MAX_DEFINE_LENGTH];
    int  arg_locations[MAX_DEFINE_ARG_NUM]; // location of '{' in n_af
    char argNum;
} define_item;
static define_item define_table[256];
static int def_table_ct = 0;

#define MAX_IF_NEST 16
typedef struct {
    int has_true;
    int if_type; // 0:none, 1:#if, 2:#if(n)def
    int ignore;
} if_nest_item;
static if_nest_item if_nest_table[MAX_IF_NEST];
static int if_nest_ct = 0;

#define MAX_LINE_SIZE 512

void  pre_define(Token *t);
int replace_def(Token *t, int save);
int replace_com(Token *t);
int search_def(char *text);
void enter_define();
char *fOpen_i(char *fname);
char *preprocess(char *fname);
void preprocess_sub();
int wrapNext(Token *t, int save);


#endif //_DCC_PREPROCESSOR_H_
