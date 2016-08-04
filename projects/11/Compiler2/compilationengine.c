#include "compilationengine.h"
#include "jacktokenizer.h"
#include <stdlib.h>
#include <string.h>

#define ALIGMENT ' '

static void CompileClassVarDec(FILE* , Token **, int *, int *, int );
static void CompileSubroutine(FILE* , Token **, int *, int *, int );
static void CompileParameterList(FILE* , Token **, int *, int *, int );
static void CompileVarDec(FILE* , Token **, int *, int *, int );
static void CompileStatements(FILE* , Token **, int *, int *, int );
static void CompileDo(FILE* , Token **, int *, int *, int );
static void CompileLet(FILE* , Token **, int *, int *, int );
static void CompileWhile(FILE* , Token **, int *, int *, int );
static void CompileReturn(FILE* , Token **, int *, int *, int );
static void CompileIf(FILE* , Token **, int *, int *, int );
static void CompileExpression(FILE* , Token **, int *, int *, int );
static void CompileTerm(FILE* , Token **, int *, int *, int );
static void CompileExpressionList(FILE* , Token **, int *, int *, int );

static StatementMapping statementMapping[] = {
  { "do"    , CompileDo     },
  { "let"   , CompileLet    },
  { "while" , CompileWhile  },
  { "return", CompileReturn },
  { "if"    , CompileIf     },
};

// make sure the op is in ops group
static char IsOp(char op){
  char* ops = "=+-*/&|~<>";
  char* pos;
  
  if( (pos = strchr(ops, op)) != 0)
    return *pos;

  return FALSE;
}

// make sure the keyword is in the classVarDec group
static int IsClassVarDec(char* keyword){
  char* classVarDec[] = { "static", "field"};
  int count = sizeof(classVarDec) / sizeof(char*);
  int i;

  for( i = 0; i < count; i++){
    if(strcmp(keyword, classVarDec[i]) == TRUE )
      return TRUE;
  }

  return FALSE;
}

// make sure the keyword is in the subrountine group
static int IsSubroutine(char* keyword){
  char* subroutine[] = { "constructor", "function", "method"};
  int count = sizeof(subroutine) / sizeof(char*);
  int i;

  for( i = 0; i < count; i++){
    if(strcmp(keyword, subroutine[i]) == TRUE )
      return TRUE;
  }

  return FALSE;
}

// increse by 1
static void Increse(int *num){
  (*num)++;
}

// decrese by 1
static void Decrese(int *num){
  (*num)--;
}

// write Tag with aligment
static void WriteTag(FILE* fp, char* tag, int aligment_count){
  int space = aligment_count*2; // 2 spaces for a aligment
  
  if(space)
     fprintf(fp, "%*c<%s>\n", space, ALIGMENT, tag);
  else
     fprintf(fp, "<%s>\n", tag);
}

// get Token with given index
static Token *GetToken(Token **tokens, int index, int count){
  Token *currentToken = NULL;
  if ( index < count )
       currentToken = *tokens + index;

  return currentToken;
}

// write Token to file
static void WriteToFile(FILE* fp, char* tokenType, char* token, int aligment_count){
  int space = aligment_count*2; // 2 spaces for a aligment
  fprintf(fp, "%*c<%s> %s </%s>\n", space, ALIGMENT, tokenType, token, tokenType);
}

// set the right token type and token
static void WriteToken(FILE* fp, Token *tokens, int aligment_count){
  char tokenType[16], token[512];

  if(tokens) {
    switch (tokens->tokenType) {
       case KEYWORD:
            strcpy(tokenType, "keyword");
            strcpy(token ,tokens->keyWord);
            break;
       case SYMBOL:
            strcpy(tokenType, "symbol");
            if(tokens->symbol == '>')
                strcpy(token, "&gt;");
            else if (tokens->symbol == '<')
                strcpy(token, "&lt;");
            else if (tokens->symbol == '&')
                strcpy(token, "&amp;");
            else if (tokens->symbol == '\"')
                strcpy(token, "&quot;");
            else
                sprintf(token, "%c", tokens->symbol);
            break;
       case IDENTIFIER:
            strcpy(tokenType, "identifier");
            strcpy(token , tokens->identifier);
            break;
       case INT_CONST:
            strcpy(tokenType, "integerConstant");
            sprintf(token, "%d", tokens->intVal);
            break;
       case STRING_CONST:
            strcpy(tokenType, "stringConstant");
            strcpy(token, tokens->stringVal);
            break;
    }
    WriteToFile(fp, tokenType, token, aligment_count);
  }
}

