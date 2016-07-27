#include "codewriter.h"
#include "parser.h"
#include <stdlib.h>
#include <libgen.h>

static void Init(FILE* fp)
{
 // init the SP to address 256
  fputs("@256\n", fp);
  fputs("D=A\n", fp);
  fputs("@SP\n", fp);
  fputs("M=D\n", fp);
  
}
// Cause SP point to the next address,
// SP-- indicate the current data address
static void DecreseSP(FILE* fp)
{
  if(fp != NULL) {
    fputs("@SP\n", fp);
    fputs("M=M-1\n", fp);
  }
}

// pop the data from stack to RegA
static void PopToA(FILE* fp)
{
  if(fp != NULL) {
    DecreseSP(fp);
    fputs("@SP\n", fp);
    fputs("A=M\n", fp);
    fputs("A=M\n", fp);
  }
}

// pop the data from stack to RegD
static void PopToD(FILE* fp)
{
  if(fp != NULL) {
    DecreseSP(fp);
    fputs("@SP\n", fp);
    fputs("A=M\n", fp);
    fputs("D=M\n", fp);
  }
}

// After push to stack, SP need to point to the next address
// SP++ indicate the next data address
static void IncreseSP(FILE* fp)
{
  if(fp != NULL) {
    fputs("@SP\n", fp);
    fputs("M=M+1\n", fp);
  }
}

// push the data from RegD to stack
static void PushFromD(FILE* fp)
{
  if(fp != NULL) {
    fputs("@SP\n", fp);
    fputs("A=M\n", fp);
    fputs("M=D\n", fp);
    IncreseSP(fp);
  }
}

// pop two elements from the stack and put them to RegA and RegD respectively
// this is necessary for doing some binary operations like add, sub
static void PopTwoFromStack(FILE* fp)
{
  if(fp != NULL) {
    PopToD(fp);
    PopToA(fp);
  }
}


#define BINARY_END 4
#define UNARY_END  6
static OpMapping opMapping[] = {
   // op      ,    symbol
    {"add"    ,    "+"},
    {"sub"    ,    "-"},
    {"and"    ,    "&"},
    {"or"     ,    "|"},
   // unary 
    {"neg"    ,    "-"},
    {"not"    ,    "!"},
   // compare
    {"eq"     ,   "JEQ"},
    {"gt"     ,   "JGT"},
    {"lt"     ,   "JLT"},
};

static char* isBinaryOp(char* op)
{
  int i;

  for( i = 0; i < BINARY_END; i++){
    if(strcmp(op, opMapping[i].op) == 0 )
      return opMapping[i].symbol;
  }

  return NULL;
}

static char* isUnaryOp(char* op)
{
  int i;

  for( i = BINARY_END; i < UNARY_END; i++){
    if(strcmp(op, opMapping[i].op) == 0 )
      return opMapping[i].symbol;
  }

  return NULL;
}

static char* isCompareOp(char* op)
{
  int count = sizeof(opMapping) / sizeof(OpMapping);
  int i;

  for( i = UNARY_END; i < count; i++){
    if(strcmp(op, opMapping[i].op) == 0 )
      return opMapping[i].symbol;
  }

  return NULL;
}

#define NORMAL_SEGMENT_END 4
static OpMapping segmentMapping[] = {
   // setment    ,   symbol
    {"argument"  ,    "ARG"},
    {"local"     ,    "LCL"},
    {"this"      ,   "THIS"},
    {"that"      ,   "THAT"},
    // pointer and temp
    {"pointer"   ,     "3"}, // R3
    {"temp"      ,     "5"}, // R5
};

static char* isSegment(char* segment)
{
  int i;

  for( i = 0; i < NORMAL_SEGMENT_END; i++){
    if(strcmp(segment, segmentMapping[i].op) == 0 )
      return segmentMapping[i].symbol;
  }

  return NULL;
}

static int isTempOrPointer(char* segment)
{
  int count = sizeof(segmentMapping) / sizeof(OpMapping);
  int i;
  for( i = NORMAL_SEGMENT_END; i < count; i++){
    if(strcmp(segment, segmentMapping[i].op) == 0 )
	  return atoi(segmentMapping[i].symbol);
  }

  return -1;
}

char *CheckFileExtension(char *str)
{
  char *end;

  // Find the VM_EXTENSION
  end = strstr(str, VM_EXTENSION);

  // if found, return the extension start pointer
  return end;
}

