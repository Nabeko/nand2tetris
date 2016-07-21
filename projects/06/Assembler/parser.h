#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>
#include "symboltable.h"

#define A_COMMAND 0
#define C_COMMAND 1
#define L_COMMAND 2

#define MAX_INSTRUCTION_NUM 50000

typedef struct instruction{
  short commandType;
  char symbol[8];
  char dest[8];
  char comp[8];
  char jump[8];
} Instruction;

int Parse(FILE*, Instruction **, SymbolTable **);

#endif /* __PARSER_H__ */
