#ifndef COLLECTION_H
#define COLLECTION_H

#include "Object.h"

typedef struct {
	OBJECT_HEADER;
	Value contents;
	Value firstIndex;
	Value lastIndex;
} RawOrderedCollection;
OBJECT_HANDLE(OrderedCollection);

Array *newArray(size_t size);
Object *arrayObjectAt(Array *array, ptrdiff_t index);
void arrayAtPutObject(Array *array, ptrdiff_t index, Object *object);
OrderedCollection *arrayAsOrdColl(Array *array);
OrderedCollection *newOrdColl(size_t size);
size_t ordCollSize(OrderedCollection *collection);
void ordCollAdd(OrderedCollection *collection, Value value);
void ordCollAddObject(OrderedCollection *collection, Object *object);
ptrdiff_t ordCollAddObjectIfNotExists(OrderedCollection *collection, Object *object);
void ordCollRemoveLast(OrderedCollection *collection);
Value ordCollAt(OrderedCollection *collection, ptrdiff_t index);
Object *ordCollObjectAt(OrderedCollection *collection, Value index);
RawArray *ordCollGetContents(OrderedCollection *collection);
intptr_t ordCollGetFirstIndex(OrderedCollection *collection);
intptr_t ordCollGetLastIndex(OrderedCollection *collection);
Array *ordCollAsArray(OrderedCollection *collection);

#endif
