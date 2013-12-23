#ifndef CODE_DESCRIPTORS_H
#define CODE_DESCRIPTORS_H

#include "Object.h"
#include "Iterator.h"
#include "CompiledCode.h"

typedef struct {
	OBJECT_HEADER;
	Value size;
	Value ic;
	uint8_t set[];
} RawStackmap;
OBJECT_HANDLE(Stackmap);

typedef struct {
	OBJECT_HEADER;
	Value ic;
	Value hintedClass;
} RawTypeFeedback;
OBJECT_HANDLE(TypeFeedback);

#define DESC_POS_OFFSET 48
#define DESC_LINE_OFFSET 32
#define DESC_COLUMN_OFFSET 16
#define DESC_BYTECODE_OFFSET 32

static Value createSouceCodeDescriptor(uint16_t pos, uint16_t line, uint16_t column);
static Value createBytecodeDescriptor(uint16_t pos, uint16_t bytecodePos);
static uint16_t descriptorGetPos(Value descriptor);
static uint16_t descriptorGetLine(Value descriptor);
static uint16_t descriptorGetColumn(Value descriptor);
static uint16_t descriptorGetBytecode(Value descriptor);
static Class *typeFeedbackGetHintedClass(TypeFeedback *feedback);
static void stackmapAdd(RawStackmap *stackmap, ptrdiff_t i);
static _Bool stackmapIncludes(RawStackmap *stackmap, ptrdiff_t i);
static Value descriptorsAtPosition(RawArray *descriptors, uint16_t pos);
static Value descriptorsAtBytecode(RawArray *descriptors, uint16_t bytecodePos);
static Value findSourceCode(RawObject *compiledCode, ptrdiff_t ic);
static RawStackmap *findStackmap(NativeCode *code, ptrdiff_t ic);
static TypeFeedback *findTypeFeeddback(OrderedCollection *typeFeedback, ptrdiff_t ic);


static Value createSouceCodeDescriptor(uint16_t pos, uint16_t line, uint16_t column)
{
	Value descriptor = 0;
	descriptor |= (Value) pos << DESC_POS_OFFSET;
	descriptor |= (Value) line << DESC_LINE_OFFSET;
	descriptor |= (Value) column << DESC_COLUMN_OFFSET;
	ASSERT(pos == descriptorGetPos(descriptor));
	ASSERT(line == descriptorGetLine(descriptor));
	ASSERT(column == descriptorGetColumn(descriptor));
	return descriptor;
}


static Value createBytecodeDescriptor(uint16_t pos, uint16_t bytecodePos)
{
	Value descriptor = 0;
	descriptor |= (Value) pos << DESC_POS_OFFSET;
	descriptor |= (Value) bytecodePos << DESC_BYTECODE_OFFSET;
	ASSERT(pos == descriptorGetPos(descriptor));
	ASSERT(bytecodePos == descriptorGetBytecode(descriptor));
	return descriptor;
}


static uint16_t descriptorGetPos(Value descriptor)
{
	return descriptor >> DESC_POS_OFFSET;
}


static uint16_t descriptorGetLine(Value descriptor)
{
	return descriptor >> DESC_LINE_OFFSET;
}


static uint16_t descriptorGetColumn(Value descriptor)
{
	return descriptor >> DESC_COLUMN_OFFSET;
}


static uint16_t descriptorGetBytecode(Value descriptor)
{
	return descriptor >> DESC_BYTECODE_OFFSET;
}


static Class *typeFeedbackGetHintedClass(TypeFeedback *feedback)
{
	return scopeHandle(asObject(feedback->raw->hintedClass));
}


static void stackmapAdd(RawStackmap *stackmap, ptrdiff_t i)
{
	stackmap->set[i / 8] |= 1 << (i % 8);
}


static _Bool stackmapIncludes(RawStackmap *stackmap, ptrdiff_t i)
{
	size_t size = stackmap->size * 8 - sizeof(Value);
	return i < size && (stackmap->set[i / 8] & (1 << (i % 8))) != 0;
}


static Value descriptorsAtPosition(RawArray *descriptors, uint16_t pos)
{
	size_t size = descriptors->size;

	for (size_t i = 0; i < size; i++) {
		Value descriptor = descriptors->vars[i];
		if (descriptorGetPos(descriptor) == pos) {
			return descriptor;
		}
	}
	return 0;
}


static Value descriptorsAtBytecode(RawArray *descriptors, uint16_t bytecodePos)
{
	size_t size = descriptors->size;

	for (size_t i = 0; i < size; i++) {
		Value descriptor = descriptors->vars[i];
		if (descriptorGetBytecode(descriptor) == bytecodePos) {
			return descriptor;
		}
	}
	return 0;
}


static Value findSourceCode(RawObject *compiledCode, ptrdiff_t ic)
{
	HandleScope scope;
	openHandleScope(&scope);

	NativeCode *nativeCode = ((RawCompiledMethod *) compiledCode)->nativeCode;
	if (nativeCode->descriptors == NULL) {
		closeHandleScope(&scope, NULL);
		return 0;
	}

	Value bytecode = descriptorsAtPosition(nativeCode->descriptors, ic - (ptrdiff_t) nativeCode->insts);
	RawArray *descriptors;
	if (compiledCode->class == Handles.CompiledMethod->raw) {
		descriptors = (RawArray *) asObject(((RawCompiledMethod *) compiledCode)->descriptors);
	} else {
		descriptors = (RawArray *) asObject(((RawCompiledBlock *) compiledCode)->descriptors);
	}
	Value descriptor = descriptorsAtPosition(descriptors, descriptorGetBytecode(bytecode));
	closeHandleScope(&scope, NULL);
	return descriptor;
}


static RawStackmap *findStackmap(NativeCode *code, ptrdiff_t ic)
{
	ptrdiff_t relIc = ic - (ptrdiff_t) code->insts;
	RawArray *stackmaps = code->stackmaps;
	size_t size = stackmaps->size;
	for (size_t i = 0; i < size; i++) {
		RawStackmap *stackmap = (RawStackmap *) asObject(stackmaps->vars[i]);
		if (stackmap->ic == relIc) {
			return stackmap;
		}
	}
	return NULL;
}


static TypeFeedback *findTypeFeeddback(OrderedCollection *typeFeedback, ptrdiff_t ic)
{
	Iterator iterator;
	initOrdCollIterator(&iterator, typeFeedback, 0, 0);

	while (iteratorHasNext(&iterator)) {
		TypeFeedback *type = (TypeFeedback *) iteratorNextObject(&iterator);
		if (type->raw->ic == ic) {
			return type;
		}
	}
	return NULL;
}

#endif
