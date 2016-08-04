#ifndef __COMPILATION_ENGINE_H__
#define __COMPILATION_ENGINE_H__

#include <stdio.h>
#include "jacktokenizer.h"

#define TRUE   0
#define FALSE -1

typedef struct statementMapping{
  char statement[8];
  void (*statementFunction)(FILE*, Token **, int *, int *, int);
} StatementMapping;

void CompileClass(FILE*, Token **, int);
#endif /* __COMPILATION_ENGINE_H__ */
