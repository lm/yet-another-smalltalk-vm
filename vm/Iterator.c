#include "Iterator.h"
#include "Handle.h"
#include "Assert.h"


void initArrayIterator(Iterator *iterator, Array *array, ptrdiff_t from, ptrdiff_t to)
{
	ASSERT(array->raw->class == Handles.Array->raw);
	iterator->start = array->raw->vars + from;
	iterator->end = array->raw->vars + array->raw->size + to;
	iterator->current = iterator->start;
}


void initOrdCollIterator(Iterator *iterator, OrderedCollection *ordColl, ptrdiff_t from, ptrdiff_t to)
{
	ASSERT(ordColl->raw->class == Handles.OrderedCollection->raw);
	iterator->start = ordCollGetContents(ordColl)->vars + ordCollGetFirstIndex(ordColl) + from - 1;
	iterator->end = iterator->start + ordCollSize(ordColl) + to;
	iterator->current = iterator->start;
}


void initDictIterator(Iterator *iterator, Dictionary *dict)
{
	ASSERT(dict->raw->class == Handles.Dictionary->raw);
	initArrayIterator(iterator, dictGetContents(dict), 0, 0);
}


ptrdiff_t iteratorIndex(Iterator *iterator)
{
	return iterator->current - iterator->start;
}


_Bool iteratorHasNext(Iterator *iterator)
{
	return iterator->current < iterator->end;
}


Value iteratorNext(Iterator *iterator)
{
	return *iterator->current++;
}


Object *iteratorNextObject(Iterator *iterator)
{
	return scopeHandle(asObject(iteratorNext(iterator)));
}
