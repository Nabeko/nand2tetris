#include "jacktokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <regex.h>

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

static char *jackKeywords[] = {
  "class",
  "constructor",
  "function",
  "method",
  "field",
  "static",
  "var",
  "int",
  "char",
  "boolean",
  "void",
  "true",
  "false",
  "null",
  "this",
  "let",
  "do",
  "if",
  "else",
  "while",
  "return"
};

static char jackSymbols[] = {
  '{', '}', '(', ')', '[', ']', '.', ',', ';',
  '+', '-', '*', '/', '&', '|', '<', '>', '=', '~' 
};

static char* IsKeyword(char* str)
{
  int count = sizeof(jackKeywords) / sizeof(char*);
  int i;

  for( i = 0; i < count; i++){
    if(strcmp(str, jackKeywords[i]) == 0 )
      return jackKeywords[i];
  }
  return NULL;
}

static int IsIntegerConstant(char *str)
{
  while(*str) {
    if (isdigit(*str++) == 0)
       return 0;
  }
  return 1;
}

static int IsStringConstant(const char *str)
{
  if(*str == '\"' && str[strlen(str)-1] == '\"') {
	return 1;
  }
  return 0;
}

static char IsSymbol(char* str)
{
  int count = sizeof(jackSymbols) / sizeof(char);
  int i;
  
  if(strlen(str) > 1) // symbol only have one char
    return 0;

  for( i = 0; i < count; i++){
    if(*str == jackSymbols[i])
      return jackSymbols[i];
  }
  return 0;
}

static int TokenParser(char *str, char *regexString, Token **tokens, int index)
{
  const int maxMatches = 50; // each string's maximum matches
  const int maxGroups = 3;
  int m; // Matches index
  int g; // Groups index
  int count = 0;
  regex_t regexCompiled;
  regmatch_t groupArray[maxGroups];
  char *cursor, *currentMatch;
  Token *currentToken; 

  if (regcomp(&regexCompiled, regexString, REG_EXTENDED)) {
      printf("Could not compile regular expression.\n");
      return -1;
  }
  
  cursor = str;
  for (m = 0; m < maxMatches; m++) { 
    if (regexec(&regexCompiled, cursor, maxGroups, groupArray, 0))
        break;  // No more matches

    // rm_so is the matched start offset, sm_eo is the matched end offset 
    unsigned int offset = 0;
    for (g = 0; g < maxGroups; g++) {
        if (groupArray[g].rm_so == (size_t)-1)
            break;  // No more groups

        if (g == 0)
          offset = groupArray[g].rm_eo;

        char cursorCopy[strlen(cursor) + 1];
        strcpy(cursorCopy, cursor);
        cursorCopy[groupArray[g].rm_eo] = 0;
        currentMatch = cursorCopy + groupArray[g].rm_so;

        currentToken = *tokens + index + count;
        memset(currentToken, 0, sizeof(Token));

		if (IsKeyword(currentMatch)) {
            currentToken->tokenType = KEYWORD;
            strcpy(currentToken->keyWord, currentMatch);
        } else if(IsIntegerConstant(currentMatch)) {
            currentToken->tokenType = INT_CONST;
            currentToken->intVal = atoi(currentMatch);
        } else if(IsSymbol(currentMatch)) {
            currentToken->tokenType = SYMBOL;
            currentToken->symbol = *currentMatch;
        } else if(IsStringConstant(currentMatch)) {
            currentToken->tokenType = STRING_CONST;
            // only copy the portion of string, ignore ""  
            strncpy(currentToken->stringVal, currentMatch + 1, strlen(currentMatch)-2);
		} else { // the remainder is identifier
            currentToken->tokenType = IDENTIFIER;
            strcpy(currentToken->identifier, currentMatch);
        }
		count++;
    }
    cursor += offset; // moving forward for next match
  }

  regfree(&regexCompiled);
  
  return count;
}

int JackTokenizer(FILE *jackFile, Token **tokens)
{
  char buf[128]; // buffer for each line
  char *temp = NULL; // temp for the each token
  int count = 0; // count of tokens
  int i; // array index
  Token *currentToken = NULL;
  int tokenType = -1;
  int commentFlag = 0;
  char regexString[512];
  const char *symbolReg = "[\]{}\[\\+\-\\*\\/().,;/&|<>=~]";
  const char *intReg = "[0-9]+";
  const char *strReg = "\"[^\"\n]*\"";
  const char *idReg = "[A-Za-z0-9_]+";

  // create regular expression statement
  sprintf(regexString, "%s|%s|%s|%s", symbolReg, intReg, strReg, idReg);

  while(!feof(jackFile)) {
    if(fgets(buf,128,jackFile)!=NULL) {
      // remove the comments on each line
      temp = TrimWhitespace(RemoveComments(buf));
      // if *temp is 0, it must be a space line 
	  if(*temp != 0) {
        // Handle comments
        if(commentFlag) { // got "/*" previously
           if(strstr(temp, "*/")) {
               commentFlag = 0;
		   }
           continue;
		} else if (strstr(temp, "/*")) {
           if(!strstr(temp, "*/"))
             commentFlag = 1; // got "/*"
           continue;
		}
        // end of hadling comments

        count += TokenParser(temp, regexString, tokens, count);
      }
    }
  }
  return count;
}

void JackTokenOutput(FILE* fp, Token **tokens, int count){
 Token *currentToken = NULL;
  int i;
  fputs("<tokens>\n", fp);
  for ( i = 0; i < count; i++){
    currentToken = *tokens + i;
	switch (currentToken->tokenType) {
       case KEYWORD:
            fprintf(fp, "<keyword> %s </keyword>\n", currentToken->keyWord);
            break;
       case SYMBOL:
            if(currentToken->symbol == '>')
                fprintf(fp, "<symbol> %s </symbol>\n", "&gt;");
            else if (currentToken->symbol == '<')
                fprintf(fp, "<symbol> %s </symbol>\n", "&lt;");
            else if (currentToken->symbol == '&')
                fprintf(fp, "<symbol> %s </symbol>\n", "&amp;");
            else if (currentToken->symbol == '\"')
                fprintf(fp, "<symbol> %s </symbol>\n", "&quot;");
            else
                fprintf(fp, "<symbol> %c </symbol>\n", currentToken->symbol);
            break;
       case IDENTIFIER:
            fprintf(fp, "<identifier> %s </identifier>\n", currentToken->identifier);
            break;
       case INT_CONST:
            fprintf(fp, "<integerConstant> %d </integerConstant>\n", currentToken->intVal);
            break;
       case STRING_CONST:
            fprintf(fp, "<stringConstant> %s </stringConstant>\n", currentToken->stringVal);
            break;
    }
  }
  fputs("</tokens>\n", fp);
}
