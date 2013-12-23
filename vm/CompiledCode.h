#ifndef COMPILED_CODE_H
#define COMPILED_CODE_H

#include "Object.h"
#include "Class.h"
#include "Bytecodes.h"
#include "Collection.h"
#include "String.h"
#include "Parser.h"

typedef Value (*NativeCodeEntry)();

typedef struct NativeCode {
	void *compiledCode;
	uintptr_t size:56;
	uint8_t tags;
	size_t pointersOffsetsSize;
	size_t argsSize;
	RawArray *stackmaps;
	RawArray *descriptors;
	RawOrderedCollection *typeFeedback;
	size_t counter;
	uint8_t insts[];
	// uint16_t pointersOffsets;
} NativeCode;

typedef struct {
	uint8_t tag;
	uint8_t argsSize;
	uint8_t tempsSize;
	uint8_t hasContext;
	uint8_t contextSize;
	uint8_t outerReturns;
	uint16_t primitive;
} CompiledCodeHeader;

typedef struct {
	OBJECT_HEADER;
	Value size;
	NativeCode *nativeCode;
	CompiledCodeHeader header;
	Value method;
	Value sourceCode;
	Value descriptors;
	uint8_t bytes[];
} RawCompiledBlock;
OBJECT_HANDLE(CompiledBlock);

typedef struct {
	OBJECT_HEADER;
	Value size;
	NativeCode *nativeCode;
	CompiledCodeHeader header;
	Value literals;
	Value selector;
	Value ownerClass;
	Value sourceCode;
	Value descriptors;
	uint8_t bytes[];
} RawCompiledMethod;
OBJECT_HANDLE(CompiledMethod);

typedef struct {
	OBJECT_HEADER;
	NativeCode *nativeCode;
	Value compiledBlock;
	Value receiver;
	Value outerContext;
	Value homeContext;
} RawBlock;
OBJECT_HANDLE(Block);

typedef struct {
	_Bool isBlock;
	void *methodOrBlock;
	CompiledCodeHeader header;
	Array *literals;
	Class *ownerClass;
	uint8_t *bytecodes;
	size_t bytecodesSize;
} CompiledCode;


NativeCode *findNativeCodeAtIc(uint8_t *ic);
void printMethodsUsage(void);


static void compiledBlockSetNativeCode(CompiledBlock *block, NativeCode *code)
{
	block->raw->nativeCode = code;
}


static NativeCode *compiledBlockGetNativeCode(CompiledBlock *block)
{
	return block->raw->nativeCode;
}


static void compiledBlockSetHeader(CompiledBlock *block, CompiledCodeHeader header)
{
	block->raw->header = header;
}


static CompiledCodeHeader compiledBlockGetHeader(CompiledBlock *block)
{
	return block->raw->header;
}


static void compiledBlockSetMethod(CompiledBlock *block, CompiledMethod *method)
{
	objectStorePtr((Object *) block,  &block->raw->method, (Object *) method);
}


static CompiledMethod *compiledBlockGetMethod(CompiledBlock *block)
{
	return scopeHandle(asObject(block->raw->method));
}


static void compiledBlockSetSourceCode(CompiledBlock *block, SourceCode *sourceCode)
{
	objectStorePtr((Object *) block,  &block->raw->sourceCode, (Object *) sourceCode);
}


static SourceCode *compiledBlockGetSourceCode(CompiledBlock *block)
{
	return scopeHandle(asObject(block->raw->sourceCode));
}


static void compiledBlockSetDescriptors(CompiledBlock *block, Array *descriptors)
{
	objectStorePtr((Object *) block,  &block->raw->descriptors, (Object *) descriptors);
}


static Array *compiledBlockGetDescriptors(CompiledBlock *block)
{
	return scopeHandle(asObject(block->raw->descriptors));
}


static uint8_t *compiledBlockGetBytes(CompiledBlock *block)
{
	return block->raw->bytes;
}


static void compiledMethodSetNativeCode(CompiledMethod *method, NativeCode *code)
{
	method->raw->nativeCode = code;
}


static NativeCode *compiledMethodGetNativeCode(CompiledMethod *method)
{
	return method->raw->nativeCode;
}


