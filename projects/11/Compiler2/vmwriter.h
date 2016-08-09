#ifndef __VM_WRITER_H__
#define __VM_WRITER_H__

#include <stdio.h>

#define JACK_EXTENSION        ".jack"
#define VM_EXTENSION          ".vm"

char *CheckFileExtension(char *, char *);
FILE* VMWriter(char*);
void WritePush(FILE*, char*, int);
void WritePop(FILE*, char*, int);
void WriteArithmetic(FILE*, char*);
void WriteLabel(FILE*, char*);
void WriteGoto(FILE*, char*);
void WriteIf(FILE*, char*);
void WriteCall(FILE*, char*, int);
void WriteFunction(FILE*, char*, int);
void WriteReturn(FILE*);
void Close(FILE*);

#endif /* __VM_WRITER_H__ */
