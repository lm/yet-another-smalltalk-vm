#include "Parser.h"
#include "Smalltalk.h"
#include "Heap.h"
#include "Collection.h"
#include "Handle.h"
#include "Assert.h"
#include <string.h>
#include <stdlib.h>

#define COLLECTION_SIZE 64

#define CATCH(parseFce, result) \
	parseFce; \
	if (parser->error.code != PARSER_ERROR_NONE) return result;

#define SKIP_TOKEN(tokenType, result) \
	EXPECT_TOKEN(tokenType, result) \
	nextToken(&parser->tokenizer);

#define EXPECT_TOKEN(tokenType, result) \
	if ((currentToken(&parser->tokenizer)->type & tokenType) == 0) { \
		errorExpected(parser, tokenType); \
		return result; \
	}

#define NEXT_EXPECT_TOKEN(token, tokenType, result) \
	token = nextToken(&parser->tokenizer); \
	if ((token->type & tokenType) == 0) { \
		errorExpected(parser, tokenType); \
		return result; \
	}

#define SPECIAL_CHARS_TOKENS (TOKEN_SPECIAL_CHARS | TOKEN_VERTICAL_BAR | TOKEN_MINUS | TOKEN_LESS_THAN | TOKEN_GREATER_THAN)

static OrderedCollection *parseMethods(Parser *parser);
static _Bool isMethodFirstToken(Token *token);
static String *parseMessagePattern(Parser *parser, OrderedCollection *args);
static String *parseUnaryMessagePattern(Parser *parser);
static String *parseBinaryMessagePattern(Parser *parser, OrderedCollection *args);
static String *parseKeywordMessagePattern(Parser *parser, OrderedCollection *args);
static OrderedCollection *parsePragmas(Parser *parser);
static MessageExpressionNode *parsePragma(Parser *parser);
static OrderedCollection *parseTempVars(Parser *parser);
static OrderedCollection *parseExpressions(Parser *parser);
static _Bool isExpression(Token *token);
static ExpressionNode *parseExpression(Parser *parser);
static OrderedCollection *parseAssigments(Parser *parser);
static Object *parseBinaryObject(Parser *parser);
static Object *parseUnaryObject(Parser *parser);
static ExpressionNode *convertToExpression(LiteralNode *literal, MessageExpressionNode *messageExpression);
static Object *parsePrimary(Parser *parser);
static LiteralNode *parseVariable(Parser *parser);
static OrderedCollection *parseBlockArgs(Parser *parser);
static ExpressionNode *parseBracketedStatement(Parser *parser);
static Object *parseLiteral(Parser *parser);
static LiteralNode *parseNumber(Parser *parser, int8_t sign);
static SignedValue parseInteger(Parser *parser);
static LiteralNode *parseSymbol(Parser *parser);
static LiteralNode *parseString(Parser *parser);
static LiteralNode *parseChar(Parser *parser);
static LiteralNode *parseArray(Parser *parser);
static LiteralNode *parseArrayLiteral(Parser *parser);
static _Bool isArrayLiteral(Token *token);
static MessageExpressionNode *parseCascadeExpression(Parser *parser);
static MessageExpressionNode *parseUnaryExpression(Parser *parser);
static MessageExpressionNode *parseBinaryExpression(Parser *parser);
static MessageExpressionNode *parseKeywordExpression(Parser *parser);
static MessageExpressionNode *createMessageExpressionNode(String *selector, OrderedCollection *args, SourceCode *source);
static void computeSourceCodeSize(SourceCode *code, Parser *parser);
static void errorExpected(Parser *parser, TokenType tokens);


void initParser(Parser *parser, String *source)
{
	char *buffer = malloc(source->raw->size + 1);
	stringPrintOn(source, buffer);
	initTokenizer(&parser->tokenizer, buffer);
	parser->sourceOrFileName = source;
	parser->sourceCodeClass = Handles.SourceCode;
	parser->error.code = PARSER_ERROR_NONE;
}


void initFileParser(Parser *parser, FILE *file, String *fileName)
{
	initFileTokenizer(&parser->tokenizer, file);
	parser->sourceOrFileName = fileName;
	parser->sourceCodeClass = Handles.FileSourceCode;
	parser->error.code = PARSER_ERROR_NONE;
}


void freeParser(Parser *parser)
{
	if (!parser->tokenizer.isFile) {
		free(parser->tokenizer.source.memory);
	}
	freeTokenizer(&parser->tokenizer);
}


