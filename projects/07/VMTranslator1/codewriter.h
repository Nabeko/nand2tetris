#ifndef __CODE_WRITER_H__
#define __CODE_WRITER_H__

#include <stdio.h>
#include <string.h>

#define VM_EXTENSION  ".vm"
#define ASM_EXTENSION ".asm"

char *CheckFileExtension(char *);
char *CreateOutfile(char *);
FILE* SetFileName(char*);
void WriteArithmetic(FILE*, char*);
void WritePushPop(FILE*, short, char*, int);
void Close(FILE*);

typedef struct opMapping{
  char op[16];
  char symbol[8];
} OpMapping;

#endif /* __CODE_WRITER_H__ */
