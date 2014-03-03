#include "Entry.h"
#include "Lookup.h"
#include "Handle.h"
#include "Class.h"
#include "Lookup.h"
#include "StubCode.h"
#include "Compiler.h"
#include "Iterator.h"
#include "Heap.h"
#include "Smalltalk.h"
#include "Thread.h"
#include <string.h>
#include <errno.h>

static void initArgs(Value *rawArgs, EntryArgs *args);
static void patchMethodNode(MethodNode *method);
static Value evalBlockNode(BlockNode *block);


Value invokeMethod(CompiledMethod *method, EntryArgs *args)
{
	HandleScope scope;
	openHandleScope(&scope);

	Class *class = scopeHandle(args->values[0].isHandle ? args->values[0].handle->raw->class : getClassOf(args->values[0].value));
	NativeCodeEntry entry = (NativeCodeEntry) getStubNativeCode(&SmalltalkEntry)->insts;

	Value rawArgs[args->size];
	initArgs(rawArgs, args);
	initThreadContext(&CurrentThread);
	Value result = entry(method->raw, getNativeCode(class, method)->insts, rawArgs, &CurrentThread);

	closeHandleScope(&scope, NULL);
	return result;
}


Value invokeInititalize(Object *object)
{
	Dictionary *methods = scopeHandle(asObject(object->raw->class->methodDictionary));
	Object *init = symbolDictObjectAt(methods, Handles.initializeSymbol);
	if (!isNil(init)) {
		EntryArgs args = { .size = 0 };
		entryArgsAddObject(&args, object);
		return invokeMethod((CompiledMethod *) init, &args);
	}
	return getTaggedPtr(object);
}


Value sendMessage(String *selector, EntryArgs *args)
{
	NativeCodeEntry entry = (NativeCodeEntry) getStubNativeCode(&SmalltalkEntry)->insts;
	RawClass *class = args->values[0].isHandle ? args->values[0].handle->raw->class : getClassOf(args->values[0].value);
	NativeCodeEntry nativeCodeEntry = cachedLookupNativeCode(class, selector->raw);
	NativeCode *nativeCode = (NativeCode *) ((uint8_t *) nativeCodeEntry - offsetof(NativeCode, insts));
	Value rawArgs[args->size];
	initArgs(rawArgs, args);
	initThreadContext(&CurrentThread);
	return entry(nativeCode->compiledCode, nativeCodeEntry, rawArgs, &CurrentThread);
}


static void initArgs(Value *rawArgs, EntryArgs *args)
{
	for (size_t i = 0; i < args->size; i++) {
		EntryArg *arg = &args->values[i];
		rawArgs[i] = arg->isHandle ? getTaggedPtr(arg->handle) : arg->value;
	}
}


Value evalCode(char *source)
{
	HandleScope scope;
	openHandleScope(&scope);

	Value error;
	size_t sourceSize = strlen(source);
	char *eval = malloc(sourceSize + 7); // +7 for "eval[]" and \0
	Parser parser;
	Value result;

	memcpy(eval, "eval[", 5);
	memcpy(eval + 5, source, sourceSize);
	eval[sourceSize + 5] = ']';
	eval[sourceSize + 6] = '\0';

	initParser(&parser, asString(eval));
	MethodNode *node = parseMethod(&parser);
	if (node == NULL) {
		printParseError(&parser, eval);
		return tagInt(1);
	}

	patchMethodNode(node);
	Object *method = compileMethod(node, Handles.UndefinedObject);
	if (method->raw->class == Handles.CompiledMethod->raw) {
		EntryArgs args = { .size = 0 };
		entryArgsAddObject(&args, Handles.nil);
		result = invokeMethod((CompiledMethod *) method, &args);
	} else {
		printCompileError((CompileError *) method);
		result = tagInt(1);
	}

	freeParser(&parser);
	free(eval);

	EntryArgs args = { .size = 0 };
	if (valueTypeOf(result, VALUE_POINTER)) {
		entryArgsAddObject(&args, scopeHandle(asObject(result)));
	} else {
		entryArgsAdd(&args, result);
	}
	sendMessage(getSymbol("printNl"), &args);

	closeHandleScope(&scope, NULL);
	return valueTypeOf(result, VALUE_INT) ? result : tagInt(0);
}