void printParseError(Parser *parser, char *filename)
{
	Token *token = currentToken(&parser->tokenizer);
	char *format = "Parse error: unexpected %s%s%s in '%s' line %li column %li\n";
	char *prefix = "";
	char *suffix = "";
	if (token->type == TOKEN_STRING) {
		prefix = suffix = "'";
	} else if (token->type == TOKEN_UNCLOSED_STRING) {
		prefix = "'";
	} else if (token->type == TOKEN_UNCLOSED_COMMENT) {
		prefix = "\"";
	} else if (token->type == TOKEN_END_OF_INPUT) {
		prefix = parser->tokenizer.isFile ? "end of file" : "end of input";
	}
	printf(format, prefix, token->content, suffix, filename, token->line, token->column);
}


/**
 * classHeader := identifier "subclass:" identifier "[" pragmas vars
 */
ClassNode *parseClass(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	ClassNode *class = newObject(Handles.ClassNode, 0);
	SourceCode *sourceCode = createSourceCode(parser, 0);
	OrderedCollection *tmp;

	classNodeSetSourceCode(class, sourceCode);

	EXPECT_TOKEN(TOKEN_IDENTIFIER, closeHandleScope(&scope, NULL));
	classNodeSetName(class, parseVariable(parser));

	SKIP_TOKEN(TOKEN_ASSIGN, closeHandleScope(&scope, NULL));

	EXPECT_TOKEN(TOKEN_IDENTIFIER, closeHandleScope(&scope, NULL));
	classNodeSetSuperName(class, parseVariable(parser));

	SKIP_TOKEN(TOKEN_OPEN_SQUARE_BRACKET, closeHandleScope(&scope, NULL));

	CATCH(tmp = parsePragmas(parser), closeHandleScope(&scope, NULL));
	classNodeSetPragmas(class, tmp);

	CATCH(tmp = parseTempVars(parser), closeHandleScope(&scope, NULL));
	classNodeSetVars(class, tmp);

	CATCH(tmp = parseMethods(parser), closeHandleScope(&scope, NULL));
	classNodeSetMethods(class, tmp);

	SKIP_TOKEN(TOKEN_CLOSE_SQUARE_BRACKET, closeHandleScope(&scope, NULL));
	computeSourceCodeSize(sourceCode, parser);

	return closeHandleScope(&scope, class);
}


static OrderedCollection *parseMethods(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);
	OrderedCollection *methods = newOrdColl(32);

	while (isMethodFirstToken(currentToken(&parser->tokenizer))) {
		CATCH(MethodNode *method = parseMethod(parser), closeHandleScope(&scope, NULL));
		ordCollAddObject(methods, (Object *) method);
	}

	return closeHandleScope(&scope, methods);
}


static _Bool isMethodFirstToken(Token *token)
{
	return token->type & (TOKEN_IDENTIFIER | SPECIAL_CHARS_TOKENS | TOKEN_KEYWORD);
}


/**
 * method := messagePattern tempVars
 */
MethodNode *parseMethod(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	MethodNode *method = newObject(Handles.MethodNode, 0);
	BlockNode *block = newObject(Handles.BlockNode, 0);
	SourceCode *sourceCode = createSourceCode(parser, 0);
	OrderedCollection *args = newOrdColl(16);
	Object *tmp;

	methodNodeSetSourceCode(method, sourceCode);

	if (currentToken(&parser->tokenizer)->type == TOKEN_IDENTIFIER && isMethodFirstToken(peekToken(&parser->tokenizer)))  {
		methodNodeSetClassName(method, asString(currentToken(&parser->tokenizer)->content));
		nextToken(&parser->tokenizer);
	}

	CATCH(tmp = (Object *) parseMessagePattern(parser, args), closeHandleScope(&scope, NULL));
	methodNodeSetSelector(method, (String *) tmp);

	SKIP_TOKEN(TOKEN_OPEN_SQUARE_BRACKET, closeHandleScope(&scope, NULL));

	CATCH(tmp = (Object *) parsePragmas(parser), closeHandleScope(&scope, NULL));
	methodNodeSetPragmas(method, (OrderedCollection *) tmp);

	CATCH(tmp = (Object *) parseTempVars(parser), closeHandleScope(&scope, NULL));
	blockNodeSetTempVars(block, (OrderedCollection *) tmp);

	CATCH(tmp = (Object *) parseExpressions(parser), closeHandleScope(&scope, NULL));
	blockNodeSetExpressions(block, (OrderedCollection *) tmp);

	SKIP_TOKEN(TOKEN_CLOSE_SQUARE_BRACKET, closeHandleScope(&scope, NULL));
	computeSourceCodeSize(sourceCode, parser);

	blockNodeSetArgs(block, args);
	methodNodeSetBody(method, block);
	return closeHandleScope(&scope, method);
}


