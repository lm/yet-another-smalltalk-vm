#include "Collection.h"
#include "Thread.h"
#include "Smalltalk.h"
#include "Thread.h"
#include "Heap.h"
#include "Handle.h"
#include "Class.h"
#include "Iterator.h"
#include "Assert.h"
#include <string.h>

static _Bool compareValues(Object *object, Value value);


Array *newArray(size_t size)
{
	return newObject(Handles.Array, size);
}


Object *arrayObjectAt(Array *array, ptrdiff_t index)
{
	return scopeHandle(asObject(array->raw->vars[index]));
}


void arrayAtPutObject(Array *array, ptrdiff_t index, Object *object)
{
	objectStorePtr((Object *) array, &array->raw->vars[index], object);
}


OrderedCollection *arrayAsOrdColl(Array *array)
{
	size_t size = array->raw->size;
	OrderedCollection *ordColl = newOrdColl(size);
	RawArray *contents = ordCollGetContents(ordColl);
	for (size_t i = 0; i < size; i++) {
		Value value = array->raw->vars[i];
		if (valueTypeOf(value, VALUE_POINTER)) {
			rawObjectStorePtr((RawObject *) contents, &contents->vars[i], asObject(value));
		} else {
			contents->vars[i] = value;
		}
	}
	ordColl->raw->lastIndex += tagInt(size);
	return ordColl;
}


OrderedCollection *newOrdColl(size_t size)
{
	OrderedCollection *collection = newObject(Handles.OrderedCollection, 0);
	RawObject *contents = allocateObject(&CurrentThread.heap, Handles.Array->raw, size);
	rawObjectStorePtr((RawObject *) collection->raw, &collection->raw->contents, contents);
	collection->raw->firstIndex = tagInt(1);
	collection->raw->lastIndex = 0;
	return collection;
}


void ordCollAdd(OrderedCollection *collection, Value value)
{
	ASSERT(collection->raw->class == Handles.OrderedCollection->raw);
	Array *contents = scopeHandle(ordCollGetContents(collection));
	ASSERT(contents->raw->class == Handles.Array->raw);
	size_t size = ordCollSize(collection);

	if (size == contents->raw->size) {
		Array *newContents = newObject(Handles.Array, size + 8);
		memcpy(newContents->raw->vars, contents->raw->vars, size * sizeof(Value));
		objectStorePtr((Object *) collection, &collection->raw->contents, (Object *) newContents);
		contents = newContents;
	}
	intptr_t lastIndex = ordCollGetLastIndex(collection);
	contents->raw->vars[lastIndex] = value;
	collection->raw->lastIndex = tagInt(lastIndex + 1);
}


void ordCollAddObject(OrderedCollection *collection, Object *object)
{
	ASSERT(collection->raw->class == Handles.OrderedCollection->raw);
	Array *contents = scopeHandle(ordCollGetContents(collection));
	ASSERT(contents->raw->class == Handles.Array->raw);
	size_t size = ordCollSize(collection);

	if (size == contents->raw->size) {
		Array *newContents = newObject(Handles.Array, size + 8);
		memcpy(newContents->raw->vars, contents->raw->vars, size * sizeof(Value));
		objectStorePtr((Object *) collection, &collection->raw->contents, (Object *) newContents);
		contents = newContents;
	}
	intptr_t lastIndex = ordCollGetLastIndex(collection);
	objectStorePtr((Object *) contents, &contents->raw->vars[lastIndex], object);
	collection->raw->lastIndex = tagInt(lastIndex + 1);
}


ptrdiff_t ordCollAddObjectIfNotExists(OrderedCollection *collection, Object *object)
{
	Iterator iterator;
	initOrdCollIterator(&iterator, collection, 0, 0);
	while (iteratorHasNext(&iterator)) {
		Value value = iteratorNext(&iterator);
		if (compareValues(object, value)) {
			return iteratorIndex(&iterator) - 1;
		}
	}
	ordCollAddObject(collection, object);
	return ordCollSize(collection) - 1;
}


static _Bool compareValues(Object *object, Value value)
{
	if (getTaggedPtr(object) == value) {
		return 1;
	}
	if (object->raw->class == Handles.String->raw && getClassOf(value) == Handles.String->raw) {
		return stringEquals((String *) object, (String *) scopeHandle(asObject(value)));
	}
	return 0;
}


void ordCollRemoveLast(OrderedCollection *collection)
{
	RawArray *contents = ordCollGetContents(collection);
	intptr_t index = ordCollGetLastIndex(collection) - 1;
	rawObjectStorePtr((RawObject *) contents, &contents->vars[index], Handles.nil->raw);
	collection->raw->lastIndex = tagInt(index);
}


size_t ordCollSize(OrderedCollection *collection)
{
	return ordCollGetLastIndex(collection) - ordCollGetFirstIndex(collection) + 1;
}


Value ordCollAt(OrderedCollection *collection, ptrdiff_t index)
{
	return ordCollGetContents(collection)->vars[ordCollGetFirstIndex(collection) + index - 1];
}


Object *ordCollObjectAt(OrderedCollection *collection, Value index)
{
	return scopeHandle(asObject(ordCollAt(collection, index)));
}


RawArray *ordCollGetContents(OrderedCollection *collection)
{
	RawArray *contents = (RawArray *) asObject(collection->raw->contents);
	ASSERT(contents->class == Handles.Array->raw);
	return contents;
}


intptr_t ordCollGetFirstIndex(OrderedCollection *collection)
{
	return asCInt(collection->raw->firstIndex);
}


intptr_t ordCollGetLastIndex(OrderedCollection *collection)
{
	return asCInt(collection->raw->lastIndex);
}


Array *ordCollAsArray(OrderedCollection *collection)
{
	ASSERT(collection->raw->class == Handles.OrderedCollection->raw);
	size_t size = ordCollSize(collection);
	Array *array = newArray(size);
	memcpy(array->raw->vars, ordCollGetContents(collection)->vars, size * sizeof(Value));
	return array;
}
