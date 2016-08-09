#include "compilationengine.h"
#include "jacktokenizer.h"
#include "symboltable.h"
#include "vmwriter.h"

#include <stdlib.h>
#include <string.h>

#define TRUE   0
#define FALSE -1

static int whileCount = 0;
static int ifCount = 0;

static void CompileClassVarDec(Token **, int *, int);
static void CompileSubroutine(FILE* , Token **, int *, int, char*);
static void CompileParameterList(Token **, int *, int);
static void CompileVarDec(Token **, int *, int);
static void CompileStatements(FILE* , Token **, int *, int, char*);
static void CompileDo(FILE* , Token **, int *, int, char*);
static void CompileLet(FILE* , Token **, int *, int, char*);
static void CompileWhile(FILE* , Token **, int *, int, char*);
static void CompileReturn(FILE* , Token **, int *, int, char*);
static void CompileIf(FILE* , Token **, int *, int, char*);
static void CompileExpression(FILE* , Token **, int *, int, char*);
static void CompileTerm(FILE* , Token **, int *, int, char*);
static int CompileExpressionList(FILE* , Token **, int *, int, char*);

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
      return i;
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

// get Token with given index
static Token *GetToken(Token **tokens, int index, int count){
  Token *currentToken = NULL;
  if ( index < count )
       currentToken = *tokens + index;

  return currentToken;
}

// Index Move forward
static void Advance(int *index){
  (*index)++;
}

static void ClearLabel(){
  whileCount = 0;
  ifCount = 0;
}

static char *GetSegment(short kind){
  switch(kind){
    case STATIC:
        return "static";
    case FIELD:
        return "this";
    case VAR:
        return "local";
    case ARG:
        return "argument";
    default:
        return "";
  }
}

// Get next token to validate it and get its object
static char* ValidateNextToken(Token **tokens, int *index, int count, short tokenType, char* token){
  Token *nextToken = NULL;
  
  nextToken = GetToken(tokens, *index, count);
  if(nextToken) {
    Advance(index);
    if(nextToken->tokenType == tokenType){
        if (tokenType >= IDENTIFIER) { // tokenType is IDENTIFIER, INT_CONST, STRING_CONST
             return nextToken->identifier;
        } else if ( tokenType < IDENTIFIER){ // tokenType is KEYWORD, SYMBOL
             if ( tokenType == KEYWORD ) {
                 if(strcmp(token, nextToken->keyWord) == TRUE)
                    return nextToken->keyWord;
             } else {
                 if(*token == nextToken->symbol)
                    return token;
             }
        }
    }
  }
  printf("Error, next Token should be %s.\n", (token!=NULL)?token:"known");
  return NULL;
}

// ('static' | 'field' ) type varName (',' varName)* ';'
static void CompileClassVarDec(Token **tokens, int *index, int count){
  Token *nextToken = NULL;
  char *token;
  char *name;
  char *type;
  short kind;

  // handle 'static' or 'field'
  nextToken = GetToken(tokens, *index, count);
  token = ValidateNextToken(tokens, index, count, KEYWORD, nextToken->keyWord);
  kind = IsClassVarDec(token);
  
  // handle KEYWORD or IDENTIFIER  
  nextToken = GetToken(tokens, *index, count);
  type = ValidateNextToken(tokens, index, count, nextToken->tokenType, nextToken->keyWord);

  // handle IDENTIFIER
  nextToken = GetToken(tokens, *index, count);
  name = ValidateNextToken(tokens, index, count, IDENTIFIER, NULL);
  
  // define name, type, kind
  Define(name, type, kind);

  // handle ',' if ';' not showing
  nextToken = GetToken(tokens, *index, count);
  while((nextToken->tokenType == SYMBOL) && nextToken->symbol != ';'){
        ValidateNextToken(tokens, index, count, SYMBOL, ",");
        name = ValidateNextToken(tokens, index, count, IDENTIFIER, NULL);
        Define(name, type, kind);
        nextToken = GetToken(tokens, *index, count);
  }
  ValidateNextToken(tokens, index, count, SYMBOL, ";");
}

