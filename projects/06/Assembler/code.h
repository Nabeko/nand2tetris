#ifndef __CODE_H__
#define __CODE_H__

#define DEST 0
#define COMP 1
#define JUMP 2

typedef struct SymbolicToBinary{
  char* symbolic;
  char* binary;
} SymbolicToBinary;

char *Dest(char*);
char *Comp(char*);
char *Jump(char*);

#endif /* __CODE_H__ */
