#include "Parser.h"
#include "Assert.h"
#include <string.h>


void assertMethod(MethodNode *method, char *selector, size_t argsSize, size_t varsSize, size_t statementsSize)
{
	assert(strcmp(selector, method->selector) == 0);
	assert(listSize(&method->body.args) == argsSize);
	assert(listSize(&method->body.tempVars) == varsSize);
	assert(listSize(&method->body.statements) == statementsSize);
}


void assertStringLiteral(LiteralNode *literal, LiteralType type, char *string)
{
	assert(literal->type == type);
	assert(strcmp(string, literal->value.stringVal) == 0);
}


void assertExpression(ExpressionNode *expression, char *selector, size_t argsSize)
{
	assert(strcmp(selector, expression->selector) == 0);
	assert(listSize(&expression->args) == argsSize);
}


void assertUnaryStatement(StatementNode *statement, char *varName, char *selector, size_t size, size_t i)
{
	assertStringLiteral(&statement->receiver, LITERAL_VAR, varName);
	assert(listSize(&statement->expressions) == size);
	assertExpression((ExpressionNode *) listAt(&statement->expressions, i), selector, 0);
}


void assertBinaryStatement(StatementNode *statement, char *varName, char *selector, char *arg, size_t size, size_t i)
{
	assertStringLiteral(&statement->receiver, LITERAL_VAR, varName);
	assert(listSize(&statement->expressions) == size);
	assertExpression((ExpressionNode *) listAt(&statement->expressions, i), selector, 1);
}


void testParsePattern(void)
{
	Parser parser;
	MethodNode method;

	initParser(&parser, "foo [ ]");
	parseMethod(&parser, &method);
	assertMethod(&method, "foo", 0, 0, 0);
	freeMethodNode(&method);
	freeParser(&parser);

	initParser(&parser, "+ a [ ]");
	parseMethod(&parser, &method);
	assertMethod(&method, "+", 1, 0, 0);
	assert(strcmp("a", *(char **) listAt(&method.body.args, 0)) == 0);
	freeMethodNode(&method);
	freeParser(&parser);

	initParser(&parser, "foo: a bar: b [ ]");
	parseMethod(&parser, &method);
	assertMethod(&method, "foo:bar:", 2, 0, 0);
	assert(strcmp("a", *(char **) listAt(&method.body.args, 0)) == 0);
	assert(strcmp("b", *(char **) listAt(&method.body.args, 1)) == 0);
	freeMethodNode(&method);
	freeParser(&parser);
}


void testParseTempVars(void)
{
	Parser parser;
	MethodNode method;

	initParser(&parser, "foo [ | a b | ]");
	parseMethod(&parser, &method);
	assertMethod(&method, "foo", 0, 2, 0);
	assert(strcmp("a", *(char **) listAt(&method.body.tempVars, 0)) == 0);
	assert(strcmp("b", *(char **) listAt(&method.body.tempVars, 1)) == 0);
	freeMethodNode(&method);
	freeParser(&parser);
}


void testParseError(void)
{
	Parser parser;
	MethodNode method;

	initParser(&parser, "1");
	parseMethod(&parser, &method);
	assert(parser.error.code == PARSER_ERROR_UNEXPECTED_TOKEN);
	assert(parser.error.tokens == (TOKEN_IDENTIFIER | TOKEN_SPECIAL_CHARS | TOKEN_KEYWORD | TOKEN_VERTICAL_BAR | TOKEN_LESS_THAN | TOKEN_GREATER_THAN | TOKEN_MINUS));
	assert(currentToken(&parser.tokenizer)->type == TOKEN_DIGIT);
	assert(strcmp("1", currentToken(&parser.tokenizer)->content) == 0);
	freeMethodNode(&method);
	freeParser(&parser);

	initParser(&parser, "foo x");
	parseMethod(&parser, &method);
	assert(parser.error.code == PARSER_ERROR_UNEXPECTED_TOKEN);
	assert(parser.error.tokens == TOKEN_OPEN_SQUARE_BRACKET);
	assert(currentToken(&parser.tokenizer)->type == TOKEN_IDENTIFIER);
	assert(strcmp("x", currentToken(&parser.tokenizer)->content) == 0);
	freeMethodNode(&method);
	freeParser(&parser);

	initParser(&parser, "+ 1");
	parseMethod(&parser, &method);
	assert(parser.error.code == PARSER_ERROR_UNEXPECTED_TOKEN);
	assert(parser.error.tokens == TOKEN_IDENTIFIER);
	assert(currentToken(&parser.tokenizer)->type == TOKEN_DIGIT);
	assert(strcmp("1", currentToken(&parser.tokenizer)->content) == 0);
	freeMethodNode(&method);
	freeParser(&parser);

	initParser(&parser, "foo: 1");
	parseMethod(&parser, &method);
	assert(parser.error.code == PARSER_ERROR_UNEXPECTED_TOKEN);
	assert(parser.error.tokens == TOKEN_IDENTIFIER);
	assert(currentToken(&parser.tokenizer)->type == TOKEN_DIGIT);
	assert(strcmp("1", currentToken(&parser.tokenizer)->content) == 0);
	freeMethodNode(&method);
	freeParser(&parser);

	initParser(&parser, "foo: a bar: 1");
	parseMethod(&parser, &method);
	assert(parser.error.code == PARSER_ERROR_UNEXPECTED_TOKEN);
	assert(parser.error.tokens == TOKEN_IDENTIFIER);
	assert(currentToken(&parser.tokenizer)->type == TOKEN_DIGIT);
	assert(strcmp("1", currentToken(&parser.tokenizer)->content) == 0);
	freeMethodNode(&method);
	freeParser(&parser);
}