// ('constructor' | 'function' | 'method') ('void' | type) subroutineName '(' parameterList ')' subroutineBody
static void CompileSubroutine(FILE* fp, Token **tokens, int *index, int count, char* class){
  Token *nextToken = NULL;
  char *kind;
  char *subroutineName;

  ClearLabel();
  StartSubroutine();

  // handle 'constructor', 'function', or 'method'
  nextToken = GetToken(tokens, *index, count);
  kind = ValidateNextToken(tokens, index, count, KEYWORD, nextToken->keyWord);
  if(strcmp(kind, "method") == TRUE) { // if it is method, its first arg is 'this'.
    Define("this", class, ARG);
  }

  // handle KEYWORD or IDENTIFIER, but we don't need it for functions actually, just move forward.
  nextToken = GetToken(tokens, *index, count);
  ValidateNextToken(tokens, index, count, nextToken->tokenType, nextToken->keyWord);

  // handle IDENTIFIER, this is subroutine name
  nextToken = GetToken(tokens, *index, count);
  subroutineName = ValidateNextToken(tokens, index, count, IDENTIFIER, NULL);
  

  // handle '(' parameters ')'
  ValidateNextToken(tokens, index, count, SYMBOL, "(");
  CompileParameterList(tokens, index, count);
  ValidateNextToken(tokens, index, count, SYMBOL, ")");
  
  // Start of subruntineBody
  // '{' varDec* statements '}'
  ValidateNextToken(tokens, index, count, SYMBOL, "{");

  // handle var, if there is not var, the CompileVarDec() should not be executed.
  nextToken = GetToken(tokens, *index, count);
  while((nextToken->tokenType == KEYWORD) && (strcmp(nextToken->keyWord, "var") == TRUE)){
     CompileVarDec(tokens, index, count);
	 nextToken = GetToken(tokens, *index, count);
  }

  // handle statements
  CompileStatements(fp, tokens, index, count, class);

  ValidateNextToken(tokens, index, count, SYMBOL, "}");
  // End of subruntineBody
}

// ( (type varName) (',' type varName)*)?
static void CompileParameterList(Token **tokens, int *index, int count){
  Token *nextToken = NULL;
  char *type = NULL, *name = NULL;
  
  // handle parameters
  nextToken = GetToken(tokens, *index, count);
  while(nextToken->symbol != ')'){ // parameters format ( int X, var XX )
    // handle KEYWORD or IDENTIFIER, it is a type.
    type = ValidateNextToken(tokens, index, count, nextToken->tokenType, nextToken->keyWord);

    // handle IDENTIFIER, this is parameter name.
    nextToken = GetToken(tokens, *index, count);
    name = ValidateNextToken(tokens, index, count, IDENTIFIER, NULL);

    Define(name, type, ARG);
	nextToken = GetToken(tokens, *index, count);
    if(nextToken->symbol != ')') {
        ValidateNextToken(tokens, index, count, SYMBOL, ",");
        nextToken = GetToken(tokens, *index, count);
    }
  }

}

// 'var' type varName (',' varName)* ';'
static void CompileVarDec(Token **tokens, int *index, int count){
  Token *nextToken = NULL;
  char *type = NULL, *name = NULL;
 
  // handle 'var'
  ValidateNextToken(tokens, index, count, KEYWORD, "var");

  // handle KEYWORD or IDENTIFIER, it is a type.
  nextToken = GetToken(tokens, *index, count);
  type = ValidateNextToken(tokens, index, count, nextToken->tokenType, nextToken->keyWord);

  // handle IDENTIFIER, this is var name.
  nextToken = GetToken(tokens, *index, count);
  name = ValidateNextToken(tokens, index, count, IDENTIFIER, NULL);

  Define(name, type, VAR);

  // handle ',' if ';' not showing
  nextToken = GetToken(tokens, *index, count);
  while((nextToken->tokenType == SYMBOL) && (nextToken->symbol != ';')){
        ValidateNextToken(tokens, index, count, SYMBOL, ",");
        name = ValidateNextToken(tokens, index, count, IDENTIFIER, NULL);
        Define(name, type, VAR);
        nextToken = GetToken(tokens, *index, count);
  }
  ValidateNextToken(tokens, index, count, SYMBOL, ";");
}

// statement*
static void CompileStatements(FILE* fp, Token **tokens, int *index, int count, char *class){
  Token *nextToken = NULL;
  int i; // array index
  int mappingCount = sizeof(statementMapping) / sizeof(StatementMapping);

  nextToken = GetToken(tokens, *index, count);
  while(nextToken->symbol != '}'){ // statement end is '}'
    for( i = 0; i < mappingCount; i++ ){
        if(strcmp(nextToken->keyWord, statementMapping[i].statement) == TRUE){
            statementMapping[i].statementFunction(fp, tokens, index, count, class);
            break;
        }
    }
	nextToken = GetToken(tokens, *index, count);
  }
}

