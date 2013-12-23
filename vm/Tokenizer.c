#include "Tokenizer.h"
#include "Assert.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stddef.h>

enum {
	SEP = 1,
	SPEC = 2,
	NUM = 3,
	LET = 4,
	IDENT_BEGIN = 5,
	IDENT = 6,
};

static uint8_t Characters[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, SEP, SEP, 0, 0, SEP, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	SEP, SPEC, 0, 0, 0, SPEC, SPEC, 0,
	0, 0, SPEC, SPEC, SPEC, SPEC, 0, SPEC,
	NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM,
	NUM, NUM, 0, 0, SPEC, SPEC, SPEC, SPEC,
	SPEC, LET, LET, LET, LET, LET, LET, LET,
	LET, LET, LET, LET, LET, LET, LET, LET,
	LET, LET, LET, LET, LET, LET, LET, LET,
	LET, LET, LET, 0, SPEC, 0, 0, 0,
	0, LET, LET, LET, LET, LET, LET, LET,
	LET, LET, LET, LET, LET, LET, LET, LET,
	LET, LET, LET, LET, LET, LET, LET, LET,
	LET, LET, LET, 0, SPEC, 0, SPEC, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};
static uint8_t CharacterClasses[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	IDENT, IDENT, IDENT, IDENT, IDENT, IDENT, IDENT, IDENT,
	IDENT, IDENT, 0, 0, 0, 0, 0, 0,
	0, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN,
	IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN,
	IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN,
	IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, 0, 0, 0, 0, IDENT_BEGIN,
	0, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN,
	IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN,
	IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN,
	IDENT_BEGIN, IDENT_BEGIN, IDENT_BEGIN, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};
static uint8_t CharacterDigitValues[] = {
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 36, 36, 36, 36, 36, 36,
	36, 10, 11, 12, 13, 14, 15, 16,
	17, 18, 19, 20, 21, 22, 23, 24,
	25, 26, 27, 28, 29, 30, 31, 32,
	33, 34, 35, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
	36, 36, 36, 36, 36, 36, 36, 36,
};

#define APPEND() \
	appendChar(token, i++, tokenizer->ch); \
	nextChar(tokenizer);
#define APPEND_WHILE(cond) do { APPEND(); } while (cond);
#define SKIP_WHILE(cond) while (cond) { nextChar(tokenizer); };

static void readToken(Tokenizer *tokenizer, Token *token, _Bool isSeparated);
static void appendChar(Token *token, size_t pos, char ch);
static char peekChar(Tokenizer *tokenizer);
static char nextChar(Tokenizer *tokenizer);
static size_t getPosition(Tokenizer *tokenizer);
static _Bool isNumeric(char ch);
static _Bool isDigit(char ch, uint8_t base);
static _Bool isLetter(char ch);
static _Bool isSeparator(char ch);
static _Bool isSpecial(char ch);
static _Bool isIdentifier(char ch);
static _Bool isIdentifierBegining(char ch);


void initBaseTokenizer(Tokenizer *tokenizer)
{
	tokenizer->tokenIndex = tokenizer->tokenized = 0;
	tokenizer->line = 1;
	tokenizer->column = 1;

	for (size_t i = 0; i < MAX_TOKENS; i++) {
		tokenizer->tokens[i].contentSize = TOKEN_BUFFER_SIZE;
		tokenizer->tokens[i].content = tokenizer->tokens[i].buffer;
	}
	tokenizer->comment.contentSize = TOKEN_BUFFER_SIZE;
	tokenizer->comment.content = tokenizer->comment.buffer;
}


void initTokenizer(Tokenizer *tokenizer, char *source)
{
	tokenizer->isFile = 0;
	tokenizer->source.memory = source;
	tokenizer->current = source;
	initBaseTokenizer(tokenizer);
	nextChar(tokenizer);
	nextToken(tokenizer);
}


void initFileTokenizer(Tokenizer *tokenizer, FILE *file)
{
	tokenizer->isFile = 1;
	tokenizer->source.file = file;
	initBaseTokenizer(tokenizer);
	nextChar(tokenizer);
	nextToken(tokenizer);
}


void freeTokenizer(Tokenizer *tokenizer)
{
	for (size_t i = 0; i < MAX_TOKENS; i++) {
		if (tokenizer->tokens[i].contentSize > TOKEN_BUFFER_SIZE) {
			free(tokenizer->tokens[i].content);
		}
	}
	if (tokenizer->comment.contentSize > TOKEN_BUFFER_SIZE) {
		free(tokenizer->comment.content);
	}
}


