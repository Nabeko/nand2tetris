#ifndef __CODE_WRITER_H__
#define __CODE_WRITER_H__

#include <stdio.h>
#include <string.h>

#define VM_EXTENSION  ".vm"
#define ASM_EXTENSION ".asm"

char *CheckFileExtension(char *);

FILE* Open(char*, char*);
void Close(FILE*);

void SetFileName(char*);
void WriteInit(FILE*);
void WriteArithmetic(FILE*, char*);
void WriteLabel(FILE*, char*);
void WriteGoTo(FILE*, char*);
void WriteIf(FILE*, char*);
void WritePushPop(FILE*, short, char*, int);
void WriteFunction(FILE*, char*, int);
void WriteCall(FILE*, char*, int);
void WriteReturn(FILE*);

typedef struct opMapping{
  char op[16];
  char symbol[8];
} OpMapping;

#endif /* __CODE_WRITER_H__ */
