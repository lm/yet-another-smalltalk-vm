#include "Handle.h"
#include "Heap.h"
#include "Assert.h"
#include <stdlib.h>

Handle *HeapHandles = NULL;
SmalltalkHandles Handles = { NULL };
HandleScope *ScopeTail = NULL;


void freeHandle(void *handle)
{
	Handle *p = (Handle *) handle;
	if (p->prev != NULL) {
		p->prev->next = p->next;
	} else {
		HeapHandles = p->next;
	}
	free(handle);
}


void freeHandles(void)
{
	Handle *p = HeapHandles;
	HeapHandles = NULL;
	while (p != NULL) {
		Handle *next = p->next;
		free(p);
		p = next;
	}
}


Object *newObject(Class *class, size_t size)
{
	return scopeHandle(allocateObject(class->raw, size));
}


Object *copyResizedObject(Object *object, size_t newSize)
{
	Object *newObject = scopeHandle(allocateObject(object->raw->class, newSize));
	size_t size = objectSize(object);
	size = computeInstanceSize(object->raw->class->instanceShape, newSize > size ? size : newSize);
	size_t offset = HEADER_SIZE + object->raw->class->instanceShape.isIndexed * sizeof(Value);
	memcpy((uint8_t *) newObject->raw + offset, (uint8_t *) object->raw + offset, size - offset);
	return newObject;
}


void initHandlesIterator(HandlesIterator *iterator)
{
	iterator->current = HeapHandles;
}


_Bool handlesIteratorHasNext(HandlesIterator *iterator)
{
	return iterator->current != NULL;
}


Object *handlesIteratorNext(HandlesIterator *iterator)
{
	Handle *current = iterator->current;
	iterator->current = current->next;
	return (Object *) current;
}


void initHandleScopeIterator(HandleScopeIterator *iterator)
{
	iterator->current = ScopeTail;
}


_Bool handleScopeIteratorHasNext(HandleScopeIterator *iterator)
{
	return iterator->current != NULL;
}


HandleScope *handleScopeIteratorNext(HandleScopeIterator *iterator)
{
	HandleScope *current = iterator->current;
	iterator->current = current->parent;
	return current;
}
