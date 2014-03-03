#ifndef AST_H
#define AST_H

#include "Object.h"
#include "Thread.h"
#include "Heap.h"
#include "String.h"
#include "Collection.h"
#include "Dictionary.h"
#include "Handle.h"
#include "Smalltalk.h"

typedef struct
{
	OBJECT_HEADER;
	Value name;
	Value superName;
	Value pragmas;
	Value vars;
	Value methods;
	Value sourceCode;
} RawClassNode;
OBJECT_HANDLE(ClassNode);

typedef struct
{
	OBJECT_HEADER;
	Value className;
	Value selector;
	Value pragmas;
	Value body;
	Value sourceCode;
} RawMethodNode;
OBJECT_HANDLE(MethodNode);

typedef struct
{
	OBJECT_HEADER;
	Value args;
	Value tempVars;
	Value expressions;
	Value scope;
	Value sourceCode;
} RawBlockNode;
OBJECT_HANDLE(BlockNode);

typedef struct
{
	OBJECT_HEADER;
	Value returns;
	Value assigments;
	Value receiver;
	Value messageExpressions;
	Value sourceCode;
} RawExpressionNode;
OBJECT_HANDLE(ExpressionNode);

typedef struct
{
	OBJECT_HEADER;
	Value selector;
	Value args;
	Value sourceCode;
} RawMessageExpressionNode;
OBJECT_HANDLE(MessageExpressionNode);

typedef struct
{
	OBJECT_HEADER;
	Value value;
	Value sourceCode;
} RawLiteralNode;
OBJECT_HANDLE(LiteralNode);

typedef struct {
	OBJECT_HEADER;
	Value sourceOrFileName;
	Value position;
	Value sourceSize;
	Value line;
	Value column;
} RawSourceCode;
OBJECT_HANDLE(SourceCode);

union BlockScope;

static void classNodeSetName(ClassNode *class, LiteralNode *node);
static LiteralNode *classNodeGetName(ClassNode *class);
static void classNodeSetSuperName(ClassNode *class, LiteralNode *superName);
static LiteralNode *classNodeGetSuperName(ClassNode *class);
static void classNodeSetPragmas(ClassNode *class, OrderedCollection *pragmas);
static OrderedCollection *classNodeGetPragmas(ClassNode *class);
static void classNodeSetVars(ClassNode *class, OrderedCollection *vars);
static OrderedCollection *classNodeGetVars(ClassNode *class);
static void classNodeSetMethods(ClassNode *class, OrderedCollection *methods);
static OrderedCollection *classNodeGetMethods(ClassNode *class);
static void classNodeSetSourceCode(ClassNode *class, SourceCode *sourceCode);
static SourceCode *classNodeGetSourceCode(ClassNode *class);

static void methodNodeSetClassName(MethodNode *method, String *className);
static String *methodNodeGetClassName(MethodNode *method);
static void methodNodeSetSelector(MethodNode *method, String *selector);
static String *methodNodeGetSelector(MethodNode *method);
static void methodNodeSetPragmas(MethodNode *method, OrderedCollection *pragmas);
static OrderedCollection *methodNodeGetPragmas(MethodNode *method);
static void methodNodeSetBody(MethodNode *method, BlockNode *body);
static BlockNode *methodNodeGetBody(MethodNode *method);
static void methodNodeSetSourceCode(MethodNode *method, SourceCode *sourceCode);
static SourceCode *methodNodeGetSourceCode(MethodNode *method);

static void blockNodeSetArgs(BlockNode *block, OrderedCollection *args);
static OrderedCollection *blockNodeGetArgs(BlockNode *block);
static void blockNodeSetTempVars(BlockNode *block, OrderedCollection *vars);
static OrderedCollection *blockNodeGetTempVars(BlockNode *block);
static void blockNodeSetExpressions(BlockNode *block, OrderedCollection *expressions);
static OrderedCollection *blockNodeGetExpressions(BlockNode *block);
static void blockNodeSetScope(BlockNode *block, union BlockScope *scope);
static union BlockScope *blockNodeGetScope(BlockNode *block);
static void blockNodeSetSourceCode(BlockNode *block, SourceCode *sourceCode);
static SourceCode *blockNodeGetSourceCode(BlockNode *block);

static void expressionNodeEnableReturn(ExpressionNode *expression);
static void expressionNodeDisableReturn(ExpressionNode *expression);
static _Bool expressionNodeReturns(ExpressionNode *expression);
static void expressionNodeSetAssigments(ExpressionNode *expression, OrderedCollection *assigments);
static OrderedCollection *expressionNodeGetAssigments(ExpressionNode *expression);
static void expressionNodeSetReceiver(ExpressionNode *expression, LiteralNode *receiver);
static LiteralNode *expressionNodeGetReceiver(ExpressionNode *expression);
static void expressionNodeSetMessageExpressions(ExpressionNode *expression, OrderedCollection *messageExpressions);
static OrderedCollection *expressionNodeGetMessageExpressions(ExpressionNode *expression);
static void expressionNodeSetSourceCode(ExpressionNode *expression, SourceCode *sourceCode);
static SourceCode *expressionNodeGetSourceCode(ExpressionNode *expression);

