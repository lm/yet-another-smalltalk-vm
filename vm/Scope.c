#include "Scope.h"
#include "Variable.h"
#include "Dictionary.h"
#include "Compiler.h"
#include "Smalltalk.h"
#include "Heap.h"
#include "Handle.h"
#include "Iterator.h"
#include "Class.h"
#include "Bytecodes.h"
#include "Assert.h"
#include <string.h>

#define RETURN_IF_ERROR() \
	if (blockScopeHasError(blockScope)) { \
		closeHandleScope(&scope, NULL); \
		return; \
	}

static void analyzeInstanceVars(BlockScope *blockScope, Array *instVars);
static void analyzeBlock(BlockScope *blockScope, BlockNode *node);
static void analyzeDefinitions(BlockScope *blockScope, BlockNode *node);
static _Bool isDuplicateVariable(Dictionary *vars, String *name);
static void analyzeExpression(BlockScope *blockScope, ExpressionNode *node);
static void analyzeAssigments(BlockScope *blockScope, ExpressionNode *node);
static void analyzeAssigment(BlockScope *blockScope, LiteralNode *literal);
static CompileError *createReadonlyVariableError(LiteralNode *node);
static void analyzeMessageExpression(BlockScope *blockScope, MessageExpressionNode *node);
static void analyzeLiteral(BlockScope *blockScope, Object *literal);
static void analyzeVar(BlockScope *blockScope, LiteralNode *name);
static _Bool analyzeContextVar(BlockScope *blockScope, String *name);
static void setupBlockMetasAsContexts(BlockScope *blockScope, BlockScope *upTo);
static _Bool analyzeGlobalVar(BlockScope *blockScope, String *name);
static _Bool analyzeDictionaryVar(BlockScope *blockScope, Dictionary *dict, String *name);
static BlockScope *createBlockScope(BlockScope *parent);


BlockScope *analyzeMethod(MethodNode *node, Class *class)
{
	HandleScope scope;
	openHandleScope(&scope);

	BlockScope *blockScope = createBlockScope((BlockScope *) Handles.nil);
	Array *instVars = classGetInstanceVariables(class);

	if (class->raw->class == Handles.MetaClass->raw) {
		blockScopeSetOwnerClass(blockScope, metaClassGetInstanceClass((MetaClass *) class));
	} else {
		blockScopeSetOwnerClass(blockScope, class);
	}

	blockScopeSetLiterals(blockScope, newOrdColl(64));

	if (!isNil(instVars)) {
		analyzeInstanceVars(blockScope, instVars);
	}
	analyzeBlock(blockScope, methodNodeGetBody(node));
	return closeHandleScope(&scope, blockScope);
}


static void analyzeInstanceVars(BlockScope *blockScope, Array *instVars)
{
	HandleScope scope;
	openHandleScope(&scope);

	Dictionary *vars = blockScopeGetVars(blockScope);
	Iterator iterator;

	initArrayIterator(&iterator, instVars, 0, 0);
	while (iteratorHasNext(&iterator)) {
		Value var = defineVariable(OPERAND_INST_VAR, iteratorIndex(&iterator), 0);
		stringDictAtPut(vars, (String *) iteratorNextObject(&iterator), var);
	}

	closeHandleScope(&scope, NULL);
}


static void analyzeBlock(BlockScope *blockScope, BlockNode *node)
{
	blockNodeSetScope(node, blockScope);
	analyzeDefinitions(blockScope, node);
	if (blockScopeHasError(blockScope)) {
		return;
	}

	Iterator iterator;
	initOrdCollIterator(&iterator, blockNodeGetExpressions(node), 0, 0);

	while (iteratorHasNext(&iterator)) {
		HandleScope scope;
		openHandleScope(&scope);
		analyzeExpression(blockScope, (ExpressionNode *) iteratorNextObject(&iterator));
		RETURN_IF_ERROR();
		closeHandleScope(&scope, NULL);
	}
}


