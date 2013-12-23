#include "Tokenizer.h"
#include "Assert.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


void testNextToken(Tokenizer *tokenizer)
{
	Token *token;

	token = currentToken(tokenizer);
	ASSERT(token->type == TOKEN_DIGIT);
	ASSERT(strcmp(token->content, "123") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_IDENTIFIER);
	ASSERT(strcmp(token->content, "abc") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_KEYWORD);
	ASSERT(strcmp(token->content, "efg:") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_OPEN_SQUARE_BRACKET);
	ASSERT(strcmp(token->content, "[") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_CLOSE_SQUARE_BRACKET);
	ASSERT(strcmp(token->content, "]") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_SYMBOL_START);
	ASSERT(strcmp(token->content, "#") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_IDENTIFIER);
	ASSERT(strcmp(token->content, "foo") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_CHAR);
	ASSERT(strcmp(token->content, "$a") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_STRING);
	ASSERT(strcmp(token->content, "abc") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_STRING);
	ASSERT(strcmp(token->content, "a'b") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_STRING);
	ASSERT(strcmp(token->content, "") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_STRING);
	ASSERT(strcmp(token->content, "'") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_ASSIGN);
	ASSERT(strcmp(token->content, ":=") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_DIGIT);
	ASSERT(strcmp(token->content, "16r09AF") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_ARRAY_OPEN_BRACKET);
	ASSERT(strcmp(token->content, "#(") == 0);

	token = nextToken(tokenizer);
	ASSERT(token->type == TOKEN_SEMICOLON);
	ASSERT(strcmp(token->content, ";") == 0);
}


void testNextTokenWithString(void)
{
	Tokenizer tokenizer;

	initTokenizer(&tokenizer, "123 abc efg: [ ] #foo $a 'abc' 'a''b' '' '''' := 16r09AF #( ;");
	testNextToken(&tokenizer);
	freeTokenizer(&tokenizer);

}


void testNextTokenWithFile(void)
{
	Tokenizer tokenizer;
	FILE *file = tmpfile();

	fputs("   	                                                                                             123 abc efg: [ ] #foo $a 'abc' 'a''b' '' '''' := 16r09AF #( ;", file);
	rewind(file);
	initFileTokenizer(&tokenizer, file);
	testNextToken(&tokenizer);
	freeTokenizer(&tokenizer);
	fclose(file);
}


void testSkipToken(void)
{
	Tokenizer tokenizer;
	Token *token;

	initTokenizer(&tokenizer, "1 a ( ) [ ]");
	token = skipToken(&tokenizer, 4);
	ASSERT(token->type == TOKEN_OPEN_SQUARE_BRACKET);

	token = skipToken(&tokenizer, -1);
}


int main(void)
{
	testNextTokenWithString();
	testNextTokenWithFile();
	testSkipToken();

	return 0;
}