Token *currentToken(Tokenizer *tokenizer)
{
	return tokenizer->tokens + (tokenizer->tokenIndex - 1) % MAX_TOKENS;
}


Token *skipToken(Tokenizer *tokenizer, int8_t steps)
{
	Token *token = NULL;
	ASSERT(steps < MAX_TOKENS && steps > -MAX_TOKENS);

	if (steps > 0) {
		for (int8_t i = steps; i > 0; i--) {
			token = nextToken(tokenizer);
		}

	} else if ((ptrdiff_t) tokenizer->tokenIndex + steps >= (ptrdiff_t) tokenizer->tokenized - MAX_TOKENS) {
		tokenizer->tokenIndex += steps;
		token = tokenizer->tokens + (tokenizer->tokenIndex - 1) % MAX_TOKENS;
	}

	return token;
}


Token *prevToken(Tokenizer *tokenizer)
{
	return tokenizer->tokens + (tokenizer->tokenIndex - 2) % MAX_TOKENS;
}


Token *nextToken(Tokenizer *tokenizer)
{
	Token *token = peekToken(tokenizer);

	tokenizer->tokenIndex++;
	return token;
}


Token *peekToken(Tokenizer *tokenizer)
{
	size_t i = tokenizer->tokenIndex % MAX_TOKENS;

	if (tokenizer->tokenized <= tokenizer->tokenIndex) {
		readToken(tokenizer, tokenizer->tokens + i, 0);
	}
	return tokenizer->tokens + i;
}


Token *peekNthToken(Tokenizer *tokenizer, int8_t steps)
{
	Token *token = skipToken(tokenizer, steps);
	skipToken(tokenizer, -steps);
	return token;
}


Token *peekForToken(Tokenizer *tokenizer, TokenType type)
{
	Token *token = peekToken(tokenizer);

	if (token->type == type) {
		tokenizer->tokenIndex++;
	} else {
		token = NULL;
	}
	return token;
}


static void readToken(Tokenizer *tokenizer, Token *token, _Bool isSeparated)
{
	size_t i = 0;
	uint8_t base;

	tokenizer->comment.type = TOKEN_UNKNOWN;

	token->isSeparated = isSeparated || isSeparator(tokenizer->ch);
	SKIP_WHILE(isSeparator(tokenizer->ch));
	token->position = getPosition(tokenizer);
	token->line = tokenizer->line;
	token->column = tokenizer->column - 1;

	if (tokenizer->ch == '[') {
		token->type = TOKEN_OPEN_SQUARE_BRACKET;
		APPEND();

	} else if (tokenizer->ch == ']') {
		token->type = TOKEN_CLOSE_SQUARE_BRACKET;
		APPEND();

	} else if (tokenizer->ch == '(') {
		token->type = TOKEN_OPEN_BRACKET;
		APPEND();

	} else if (tokenizer->ch == ')') {
		token->type = TOKEN_CLOSE_BRACKET;
		APPEND();

	} else if (tokenizer->ch == '.') {
		token->type = TOKEN_DOT;
		APPEND();

	} else if (tokenizer->ch == '-' && !isSpecial(peekChar(tokenizer))) {
		token->type = TOKEN_MINUS;
		APPEND();

	} else if (tokenizer->ch == '<' && !isSpecial(peekChar(tokenizer))) {
		token->type = TOKEN_LESS_THAN;
		APPEND();

	} else if (tokenizer->ch == '>' && !isSpecial(peekChar(tokenizer))) {
		token->type = TOKEN_GREATER_THAN;
		APPEND();

	} else if (tokenizer->ch == ':' && peekChar(tokenizer) == '=') {
		token->type = TOKEN_ASSIGN;
		APPEND();
		APPEND();

	} else if (tokenizer->ch == '|' && peekChar(tokenizer) != '|') {
		token->type = TOKEN_VERTICAL_BAR;
		APPEND();

	} else if (tokenizer->ch == ':') {
		token->type = TOKEN_COLON;
		APPEND();

	} else if (tokenizer->ch == ';') {
		token->type = TOKEN_SEMICOLON;
		APPEND();

	} else if (tokenizer->ch == '^') {
		token->type = TOKEN_RETURN;
		APPEND();

	} else if (tokenizer->ch == '$') {
		token->type = TOKEN_CHAR;
		APPEND();
		APPEND();

	} else if (tokenizer->ch == '#' && peekChar(tokenizer) == '(') {
		token->type = TOKEN_ARRAY_OPEN_BRACKET;
		APPEND();
		APPEND();

	} else if (tokenizer->ch == '#') {
		token->type = TOKEN_SYMBOL_START;
		APPEND();

	} else if (tokenizer->ch == '\'') {
		nextChar(tokenizer);
		while (tokenizer->ch != '\0' && (tokenizer->ch != '\'' || nextChar(tokenizer) == '\'')) {
			APPEND();
		}
		token->type = tokenizer->ch == '\0' ? TOKEN_UNCLOSED_STRING : TOKEN_STRING;

	} else if (tokenizer->ch == '"') {
		Token *next = token;
		token = &tokenizer->comment;

		nextChar(tokenizer);
		while (tokenizer->ch != '\0' && (tokenizer->ch != '"' || nextChar(tokenizer) == '"')) {
			APPEND();
		}
		if (tokenizer->ch == '\0') {
			next->type = TOKEN_UNCLOSED_COMMENT;
			memcpy(next->content, token->content, i);
		} else {
			token->type = TOKEN_COMMENT;
			token->position = next->position;
			token->line = next->line;
			token->column = next->column;
			appendChar(token, i, '\0');
			readToken(tokenizer, next, 1);
			return;
		}
		token = next;

	} else if (tokenizer->ch == '\0') {
		token->type = TOKEN_END_OF_INPUT;

	} else if (isNumeric(tokenizer->ch)) {
		token->type = TOKEN_DIGIT;
		APPEND_WHILE(isNumeric(tokenizer->ch));

		if (tokenizer->ch == 'r') {
			token->content[i] = '\0';
			base = atoi(token->content);
			APPEND();
			if (tokenizer->ch == '-') {
				APPEND();
			}
			while (isDigit(tokenizer->ch, base)) {
				APPEND();
			}
		}

	} else if (isIdentifierBegining(tokenizer->ch)) {
		APPEND_WHILE(isIdentifier(tokenizer->ch));
		if (tokenizer->ch == ':' && peekChar(tokenizer) != '=') {
			token->type = TOKEN_KEYWORD;
			APPEND();
		} else {
			token->type = TOKEN_IDENTIFIER;
		}

	} else if (isSpecial(tokenizer->ch)) {
		token->type = TOKEN_SPECIAL_CHARS;
		APPEND_WHILE(isSpecial(tokenizer->ch));

	} else {
		token->type = TOKEN_UNKNOWN;
		APPEND();
	}

	appendChar(token, i, '\0');
	tokenizer->tokenized++;
}