// Get next token to write and increse the index of token
static void WriteNextToken(FILE* fp, Token **tokens, int *index, int aligment_count, int count, short tokenType, char* token){
  int writeFlag = 0;
  Token *nextToken = NULL;
  
  nextToken = GetToken(tokens, *index, count);
  if(nextToken) {
    if(nextToken->tokenType == tokenType){
        if (tokenType >= IDENTIFIER) { // tokenType is IDENTIFIER, INT_CONST, STRING_CONST
             writeFlag = 1;
        } else if ( tokenType < IDENTIFIER){ // tokenType is KEYWORD, SYMBOL
             if ( tokenType == KEYWORD ) {
                 if(strcmp(token, nextToken->keyWord) == TRUE)
                    writeFlag = 1;
             } else {
                 if(*token == nextToken->symbol)
                    writeFlag = 1;
             }
        }
    }

    if(writeFlag) {
        WriteToken(fp, nextToken, aligment_count);
        Increse(index);
    } else {
        printf("Error, next Token should be %s.\n", (token!=NULL)?token:"known");
    }
  }
}

// type varName
static void HandleDeclaration(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;

  // handle KEYWORD or IDENTIFIER  
  nextToken = GetToken(tokens, *index, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, nextToken->tokenType, nextToken->keyWord);

  // handle IDENTIFIER
  nextToken = GetToken(tokens, *index, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, IDENTIFIER, NULL);
}

// ('static' | 'field' ) type varName (',' varName)* ';'
static void CompileClassVarDec(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;
  
  WriteTag(fp, "classVarDec", *aligment_count);
  Increse(aligment_count);

  // handle 'static' or 'field'
  nextToken = GetToken(tokens, *index, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, KEYWORD, nextToken->keyWord);

  HandleDeclaration(fp, tokens, index, aligment_count, count);

  // handle ',' if ';' not showing
  nextToken = GetToken(tokens, *index, count);
  while((nextToken->tokenType == SYMBOL) && nextToken->symbol != ';'){
        WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ",");
        WriteNextToken(fp, tokens, index, *aligment_count, count, IDENTIFIER, NULL);
        nextToken = GetToken(tokens, *index, count);
  }
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ";");

  Decrese(aligment_count);
  WriteTag(fp, "/classVarDec", *aligment_count);
}

// ('constructor' | 'function' | 'method') ('void' | type) subroutineName '(' parameterList ')' subroutineBody
static void CompileSubroutine(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;

  WriteTag(fp, "subroutineDec", *aligment_count);
  Increse(aligment_count);
  
  // handle 'constructor', 'function', or 'method'
  nextToken = GetToken(tokens, *index, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, KEYWORD, nextToken->keyWord);

  HandleDeclaration(fp, tokens, index, aligment_count, count);

  // handle '(' parameters ')'
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "(");
  CompileParameterList(fp, tokens, index, aligment_count, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ")");
  
  // Start of subruntineBody
  // '{' varDec* statements '}'
  WriteTag(fp, "subroutineBody", *aligment_count);
  Increse(aligment_count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "{");

  // handle var, if there is not var, the CompileVarDec() should not be executed.
  nextToken = GetToken(tokens, *index, count);
  while((nextToken->tokenType == KEYWORD) && (strcmp(nextToken->keyWord, "var") == TRUE)){
     CompileVarDec(fp, tokens, index, aligment_count, count);
	 nextToken = GetToken(tokens, *index, count);
  }

  // handle statements
  CompileStatements(fp, tokens, index, aligment_count, count);

  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "}");
  Decrese(aligment_count);
  WriteTag(fp, "/subroutineBody", *aligment_count);
  // End of subruntineBody

  Decrese(aligment_count);
  WriteTag(fp, "/subroutineDec", *aligment_count);
}