/**
 * messagePattern := unaryMessagePattern | binaryMessagePattern | keywordMessagePattern
 */
static String *parseMessagePattern(Parser *parser, OrderedCollection *args)
{
	switch (currentToken(&parser->tokenizer)->type) {
	case TOKEN_IDENTIFIER:
		return parseUnaryMessagePattern(parser);

	case TOKEN_SPECIAL_CHARS:
	case TOKEN_VERTICAL_BAR:
	case TOKEN_MINUS:
	case TOKEN_LESS_THAN:
	case TOKEN_GREATER_THAN:
		return parseBinaryMessagePattern(parser, args);

	case TOKEN_KEYWORD:
		return parseKeywordMessagePattern(parser, args);

	default:
		errorExpected(parser, TOKEN_IDENTIFIER | SPECIAL_CHARS_TOKENS | TOKEN_VERTICAL_BAR | TOKEN_KEYWORD);
		return NULL;
	}
}


/**
 * unaryMessagePattern := identifier
 */
static String *parseUnaryMessagePattern(Parser *parser)
{
	String *selector = asString(currentToken(&parser->tokenizer)->content);
	nextToken(&parser->tokenizer);
	return selector;
}


/**
 * binaryMessagePattern := specialChars identifier
 */
static String *parseBinaryMessagePattern(Parser *parser, OrderedCollection *args)
{
	Token *token;
	String *selector = asString(currentToken(&parser->tokenizer)->content);

	NEXT_EXPECT_TOKEN(token, TOKEN_IDENTIFIER, NULL);
	ordCollAddObject(args, (Object *) parseVariable(parser));
	return selector;
}


/**
 * keywordMessagePattern := keywordMessagePatternPair
 * keywordMessagePatternPair := keyword identifier keywordMessagePatternPair | keywordMessagePatternPair
 */
static String *parseKeywordMessagePattern(Parser *parser, OrderedCollection *args)
{
	Token *token = currentToken(&parser->tokenizer);
	String *selector = newString(64);
	size_t size = 0;

	while (token->type == TOKEN_KEYWORD) {
		size_t contentSize = strlen(token->content);
		size += contentSize;
		if (selector->raw->size < size) {
			selector = (String *) copyResizedObject((Object *) selector, size + 64);
		}
		memcpy(selector->raw->contents + size - contentSize, token->content, contentSize);

		NEXT_EXPECT_TOKEN(token, TOKEN_IDENTIFIER, NULL);
		ordCollAddObject(args, (Object *) parseVariable(parser));
		token = currentToken(&parser->tokenizer);
	}

	return (String *) copyResizedObject((Object *) selector, size);
}


/**
 * pragmas := "<" pragma ">" pragmas | "<" pragma ">" | ""
 */
static OrderedCollection *parsePragmas(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	OrderedCollection *pragmas = newOrdColl(8);
	MessageExpressionNode *pragma;

	while (currentToken(&parser->tokenizer)->type == TOKEN_LESS_THAN && peekToken(&parser->tokenizer)->type == TOKEN_KEYWORD) {
		nextToken(&parser->tokenizer);
		CATCH(pragma = parsePragma(parser), closeHandleScope(&scope, NULL));
		SKIP_TOKEN(TOKEN_GREATER_THAN, closeHandleScope(&scope, NULL));
		ordCollAddObject(pragmas, (Object *) pragma);
	}
	return closeHandleScope(&scope, pragmas);
}


/**
 * pragma := keyword literal pragma | keyword literal
 */
static MessageExpressionNode *parsePragma(Parser *parser)
{
	Token *token;
	SourceCode *source = createSourceCode(parser, 0);
	OrderedCollection *args = newOrdColl(8);
	String *selector = newString(64);
	size_t size = 0;

	while ((token = currentToken(&parser->tokenizer))->type == TOKEN_KEYWORD) {
		size_t contentSize = strlen(token->content);
		size += contentSize;
		if (selector->raw->size < size) {
			selector = (String *) copyResizedObject((Object *) selector, size + 64);
		}
		memcpy(selector->raw->contents + size - contentSize, token->content, contentSize);

		if (nextToken(&parser->tokenizer)->type == TOKEN_IDENTIFIER) {
			ordCollAddObject(args, (Object *) parseVariable(parser));
		} else {
			ordCollAddObject(args, (Object *) parseLiteral(parser));
		}
	}

	selector = (String *) copyResizedObject((Object *) selector, size);
	computeSourceCodeSize(source, parser);
	return createMessageExpressionNode(selector, args, source);
}


