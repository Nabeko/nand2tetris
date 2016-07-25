#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "codewriter.h"

int main(int argc, char* argv[])
{

  FILE *inP, *outP; // input and output file
  Command *vmCommands; // all VM Commands
  int count=0; // count of instructions
  int i; // loop index
  Command *currentCommand = NULL;
  
  if(argc != 2) {
    perror("Usage : VMTranslator <filename>");
    return -1;
  }
  
  // make surce the file extension is .vm
  if(!CheckFileExtension(argv[1])) {
    perror("File extension must be .vm");
    return -1;  
  }
  
  vmCommands = (Command*)malloc(sizeof(Command) * MAX_COMMAND_NUM);

  // open the input file
  inP = fopen(argv[1], "r");
  if(!inP) {
    perror("File open failed!");
    goto failed;
  }
  
  count = Parse(inP, &vmCommands);
  //Show(&vmCommands, count);
  fclose(inP);

  // create the output file
  outP = SetFileName(argv[1]);
  if(!outP) {
    perror("File open failed!");
    goto failed;
  }


  for( i = 0; i < count; i++) {
    currentCommand = vmCommands + i;
    switch(currentCommand->commandType){
      case C_ARITHMETIC:
	       WriteArithmetic(outP, currentCommand->arg1);
           break;
      case C_PUSH:
      case C_POP:
           WritePushPop(outP, currentCommand->commandType, currentCommand->arg1, currentCommand->arg2);
           break;
      default:
           break;
	}
  }

  Close(outP);

failed:
  free(vmCommands);

  return 0;
}
