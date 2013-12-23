#ifndef ITERATOR_H
#define ITERATOR_H

#include "Object.h"
#include "Collection.h"
#include "Dictionary.h"

typedef struct {
	Value *start;
	Value *end;
	Value *current;
} Iterator;

void initArrayIterator(Iterator *iterator, Array *array, ptrdiff_t from, ptrdiff_t to);
void initOrdCollIterator(Iterator *iterator, OrderedCollection *ordColl, ptrdiff_t from, ptrdiff_t to);
void initDictIterator(Iterator *iterator, Dictionary *dict);
ptrdiff_t iteratorIndex(Iterator *iterator);
_Bool iteratorHasNext(Iterator *iterator);
Value iteratorNext(Iterator *iterator);
Object *iteratorNextObject(Iterator *iterator);

#endif