static void messageExpressionNodeSetSelector(MessageExpressionNode *messageExpression, String *selector);
static String *messageExpressionNodeGetSelector(MessageExpressionNode *messageExpression);
static void messageExpressionNodeSetArgs(MessageExpressionNode *messageExpression, OrderedCollection *args);
static OrderedCollection *messageExpressionNodeGetArgs(MessageExpressionNode *messageExpression);
static void messageExpressionNodeSetSourceCode(MessageExpressionNode *messageExpression, SourceCode *sourceCode);
static SourceCode *messageExpressionNodeGetSourceCode(MessageExpressionNode *messageExpression);

static void literalNodeSetValue(LiteralNode *literal, Object *value);
static void literalNodeSetIntValue(LiteralNode *literal, intptr_t value);
static void literalNodeSetCharValue(LiteralNode *literal, char value);
static Value literalNodeGetValue(LiteralNode *literal);
static intptr_t literalNodeGetIntValue(LiteralNode *literal);
static String *literalNodeGetStringValue(LiteralNode *literal);
static OrderedCollection *literalNodeGetOrdCollValue(LiteralNode *literal);
static void literalNodeSetSourceCode(LiteralNode *literal, SourceCode *sourceCode);
static SourceCode *literalNodeGetSourceCode(LiteralNode *literal);

static void sourceCodeSetSourceOrFileName(SourceCode *sourceCode, String *sourceOrFileName);
static String *sourceCodeGetSourceOrFileName(SourceCode *sourceCode);
static void sourceCodeSetPosition(SourceCode *sourceCode, uintptr_t position);
static uintptr_t sourceCodeGetPosition(SourceCode *sourceCode);
static void sourceCodeSetSourceSize(SourceCode *sourceCode, uintptr_t sourceSize);
static uintptr_t sourceCodeGetSourceSize(SourceCode *sourceCode);
static void sourceCodeSetLine(SourceCode *sourceCode, uintptr_t line);
static uintptr_t sourceCodeGetLine(SourceCode *sourceCode);
static void sourceCodeSetColumn(SourceCode *sourceCode, uintptr_t column);
static uintptr_t sourceCodeGetColumn(SourceCode *sourceCode);


static void classNodeSetName(ClassNode *class, LiteralNode *name)
{
	objectStorePtr((Object *) class,  &class->raw->name, (Object *) name);
}


static LiteralNode *classNodeGetName(ClassNode *class)
{
	return (LiteralNode *) scopeHandle(asObject(class->raw->name));
}


static void classNodeSetSuperName(ClassNode *class, LiteralNode *superName)
{
	objectStorePtr((Object *) class,  &class->raw->superName, (Object *) superName);
}


static LiteralNode *classNodeGetSuperName(ClassNode *class)
{
	return (LiteralNode *) scopeHandle(asObject(class->raw->superName));
}


static void classNodeSetPragmas(ClassNode *class, OrderedCollection *pragmas)
{
	objectStorePtr((Object *) class,  &class->raw->pragmas, (Object *) pragmas);
}


static OrderedCollection *classNodeGetPragmas(ClassNode *class)
{
	return (OrderedCollection *) scopeHandle(asObject(class->raw->pragmas));
}


static void classNodeSetVars(ClassNode *class, OrderedCollection *vars)
{
	objectStorePtr((Object *) class,  &class->raw->vars, (Object *) vars);
}


static OrderedCollection *classNodeGetVars(ClassNode *class)
{
	return (OrderedCollection *) scopeHandle(asObject(class->raw->vars));
}


static void classNodeSetMethods(ClassNode *class, OrderedCollection *methods)
{
	objectStorePtr((Object *) class,  &class->raw->methods, (Object *) methods);
}


static OrderedCollection *classNodeGetMethods(ClassNode *class)
{
	return (OrderedCollection *) scopeHandle(asObject(class->raw->methods));
}


static void classNodeSetSourceCode(ClassNode *class, SourceCode *sourceCode)
{
	objectStorePtr((Object *) class,  &class->raw->sourceCode, (Object *) sourceCode);
}


static SourceCode *classNodeGetSourceCode(ClassNode *class)
{
	return (SourceCode *) scopeHandle(asObject(class->raw->sourceCode));
}


static void methodNodeSetClassName(MethodNode *method, String *className)
{
	objectStorePtr((Object *) method,  &method->raw->className, (Object *) className);
}


