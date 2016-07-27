#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>

#define MAX_COMMAND_NUM 50000

#define C_ARITHMETIC        0

#define C_LABEL             1
#define C_GOTO              2
#define C_IF                3

#define C_PUSH              4
#define C_POP               5
#define C_FUNCTION          6
#define C_CALL              7

#define C_RETURN            8


typedef struct command{
  short commandType;
  char arg1[8];
  long arg2;
} Command;

int Parse(FILE*, Command **);
void Show(Command **, int);
#endif /* __PARSER_H__ */