/**
 * tempVars := "|" tempVarNames "|" : ""
 * tempVarNames := variable tempVarNames | variable | ""
 */
static OrderedCollection *parseTempVars(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	OrderedCollection *tempVars = newOrdColl(16);

	if (currentToken(&parser->tokenizer)->type != TOKEN_VERTICAL_BAR) {
		return closeHandleScope(&scope, tempVars);
	}

	nextToken(&parser->tokenizer);

	while (currentToken(&parser->tokenizer)->type == TOKEN_IDENTIFIER) {
		ordCollAddObject(tempVars, (Object *) parseVariable(parser));
	}

	SKIP_TOKEN(TOKEN_VERTICAL_BAR, closeHandleScope(&scope, NULL));
	return closeHandleScope(&scope, tempVars);
}


/**
 * expressions := expression "." expressions returnExpression | expression "." expressions | ""
 */
static OrderedCollection *parseExpressions(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	OrderedCollection *expressions = newOrdColl(16);
	ExpressionNode *expression;

	while (isExpression(currentToken(&parser->tokenizer))) {
		CATCH(expression = parseExpression(parser), closeHandleScope(&scope, NULL));
		ordCollAddObject(expressions, (Object *) expression);

		if (currentToken(&parser->tokenizer)->type == TOKEN_DOT) {
			nextToken(&parser->tokenizer);
		} else {
			break;
		}
		if (expressionNodeReturns(expression)) {
			break;
		}
	}

	return closeHandleScope(&scope, expressions);
}


static _Bool isExpression(Token *token)
{
	switch (token->type) {
	case TOKEN_RETURN:
	case TOKEN_DIGIT:
	case TOKEN_IDENTIFIER:
	case TOKEN_SYMBOL_START:
	case TOKEN_CHAR:
	case TOKEN_STRING:
	case TOKEN_ARRAY_OPEN_BRACKET:
	case TOKEN_OPEN_BRACKET:
	case TOKEN_OPEN_SQUARE_BRACKET:
	case TOKEN_MINUS:
		return 1;

	default:
		return 0;
	}
}


/**
 * statement := "^" assigments unaryObject expression cascadeExpressions
 * cascadeExpressions := ";" cascadeExpression cascadeExpressions | ""
 */
static ExpressionNode *parseExpression(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	ExpressionNode *expression = newObject(Handles.ExpressionNode, 0);
	SourceCode *sourceCode = createSourceCode(parser, 0);
	Object *receiver;
	OrderedCollection *messageExpressions;
	Object *tmp;

	expressionNodeSetSourceCode(expression, sourceCode);

	if (currentToken(&parser->tokenizer)->type == TOKEN_RETURN) {
		expressionNodeEnableReturn(expression);
		nextToken(&parser->tokenizer);
	} else {
		expressionNodeDisableReturn(expression);
	}

	CATCH(tmp = (Object *) parseAssigments(parser), closeHandleScope(&scope, NULL));
	expressionNodeSetAssigments(expression, (OrderedCollection *) tmp);

	CATCH(receiver = parseBinaryObject(parser), closeHandleScope(&scope, NULL));

	if (currentToken(&parser->tokenizer)->type == TOKEN_KEYWORD) {
		CATCH(tmp = (Object *) parseKeywordExpression(parser), closeHandleScope(&scope, NULL));
		messageExpressions = newOrdColl(8);
		ordCollAddObject(messageExpressions, tmp);
		expressionNodeSetReceiver(expression, (LiteralNode *) receiver);

	} else if (receiver->raw->class == Handles.ExpressionNode->raw) {
		messageExpressions = expressionNodeGetMessageExpressions((ExpressionNode *) receiver);
		expressionNodeSetReceiver(expression, expressionNodeGetReceiver((ExpressionNode *) receiver));

	} else {
		messageExpressions = newOrdColl(0);
		expressionNodeSetReceiver(expression, (LiteralNode *) receiver);
	}

	while (currentToken(&parser->tokenizer)->type == TOKEN_SEMICOLON) {
		HandleScope scope2;
		openHandleScope(&scope2);
		CATCH(tmp = (Object *) parseCascadeExpression(parser), (closeHandleScope(&scope2, NULL), closeHandleScope(&scope, NULL)));
		ordCollAddObject(messageExpressions, tmp);
		closeHandleScope(&scope2, NULL);
	}

	expressionNodeSetMessageExpressions(expression, messageExpressions);
	computeSourceCodeSize(sourceCode, parser);
	return closeHandleScope(&scope, expression);
}


/**
 * assigments := identifier ":=" assigments | ""
 */