// 'do' subroutineCall ';'
// subroutineCall 
// -> subroutineName '(' expressionList ')' | ( className | varName) '.' subroutineName '(' expressionList ')'
static void CompileDo(FILE* fp, Token **tokens, int *index, int count, char *class){
  Token *nextToken = NULL;

  // handle do subroutineName
  ValidateNextToken(tokens, index, count, KEYWORD, "do");
  CompileTerm(fp, tokens, index, count, class);
  WritePop(fp, "temp", 0);
  ValidateNextToken(tokens, index, count, SYMBOL, ";");
}

// let' varName ('[' expression ']')? '=' expression ';'
static void CompileLet(FILE* fp, Token **tokens, int *index, int count, char * class){
  Token *nextToken = NULL;
  char* name = NULL;
  short kind = NONE;
  int symbolIndex = -1;

  // handle let XXX
  ValidateNextToken(tokens, index, count, KEYWORD, "let");
  name = ValidateNextToken(tokens, index, count, IDENTIFIER, NULL);
  kind = KindOf(name);
  symbolIndex = IndexOf(name);

  // handle if there is a '[' expression ']' case
  nextToken = GetToken(tokens, *index, count);
  if((nextToken->tokenType == SYMBOL) && (nextToken->symbol == '[')){
      ValidateNextToken(tokens, index, count, SYMBOL, "[");
      CompileExpression(fp, tokens, index, count, class);
	  nextToken = GetToken(tokens, *index, count);
      ValidateNextToken(tokens, index, count, SYMBOL, "]");
      WritePop(fp, "temp", 0);
      WritePop(fp, "pointer", 1);
      WritePush(fp, "temp", 0);
      WritePop(fp, "that", 0);
  } else {
      // handle '=' expression ';'
      nextToken = GetToken(tokens, *index, count);
      ValidateNextToken(tokens, index, count, SYMBOL, "=");
      CompileExpression(fp, tokens, index, count, class);  
      WritePop(fp, GetSegment(kind), symbolIndex);
  }
  ValidateNextToken(tokens, index, count, SYMBOL, ";");
}

// 'while' '(' expression ')' '{' statements '}'
static void CompileWhile(FILE* fp, Token **tokens, int *index, int count, char *class){
  char labelStart[16];
  char labelEnd[16];
  // 'while' '(' expression ')'
  ValidateNextToken(tokens, index, count, KEYWORD, "while");

  sprintf(labelStart, "WHILE_START%d", whileCount);
  sprintf(labelEnd, "WHILE_END%d", whileCount);
  whileCount++;

  WriteLabel(fp, labelStart);
  ValidateNextToken(tokens, index, count, SYMBOL, "(");
  CompileExpression(fp, tokens, index, count, class);
  ValidateNextToken(tokens, index, count, SYMBOL, ")");

  WriteArithmetic(fp, "not");
  WriteIf(fp, labelEnd);

  // '{' statements '}'
  ValidateNextToken(tokens, index, count, SYMBOL, "{");
  CompileStatements(fp, tokens, index, count, class);
  ValidateNextToken(tokens, index, count, SYMBOL, "}");

  WriteGoto(fp, labelStart);
  WriteLabel(fp, labelEnd);
}

// 'return' expression? ';'
static void CompileReturn(FILE* fp, Token **tokens, int *index, int count, char *class){
  Token *nextToken = NULL;
  // 'return'
  ValidateNextToken(tokens, index, count, KEYWORD, "return");

  // handle expression ';'
  nextToken = GetToken(tokens, *index, count);
  if(nextToken->tokenType != SYMBOL || nextToken->symbol != ';') {
    CompileExpression(fp, tokens, index, count, class);
  } else {
    WritePush(fp, "constant", 0); // if is void return 0
  }
  ValidateNextToken(tokens, index, count, SYMBOL, ";");
}

