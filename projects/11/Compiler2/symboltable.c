#include "symboltable.h"
#include <string.h>

#define TRUE   0
#define FALSE -1

Symbol *gClassScopeTable = NULL;
Symbol *gSubroutineScopeTable = NULL;

int gClassSymbolCount = 0;
int gSubroutineSymbolCount = 0;

void SymbolTable(Symbol **classScopeTable, Symbol **subroutineScopeTable){
  gClassScopeTable = *classScopeTable;
  gSubroutineScopeTable = *subroutineScopeTable;
}

// Starts a new subroutine scope
// erases all names in the previous subroutine’s scope.
void StartSubroutine(){
  memset(gSubroutineScopeTable, 0, MAX_SYMBOL_NUM);
  gSubroutineSymbolCount = 0;
}

// Defines a new identifier of a given name, type,
// and kind and assigns it a running index.
void Define(char* name, char* type, short kind){
  Symbol *currentSymbol = NULL;
  int *symbolCount = NULL;

  if( kind <= FIELD ) { // class
    currentSymbol = gClassScopeTable + gClassSymbolCount;
    symbolCount = &gClassSymbolCount;
  } else if ( kind >= VAR ) { // subroutine
    currentSymbol = gSubroutineScopeTable + gSubroutineSymbolCount;
    symbolCount = &gSubroutineSymbolCount;
  }
  strcpy(currentSymbol->name, name);
  strcpy(currentSymbol->type, type);
  currentSymbol->kind = kind;
  currentSymbol->index = VarCount(kind);
  (*symbolCount)++;
}

// Returns the number of variables of the given kind
// already defined in the current scope.
int VarCount(short kind){
  Symbol *currentSymbol = NULL;
  int *symbolCount = NULL;
  int kindCount = 0;
  int i; // index

  if( kind <= FIELD ) { // class
    currentSymbol = gClassScopeTable;
    symbolCount = &gClassSymbolCount;
  } else if ( kind >= VAR ) { // subroutine
    currentSymbol = gSubroutineScopeTable;
    symbolCount = &gSubroutineSymbolCount;
  }

  for ( i = 0; i < *symbolCount; currentSymbol++, i++) {
      if (kind == currentSymbol->kind) {
          kindCount++;
	  }
  }
  return kindCount;
}

// find the symbol by the given name
static Symbol *findSymbol(char* name){
  Symbol *currentSymbol = NULL;
  int *symbolCount = NULL;
  int i;

  currentSymbol = gSubroutineScopeTable;
  symbolCount = &gSubroutineSymbolCount;
  for ( i = 0; i < *symbolCount; currentSymbol++, i++) {
      if (strcmp(currentSymbol->name, name) == TRUE) {
          return currentSymbol;
	  }
  }
  
  currentSymbol = gClassScopeTable;
  symbolCount = &gClassSymbolCount;
  for ( i = 0; i < *symbolCount; currentSymbol++, i++) {
      if (strcmp(currentSymbol->name, name) == TRUE) {
          return currentSymbol;
	  }
  }

  return NULL;
}

// Returns the kind of the named identifier in the current scope.
// Returns NONE if the identifier is unknown in the current scope.
short KindOf(char* name){
  Symbol *currentSymbol = findSymbol(name);
  if(currentSymbol != NULL)
      return currentSymbol->kind;
  return NONE;
}

// Returns the type of the named identifier in the current scope.
char* TypeOf(char* name){
  Symbol *currentSymbol = findSymbol(name);
  if(currentSymbol != NULL)
      return currentSymbol->type;
  return NULL;
}

// Returns the index assigned to named identifier.
int IndexOf(char* name){
  Symbol *currentSymbol = findSymbol(name);
  if(currentSymbol != NULL)
      return currentSymbol->index;
  return -1;
}
