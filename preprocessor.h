#ifndef _DCC_PREPROCESSOR_H_
#define _DCC_PREPROCESSOR_H_

#include <stdio.h>
#include "letter_analysis.h"

#define LINE_LENGTH 512
#define MAX_DEFINE_LENGTH 128
#define MAX_DEFINE_ARG_NUM 16
#define MAX_DEFINE_ARG_LEN 14

static FILE *i_file; // for .i file
char *i_fname[126];

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


void  pre_define(Token *t);
int replace_def(Token *t);
int replace_com(Token *t);
int search_define();
void enter_define();
char *fOpen_i(char *fname);
char *preprocess(char *fname);
int wrapNext(Token *t, int save);


#endif //_DCC_PREPROCESSOR_H_
