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

static int isArithmetic(char* op)
{
  char* arithmetic[] = { "add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"};
  int count = sizeof(arithmetic) / sizeof(char*);
  int i;

  for( i = 0; i < count; i++){
    if(strcmp(op, arithmetic[i]) == 0 )
      return C_ARITHMETIC;
  }

  return -1;
}

static int isOneArg(char* op)
{
  char* arg[] = { "label", "goto", "if-goto" };
  int count = sizeof(arg) / sizeof(char*);
  int i;
  
  for( i = 0; i < count; i++){
    if(strcmp(op, arg[i]) == 0 )
      return (i + C_LABEL);
  }
  
  return -1;
}

static int isTwoArg(char* op)
{
  char* arg[] = { "push", "pop", "function", "call" };
  int count = sizeof(arg) / sizeof(char*);
  int i;
  
  for( i = 0; i < count; i++){
    if(strcmp(op, arg[i]) == 0 )
      return (i + C_PUSH);
  }
  
  return -1;
}

static int isReturn(char* op)
{
  if(strcmp(op, "return") == 0 ) {
      return C_RETURN;
  }
  
  return -1;
}

void Show(Command **commands, int count)
{
  Command *currentCommand = NULL;
  int i;
  
  for ( i = 0; i < count; i++){
    currentCommand = *commands + i;
	printf("Command Type = %d, arg1 = %s, arg2 = %ld\n", 
            currentCommand->commandType,
            currentCommand->arg1,
            currentCommand->arg2);
  }
}

int Parse(FILE *vmFile, Command **commands)
{
  char buf[128]; // buffer for each line
  char *temp = NULL; // temp for the each instruction
  int count=0; // count of instructions
  int i; // array index
  Command *currentCommand = NULL;
  int commandType = -1;
  char *arg;
  char *delim=" ";

  while(!feof(vmFile)) { // hasMoreCommands
    if(fgets(buf,128,vmFile)!=NULL) { // advance
      // remove the comments on each line
      temp = TrimWhitespace(RemoveComments(buf));
      // if *temp is 0, it must be a space line 
	  if(*temp != 0) {
        arg = strtok(temp, delim); // get first element 

        if(arg != NULL) {
          currentCommand = *commands + count;
          memset(currentCommand, 0, sizeof(Command));

          if( (commandType = isArithmetic(arg)) != -1 ) {   // is arithmetic operation
            strcpy(currentCommand->arg1, arg);
		  } else if( (commandType = isOneArg(arg)) != -1) { // got one argument
		    arg = strtok(NULL, delim);
            strcpy(currentCommand->arg1, arg);
          } else if ( (commandType = isTwoArg(arg)) != -1) { // got two argument
            arg = strtok(NULL, delim);
            strcpy(currentCommand->arg1, arg);
            arg = strtok(NULL, delim);
            currentCommand->arg2 = atol(arg);
          } else if ( (commandType = isReturn(arg)) != -1) { // return
            strcpy(currentCommand->arg1, "");
          } else {
            continue;
          }

          currentCommand->commandType = commandType;
          count++;
        }
      }
    }
  }

  return count;
}
