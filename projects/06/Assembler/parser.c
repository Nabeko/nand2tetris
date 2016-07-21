#include "parser.h"
#include <stdlib.h>
#include <string.h>

static char *TrimWhitespace(char *str)
{
  char *end;

  // trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // all spaces?
    return str;

  // trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // write new null terminator
  *(end+1) = 0;

  return str;
}

static char *RemoveComments(char *str)
{
  char *end;

  // Find the comment start
  end = strstr(str, "//");

  // if found, set it to null terminator
  if(end)
  	*(end) = 0;

  return str;
}

static int IsDecimalOnly(const char *str)
{
  while(*str) {
    if (isdigit(*str++) == 0) return 0;
  }

  return -1;
}

int Parse(FILE *asmFile, Instruction **instructions, SymbolTable **symbolTables)
{
  char buf[128]; // buffer for each line
  char *temp = NULL; // temp for the each instruction
  char *symbol = NULL; // symbol address for C command
  int count=0; // count of instructions
  int i; // array index
  Instruction *currentInstruction = NULL;

  while(!feof(asmFile)) { // hasMoreCommands
    if(fgets(buf,128,asmFile)!=NULL) { // advance
      // remove the comments and white space on each line
      temp = TrimWhitespace(RemoveComments(buf));
      // if *temp is 0, it must be a space line 
      if(*temp != 0) {
        if(*temp == '(') { // Label
          symbol = temp + 1;
		  symbol[strcspn(symbol,")")] = '\0';
          AddEntry(symbol, count, symbolTables); // Add Label to Symbol Table
          continue;
        }

        currentInstruction = *instructions+count;
		memset(currentInstruction, 0, sizeof(Instruction));
        if(*temp == '@') { // A Command or L COMMAND
          strcpy(currentInstruction->symbol, temp+1);
          if(IsDecimalOnly(currentInstruction->symbol)) {
            currentInstruction->commandType = A_COMMAND;
          } else {
            currentInstruction->commandType = L_COMMAND;
          }
        } else { // C Command dest=comp;jump
          currentInstruction->commandType = C_COMMAND;

          symbol = strchr(temp, '='); // check if '=' is existed
          if(symbol != 0) { // if '=' is existed, copy L value to dest, change temp to be the remainder 
            strncpy(currentInstruction->dest, temp, strcspn(temp,"="));  
            temp = symbol+1;
          } else { // else, dest is empty
            strcpy(currentInstruction->dest, "");
		  }

          symbol = strchr(temp, ';'); // check if ';' is existed
          if(symbol != 0) { // if ';' is existed, copy L value to comp, R value to jump
            strncpy(currentInstruction->comp, temp, strcspn(temp,";"));
            strcpy(currentInstruction->jump, symbol+1);
          } else { // since ';' is not existed, there is no jump op, and the remainder is comp op only
            strcpy(currentInstruction->comp, temp);
			strcpy(currentInstruction->jump, "");
		  }
        }
        count++;
      }
    }
  }

  // Add variables to symbol table
  for( i = 0; i < count; i++ ){
    currentInstruction = *instructions + i;
    if( currentInstruction->commandType == L_COMMAND ){
      AddEntry(currentInstruction->symbol, VARIABLE, symbolTables);
    }
  }

  return count;
}
