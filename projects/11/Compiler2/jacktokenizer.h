#ifndef __JACK_TOKENIZER_H__
#define __JACK_TOKENIZER_H__

#include <stdio.h>

#define MAX_TOKEN_NUM   50000

#define KEYWORD             0
#define SYMBOL              1
#define IDENTIFIER          2
#define INT_CONST           3
#define STRING_CONST        4


typedef struct token{
  short tokenType;
  char  keyWord[8];      // tokenType is KEYWORD
  char  symbol;          // tokenType is SYMBOL
  char  identifier[32];  // tokenType is IDENTIFIER
  int   intVal;          // tokenType is INT_CONST
  char  stringVal[512];  // tokenType is STRING_CONST
} Token;

int JackTokenizer(FILE*, Token **);
void JackTokenOutput(FILE*, Token **, int);
#endif /* __JACK_TOKENIZER_H__ */
