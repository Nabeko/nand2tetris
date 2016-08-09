#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <libgen.h>
#include "jacktokenizer.h"
#include "compilationengine.h"
#include "vmwriter.h"

int main(int argc, char* argv[])
{
  struct dirent *pDirent;
  DIR *pDir = NULL;
  char *dirName = NULL;
  char *fileName = NULL;
  char inputFilename[128];

  FILE *inP; // input file
  Token *jackTokens; // all Jack Tokens
  int count=0; // count of instructions
  int i; // loop index
  Token *currentToken = NULL;
  
  if(argc != 2) {
    printf("Usage : JackCompiler <directory | jack file >");
    return -1;
  }
  
  jackTokens = (Token*)malloc(sizeof(Token) * MAX_TOKEN_NUM);

  if (!CheckFileExtension(argv[1], JACK_EXTENSION)) { // argument is not a *.jack file, might be dir
    dirName = argv[1];

    // open directory  
    pDir = opendir (dirName);
    if(pDir == NULL) {
      printf("Can not open directory '%s'\n", dirName);
    }
  } else {
    fileName = argv[1];
  }

  // Handle directory
  while ((pDir != NULL) && ((pDirent = readdir(pDir)) != NULL)) {
    // only open the *.jack files in directory
    if( CheckFileExtension(pDirent->d_name, JACK_EXTENSION) ) {
        sprintf(inputFilename, "%s/%s", dirName, pDirent->d_name);
        // open the input file
        inP = fopen(inputFilename, "r");
        if(!inP) {
          printf("File '%s' open failed!\n", inputFilename);
          continue;
        }
        printf("Compiling %s\n", basename(inputFilename));

        count = JackTokenizer(inP, &jackTokens);
        fclose(inP);

        CompileClass(inputFilename, &jackTokens, count);
    }
  }
  closedir(pDir);
  // End of Handling directory

  // Handle single jack file
  if (fileName != NULL) {
    // open the input file
    inP = fopen(fileName, "r");
    if(!inP) {
      printf("File '%s' open failed!\n", fileName);
      goto failed;
    }
    printf("Compiling %s\n", basename(fileName));

    count = JackTokenizer(inP, &jackTokens);
    fclose(inP);

    CompileClass(fileName, &jackTokens, count);
  }

failed:
  free(jackTokens);

  return 0;
}