static OrderedCollection *parseAssigments(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	OrderedCollection *assigments = newOrdColl(8);
	Token *token = currentToken(&parser->tokenizer);

	while (token->type == TOKEN_IDENTIFIER && peekToken(&parser->tokenizer)->type == TOKEN_ASSIGN) {
		ordCollAddObject(assigments, (Object *) parseVariable(parser));
		token = nextToken(&parser->tokenizer);
	}

	return closeHandleScope(&scope, assigments);
}


/**
 * binaryObject := unaryObject binaryExpression
 */
static Object *parseBinaryObject(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	Object *literal;
	MessageExpressionNode *tmp;

	CATCH(literal = parseUnaryObject(parser), closeHandleScope(&scope, NULL));

	while (currentToken(&parser->tokenizer)->type & SPECIAL_CHARS_TOKENS) {
		CATCH(tmp = parseBinaryExpression(parser), closeHandleScope(&scope, NULL));
		literal = (Object *) convertToExpression((LiteralNode *) literal, tmp);
	}
	return closeHandleScope(&scope, literal);
}


/**
 * unaryObject := unaryObject unaryExpression | primary
 */
static Object *parseUnaryObject(Parser *parser)
{
	Object *literal;
	MessageExpressionNode *tmp;

	CATCH(literal = parsePrimary(parser), NULL);

	while (currentToken(&parser->tokenizer)->type == TOKEN_IDENTIFIER) {
		CATCH(tmp = parseUnaryExpression(parser), NULL);
		literal = (Object *) convertToExpression((LiteralNode *) literal, tmp);
	}
	return literal;
}


static ExpressionNode *convertToExpression(LiteralNode *literal, MessageExpressionNode *messageExpression)
{
	ExpressionNode *expression = newObject(Handles.ExpressionNode, 0);
	OrderedCollection *messageExpressions = newOrdColl(8);

	expressionNodeSetSourceCode(expression, literalNodeGetSourceCode(literal));
	ordCollAddObject(messageExpressions, (Object *) messageExpression);
	expressionNodeDisableReturn(expression);
	expressionNodeSetAssigments(expression, newOrdColl(0));
	expressionNodeSetReceiver(expression, literal);
	expressionNodeSetMessageExpressions(expression, messageExpressions);
	return expression;
}


/**
 * primary := variable | block | bracketedStatement | literal
 */
static Object *parsePrimary(Parser *parser)
{
	Object *literal;

	switch (currentToken(&parser->tokenizer)->type) {
	case TOKEN_IDENTIFIER:
		return (Object *) parseVariable(parser);

	case TOKEN_OPEN_SQUARE_BRACKET:
		return (Object *) parseBlock(parser);

	case TOKEN_OPEN_BRACKET:
		return (Object *) parseBracketedStatement(parser);

	default:
		literal = parseLiteral(parser);
		if (parser->error.code != PARSER_ERROR_NONE) {
			parser->error.expected |= TOKEN_IDENTIFIER | TOKEN_OPEN_SQUARE_BRACKET | TOKEN_OPEN_BRACKET;
			return NULL;
		}
		return literal;
	}
}


/**
 * variable := identifier
 */
static LiteralNode *parseVariable(Parser *parser)
{
	char *content = currentToken(&parser->tokenizer)->content;
	LiteralNode *literal;
	Class *class;

	if (strcmp("nil", content) == 0) {
		class = Handles.NilNode;
	} else if (strcmp("true", content) == 0) {
		class = Handles.TrueNode;
	} else if (strcmp("false", content) == 0) {
		class = Handles.FalseNode;
	} else {
		class = Handles.VariableNode;
	}

	literal = (LiteralNode *) newObject(class, 0);
	literalNodeSetValue(literal, (Object *) asString(content));
	literalNodeSetSourceCode(literal, createSourceCode(parser, 1));
	nextToken(&parser->tokenizer);
	return literal;
}


/**
 * block := "[" blockArgs tempVars statements "]"
 */
BlockNode *parseBlock(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	BlockNode *block = newObject(Handles.BlockNode, 0);
	SourceCode *sourceCode = createSourceCode(parser, 0);
	Object *tmp;

	blockNodeSetSourceCode(block, sourceCode);
	SKIP_TOKEN(TOKEN_OPEN_SQUARE_BRACKET, closeHandleScope(&scope, NULL));

	CATCH(tmp = (Object *) parseBlockArgs(parser), closeHandleScope(&scope, NULL));
	blockNodeSetArgs(block, (OrderedCollection *) tmp);

	CATCH(tmp = (Object *) parseTempVars(parser), closeHandleScope(&scope, NULL));
	blockNodeSetTempVars(block, (OrderedCollection *) tmp);

	CATCH(tmp = (Object *) parseExpressions(parser), closeHandleScope(&scope, NULL));
	blockNodeSetExpressions(block, (OrderedCollection *) tmp);

	SKIP_TOKEN(TOKEN_CLOSE_SQUARE_BRACKET, closeHandleScope(&scope, NULL));
	computeSourceCodeSize(sourceCode, parser);
	return closeHandleScope(&scope, block);
}