static String *methodNodeGetClassName(MethodNode *method)
{
	return (String *) scopeHandle(asObject(method->raw->className));
}


static void methodNodeSetSelector(MethodNode *method, String *selector)
{
	objectStorePtr((Object *) method,  &method->raw->selector, (Object *) selector);
}


static String *methodNodeGetSelector(MethodNode *method)
{
	return (String *) scopeHandle(asObject(method->raw->selector));
}


static void methodNodeSetPragmas(MethodNode *method, OrderedCollection *pragmas)
{
	objectStorePtr((Object *) method,  &method->raw->pragmas, (Object *) pragmas);
}


static OrderedCollection *methodNodeGetPragmas(MethodNode *method)
{
	return (OrderedCollection *) scopeHandle(asObject(method->raw->pragmas));
}


static void methodNodeSetBody(MethodNode *method, BlockNode *body)
{
	objectStorePtr((Object *) method,  &method->raw->body, (Object *) body);
}


static BlockNode *methodNodeGetBody(MethodNode *method)
{
	return (BlockNode *) scopeHandle(asObject(method->raw->body));
}


static void methodNodeSetSourceCode(MethodNode *method, SourceCode *sourceCode)
{
	objectStorePtr((Object *) method,  &method->raw->sourceCode, (Object *) sourceCode);
}


static SourceCode *methodNodeGetSourceCode(MethodNode *method)
{
	return (SourceCode *) scopeHandle(asObject(method->raw->sourceCode));
}


static void blockNodeSetArgs(BlockNode *block, OrderedCollection *args)
{
	objectStorePtr((Object *) block,  &block->raw->args, (Object *) args);
}


static OrderedCollection *blockNodeGetArgs(BlockNode *block)
{
	return (OrderedCollection *) scopeHandle(asObject(block->raw->args));
}


static void blockNodeSetTempVars(BlockNode *block, OrderedCollection *vars)
{
	objectStorePtr((Object *) block,  &block->raw->tempVars, (Object *) vars);
}


static OrderedCollection *blockNodeGetTempVars(BlockNode *block)
{
	return (OrderedCollection *) scopeHandle(asObject(block->raw->tempVars));
}


static void blockNodeSetExpressions(BlockNode *block, OrderedCollection *expressions)
{
	objectStorePtr((Object *) block,  &block->raw->expressions, (Object *) expressions);
}


static OrderedCollection *blockNodeGetExpressions(BlockNode *block)
{
	return (OrderedCollection *) scopeHandle(asObject(block->raw->expressions));
}


static void blockNodeSetScope(BlockNode *block, union BlockScope *scope)
{
	objectStorePtr((Object *) block,  &block->raw->scope, (Object *) scope);
}


static union BlockScope *blockNodeGetScope(BlockNode *block)
{
	return scopeHandle(asObject(block->raw->scope));
}


static void blockNodeSetSourceCode(BlockNode *block, SourceCode *sourceCode)
{
	objectStorePtr((Object *) block,  &block->raw->sourceCode, (Object *) sourceCode);
}


static SourceCode *blockNodeGetSourceCode(BlockNode *block)
{
	return (SourceCode *) scopeHandle(asObject(block->raw->sourceCode));
}


static void expressionNodeEnableReturn(ExpressionNode *expression)
{
	objectStorePtr((Object *) expression,  &expression->raw->returns, (Object *) Handles.true);
}


static void expressionNodeDisableReturn(ExpressionNode *expression)
{
	objectStorePtr((Object *) expression,  &expression->raw->returns, (Object *) Handles.false);
}


static _Bool expressionNodeReturns(ExpressionNode *expression)
{
	return isTaggedTrue(expression->raw->returns);
}


static void expressionNodeSetAssigments(ExpressionNode *expression, OrderedCollection *assigments)
{
	objectStorePtr((Object *) expression,  &expression->raw->assigments, (Object *) assigments);
}


static OrderedCollection *expressionNodeGetAssigments(ExpressionNode *expression)
{
	return (OrderedCollection *) scopeHandle(asObject(expression->raw->assigments));
}


static void expressionNodeSetReceiver(ExpressionNode *expression, LiteralNode *receiver)
{
	objectStorePtr((Object *) expression,  &expression->raw->receiver, (Object *) receiver);
}


static LiteralNode *expressionNodeGetReceiver(ExpressionNode *expression)
{
	return (LiteralNode *) scopeHandle(asObject(expression->raw->receiver));
}


static void expressionNodeSetMessageExpressions(ExpressionNode *expression, OrderedCollection *messageExpressions)
{
	objectStorePtr((Object *) expression,  &expression->raw->messageExpressions, (Object *) messageExpressions);
}


