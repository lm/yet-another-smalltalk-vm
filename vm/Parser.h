#ifndef PARSER_H
#define PARSER_H

#include "Tokenizer.h"
#include "Object.h"
#include "String.h"
#include "Ast.h"
#include <stdint.h>

typedef enum
{
	PARSER_ERROR_NONE,
	PARSER_ERROR_UNEXPECTED_TOKEN,
} ParserErrorCode;

typedef struct
{
	ParserErrorCode code;
	TokenType expected;
} ParserError;

typedef struct
{
	Tokenizer tokenizer;
	String *sourceOrFileName;
	Class *sourceCodeClass;
	ParserError error;
} Parser;

typedef struct {
	OBJECT_HEADER;
	Value stream;
	Value source;
	Value atEnd;
} RawParserObject;
OBJECT_HANDLE(ParserObject);

typedef struct {
	OBJECT_HEADER;
	Value messageText;
	Value token;
	Value sourceCode;
} RawParseError;
OBJECT_HANDLE(ParseError);

void initParser(Parser *parser, String *source);
void initFileParser(Parser *parser, FILE *file, String *fileName);
void freeParser(Parser *parser);
void printParseError(Parser *parser, char *filename);
ClassNode *parseClass(Parser *parser);
MethodNode *parseMethod(Parser *parser);
BlockNode *parseBlock(Parser *parser);
SourceCode *createSourceCode(Parser *parser, _Bool computeSize);
_Bool parserAtEnd(Parser *parser);

#endif