// ( (type varName) (',' type varName)*)?
static void CompileParameterList(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;

  WriteTag(fp, "parameterList", *aligment_count);
  Increse(aligment_count);
  
  // handle parameters
  nextToken = GetToken(tokens, *index, count);
  while(nextToken->symbol != ')'){ // parameters format ( int X, var XX )
    HandleDeclaration(fp, tokens, index, aligment_count, count);
	nextToken = GetToken(tokens, *index, count);
    if(nextToken->symbol != ')') {
        WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ",");
        nextToken = GetToken(tokens, *index, count);
    }
  }
  
  Decrese(aligment_count);
  WriteTag(fp, "/parameterList", *aligment_count);
}

// 'var' type varName (',' varName)* ';'
static void CompileVarDec(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;
  
  WriteTag(fp, "varDec", *aligment_count);
  Increse(aligment_count);

  // handle 'var'
  nextToken = GetToken(tokens, *index, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, KEYWORD, "var");

  HandleDeclaration(fp, tokens, index, aligment_count, count);

  // handle ',' if ';' not showing
  nextToken = GetToken(tokens, *index, count);
  while((nextToken->tokenType == SYMBOL) && (nextToken->symbol != ';')){
        WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ",");
        WriteNextToken(fp, tokens, index, *aligment_count, count, IDENTIFIER, NULL);
        nextToken = GetToken(tokens, *index, count);
  }
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ";");

  Decrese(aligment_count);
  WriteTag(fp, "/varDec", *aligment_count);
}

// statement*
static void CompileStatements(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;
  int i; // array index
  int mappingCount = sizeof(statementMapping) / sizeof(StatementMapping);

  WriteTag(fp, "statements", *aligment_count);
  Increse(aligment_count);

  nextToken = GetToken(tokens, *index, count);
  while(nextToken->symbol != '}'){ // statement end is '}'
    for( i = 0; i < mappingCount; i++ ){
        if(strcmp(nextToken->keyWord, statementMapping[i].statement) == TRUE){
            statementMapping[i].statementFunction(fp, tokens, index, aligment_count, count);
            break;
        }
    }
	nextToken = GetToken(tokens, *index, count);
  }

  Decrese(aligment_count);
  WriteTag(fp, "/statements", *aligment_count);
}

// 'do' subroutineCall ';'
// subroutineCall 
// -> subroutineName '(' expressionList ')' | ( className | varName) '.' subroutineName '(' expressionList ')'
static void CompileDo(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;

  WriteTag(fp, "doStatement", *aligment_count);
  Increse(aligment_count);

  // handle do subroutineName
  WriteNextToken(fp, tokens, index, *aligment_count, count, KEYWORD, "do");
  WriteNextToken(fp, tokens, index, *aligment_count, count, IDENTIFIER, NULL);

  // handle if there is a ( className | varName)'.' subroutineName case.
  nextToken = GetToken(tokens, *index, count);
  if((nextToken->tokenType == SYMBOL) && (nextToken->symbol == '.')){
      WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ".");

      nextToken = GetToken(tokens, *index, count);
      WriteNextToken(fp, tokens, index, *aligment_count, count, IDENTIFIER, NULL);
  }

  // handle '(' expression ')'';'
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "(");
  CompileExpressionList(fp, tokens, index, aligment_count, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ")");
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ";");

  Decrese(aligment_count);
  WriteTag(fp, "/doStatement", *aligment_count);
}

