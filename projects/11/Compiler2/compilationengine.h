#ifndef __COMPILATION_ENGINE_H__
#define __COMPILATION_ENGINE_H__

#include <stdio.h>
#include "jacktokenizer.h"

typedef struct statementMapping{
  char statement[8];
  void (*statementFunction)(FILE*, Token**, int*, int, char*);
} StatementMapping;

void CompileClass(char*, Token **, int);
#endif /* __COMPILATION_ENGINE_H__ */
