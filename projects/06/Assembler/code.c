#include "code.h"
#include <stdio.h>

static SymbolicToBinary destMap[] = {
// dest   , d1 d2 d3
   {""    , "000"},
   {"M"   , "001"},
   {"D"   , "010"},
   {"MD"  , "011"},
   {"A"   , "100"},
   {"AM"  , "101"},
   {"AD"  , "110"},
   {"AMD" , "111"},
};

static SymbolicToBinary jumpMap[] = {
// jump   , j1 j2 j3
   {""    , "000"},
   {"JGT" , "001"},
   {"JEQ" , "010"},
   {"JGE" , "011"},
   {"JLT" , "100"},
   {"JNE" , "101"},
   {"JLE" , "110"},
   {"JMP" , "111"},
};

static SymbolicToBinary compMap[] = {
// comp   ,  a c1 c2 c3 c4 c5 c6
   {"0"   , "0101010"},
   {"1"   , "0111111"},
   {"-1"  , "0111010"},
   {"D"   , "0001100"},
   {"A"   , "0110000"},
   {"M"   , "1110000"},
   {"!D"  , "0001101"},
   {"!A"  , "0110001"},
   {"!M"  , "1110001"},
   {"-D"  , "0001111"},
   {"-A"  , "0110011"},
   {"-M"  , "1110011"},
   {"D+1" , "0011111"},
   {"A+1" , "0110111"},
   {"M+1" , "1110111"},
   {"D-1" , "0001110"},
   {"A-1" , "0110010"},
   {"M-1" , "1110010"},
   {"D+A" , "0000010"},
   {"D+M" , "1000010"},
   {"D-A" , "0010011"},
   {"D-M" , "1010011"},
   {"A-D" , "0000111"},
   {"M-D" , "1000111"},
   {"D&A" , "0000000"},
   {"D&M" , "1000000"},
   {"D|A" , "0010101"},
   {"D|M" , "1010101"}
};

static char *codeMapping(int type, char* symbol){
  int i, count = 0;
  SymbolicToBinary *table = NULL;

  switch(type){
	case DEST:
      count = sizeof(destMap) / sizeof(SymbolicToBinary);
      table = destMap;
      break;
	case COMP:
      count = sizeof(compMap) / sizeof(SymbolicToBinary);
      table = compMap;
      break;
	case JUMP:
      count = sizeof(jumpMap) / sizeof(SymbolicToBinary);
      table = jumpMap;
      break;
  }

  if(table){
    for(i=0; i<count; i++){
      if(strcmp(symbol, table[i].symbolic) == 0 )
        return table[i].binary;
    }
  }

  return NULL;
}

char *dest(char* hackDest){
  return codeMapping(DEST , hackDest);
}

char *comp(char* hackComp){
  return codeMapping(COMP , hackComp);
}

char *jump(char* hackJump){
  return codeMapping(JUMP , hackJump);
}