/**
 * blockArgs := blockArg "|"
 * blockArg := ":" identifier blockArg | ":" identifier
 */
static OrderedCollection *parseBlockArgs(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	Token *token = currentToken(&parser->tokenizer);
	OrderedCollection *args = newOrdColl(8);
	if (token->type == TOKEN_COLON) {
		while (token->type == TOKEN_COLON) {
			NEXT_EXPECT_TOKEN(token, TOKEN_IDENTIFIER, closeHandleScope(&scope, NULL));
			ordCollAddObject(args, (Object *) parseVariable(parser));
			token = currentToken(&parser->tokenizer);
		}

		SKIP_TOKEN(TOKEN_VERTICAL_BAR, closeHandleScope(&scope, NULL));
	}

	return closeHandleScope(&scope, args);
}


/**
 * bracketedStatmenet := "(" statement ")"
 */
static ExpressionNode *parseBracketedStatement(Parser *parser)
{
	ExpressionNode *expression;
	SKIP_TOKEN(TOKEN_OPEN_BRACKET, NULL);
	CATCH(expression = parseExpression(parser), NULL);
	SKIP_TOKEN(TOKEN_CLOSE_BRACKET, NULL);
	return expression;
}


/**
 * literal := number | symbol | string | char | array
 */
static Object *parseLiteral(Parser *parser)
{
	Token *token = currentToken(&parser->tokenizer);

	switch (token->type) {
	case TOKEN_MINUS:
		nextToken(&parser->tokenizer);
		EXPECT_TOKEN(TOKEN_DIGIT, NULL);
		return (Object *) parseNumber(parser, -1);

	case TOKEN_DIGIT:
		return (Object *) parseNumber(parser, 1);

	case TOKEN_SYMBOL_START:
		return (Object *) parseSymbol(parser);

	case TOKEN_STRING:
		return (Object *) parseString(parser);

	case TOKEN_CHAR:
		return (Object *) parseChar(parser);

	case TOKEN_ARRAY_OPEN_BRACKET:
	case TOKEN_OPEN_BRACKET:
		return (Object *) parseArray(parser);

	default:
		errorExpected(parser, TOKEN_DIGIT | TOKEN_SYMBOL_START | TOKEN_STRING | TOKEN_CHAR | TOKEN_ARRAY_OPEN_BRACKET);
		return NULL;
	}
}


/**
 * number := radius sign digits float exponent
 * radius := digits "r" | ""
 * sign := "-" | ""
 * float := "." digits | ""
 * exponent := "e" sign digits
 * digits := 0-9
 */
static LiteralNode *parseNumber(Parser *parser, int8_t sign)
{
	LiteralNode *literal = newObject(Handles.IntegerNode, 0);
	literalNodeSetSourceCode(literal, createSourceCode(parser, 1));
	CATCH(SignedValue value = parseInteger(parser), NULL);
	literalNodeSetIntValue(literal, value * sign);
	return literal;
}


static SignedValue parseInteger(Parser *parser)
{
	uint8_t base;
	Token *token = currentToken(&parser->tokenizer);
	char *ch = token->content;

	ch = strchr(ch, 'r');
	if (ch != NULL) {
		base = atoi(token->content);
		ch++;
		if (*ch == '\0') {
			errorExpected(parser, TOKEN_DIGIT);
			return 0;
		}

	} else {
		base = 10;
		ch = token->content;
	}

	nextToken(&parser->tokenizer);
	return (SignedValue) strtol(ch, NULL, base);
}


/**
 * symbol := "#" identifier | "#" keyword | "#" ...
 */
static LiteralNode *parseSymbol(Parser *parser)
{
	LiteralNode *literal = newObject(Handles.SymbolNode, 0);
	Token *token;

	NEXT_EXPECT_TOKEN(token, TOKEN_IDENTIFIER | TOKEN_KEYWORD | SPECIAL_CHARS_TOKENS | TOKEN_STRING, NULL);
	literalNodeSetSourceCode(literal, createSourceCode(parser, 1));

	if (token->type == TOKEN_KEYWORD) {
		String *symbol = newString(64);
		size_t size = 0;

		do {
			size_t contentSize = strlen(token->content);
			size += contentSize;
			if (symbol->raw->size < size) {
				symbol = (String *) copyResizedObject((Object *) symbol, size + 64);
			}
			memcpy(symbol->raw->contents + size - contentSize, token->content, contentSize);
			token = nextToken(&parser->tokenizer);
		} while (token->type == TOKEN_KEYWORD && !token->isSeparated);
		literalNodeSetValue(literal, copyResizedObject((Object *) symbol, size));
	} else {
		literalNodeSetValue(literal, (Object *) asString(token->content));
		nextToken(&parser->tokenizer);
	}
	return literal;
}