static void patchMethodNode(MethodNode *method)
{
	OrderedCollection *expressions = blockNodeGetExpressions(methodNodeGetBody(method));
	size_t size = ordCollSize(expressions);
	if (size > 0) {
		expressionNodeEnableReturn((ExpressionNode *) ordCollObjectAt(expressions, size - 1));
	}
}


_Bool parseFileAndInitialize(char *filename, Value *lastBlockResult)
{
	HandleScope scope;
	openHandleScope(&scope);

	OrderedCollection *classes = newOrdColl(8);
	OrderedCollection *blocks = newOrdColl(8);
	if (!parseFile(filename, classes, blocks)) {
		return 0;
	}

	Iterator iterator;
	initOrdCollIterator(&iterator, classes, 0, 0);
	while (iteratorHasNext(&iterator)) {
		invokeInititalize(iteratorNextObject(&iterator));
	}

	initOrdCollIterator(&iterator, blocks, 0, 0);
	while (iteratorHasNext(&iterator)) {
		*lastBlockResult = evalBlockNode((BlockNode *) iteratorNextObject(&iterator));
	}

	closeHandleScope(&scope, NULL);
	return 1;
}


_Bool parseFile(char *filename, OrderedCollection *classes, OrderedCollection *blocks)
{
	HandleScope scope;
	openHandleScope(&scope);

	FILE *file = fopen(filename, "r");
	Parser parser;

	if (file == NULL) {
		printf("Cannot open file '%s' (errno: %i)\n", filename, errno);
		closeHandleScope(&scope, NULL);
		return 0;
	}
	initFileParser(&parser, file, asString(filename));

	while (!parserAtEnd(&parser)) {
		if (blocks != NULL && currentToken(&parser.tokenizer)->type == TOKEN_OPEN_SQUARE_BRACKET) {
			BlockNode *node = parseBlock(&parser);
			if (node == NULL) {
				printParseError(&parser, filename);
				closeHandleScope(&scope, NULL);
				return 0;
			}
			ordCollAddObject(blocks, (Object *) node);
		} else {
			ClassNode *node = parseClass(&parser);
			if (node == NULL) {
				printParseError(&parser, filename);
				closeHandleScope(&scope, NULL);
				return 0;
			}

			Object *class = buildClass(node);
			if (isCompileError(class)) {
				printCompileError((CompileError *) class);
				closeHandleScope(&scope, NULL);
				return 0;
			}

			if (classes != NULL) {
				ordCollAddObject(classes, class);
			}
		}
	}

	freeParser(&parser);
	fclose(file);
	closeHandleScope(&scope, NULL);
	return 1;
}


static Value evalBlockNode(BlockNode *block)
{
	HandleScope scope;
	openHandleScope(&scope);

	Value result;
	MethodNode *node = newObject(Handles.MethodNode, 0);
	methodNodeSetSelector(node, asString("eval"));
	methodNodeSetPragmas(node, newOrdColl(0));
	methodNodeSetBody(node, block);
	methodNodeSetSourceCode(node, blockNodeGetSourceCode(block));

	Object *method = compileMethod(node, Handles.UndefinedObject);
	if (method->raw->class == Handles.CompiledMethod->raw) {
		EntryArgs args = { .size = 0 };
		entryArgsAddObject(&args, Handles.nil);
		result = invokeMethod((CompiledMethod *) method, &args);
	} else {
		printCompileError((CompileError *) method);
		result = getTaggedPtr(Handles.nil);
	}

	closeHandleScope(&scope, NULL);
	return result;
}
