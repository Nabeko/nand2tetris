#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <libgen.h>
#include "parser.h"
#include "codewriter.h"

int main(int argc, char* argv[])
{
  struct dirent *pDirent;
  DIR *pDir;
  char *dirName;
  char inputFilename[128];

  FILE *inP, *outP; // input and output file
  Command *vmCommands; // all VM Commands
  int count=0; // count of instructions
  int i; // loop index
  Command *currentCommand = NULL;
  
  if(argc != 2) {
    printf("Usage : VMTranslator <directory>");
    return -1;
  }
  
  dirName = argv[1];

  // open directory  
  pDir = opendir (dirName);
  if(pDir == NULL) {
    printf("Can not open directory '%s'\n", dirName);
  }

  vmCommands = (Command*)malloc(sizeof(Command) * MAX_COMMAND_NUM);

  // create the output file
  outP = Open(dirName, basename(dirName));
  if(!outP) {
    printf("File open failed!\n");
    goto failed;
  }
  WriteInit(outP);

  
  while ((pDirent = readdir(pDir)) != NULL) {
    // only open the *.vm files in directory
    if( CheckFileExtension(pDirent->d_name) ) {
        sprintf(inputFilename, "%s/%s", dirName, pDirent->d_name);
        // open the input file
        inP = fopen(inputFilename, "r");
        if(!inP) {
          printf("File '%s' open failed!\n", inputFilename);
          continue;
        }

        SetFileName(basename(inputFilename));

        memset(vmCommands, 0, MAX_COMMAND_NUM);

		count = Parse(inP, &vmCommands);
        for( i = 0; i < count; i++) {
           currentCommand = vmCommands + i;
           switch(currentCommand->commandType){
              case C_ARITHMETIC:
	               WriteArithmetic(outP, currentCommand->arg1);
                   break;
              case C_LABEL:
                   WriteLabel(outP, currentCommand->arg1);
                   break;
              case C_GOTO:
                   WriteGoTo(outP, currentCommand->arg1);
                   break;
              case C_IF:
                   WriteIf(outP, currentCommand->arg1);
                   break;
              case C_PUSH:
              case C_POP:
                   WritePushPop(outP, currentCommand->commandType, currentCommand->arg1, currentCommand->arg2);
                   break;
              case C_FUNCTION:
                   WriteFunction(outP, currentCommand->arg1, currentCommand->arg2);
                   break;
              case C_CALL:
                   WriteCall(outP, currentCommand->arg1, currentCommand->arg2);
                   break;
              case C_RETURN:
                   WriteReturn(outP);
                   break;
              default:
                   break;
	        }
        }
        fclose(inP);
    }
  }
  closedir (pDir);
  Close(outP);

failed:
  free(vmCommands);

  return 0;
}