/**
 * string := "'" .* "'"
 */
static LiteralNode *parseString(Parser *parser)
{
	LiteralNode *literal = newObject(Handles.StringNode, 0);
	literalNodeSetValue(literal, (Object *) asString(currentToken(&parser->tokenizer)->content));
	literalNodeSetSourceCode(literal, createSourceCode(parser, 1));
	nextToken(&parser->tokenizer);
	return literal;
}


/**
 * char := "$" .
 */
static LiteralNode *parseChar(Parser *parser)
{
	Token *token = currentToken(&parser->tokenizer);
	LiteralNode *literal = newObject(Handles.CharacterNode, 0);
	literalNodeSetSourceCode(literal, createSourceCode(parser, 1));
	if (strcmp("$<", token->content) == 0 && peekToken(&parser->tokenizer)->type == TOKEN_DIGIT) {
		nextToken(&parser->tokenizer);
		CATCH(SignedValue value = parseInteger(parser), NULL);
		SKIP_TOKEN(TOKEN_GREATER_THAN, NULL);
		literalNodeSetCharValue(literal, (char) value);
	} else {
		literalNodeSetCharValue(literal, token->content[1]);
		nextToken(&parser->tokenizer);
	}
	return literal;
}


/**
 * array := "#(" arrayItems ")"
 * arrayItems := arrayLiteral arrayItems | arrayLiteral | "(" arrayItems ")" | ""
 */
static LiteralNode *parseArray(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	LiteralNode *arrayLiteral = newObject(Handles.ArrayNode, 0);
	SourceCode *sourceCode = createSourceCode(parser, 0);
	OrderedCollection *array = newOrdColl(16);

	literalNodeSetValue(arrayLiteral, (Object *) array);
	literalNodeSetSourceCode(arrayLiteral, sourceCode);
	nextToken(&parser->tokenizer);

	do {
		HandleScope scope2;
		openHandleScope(&scope2);
		LiteralNode *literal = parseArrayLiteral(parser);
		if (literal == NULL) {
			closeHandleScope(&scope2, NULL);
			break;
		}
		ordCollAddObject(array, (Object *) literal);
		closeHandleScope(&scope2, NULL);
	} while(1);

	SKIP_TOKEN(TOKEN_CLOSE_BRACKET, closeHandleScope(&scope, NULL));
	computeSourceCodeSize(sourceCode, parser);
	return closeHandleScope(&scope, arrayLiteral);
}


/**
 * arrayLiteral := literal | nil | true | false
 */
static LiteralNode *parseArrayLiteral(Parser *parser)
{
	Token *token = currentToken(&parser->tokenizer);
	if (token->type == TOKEN_IDENTIFIER) {
		Class *class;
		if (strcmp("nil", token->content) == 0) {
			class = Handles.NilNode;
		} else if (strcmp("true", token->content) == 0) {
			class = Handles.TrueNode;
		} else if (strcmp("false", token->content) == 0) {
			class = Handles.FalseNode;
		} else {
			return NULL;
		}
		LiteralNode *literal = (LiteralNode *) newObject(class, 0);
		literalNodeSetValue(literal, (Object *) asString(token->content));
		literalNodeSetSourceCode(literal, createSourceCode(parser, 1));
		nextToken(&parser->tokenizer);
		return literal;

	} else if (isArrayLiteral(token)) {
		return (LiteralNode *) parseLiteral(parser);

	} else {
		return NULL;
	}
}


static _Bool isArrayLiteral(Token *token)
{
	switch (token->type) {
	case TOKEN_DIGIT:
	case TOKEN_SYMBOL_START:
	case TOKEN_STRING:
	case TOKEN_CHAR:
	case TOKEN_ARRAY_OPEN_BRACKET:
	case TOKEN_OPEN_BRACKET:
		return 1;

	default:
		return 0;
	}
}


/**
 * cascadeExpression := ";" unaryExpression | ";" binaryExpression | ":" keywordExpression
 */