static void analyzeDefinitions(BlockScope *blockScope, BlockNode *node)
{
	HandleScope scope;
	openHandleScope(&scope);

	Dictionary *vars = blockScopeGetVars(blockScope);
	Iterator iterator;
	uint8_t index = 0;

	stringDictAtPut(vars, asString("thisContext"), defineVariable(OPERAND_THIS_CONTEXT, index++, 0));
	stringDictAtPut(vars, asString("super"), defineVariable(OPERAND_SUPER, index, 0));
	stringDictAtPut(vars, asString("self"), defineVariable(OPERAND_ARG_VAR, index++, 0));

	initOrdCollIterator(&iterator, blockNodeGetArgs(node), 0, 0);
	while (iteratorHasNext(&iterator)) {
		LiteralNode *arg = (LiteralNode *) iteratorNextObject(&iterator);
		String *name = literalNodeGetStringValue(arg);
		if (isDuplicateVariable(vars, name)) {
			blockScopeSetError(blockScope, createRedefinitionError(arg));
		}
		stringDictAtPut(vars, name, defineVariable(OPERAND_ARG_VAR, index++, 0));
	}

	blockScope->raw->header.argsSize = index - 2; // -2 for thisContext and self

	initOrdCollIterator(&iterator, blockNodeGetTempVars(node), 0, 0);
	while (iteratorHasNext(&iterator)) {
		LiteralNode *arg = (LiteralNode *) iteratorNextObject(&iterator);
		String *name = literalNodeGetStringValue(arg);
		if (isDuplicateVariable(vars, name)) {
			blockScopeSetError(blockScope, createRedefinitionError(arg));
		}
		stringDictAtPut(vars, name, defineVariable(OPERAND_TEMP_VAR, index++, 0));
	}

	closeHandleScope(&scope, NULL);
}


static _Bool isDuplicateVariable(Dictionary *vars, String *name)
{
	Value var = stringDictAt(vars, name);
	if (isTaggedNil(var)) {
		return 0;
	}
	switch (getVarType(var)) {
	case OPERAND_THIS_CONTEXT:
	case OPERAND_TEMP_VAR:
	case OPERAND_ARG_VAR:
	case OPERAND_SUPER:
	case OPERAND_CONTEXT_VAR:
		return 1;
	default:
		return 0;
	}
}


static void analyzeExpression(BlockScope *blockScope, ExpressionNode *node)
{
	analyzeAssigments(blockScope, node);
	analyzeLiteral(blockScope, (Object *) expressionNodeGetReceiver(node));
	if (blockScopeHasError(blockScope)) {
		return;
	}

	Iterator iterator;
	initOrdCollIterator(&iterator, expressionNodeGetMessageExpressions(node), 0, 0);
	while (iteratorHasNext(&iterator)) {
		HandleScope scope;
		openHandleScope(&scope);
		analyzeMessageExpression(blockScope, (MessageExpressionNode *) iteratorNextObject(&iterator));
		RETURN_IF_ERROR();
		closeHandleScope(&scope, NULL);
	}
}


static void analyzeAssigments(BlockScope *blockScope, ExpressionNode *node)
{
	HandleScope scope;
	openHandleScope(&scope);

	Iterator iterator;
	initOrdCollIterator(&iterator, expressionNodeGetAssigments(node), 0, 0);
	while (iteratorHasNext(&iterator)) {
		analyzeAssigment(blockScope, (LiteralNode *) iteratorNextObject(&iterator));
		RETURN_IF_ERROR();
	}

	closeHandleScope(&scope, NULL);
}


static void analyzeAssigment(BlockScope *blockScope, LiteralNode *literal)
{
	Value var;

	if (literal->raw->class != Handles.VariableNode->raw) {
		blockScopeSetError(blockScope, createReadonlyVariableError(literal));
	} else {
		analyzeVar(blockScope, literal);
		var = stringDictAt(blockScopeGetVars(blockScope), literalNodeGetStringValue(literal));
		if (!isTaggedNil(var) && (getVarType(var) == OPERAND_ARG_VAR || getVarType(var) == OPERAND_SUPER || hasVarCtxCopy(var))) {
			blockScopeSetError(blockScope, createReadonlyVariableError(literal));
		}
	}
}


static CompileError *createReadonlyVariableError(LiteralNode *node)
{
	CompileError *error = (CompileError *) newObject(Handles.ReadonlyVariableError, 0);
	objectStorePtr((Object *) error,  &error->raw->variable, (Object *) node);
	return error;
}


static void analyzeMessageExpression(BlockScope *blockScope, MessageExpressionNode *node)
{
	HandleScope scope;
	openHandleScope(&scope);

	Iterator iterator;
	initOrdCollIterator(&iterator, messageExpressionNodeGetArgs(node), 0, 0);
	while (iteratorHasNext(&iterator)) {
		analyzeLiteral(blockScope, iteratorNextObject(&iterator));
		RETURN_IF_ERROR();
	}

	closeHandleScope(&scope, NULL);
}


static void analyzeLiteral(BlockScope *blockScope, Object *literal)
{
	if (literal->raw->class == Handles.VariableNode->raw) {
		analyzeVar(blockScope, (LiteralNode *) literal);

	} else if (literal->raw->class == Handles.ExpressionNode->raw) {
		analyzeExpression(blockScope, (ExpressionNode *) literal);

	} else if (literal->raw->class == Handles.BlockNode->raw) {
		BlockScope *blockMeta = createBlockScope(blockScope);
		blockScopeSetOwnerClass(blockMeta, blockScopeGetOwnerClass(blockScope));
		blockScopeSetLiterals(blockMeta, blockScopeGetLiterals(blockScope));
		analyzeBlock(blockMeta, (BlockNode *) literal);
		blockScopeSetError(blockScope, blockScopeGetError(blockMeta));
	}
}