// let' varName ('[' expression ']')? '=' expression ';'
static void CompileLet(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;

  WriteTag(fp, "letStatement", *aligment_count);
  Increse(aligment_count);

  // handle let XXX
  WriteNextToken(fp, tokens, index, *aligment_count, count, KEYWORD, "let");
  WriteNextToken(fp, tokens, index, *aligment_count, count, IDENTIFIER, NULL);

  // handle if there is a '[' expression ']' case
  nextToken = GetToken(tokens, *index, count);
  if((nextToken->tokenType == SYMBOL) && (nextToken->symbol == '[')){
      WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "[");

      CompileExpression(fp, tokens, index, aligment_count, count);

	  nextToken = GetToken(tokens, *index, count);
      WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "]");
  }

  // handle '=' expression ';'
  nextToken = GetToken(tokens, *index, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "=");
  CompileExpression(fp, tokens, index, aligment_count, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ";");

  Decrese(aligment_count);
  WriteTag(fp, "/letStatement", *aligment_count);
}

// 'while' '(' expression ')' '{' statements '}'
static void CompileWhile(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  WriteTag(fp, "whileStatement", *aligment_count);
  Increse(aligment_count);

  // 'while' '(' expression ')'
  WriteNextToken(fp, tokens, index, *aligment_count, count, KEYWORD, "while");
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "(");
  CompileExpression(fp, tokens, index, aligment_count, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ")");

  // '{' statements '}'
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "{");
  CompileStatements(fp, tokens, index, aligment_count, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "}");

  Decrese(aligment_count);
  WriteTag(fp, "/whileStatement", *aligment_count);
}

// 'return' expression? ';'
static void CompileReturn(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;

  WriteTag(fp, "returnStatement", *aligment_count);
  Increse(aligment_count);

  // 'return'
  WriteNextToken(fp, tokens, index, *aligment_count, count, KEYWORD, "return");

  // handle expression ';'
  nextToken = GetToken(tokens, *index, count);
  if(nextToken->tokenType != SYMBOL || nextToken->symbol != ';') {
    CompileExpression(fp, tokens, index, aligment_count, count);
  }
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ";");

  Decrese(aligment_count);
  WriteTag(fp, "/returnStatement", *aligment_count);
}

// 'if' '(' expression ')' '{' statements '}' ( 'else' '{' statements '}' )?
static void CompileIf(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;

  WriteTag(fp, "ifStatement", *aligment_count);
  Increse(aligment_count);

  // 'if' '(' expression ')'
  WriteNextToken(fp, tokens, index, *aligment_count, count, KEYWORD, "if");
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "(");
  CompileExpression(fp, tokens, index, aligment_count, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ")");

  // '{' statements '}'
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "{");
  CompileStatements(fp, tokens, index, aligment_count, count);
  WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "}");

  // 'else' '{' statements '}'
  nextToken = GetToken(tokens, *index, count);
  if(nextToken->tokenType == KEYWORD && strcmp(nextToken->keyWord, "else") == TRUE) {
    WriteNextToken(fp, tokens, index, *aligment_count, count, KEYWORD, "else");
	WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "{");
    CompileStatements(fp, tokens, index, aligment_count, count);
    WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "}");
  }

  Decrese(aligment_count);
  WriteTag(fp, "/ifStatement", *aligment_count);
}

// term (op term)*
static void CompileExpression(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;
  char symbol[2];

  WriteTag(fp, "expression", *aligment_count);
  Increse(aligment_count);

  // term
  CompileTerm(fp, tokens, index, aligment_count, count);

  // (op term)*
  nextToken = GetToken(tokens, *index, count);
  while( (nextToken->tokenType == SYMBOL) && (IsOp(nextToken->symbol) != FALSE)){
    sprintf(symbol, "%c", nextToken->symbol);
    WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, symbol);
    CompileTerm(fp, tokens, index, aligment_count, count);
	nextToken = GetToken(tokens, *index, count);
  }

  Decrese(aligment_count);
  WriteTag(fp, "/expression", *aligment_count);
}