// 'if' '(' expression ')' '{' statements '}' ( 'else' '{' statements '}' )?
static void CompileIf(FILE* fp, Token **tokens, int *index, int count, char *class){
  Token *nextToken = NULL;
  char labelTrue[16];
  char labelFalse[16];
  char labelEnd[16];

  // 'if' '(' expression ')'
  ValidateNextToken(tokens, index, count, KEYWORD, "if");
  sprintf(labelTrue, "IF_TRUE%d", ifCount);
  sprintf(labelFalse, "IF_FALSE%d", ifCount);
  sprintf(labelEnd, "IF_END%d", ifCount);
  ifCount++;

  ValidateNextToken(tokens, index, count, SYMBOL, "(");
  CompileExpression(fp, tokens, index, count, class);
  ValidateNextToken(tokens, index, count, SYMBOL, ")");

  WriteIf(fp, labelTrue);    // if-goto IF_TRUE
  WriteGoto(fp, labelFalse); // goto IF_FALSE
  WriteLabel(fp, labelTrue); // IF_TRUE

  // '{' statements '}'
  ValidateNextToken(tokens, index, count, SYMBOL, "{");
  CompileStatements(fp, tokens, index, count, class);
  ValidateNextToken(tokens, index, count, SYMBOL, "}");

  // 'else' '{' statements '}'
  nextToken = GetToken(tokens, *index, count);
  if(nextToken->tokenType == KEYWORD && strcmp(nextToken->keyWord, "else") == TRUE) {
     WriteGoto(fp, labelEnd); // goto IF_END
  }
  WriteLabel(fp, labelFalse);
  if(nextToken->tokenType == KEYWORD && strcmp(nextToken->keyWord, "else") == TRUE) {
    ValidateNextToken(tokens, index, count, KEYWORD, "else");
	ValidateNextToken(tokens, index, count, SYMBOL, "{");
    CompileStatements(fp, tokens, index, count, class);
    ValidateNextToken(tokens, index, count, SYMBOL, "}");
    WriteLabel(fp, labelEnd);
  }
}

// term (op term)*
static void CompileExpression(FILE* fp, Token **tokens, int *index, int count, char* class){
  Token *nextToken = NULL;
  char *symbol;

  // term
  CompileTerm(fp, tokens, index, count, class);

  // (op term)*
  nextToken = GetToken(tokens, *index, count);
  while( (nextToken->tokenType == SYMBOL) && (IsOp(nextToken->symbol) != FALSE)){
    if( nextToken->symbol == '/' ) {
        WriteCall(fp, "Math.divide", 2);
	} else if ( nextToken->symbol == '*' ) {
        WriteCall(fp, "Math.multiply", 2);
	} else {
        if( nextToken->symbol == '=')
            symbol = "eq";
        else if( nextToken->symbol == '+')
            symbol = "add";
        else if( nextToken->symbol == '-')
            symbol = "sub";
        else if( nextToken->symbol == '&')
            symbol = "and";
        else if( nextToken->symbol == '|')
            symbol = "or";
        else if( nextToken->symbol == '~')
            symbol = "not";
        else if( nextToken->symbol == '<')
            symbol = "lt";
        else if( nextToken->symbol == '>')
            symbol = "gt";
        WriteArithmetic(fp, symbol);
	}
    Advance(index);
    CompileTerm(fp, tokens, index, count, class);
	nextToken = GetToken(tokens, *index, count);
  }
}