static void compiledMethodSetHeader(CompiledMethod *method, CompiledCodeHeader header)
{
	method->raw->header = header;
}


static CompiledCodeHeader compiledMethodGetHeader(CompiledMethod *method)
{
	return method->raw->header;
}


static void compiledMethodSetLiterals(CompiledMethod *method, Array *literals)
{
	objectStorePtr((Object *) method,  &method->raw->literals, (Object *) literals);
}


static Array *compiledMethodGetLiterals(CompiledMethod *method)
{
	return scopeHandle(asObject(method->raw->literals));
}


static void compiledMethodSetSelector(CompiledMethod *method, String *selector)
{
	objectStorePtr((Object *) method,  &method->raw->selector, (Object *) selector);
}


static String *compiledMethodGetSelector(CompiledMethod *method)
{
	return scopeHandle(asObject(method->raw->selector));
}


static void compiledMethodSetOwnerClass(CompiledMethod *method, Class *class)
{
	objectStorePtr((Object *) method,  &method->raw->ownerClass, (Object *) class);
}


static Class *compiledMethodGetOwnerClass(CompiledMethod *method)
{
	return scopeHandle(asObject(method->raw->ownerClass));
}


static void compiledMethodSetSourceCode(CompiledMethod *method, SourceCode *sourceCode)
{
	objectStorePtr((Object *) method,  &method->raw->sourceCode, (Object *) sourceCode);
}


static SourceCode *compiledMethodGetSourceCode(CompiledMethod *method)
{
	return scopeHandle(asObject(method->raw->sourceCode));
}


static void compiledMethodSetDescriptors(CompiledMethod *method, Array *descriptors)
{
	objectStorePtr((Object *) method,  &method->raw->descriptors, (Object *) descriptors);
}


static Array *compiledMethodGetDescriptors(CompiledMethod *method)
{
	return scopeHandle(asObject(method->raw->descriptors));
}


static uint8_t *compiledMethodGetBytes(CompiledMethod *method)
{
	return method->raw->bytes;
}


static void initBlockCompiledCode(CompiledCode *code, CompiledBlock *block)
{
	CompiledMethod *method = compiledBlockGetMethod(block);
	code->isBlock = 1;
	code->methodOrBlock = block;
	code->header = compiledBlockGetHeader(block);
	code->literals = compiledMethodGetLiterals(method);
	code->ownerClass = compiledMethodGetOwnerClass(method);
	code->bytecodes = compiledBlockGetBytes(block);
	code->bytecodesSize = objectSize((Object *) block);
}


static void initMethodCompiledCode(CompiledCode *code, CompiledMethod *method)
{
	code->isBlock = 0;
	code->methodOrBlock = method;
	code->header = compiledMethodGetHeader(method);
	code->literals = compiledMethodGetLiterals(method);
	code->ownerClass = compiledMethodGetOwnerClass(method);
	code->bytecodes = compiledMethodGetBytes(method);
	code->bytecodesSize = objectSize((Object *) method);
}


static RawObject *compiledCodeLiteralAt(CompiledCode *code, ptrdiff_t index)
{
	return asObject(code->literals->raw->vars[index]);
}


static RawClass *compiledCodeResolveOperandClass(CompiledCode *code, Operand operand)
{
	switch (operand.type) {
	case OPERAND_VALUE:
		return getClassOf(operand.value);
	case OPERAND_NIL:
		return Handles.UndefinedObject->raw;
	case OPERAND_TRUE:
		return Handles.True->raw;
	case OPERAND_FALSE:
		return Handles.False->raw;
	case OPERAND_THIS_CONTEXT:
		return (code->isBlock ? Handles.BlockContext : Handles.MethodContext)->raw;
	case OPERAND_SUPER:
		return (RawClass *) asObject(code->ownerClass->raw->superClass);
	case OPERAND_LITERAL:
		return compiledCodeLiteralAt(code, operand.index)->class;
	case OPERAND_BLOCK:
		return Handles.Block->raw;
	default:
		return NULL;
	}
}


static size_t computeNativeCodeSize(NativeCode *code)
{
	return sizeof(NativeCode) + code->size + code->pointersOffsetsSize * sizeof(uint16_t);
}

#endif
