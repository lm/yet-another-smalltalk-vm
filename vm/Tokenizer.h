#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdint.h>

#define MAX_TOKENS 6
#define TOKEN_BUFFER_SIZE 64

typedef enum
{
	TOKEN_NONE = 1,
	TOKEN_UNKNOWN = 1 << 1,
	TOKEN_DIGIT = 1 << 2,
	TOKEN_IDENTIFIER = 1 << 3,
	TOKEN_SYMBOL_START = 1 << 4,
	TOKEN_CHAR = 1 << 5,
	TOKEN_STRING = 1 << 6,
	TOKEN_COMMENT = 1 << 7,
	TOKEN_SPECIAL_CHARS = 1 << 8,
	TOKEN_KEYWORD = 1 << 9,
	TOKEN_ASSIGN = 1 << 10,
	TOKEN_RETURN = 1 << 11,
	TOKEN_DOT = 1 << 12,
	TOKEN_ARRAY_OPEN_BRACKET = 1 << 13,
	TOKEN_OPEN_BRACKET = 1 << 14,
	TOKEN_CLOSE_BRACKET = 1 << 15,
	TOKEN_OPEN_SQUARE_BRACKET = 1 << 16,
	TOKEN_CLOSE_SQUARE_BRACKET = 1 << 17,
	TOKEN_END_OF_INPUT = 1 << 18,
	TOKEN_COLON = 1 << 19,
	TOKEN_SEMICOLON = 1 << 20,
	TOKEN_VERTICAL_BAR = 1 << 21,
	TOKEN_LESS_THAN = 1 << 22,
	TOKEN_GREATER_THAN = 1 << 23,
	TOKEN_MINUS = 1 << 24,
	TOKEN_UNCLOSED_STRING = 1 << 25,
	TOKEN_UNCLOSED_COMMENT = 1 << 26,
} TokenType;

typedef struct Token {
	TokenType type;
	size_t contentSize;
	char *content;
	char buffer[TOKEN_BUFFER_SIZE];
	size_t position, line, column;
	_Bool isSeparated;
} Token;

typedef struct {
	_Bool isFile;
	union {
		char *memory;
		FILE *file;
	} source;

	char *current, ch;
	size_t line, column;

	Token tokens[MAX_TOKENS];
	size_t tokenIndex, tokenized;
	Token comment;
} Tokenizer;


void initTokenizer(Tokenizer *tokenizer, char *source);
void initFileTokenizer(Tokenizer *tokenizer, FILE *file);
void freeTokenizer(Tokenizer *tokenizer);

Token *currentToken(Tokenizer *tokenizer);
Token *skipToken(Tokenizer *tokenizer, int8_t steps);
Token *prevToken(Tokenizer *tokenizer);
Token *nextToken(Tokenizer *tokenizer);
Token *peekToken(Tokenizer *tokenizer);
Token *peekNthToken(Tokenizer *tokenizer, int8_t steps);
Token *peekForToken(Tokenizer *tokenizer, TokenType type);

#endif