// integerConstant | stringConstant | keywordConstant | varName |
// varName '[' expression ']' | subroutineCall | '(' expression ')' | unaryOp term
static void CompileTerm(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;
  char symbol[2];

  WriteTag(fp, "term", *aligment_count);
  Increse(aligment_count);

  // integerConstant | stringConstant | keywordConstant
  nextToken = GetToken(tokens, *index, count);
  if( nextToken->tokenType == INT_CONST    ||
      nextToken->tokenType == STRING_CONST ||
      nextToken->tokenType == KEYWORD ) {
      WriteNextToken(fp, tokens, index, *aligment_count, count, nextToken->tokenType, nextToken->keyWord);
  // varName | varName '[' expression ']' | subroutineCall
  } else if( nextToken->tokenType == IDENTIFIER ) {
      WriteNextToken(fp, tokens, index, *aligment_count, count, IDENTIFIER, NULL);

      nextToken = GetToken(tokens, *index, count);
	  if( nextToken->symbol == '[' ) {
         WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "[");
         CompileExpression(fp, tokens, index, aligment_count, count);
         WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "]");
      } else  if ( nextToken->symbol == '(' ){
         WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "(");
         CompileExpressionList(fp, tokens, index, aligment_count, count);
         WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ")"); 
      } else  if ( nextToken->symbol == '.' ){
         WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ".");
         WriteNextToken(fp, tokens, index, *aligment_count, count, IDENTIFIER, NULL);
         WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "(");
         CompileExpressionList(fp, tokens, index, aligment_count, count);
         WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ")"); 
      }
  // '(' expression ')' | unaryOp term
  } else if ( nextToken->tokenType == SYMBOL ) {
      if( nextToken->symbol == '(' ) {
         WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, "(");
         CompileExpression(fp, tokens, index, aligment_count, count);
         WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ")");
      } else {
         *symbol = IsOp(nextToken->symbol);
         if(*symbol == '-' || *symbol == '~') {
            symbol[1] = '\0';
            WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, symbol);
         }
         CompileTerm(fp, tokens, index, aligment_count, count);
      }
  }

  Decrese(aligment_count);
  WriteTag(fp, "/term", *aligment_count);
}

// (expression (',' expression)* )?
static void CompileExpressionList(FILE* fp, Token **tokens, int *index, int *aligment_count, int count){
  Token *nextToken = NULL;

  WriteTag(fp, "expressionList", *aligment_count);
  Increse(aligment_count);

  // handle expressions
  nextToken = GetToken(tokens, *index, count);
  while(nextToken->symbol != ')'){ // expressions format ( X, XX )
    CompileExpression(fp, tokens, index, aligment_count, count);
	nextToken = GetToken(tokens, *index, count);
    if(nextToken->symbol != ')') {
        WriteNextToken(fp, tokens, index, *aligment_count, count, SYMBOL, ",");
        nextToken = GetToken(tokens, *index, count);
    }
  }

  Decrese(aligment_count);
  WriteTag(fp, "/expressionList", *aligment_count);
}

void CompileClass(FILE* fp, Token **tokens, int count){
  Token *nextToken = NULL;
  int index = 0; // current token index
  int aligment_count = 0;
  
  WriteTag(fp, "class", aligment_count);
  Increse(&aligment_count);
  WriteNextToken(fp, tokens, &index, aligment_count, count, KEYWORD, "class");
  WriteNextToken(fp, tokens, &index, aligment_count, count, IDENTIFIER, NULL);
  WriteNextToken(fp, tokens, &index, aligment_count, count, SYMBOL, "{");

  nextToken = GetToken(tokens, index, count);
  while(nextToken->tokenType == KEYWORD){
    if (IsClassVarDec(nextToken->keyWord) == TRUE ){
        CompileClassVarDec(fp, tokens, &index, &aligment_count, count);
    }

	if (IsSubroutine(nextToken->keyWord) == TRUE ){
        CompileSubroutine(fp, tokens, &index, &aligment_count, count);
    }
	nextToken = GetToken(tokens, index, count);
  }
  WriteNextToken(fp, tokens, &index, aligment_count, count, SYMBOL, "}");
  Decrese(&aligment_count);
  WriteTag(fp, "/class", aligment_count);
}
