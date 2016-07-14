#include <string.h>
#include "parser.h"
#include "code.h"

#define ASM_EXTENSION ".asm"
#define HACK_EXTENSION ".hack"

static char *CheckFileExtension(char *str)
{
  char *end;

  // Find the ASM_EXTENSION
  end = strstr(str, ASM_EXTENSION);

  // if found, return the extension start pointer
  return end;
}

static char *CreateOutfile(char *str)
{
  char *end;

  // get ASM_EXTENSION pos
  end = CheckFileExtension(str);

  // if found, replace it with HACK_EXTENSION
  if(end)
  	strcpy(end, HACK_EXTENSION);

   return str;
}

// size is represented the digit number for binary 
static char* IntToBinary(int size, int value, char* binary)
{
  int i = size -1;

  binary[size] = '\0';

  while(i+1){
    binary[i--] = (1 & value) + '0';
	value >>= 1;
  }
  
  return binary;
}

int main(int argc, char* argv[])
{
  FILE *inP, *outP; // input and output file
  Instruction *hackInstructions; // all Hack instructions
  int count=0; // count of instructions
  int i; // loop index
  char binary[32]; // address for A command
  
  if(argc != 2) {
    perror("Usage : HackAssembler <filename>");
    return -1;
  }
  
  // make surce the file extension is .asm
  if(!CheckFileExtension(argv[1])) {
    perror("File extension must be .asm");
    return -1;  
  }

  // open the input file
  inP = fopen(argv[1], "r");
  if(!inP) {
    perror("File open failed!");
    return -1;
  }

  // retrive Hack instructions from input file 
  count = parse(inP, &hackInstructions);
  fclose(inP);

  // create the output file
  outP = fopen(CreateOutfile(argv[1]), "w+");
  if(!outP) {
    perror("File open failed!");
    return -1;
  }
 
  Instruction *currentInstruction = NULL;
  for (i=0; i<count; i++){
	  currentInstruction = hackInstructions+i;
	  switch(currentInstruction->commandType){
		  case A_COMMAND: // output to file, the first bit is 0 for A command, the remainder will be generated by IntToBinary
			fprintf(outP, "0%s\n", IntToBinary(15,atoi(currentInstruction->symbol), binary));
			break;
		  case C_COMMAND: // the first three bits is 111, the remainder will be generated by 'code' module
			fprintf(outP, "111%s%s%s\n", comp(currentInstruction->comp), dest(currentInstruction->dest), jump(currentInstruction->jump));
			break;
	  }
  }

  fclose(outP);

  return 0;
}