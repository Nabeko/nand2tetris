#include "symboltable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SymbolTable predefinedTable[] = {
//  symbol  , address
   {"R0"    ,    0},
   {"R1"    ,    1},
   {"R2"    ,    2},
   {"R3"    ,    3},
   {"R4"    ,    4},
   {"R5"    ,    5},
   {"R6"    ,    6},
   {"R7"    ,    7},
   {"R8"    ,    8},
   {"R9"    ,    9},
   {"R10"   ,   10},
   {"R11"   ,   11},
   {"R12"   ,   12},
   {"R13"   ,   13},
   {"R14"   ,   14},
   {"R15"   ,   15},
   {"SCREEN",16384},
   {"KBD"   ,24576},
   {"SP"    ,    0},
   {"LCL"   ,    1},
   {"ARG"   ,    2},
   {"THIS"  ,    3},
   {"THAT"  ,    4},
};

static int count = 0;

void AddEntry(char* symbol, int address, SymbolTable **symbolTables)
{
  static int currentVarAddress = VAR_START;
  SymbolTable *currentSymbol = NULL;

  if(GetAddress(symbol, symbolTables) != -1)
    return;

  if(address == VARIABLE){ // it is a variable, not a Label
    address = currentVarAddress;
    currentVarAddress++;
  }

  currentSymbol = *symbolTables + count;
  memset (currentSymbol,0,sizeof(SymbolTable));
  strcpy(currentSymbol->symbol, symbol);
  currentSymbol->address = address;
  count++;
}

// return address if found, or return -1, if not found : contains()
int GetAddress(char* symbol, SymbolTable **symbolTables)
{
  SymbolTable *currentSymbol = NULL;
  int i;

  int preTableCount = sizeof(predefinedTable) / sizeof(SymbolTable);
  for(i=0; i < preTableCount; i++){
    if(strcmp(symbol, predefinedTable[i].symbol) == 0 )
        return predefinedTable[i].address;
  }

  if(count == 0)
    return -1;
  
  for(i=0; i < count; i++){
    currentSymbol = *symbolTables + i;
    if(strcmp(symbol, currentSymbol->symbol) == 0 )
        return currentSymbol->address;
  }

  return -1;
}

void Show(SymbolTable **symbolTables)
{
  SymbolTable *currentSymbol = NULL;
  int i;

  for(i=0; i < count; i++){
    currentSymbol = *symbolTables + i;
    printf("%s,%d\n",currentSymbol->symbol, currentSymbol->address);
  }
}
