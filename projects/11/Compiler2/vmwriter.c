#include "vmwriter.h"

#include <string.h>
#include <stdlib.h>

char *CheckFileExtension(char *str, char *extension)
{
  char *end;

  // Find the extension
  end = strstr(str, extension);

  // if found, return the extension start pointer
  return end;
}

static char *CreateOutfile(char *str)
{
  char *end;

  // get JACK_EXTENSION pos
  end = CheckFileExtension(str, JACK_EXTENSION);

  // if found, replace it with VM_EXTENSION
  if(end)
     strcpy(end, VM_EXTENSION);

   return str;
}

FILE* VMWriter(char* fileName){
  FILE* fp;

  // open the output file
  fp = fopen(CreateOutfile(fileName), "w+");
  if(!fp) {
    printf("VMWriter::File open failed!");
  }
printf("%s\n", fileName);
  return fp;
}

void WritePush(FILE* fp, char* segment, int index){
  fprintf(fp, "push %s %d\n", segment, index);
}

void WritePop(FILE* fp, char* segment, int index){
  fprintf(fp, "pop %s %d\n", segment, index);
}

void WriteArithmetic(FILE* fp, char* command){
  fprintf(fp, "%s\n", command);
}

void WriteLabel(FILE* fp, char* label){
  fprintf(fp, "label %s\n", label);
}

void WriteGoto(FILE* fp, char* label){
  fprintf(fp, "goto %s\n", label);
}

void WriteIf(FILE* fp, char* label){
  fprintf(fp, "if-goto %s\n", label);
}

void WriteCall(FILE* fp, char* name, int nArgs){
  fprintf(fp, "call %s %d\n", name, nArgs);
}

void WriteFunction(FILE* fp, char* name, int nLocals){
  fprintf(fp, "function %s %d\n", name, nLocals);
}

void WriteReturn(FILE* fp){
  fputs("return", fp);
}

void Close(FILE* fp){
  fclose(fp);
}