static OrderedCollection *expressionNodeGetMessageExpressions(ExpressionNode *expression)
{
	return (OrderedCollection *) scopeHandle(asObject(expression->raw->messageExpressions));
}


static void expressionNodeSetSourceCode(ExpressionNode *expression, SourceCode *sourceCode)
{
	objectStorePtr((Object *) expression,  &expression->raw->sourceCode, (Object *) sourceCode);
}


static SourceCode *expressionNodeGetSourceCode(ExpressionNode *expression)
{
	return (SourceCode *) scopeHandle(asObject(expression->raw->sourceCode));
}


static void messageExpressionNodeSetSelector(MessageExpressionNode *messageExpression, String *selector)
{
	objectStorePtr((Object *) messageExpression,  &messageExpression->raw->selector, (Object *) selector);
}


static String *messageExpressionNodeGetSelector(MessageExpressionNode *messageExpression)
{
	return (String *) scopeHandle(asObject(messageExpression->raw->selector));
}


static void messageExpressionNodeSetArgs(MessageExpressionNode *messageExpression, OrderedCollection *args)
{
	objectStorePtr((Object *) messageExpression,  &messageExpression->raw->args, (Object *) args);
}


static OrderedCollection *messageExpressionNodeGetArgs(MessageExpressionNode *messageExpression)
{
	return (OrderedCollection *) scopeHandle(asObject(messageExpression->raw->args));
}


static void messageExpressionNodeSetSourceCode(MessageExpressionNode *messageExpression, SourceCode *sourceCode)
{
	objectStorePtr((Object *) messageExpression,  &messageExpression->raw->sourceCode, (Object *) sourceCode);
}


static SourceCode *messageExpressionNodeGetSourceCode(MessageExpressionNode *messageExpression)
{
	return (SourceCode *) scopeHandle(asObject(messageExpression->raw->sourceCode));
}


static void literalNodeSetValue(LiteralNode *literal, Object *value)
{
	objectStorePtr((Object *) literal,  &literal->raw->value, (Object *) value);
}


static void literalNodeSetIntValue(LiteralNode *literal, intptr_t value)
{
	literal->raw->value = tagInt(value);
}


static void literalNodeSetCharValue(LiteralNode *literal, char value)
{
	literal->raw->value = tagChar(value);
}


static Value literalNodeGetValue(LiteralNode *literal)
{
	return literal->raw->value;
}


static intptr_t literalNodeGetIntValue(LiteralNode *literal)
{
	return asCInt(literalNodeGetValue(literal));
}


static String *literalNodeGetStringValue(LiteralNode *literal)
{
	return (String *) scopeHandle(asObject(literalNodeGetValue(literal)));
}


static OrderedCollection *literalNodeGetOrdCollValue(LiteralNode *literal)
{
	return scopeHandle(asObject(literalNodeGetValue(literal)));
}


static void literalNodeSetSourceCode(LiteralNode *literal, SourceCode *sourceCode)
{
	objectStorePtr((Object *) literal,  &literal->raw->sourceCode, (Object *) sourceCode);
}


static SourceCode *literalNodeGetSourceCode(LiteralNode *literal)
{
	return (SourceCode *) scopeHandle(asObject(literal->raw->sourceCode));
}


static void sourceCodeSetSourceOrFileName(SourceCode *sourceCode, String *sourceOrFileName)
{
	objectStorePtr((Object *) sourceCode,  &sourceCode->raw->sourceOrFileName, (Object *) sourceOrFileName);
}


static String *sourceCodeGetSourceOrFileName(SourceCode *sourceCode)
{
	return (String *) scopeHandle(asObject(sourceCode->raw->sourceOrFileName));
}


static void sourceCodeSetPosition(SourceCode *sourceCode, uintptr_t position)
{
	sourceCode->raw->position = tagInt(position);
}


static uintptr_t sourceCodeGetPosition(SourceCode *sourceCode)
{
	return asCInt(sourceCode->raw->position);
}


static void sourceCodeSetSourceSize(SourceCode *sourceCode, uintptr_t sourceSize)
{
	sourceCode->raw->sourceSize = tagInt(sourceSize);
}


static uintptr_t sourceCodeGetSourceSize(SourceCode *sourceCode)
{
	return asCInt(sourceCode->raw->sourceSize);
}


static void sourceCodeSetLine(SourceCode *sourceCode, uintptr_t line)
{
	sourceCode->raw->line = tagInt(line);
}


static uintptr_t sourceCodeGetLine(SourceCode *sourceCode)
{
	return asCInt(sourceCode->raw->line);
}


static void sourceCodeSetColumn(SourceCode *sourceCode, uintptr_t column)
{
	sourceCode->raw->column = tagInt(column);
}


static uintptr_t sourceCodeGetColumn(SourceCode *sourceCode)
{
	return asCInt(sourceCode->raw->column);
}


#endif