static void appendChar(Token *token, size_t pos, char ch)
{
	if (pos == token->contentSize) {
		token->contentSize *= 2;
		if (pos == TOKEN_BUFFER_SIZE) {
			token->content = malloc(token->contentSize);
			memcpy(token->content, token->buffer, pos);
		} else {
			token->content = realloc(token->content, token->contentSize);
		}
	}
	token->content[pos] = ch;
}


static char peekChar(Tokenizer *tokenizer)
{
	char ch;

	if (tokenizer->isFile) {
		ch = getc(tokenizer->source.file);
		fseek(tokenizer->source.file, -1, SEEK_CUR);
	} else {
		ch = *tokenizer->current;
	}

	return ch;
}


static char nextChar(Tokenizer *tokenizer)
{
	int16_t ch;
	if (tokenizer->isFile) {
		ch = getc(tokenizer->source.file);
		ch = ch == EOF ? '\0' : ch;
	} else {
		ch = *tokenizer->current;
		tokenizer->current++;
	}

	if (ch == '\n') {
		tokenizer->line++;
		tokenizer->column = 1;
	} else {
		tokenizer->column++;
	}

	return tokenizer->ch = ch;
}


static size_t getPosition(Tokenizer *tokenizer)
{
	if (tokenizer->isFile) {
		return ftell(tokenizer->source.file);
	} else {
		return tokenizer->current - tokenizer->source.memory;
	}
}


static _Bool isNumeric(char ch)
{
	return Characters[ch] == NUM;
}


static _Bool isDigit(char ch, uint8_t base)
{
	return CharacterDigitValues[ch] < base;
}


static _Bool isLetter(char ch)
{
	return Characters[ch] == LET;
}


static _Bool isSeparator(char ch)
{
	return Characters[ch] == SEP;
}


static _Bool isSpecial(char ch)
{
	return Characters[ch] == SPEC;
}


static _Bool isIdentifier(char ch)
{
	return CharacterClasses[ch] >= IDENT_BEGIN;
}


static _Bool isIdentifierBegining(char ch)
{
	return CharacterClasses[ch] == IDENT_BEGIN;
}
