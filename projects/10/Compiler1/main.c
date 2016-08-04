#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <libgen.h>
#include "jacktokenizer.h"
#include "compilationengine.h"

#define JACK_EXTENSION        ".jack"
#define XML_EXTENSION         ".xml"
#define TOKEN_XML_EXTENSION   "T.xml"

static char *CheckFileExtension(char *str, char *extension)
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

  // get TOKEN_XML_EXTENSION pos
  end = CheckFileExtension(str, TOKEN_XML_EXTENSION);

  // if found, replace it with XML_EXTENSION
  if(end)
  	strcpy(end, XML_EXTENSION);

   return str;
}

static char *CreateTokenOutfile(char *str)
{
  char *end;

  // get JACK_EXTENSION pos
  end = CheckFileExtension(str, JACK_EXTENSION);

  // if found, replace it with TOKEN_XML_EXTENSION
  if(end)
  	strcpy(end, TOKEN_XML_EXTENSION);

   return str;
}

int main(int argc, char* argv[])
{
  struct dirent *pDirent;
  DIR *pDir = NULL;
  char *dirName = NULL;
  char *fileName = NULL;
  char inputFilename[128];

  FILE *inP, *outTokenP, *outP; // input and output file
  Token *jackTokens; // all Jack Tokens
  int count=0; // count of instructions
  int i; // loop index
  Token *currentToken = NULL;
  
  if(argc != 2) {
    printf("Usage : JackAnalyzer <directory | jack file >");
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

        // handle token output file
        outTokenP = fopen(CreateTokenOutfile(inputFilename), "w+");
        if(!outTokenP) {
           printf("Output token file open failed!\n");
           outTokenP = stdout;
        }
        JackTokenOutput(outTokenP, &jackTokens, count);
        fclose(outTokenP);

        // handle output file
        outP = fopen(CreateOutfile(inputFilename), "w+");
        if(!outP) {
           printf("Output file open failed!\n");
           outP = stdout;
	    }
        CompileClass(outP, &jackTokens, count);
	    fclose(outP);
    }
  }
  closedir(pDir);
  // End of Handling directory

  // Handle single jack file
  if (fileName != NULL) {
    // open the input file
	printf("%s\n", fileName);
    inP = fopen(fileName, "r");
    if(!inP) {
      printf("File '%s' open failed!\n", fileName);
      goto failed;
    }
    printf("Compiling %s\n", basename(fileName));

    count = JackTokenizer(inP, &jackTokens);
    fclose(inP);

    // handle token output file
    outTokenP = fopen(CreateTokenOutfile(fileName), "w+");
    if(!outTokenP) {
       printf("Output token file open failed!\n");
       outTokenP = stdout;
	}
	JackTokenOutput(outTokenP, &jackTokens, count);
    fclose(outTokenP);

    // handle output file
    outP = fopen(CreateOutfile(fileName), "w+");
    if(!outP) {
       printf("Output file open failed!\n");
       outP = stdout;
	}
    CompileClass(outP, &jackTokens, count);
	fclose(outP);
	CompileClass(stdout, &jackTokens, count);
  }

failed:
  free(jackTokens);

  return 0;
}
