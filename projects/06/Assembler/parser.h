#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>

#define A_COMMAND 0
#define C_COMMAND 1

typedef struct instruction{
  short commandType;
  char symbol[8];
  char dest[8];
  char comp[8];
  char jump[8];
} Instruction;

int parse(FILE*, Instruction **);

#endif /* __PARSER_H__ */
