#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#define MAX_SYMBOL_NUM   1000

#define STATIC              0
#define FIELD               1
#define VAR                 2
#define ARG                 3
#define NONE               -1

typedef struct symbol {
  char name[32];
  char type[32];
  short kind;
  short index;
} Symbol;

void SymbolTable(Symbol **, Symbol **);
void StartSubroutine(void);
void Define(char*, char*, short);
int VarCount(short);
short KindOf(char*);
char* TypeOf(char*);
int IndexOf(char*);
#endif /* __SYMBOL_TABLE_H__ */
