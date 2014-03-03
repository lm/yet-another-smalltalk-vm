#include "Smalltalk.h"
#include "Thread.h"
#include "Heap.h"
#include "Handle.h"
#include "StackFrame.h"
#include "CodeDescriptors.h"
#include "Thread.h"
#include "Assert.h"
#include <string.h>
#include <stdio.h>

static void swapObjectPointers(Object *object, Object *other);
static void swapObjectInNewSpace(Object *old, Object *new);
static void swapObjectInOldSpace(Object *object, Object *other);
static void swapObjectOnStack(Object *object, Object *other);
static void iterateObject(RawObject *object, Object *old, Object *new);


String *asSymbol(String *string)
{
	HandleScope scope;
	openHandleScope(&scope);

	if (string->raw->class != Handles.String->raw) {
		FAIL();
	}
	String *symbol = scopeHandle(Handles.nil->raw);
	RawArray *table = (RawArray *) Handles.SymbolTable->raw;
	Value size = table->size;
	Value hash = computeStringHash(string) & 0xFFFFFFFF;
	size_t index = hash & size - 1;

	do {
		symbol->raw = (RawString *) asObject(table->vars[index]);
		if (isNil(symbol)) {
			String *newSymbol = (String *) copyResizedObject((Object *) string, string->raw->size);
			newSymbol->raw->class = Handles.Symbol->raw;
			newSymbol->raw->hash = hash;
			arrayAtPutObject(Handles.SymbolTable, index, (Object *) newSymbol);
			return closeHandleScope(&scope, newSymbol);
		} else if (stringEquals(string, symbol)) {
			return closeHandleScope(&scope, symbol);
		}
		index = index == size - 1 ? 0 : index + 1;
	} while (1);
}


String *getSymbol(char *string)
{
	return asSymbol(asString(string));
}


void setGlobal(char *key, Value value)
{
	HandleScope scope;
	openHandleScope(&scope);
	symbolDictAtPut(Handles.Smalltalk, getSymbol(key), value);
	closeHandleScope(&scope, NULL);
}


void setGlobalObject(char *key, Object *value)
{
	HandleScope scope;
	openHandleScope(&scope);
	symbolDictAtPutObject(Handles.Smalltalk, getSymbol(key), value);
	closeHandleScope(&scope, NULL);
}


Value getGlobal(char *key)
{
	return symbolDictAt(Handles.Smalltalk, getSymbol(key));
}


Object *getGlobalObject(char *key)
{
	return symbolDictObjectAt(Handles.Smalltalk, getSymbol(key));
}


void globalAtPut(String *key, Value value)
{
	symbolDictAtPut(Handles.Smalltalk, key, value);
}


Value globalAt(String *key)
{
	return symbolDictAt(Handles.Smalltalk, key);
}


Object *globalObjectAt(String *key)
{
	return scopeHandle(asObject(globalAt(key)));
}


Class *getClass(char *key)
{
	return (Class *) getGlobalObject(key);
}


void objectBecome(Object *old, Object *new)
{
	size_t oldSize = computeObjectSize(old);
	size_t newSize = computeObjectSize(new);

	if (oldSize == newSize) {
		memcpy(old->raw, new->raw, newSize);
	} else {
		swapObjectPointers(old, new);
	}
}


static void swapObjectPointers(Object *old, Object *new)
{
	swapObjectInNewSpace(old, new);
	swapObjectInOldSpace(old, new);
	swapObjectOnStack(old, new);
}


static void swapObjectInNewSpace(Object *old, Object *new)
{
	size_t objects = 0;
	RawObject *object = (RawObject *) ((uintptr_t) CurrentThread.heap.newSpace.fromSpace | NEW_SPACE_TAG);
	RawObject *prev = NULL;
	RawObject *end = (RawObject *) CurrentThread.heap.newSpace.top;
	while (object < end) {
		objects++;
		iterateObject(object, old, new);
		prev = object;
		object = (RawObject *) ((uint8_t *) object + align(computeRawObjectSize(object), HEAP_OBJECT_ALIGN));
	}
}


static void swapObjectInOldSpace(Object *old, Object *new)
{
	PageSpaceIterator iterator;
	pageSpaceIteratorInit(&iterator, &CurrentThread.heap.oldSpace);
	RawObject *object = pageSpaceIteratorNext(&iterator);
	while (object != NULL) {
		if ((object->tags & TAG_FREESPACE) != 0) {
			object = pageSpaceIteratorNext(&iterator);
			continue;
		}
		iterateObject(object, new, old);
		object = pageSpaceIteratorNext(&iterator);
	}
}


static void swapObjectOnStack(Object *old, Object *new)
{
	Value tOld = getTaggedPtr(old);
	Value tNew = getTaggedPtr(new);
	EntryStackFrame *entryFrame = CurrentThread.stackFramesTail;

	while (entryFrame != NULL) {
		StackFrame *prev = entryFrame->exit;
		StackFrame *frame = stackFrameGetParent(prev, entryFrame);
		while (frame != NULL) {
			NativeCode *code = stackFrameGetNativeCode(frame);

			size_t argsSize = ((RawCompiledMethod *) code->compiledCode)->header.argsSize + 1;
			for (ptrdiff_t i = 0; i < argsSize; i++) {
				Value value = stackFrameGetArg(frame, i);
				if (value == tOld) {
					stackFrameSetArg(frame, i, tNew);
				}
			}

			RawStackmap *stackmap = findStackmap(code, (ptrdiff_t) prev->parentIc);
			ASSERT(stackmap != NULL);
			size_t frameSize = (stackmap->size - sizeof(Value)) * 8;
			for (size_t i = 0; i < frameSize; i++) {
				//ASSERT(i != 0 || stackmapIncludes(stackmap, i));
				if (stackmapIncludes(stackmap, i)) {
					Value value = stackFrameGetSlot(frame, i);
					if (value == tOld) {
						stackFrameSetSlot(frame, i, tNew);
					}
				}
			}

			prev = frame;
			frame = stackFrameGetParent(frame, entryFrame);
		}
		entryFrame = entryFrame->prev;
	}
}


static void iterateObject(RawObject *object, Object *old, Object *new)
{
	ASSERT(((intptr_t) object->class & 3) == 0);
	Value *vars = getRawObjectVars(object);
	size_t size = object->class->instanceShape.varsSize;
	if (object->class->instanceShape.isIndexed && !object->class->instanceShape.isBytes) {
		size += rawObjectSize(object);
	}
	// TODO: allow swaping object class?
	//if (object->class == (RawClass *) old->raw) {
	//	object->class = (RawClass *) new->raw;
	//}
	for (size_t i = 0; i < size; i++) {
		if (vars[i] == getTaggedPtr(old)) {
			rawObjectStorePtr(object, &vars[i], new->raw);
		}
	}
}