char *CreateOutfile(char *str)
{
  char *end;

  // get VM_EXTENSION pos
  end = CheckFileExtension(str);

  // if found, replace it with ASM_EXTENSION
  if(end)
  	strcpy(end, ASM_EXTENSION);

   return str;
}

static char gFileName[32];

FILE* SetFileName(char* fileName)
{
  FILE* fp;

  // set file name without extension
  strncpy(gFileName, basename(fileName), strcspn(basename(fileName),"."));

  // open the output file
  fp = fopen(CreateOutfile(fileName), "w+");
  if(!fp) {
    perror("CodeWriter::SetFileName File open failed!");
  }

  Init(fp);
 
  return fp;
}

void WriteArithmetic(FILE* fp, char* command)
{
  char* symbol;
  // for comparison, create individual symbols by adding index number
  static labelIndex = 0;

  if (fp == NULL)
    return;

  if((symbol = isBinaryOp(command)) != NULL ) {
    PopTwoFromStack(fp);
    fprintf(fp, "D=A%sD\n", symbol); // A is first number, D is second(FILO)
    PushFromD(fp); // push the result to stack
  } else if ((symbol = isUnaryOp(command)) != NULL ) {
    PopToD(fp);
    fprintf(fp, "D=%sD\n", symbol);
    PushFromD(fp); // push the result to stack
  } else if ((symbol = isCompareOp(command)) != NULL ) {
    PopTwoFromStack(fp);
    fputs("D=A-D\n", fp); // A is first number, D is second(FILO)
    fprintf(fp, "@TRUE%d\n", labelIndex);
    fprintf(fp, "D;%s\n", symbol);
    fputs("@SP\n", fp);
	fputs("A=M\n", fp);
    fputs("M=0\n", fp); // False = 0
    IncreseSP(fp);
    fprintf(fp, "@END%d\n", labelIndex);
    fputs("0;JMP\n", fp);
    fprintf(fp, "(TRUE%d)\n", labelIndex);
    fputs("@SP\n", fp);
    fputs("A=M\n", fp);
    fputs("M=-1\n", fp); // True = -1
    IncreseSP(fp);
    fprintf(fp, "(END%d)\n", labelIndex);
    labelIndex++;
  }
}

void WritePushPop(FILE* fp, short commandType, char* segment, int index)
{
  char* symbol;
  int pos;

  if (fp == NULL)
    return;

  switch(commandType){
    case C_PUSH:
      {
        if(strcmp(segment, "constant") == 0){
            fprintf(fp, "@%d\n", index);
            fputs("D=A\n", fp);
        } else if(strcmp(segment, "static") == 0){
            fprintf(fp, "@%s.%d\n", gFileName, index);
            fputs("D=M\n", fp);
        } else if ( (symbol = isSegment(segment)) != NULL){
            fprintf(fp, "@%s\n", symbol);
            fputs("D=M\n", fp);
            fprintf(fp, "@%d\n", index);
            fputs("A=D+A\n", fp);
            fputs("D=M\n", fp);
        } else if ( (pos = isTempOrPointer(segment)) != -1 ){
            fprintf(fp, "@%d\n", pos + index);
            fputs("D=M\n", fp);
        }
	  }
      PushFromD(fp); // push the result to stack
	  break;
    case C_POP:
      {
        // use R13 for a temporary use to save the address
        if( strcmp(segment, "static") == 0 ){
            fprintf(fp, "@%s.%d\n", gFileName, index);
            fputs("D=A\n", fp);
            fputs("@R13\n", fp);
            fputs("M=D\n", fp);
        } else if ( (symbol = isSegment(segment)) != NULL){
            fprintf(fp, "@%s\n", symbol);
            fputs("D=M\n", fp);
            fprintf(fp, "@%d\n", index);
            fputs("D=D+A\n", fp);
            fputs("@R13\n", fp);
            fputs("M=D\n", fp);
        } else if ( (pos = isTempOrPointer(segment)) != -1 ){
            fprintf(fp, "@%d\n", pos + index);
            fputs("D=A\n", fp);
            fputs("@R13\n", fp);
            fputs("M=D\n", fp);
        }
      }
      PopToD(fp); // pop the result to D register
      fputs("@R13\n", fp);
      fputs("A=M\n", fp);
      fputs("M=D\n", fp);
	  break;
  }
}

void Close(FILE* fp)
{
  fclose(fp);
}