void testParseUnaryStatement(void)
{
	Parser parser;
	MethodNode method;
	StatementNode *statement;

	initParser(&parser, "foo [ bar ]");
	parseMethod(&parser, &method);
	assertMethod(&method, "foo", 0, 0, 1);

	statement = (StatementNode *) listAt(&method.body.statements, 0);
	assertStringLiteral(&statement->receiver, LITERAL_VAR, "bar");
	assert(listSize(&statement->expressions) == 0);
	freeMethodNode(&method);
	freeParser(&parser);


	initParser(&parser, "foo [ bar msg ]");
	parseMethod(&parser, &method);
	assertMethod(&method, "foo", 0, 0, 1);
	assertUnaryStatement((StatementNode *) listAt(&method.body.statements, 0), "bar", "msg", 1, 0);
	freeMethodNode(&method);
	freeParser(&parser);


	initParser(&parser, "foo [ bar msg msg2 ]");
	parseMethod(&parser, &method);
	assertMethod(&method, "foo", 0, 0, 1);

	statement = (StatementNode *) listAt(&method.body.statements, 0);
	assert(statement->receiver.type == LITERAL_STATEMENT);
	assertUnaryStatement(statement->receiver.value.statement, "bar", "msg", 1, 0);
	assert(listSize(&statement->expressions) == 1);
	assertExpression((ExpressionNode *) listAt(&statement->expressions, 0), "msg2", 0);
	freeMethodNode(&method);
	freeParser(&parser);
}


void testParseBinaryStatement(void)
{
	Parser parser;
	MethodNode method;
	StatementNode *statement;

	initParser(&parser, "foo [ a + b ]");
	parseMethod(&parser, &method);
	assertMethod(&method, "foo", 0, 0, 1);
	assertBinaryStatement((StatementNode *) listAt(&method.body.statements, 0), "a", "+", "b", 1, 0);
	freeMethodNode(&method);
	freeParser(&parser);

	initParser(&parser, "foo [ a + b + c]");
	parseMethod(&parser, &method);
	assertMethod(&method, "foo", 0, 0, 1);

	statement = (StatementNode *) listAt(&method.body.statements, 0);
	assert(statement->receiver.type == LITERAL_STATEMENT);
	assertBinaryStatement(statement->receiver.value.statement, "a", "+", "b", 1, 0);
	assert(listSize(&statement->expressions) == 1);
	assertExpression((ExpressionNode *) listAt(&statement->expressions, 0), "+", 1);
	freeMethodNode(&method);
	freeParser(&parser);
}


void testParseCascadeStatement(void)
{
	Parser parser;
	MethodNode method;
	StatementNode *statement;

	initParser(&parser, "foo [ bar msg1; msg2 ]");
	parseMethod(&parser, &method);
	assertMethod(&method, "foo", 0, 0, 1);
	statement = (StatementNode *) listAt(&method.body.statements, 0);
	assertUnaryStatement(statement, "bar", "msg1", 2, 0);
	assertUnaryStatement(statement, "bar", "msg2", 2, 1);
	freeMethodNode(&method);
	freeParser(&parser);
}


void testParseClass(void)
{
	Parser parser;
	ClassNode class;

	initParser(&parser, "Foo subclass: Bar [ <foo: #bar> | a b |");
	parseClass(&parser, &class);
	freeClassNode(&class);
	freeParser(&parser);
}


int main(void)
{
	testParsePattern();
	testParseTempVars();
	testParseError();
	testParseUnaryStatement();
	testParseBinaryStatement();
	testParseCascadeStatement();
	testParseClass();
}
