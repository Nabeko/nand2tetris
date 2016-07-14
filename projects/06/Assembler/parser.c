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

int parse(FILE *asmFile, Instruction **instructions)
{
  char buf[128]; // buffer for each line
  char *temp = NULL; // temp for the each instruction
  char *symbol = NULL; // symbol address for C command
  int count=0; // count of instructions
  Instruction *currentInstruction = NULL;

  // allocate memory for instructions array
  *instructions = (Instruction*)malloc(sizeof(Instruction));

  while(!feof(asmFile)) {
    if(fgets(buf,128,asmFile)!=NULL) {
      // remove the comments and white space on each line
      temp = TrimWhitespace(RemoveComments(buf));
      // if *temp is 0, it must be a space line 
      if(*temp != 0) {
        // change the instructions array dynamically 
        if (count > 0) {
          *instructions = (Instruction*)realloc(*instructions, (count+1)*sizeof(Instruction));
        }

        currentInstruction = *instructions+count;
		memset (currentInstruction,0,sizeof(Instruction));
        if(*temp == '@') { // A Command
          currentInstruction->commandType = A_COMMAND;
          strcpy(currentInstruction->symbol, temp+1);
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

  return count;
}
