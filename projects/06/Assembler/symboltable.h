#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#define VAR_START              16
#define VARIABLE               -1
#define MAX_SYMBOL_NUM      50000

typedef struct symbolTable{
  char symbol[64];
  int address;
} SymbolTable;

void AddEntry(char* ,int, SymbolTable **);
int GetAddress(char*, SymbolTable **);
void Show(SymbolTable **);
#endif /* __SYMBOL_TABLE_H__ */