// integerConstant | stringConstant | keywordConstant | varName |
// varName '[' expression ']' | subroutineCall | '(' expression ')' | unaryOp term
static void CompileTerm(FILE* fp, Token **tokens, int *index, int count, char* class){
  Token *nextToken = NULL;
  char symbol[2], *op = NULL;
  char *name = NULL;
  char *function = NULL;
  short kind = NONE;
  int symbolIndex = -1, i;
  char subroutine[32];
  int expressionCount = 0;

  // integerConstant
  nextToken = GetToken(tokens, *index, count);
  if( nextToken->tokenType == INT_CONST ) {
      WritePush(fp, "constant", nextToken->intVal);
  // stringConstant
  } else if ( nextToken->tokenType == STRING_CONST ) {
      WritePush(fp, "constant", strlen(nextToken->stringVal)); // argument for String.new
      WriteCall(fp, "String.new", 1);                          // Create Empty String of stringVal length
      for ( i = 0; i < strlen(nextToken->stringVal); i++ ){
          WritePush(fp, "constant", (int)nextToken->stringVal[i]);
          WriteCall(fp, "String.appendChar", 2); // 
	  }
  // keywordConstant
  } else if (nextToken->tokenType == KEYWORD ) {
      if (strcmp(nextToken->keyWord, "true") == TRUE) {
         WritePush(fp, "constant", 0);
         WriteArithmetic(fp, "not");
	  } else if (strcmp(nextToken->keyWord, "false") == TRUE ||
	             strcmp(nextToken->keyWord, "null") == TRUE ) {
         WritePush(fp, "constant", 0);
      } else if (strcmp(nextToken->keyWord, "this") == TRUE) {
         WritePush(fp, "pointer", 0);
      }
  // varName | varName '[' expression ']' | subroutineCall
  } else if( nextToken->tokenType == IDENTIFIER ) {
      name = ValidateNextToken(tokens, index, count, IDENTIFIER, NULL);
      kind = KindOf(name);
      symbolIndex = IndexOf(name);
      nextToken = GetToken(tokens, *index, count);
	  if( nextToken->symbol == '[' ) {
         ValidateNextToken(tokens, index, count, SYMBOL, "[");
         CompileExpression(fp, tokens, index, count, class);
         ValidateNextToken(tokens, index, count, SYMBOL, "]");
         WritePush(fp, GetSegment(kind), symbolIndex);
         WriteArithmetic(fp, "add");
         WritePop(fp, "pointer", 1);
         WritePush(fp, "that", 0);
      } else  if ( nextToken->symbol == '(' ){
         sprintf(subroutine, "%s.%s", class, name);
         WritePush(fp, "pointer", 0);
         ValidateNextToken(tokens, index, count, SYMBOL, "(");
         expressionCount = CompileExpressionList(fp, tokens, index, count, class) + 1;
         ValidateNextToken(tokens, index, count, SYMBOL, ")"); 
         WriteCall(fp, subroutine, expressionCount);
      } else  if ( nextToken->symbol == '.' ){
         ValidateNextToken(tokens, index, count, SYMBOL, ".");
         function = ValidateNextToken(tokens, index, count, IDENTIFIER, NULL);
         if(kind < ARG) {
            WritePush(fp, GetSegment(kind), symbolIndex);
            expressionCount = 1;
		 }
         ValidateNextToken(tokens, index, count, SYMBOL, "(");
         expressionCount += CompileExpressionList(fp, tokens, index, count, class);
         ValidateNextToken(tokens, index, count, SYMBOL, ")");
         sprintf(subroutine, "%s.%s", TypeOf(name), function);
         WriteCall(fp, subroutine, expressionCount);
      } else { // it is a varName
         WritePush(fp, GetSegment(kind), symbolIndex);
	  }
  // '(' expression ')' | unaryOp term
  } else if ( nextToken->tokenType == SYMBOL ) {
      if( nextToken->symbol == '(' ) {
         ValidateNextToken(tokens, index, count, SYMBOL, "(");
         CompileExpression(fp, tokens, index, count, class);
         ValidateNextToken(tokens, index, count, SYMBOL, ")");
      } else {
         *symbol = IsOp(nextToken->symbol);
         if ( *symbol == '-' ) {
            op = "neg";
		 } else if ( *symbol == '~') {
            op = "not";
         }
		 
         if ( op != NULL) {
            WriteArithmetic(fp, op);
            symbol[1] = '\0';
            ValidateNextToken(tokens, index, count, SYMBOL, symbol);
		 }
         CompileTerm(fp, tokens, index, count, class);
      }
  }
}

// (expression (',' expression)* )?
static int CompileExpressionList(FILE* fp, Token **tokens, int *index, int count, char *class){
  Token *nextToken = NULL;
  int expressionCount = 0;

  // handle expressions
  nextToken = GetToken(tokens, *index, count);
  while(nextToken->symbol != ')'){ // expressions format ( X, XX )
    CompileExpression(fp, tokens, index, count, class);
	nextToken = GetToken(tokens, *index, count);
    if( nextToken->tokenType == SYMBOL && nextToken->symbol != ')') {
        ValidateNextToken(tokens, index, count, SYMBOL, ",");
        nextToken = GetToken(tokens, *index, count);
    }
    expressionCount++;
  }
  return expressionCount;
}

void CompileClass(char* fileName, Token **tokens, int count){
  Token *nextToken = NULL;
  Symbol *classScopeTable = NULL;
  Symbol *subroutineScopeTable = NULL;
  char *className = NULL;
  int index = 0; // current token index

  // init the VMWriter  
  FILE* vmWriter = VMWriter(fileName);

  // Creates a new empty symbol table
  classScopeTable = (Symbol*)malloc(sizeof(Symbol) * MAX_SYMBOL_NUM);
  subroutineScopeTable = (Symbol*)malloc(sizeof(Symbol) * MAX_SYMBOL_NUM);
  SymbolTable(&classScopeTable, &subroutineScopeTable);


  ValidateNextToken(tokens, &index, count, KEYWORD, "class");
  className = ValidateNextToken(tokens, &index, count, IDENTIFIER, NULL);
  ValidateNextToken(tokens, &index, count, SYMBOL, "{");

  nextToken = GetToken(tokens, index, count);
  while(nextToken->tokenType == KEYWORD){
    if (IsClassVarDec(nextToken->keyWord) != FALSE ){
        CompileClassVarDec(tokens, &index, count);
    }

	if (IsSubroutine(nextToken->keyWord) != FALSE ){
        CompileSubroutine(vmWriter, tokens, &index, count, className);
    }
	nextToken = GetToken(tokens, index, count);
  }
  ValidateNextToken(tokens, &index, count, SYMBOL, "}");
  Close(vmWriter);
}
