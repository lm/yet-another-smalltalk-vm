#ifndef ENTRY_H
#define ENTRY_H

#include "Object.h"
#include "CompiledCode.h"
#include "String.h"

#define ENTRY_MAX_ARGS_SIZE 16

typedef struct {
	_Bool isHandle;
	union { Value value; Object *handle; };
} EntryArg;

typedef struct {
	size_t size;
	EntryArg values[ENTRY_MAX_ARGS_SIZE];
} EntryArgs;

Value invokeMethod(CompiledMethod *method, EntryArgs *args);
Value invokeInititalize(Object *object);
Value sendMessage(String *selector, EntryArgs *args);
Value evalCode(char *source);
_Bool parseFileAndInitialize(char *filename, Value *lastBlockResult);
_Bool parseFile(char *filename, OrderedCollection *classes, OrderedCollection *blocks);


static void entryArgsAddObject(EntryArgs *args, Object *object)
{
	intptr_t index = args->size++;
	ASSERT(index <= ENTRY_MAX_ARGS_SIZE);
	args->values[index].isHandle = 1;
	args->values[index].handle = object;
}


static void entryArgsAdd(EntryArgs *args, Value value)
{
	intptr_t index = args->size++;
	ASSERT(index <= ENTRY_MAX_ARGS_SIZE);
	args->values[index].isHandle = 0;
	args->values[index].value = value;
}

#endif