static MessageExpressionNode *parseCascadeExpression(Parser *parser)
{
	switch (nextToken(&parser->tokenizer)->type) {
	case TOKEN_IDENTIFIER:
		return parseUnaryExpression(parser);

	case TOKEN_SPECIAL_CHARS:
	case TOKEN_VERTICAL_BAR:
	case TOKEN_MINUS:
	case TOKEN_LESS_THAN:
	case TOKEN_GREATER_THAN:
		return parseBinaryExpression(parser);

	case TOKEN_KEYWORD:
		return parseKeywordExpression(parser);

	default:
		errorExpected(parser, TOKEN_IDENTIFIER | SPECIAL_CHARS_TOKENS | TOKEN_VERTICAL_BAR | TOKEN_KEYWORD);
		return NULL;
	}
}


/**
 * unaryExpression := identifier
 */
static MessageExpressionNode *parseUnaryExpression(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	SourceCode *source = createSourceCode(parser, 0);
	MessageExpressionNode *expression = createMessageExpressionNode(
		asString(currentToken(&parser->tokenizer)->content),
		newOrdColl(0),
		source
	);
	nextToken(&parser->tokenizer);
	computeSourceCodeSize(source, parser);
	return closeHandleScope(&scope, expression);
}


/**
 * binaryExpression := specialChars unaryObject
 */
static MessageExpressionNode *parseBinaryExpression(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	Object *tmp;
	String *selector = asString(currentToken(&parser->tokenizer)->content);
	OrderedCollection *args = newOrdColl(1);
	SourceCode *source = createSourceCode(parser, 0);

	nextToken(&parser->tokenizer);
	CATCH(tmp = parseUnaryObject(parser), closeHandleScope(&scope, NULL));
	ordCollAddObject(args, tmp);
	computeSourceCodeSize(source, parser);
	return closeHandleScope(&scope, createMessageExpressionNode(selector, args, source));
}


/**
 * keywordExpression := keywordPair keywordExpression | keywordPair
 * keywordPair := keyword binaryObject
 */
static MessageExpressionNode *parseKeywordExpression(Parser *parser)
{
	HandleScope scope;
	openHandleScope(&scope);

	Token *token;
	Object *tmp;
	SourceCode *source = createSourceCode(parser, 0);
	OrderedCollection *args = newOrdColl(16);
	String *selector = newString(64);
	size_t size = 0;

	while ((token = currentToken(&parser->tokenizer))->type == TOKEN_KEYWORD) {
		size_t contentSize = strlen(token->content);
		size += contentSize;
		if (selector->raw->size < size) {
			selector = (String *) copyResizedObject((Object *) selector, size + 64);
		}
		memcpy(selector->raw->contents + size - contentSize, token->content, contentSize);

		nextToken(&parser->tokenizer);
		CATCH(tmp = (Object *) parseBinaryObject(parser), closeHandleScope(&scope, NULL));
		ordCollAddObject(args, tmp);
	}

	selector = (String *) copyResizedObject((Object *) selector, size);
	computeSourceCodeSize(source, parser);
	return closeHandleScope(&scope, createMessageExpressionNode(selector, args, source));
}


static MessageExpressionNode *createMessageExpressionNode(String *selector, OrderedCollection *args, SourceCode *source)
{
	MessageExpressionNode *node = newObject(Handles.MessageExpressionNode, 0);
	messageExpressionNodeSetSelector(node, selector);
	messageExpressionNodeSetArgs(node, args);
	messageExpressionNodeSetSourceCode(node, source);
	return node;
}


static void computeSourceCodeSize(SourceCode *source, Parser *parser)
{
	Token *token = prevToken(&parser->tokenizer);
	sourceCodeSetSourceSize(source, token->position - sourceCodeGetPosition(source) + strlen(token->content));
}


static void errorExpected(Parser *parser, TokenType expected)
{
	parser->error.code = PARSER_ERROR_UNEXPECTED_TOKEN;
	parser->error.expected = expected;
}


SourceCode *createSourceCode(Parser *parser, _Bool computeSize)
{
	Token *token = currentToken(&parser->tokenizer);
	SourceCode *source = (SourceCode *) newObject(parser->sourceCodeClass, 0);

	sourceCodeSetSourceOrFileName(source, parser->sourceOrFileName);
	sourceCodeSetPosition(source, token->position);
	if (computeSize) {
		sourceCodeSetSourceSize(source, strlen(token->content));
	}
	sourceCodeSetLine(source, token->line);
	sourceCodeSetColumn(source, token->column);
	return source;
}


_Bool parserAtEnd(Parser *parser)
{
	return currentToken(&parser->tokenizer)->type == TOKEN_END_OF_INPUT;
}
