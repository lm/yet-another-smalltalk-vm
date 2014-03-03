#include "Handle.h"
#include "Thread.h"
#include "Heap.h"
#include "Assert.h"
#include <stdlib.h>

SmalltalkHandles Handles = { NULL };


void freeHandle(void *handle)
{
	Handle *p = (Handle *) handle;
	if (p->prev != NULL) {
		p->prev->next = p->next;
	} else {
		CurrentThread.handles = p->next;
	}
	free(handle);
}


void freeHandles(void)
{
	Handle *p = CurrentThread.handles;
	CurrentThread.handles = NULL;
	while (p != NULL) {
		Handle *next = p->next;
		free(p);
		p = next;
	}
}


void *newObject(Class *class, size_t size)
{
	return scopeHandle(allocateObject(&CurrentThread.heap, class->raw, size));
}


Object *copyResizedObject(Object *object, size_t newSize)
{
	Object *newObject = scopeHandle(allocateObject(&CurrentThread.heap, object->raw->class, newSize));
	size_t size = objectSize(object);
	size = computeInstanceSize(object->raw->class->instanceShape, newSize > size ? size : newSize);
	size_t offset = HEADER_SIZE + object->raw->class->instanceShape.isIndexed * sizeof(Value);
	memcpy((uint8_t *) newObject->raw + offset, (uint8_t *) object->raw + offset, size - offset);
	return newObject;
}


void initHandlesIterator(HandlesIterator *iterator, Handle *handles)
{
	iterator->current = handles;
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


void initHandleScopeIterator(HandleScopeIterator *iterator, HandleScope *scopes)
{
	iterator->current = scopes;
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