static void analyzeVar(BlockScope *blockScope, LiteralNode *literal)
{
	String *name = literalNodeGetStringValue(literal);
	Value var = stringDictAt(blockScopeGetVars(blockScope), name);

	if (!isTaggedNil(var)) {
		if (getVarIndex(var) == CONTEXT_INDEX) {
			blockScope->raw->header.hasContext = 1;
		}
		return;
	}
	if (analyzeContextVar(blockScope, name)) {
		return;
	}

	Class *class = blockScopeGetOwnerClass(blockScope);
	do {
		Dictionary *classVars = classGetClassVariables(class);
		if (!isNil((Object *) classVars) && analyzeDictionaryVar(blockScope, classVars, name)) {
			return;
		}
		class = classGetSuperClass(class);
	} while (!isNil((Object *) class));

	if (analyzeGlobalVar(blockScope, name)) {
		return;
	}

	blockScopeSetError(blockScope, createUndefinedVariableError(literal));
}


static _Bool analyzeContextVar(BlockScope *blockScope, String *name)
{
	uint8_t level = 1;
	BlockScope *ctx = blockScope;
	Value var;

	while (!isNil(ctx = blockScopeGetParent(ctx))) {
		Dictionary *vars = blockScopeGetVars(ctx);
		var = stringDictAt(vars, name);
		if (!isTaggedNil(var)) {
			if (getVarType(var) == OPERAND_ARG_VAR) {
				if (!hasVarCtxCopy(var)) {
					setVarCtxCopy(&var, ctx->raw->header.contextSize++);
					stringDictAtPut(vars, name, var);
				}
				setVarType(&var, OPERAND_CONTEXT_VAR);
				setVarIndex(&var, getVarCtxCopy(var));
			} else if (getVarType(var) == OPERAND_TEMP_VAR) {
				setVarType(&var, OPERAND_CONTEXT_VAR);
				setVarIndex(&var, ctx->raw->header.contextSize++);
				stringDictAtPut(vars, name, var);
			}
			setVarLevel(&var, level + getVarLevel(var));
			stringDictAtPut(blockScopeGetVars(blockScope), name, var);
			setupBlockMetasAsContexts(blockScope, ctx);
			return 1;
		}
		level++;
	}

	return 0;
}


static void setupBlockMetasAsContexts(BlockScope *blockScope, BlockScope *upTo)
{
	do {
		blockScope->raw->header.hasContext = 1;
	} while((blockScope = blockScopeGetParent(blockScope))->raw != upTo->raw);
	blockScope->raw->header.hasContext = 1;
}


static _Bool analyzeGlobalVar(BlockScope *blockScope, String *name)
{
	Value var;

	if (analyzeDictionaryVar(blockScope, Handles.Smalltalk, name)) {
		return 1;
	} else if (name->raw->contents[0] >= 'A' && name->raw->contents[0] <= 'Z') {
		OrderedCollection *literals = blockScopeGetLiterals(blockScope);
		Association *assoc = symbolDictAtPutObject(Handles.Smalltalk, asSymbol(name), Handles.nil);
		var = defineVariable(OPERAND_ASSOC, ordCollAddObjectIfNotExists(literals, (Object *) assoc), 0);
		stringDictAtPut(blockScopeGetVars(blockScope), name, var);
		return 1;
	}
	return 0;
}


static _Bool analyzeDictionaryVar(BlockScope *blockScope, Dictionary *dict, String *name)
{
	Association *assoc = symbolDictAssocAt(dict, asSymbol(name));
	Value var;

	if (isNil(assoc)) {
		return 0;
	} else {
		OrderedCollection *literals = blockScopeGetLiterals(blockScope);
		var = defineVariable(OPERAND_ASSOC, ordCollAddObjectIfNotExists(literals, (Object *) assoc), 0);
		stringDictAtPut(blockScopeGetVars(blockScope), name, var);
		return 1;
	}
}


static BlockScope *createBlockScope(BlockScope *parent)
{
	BlockScope *blockScope = (BlockScope *) newObject(Handles.BlockScope, 0);
	memset(&blockScope->raw->header, 0, sizeof(blockScope->raw->header));
	blockScopeSetParent(blockScope, parent);
	blockScopeSetVars(blockScope, newDictionary(32));
	return blockScope;
}
