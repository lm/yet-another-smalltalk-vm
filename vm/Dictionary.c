#include "Dictionary.h"
#include "Thread.h"
#include "Smalltalk.h"
#include "Heap.h"
#include "Handle.h"
#include "Iterator.h"
#include "Class.h"
#include "Assert.h"

static void growDictionary(Dictionary *dict);
static Association *createAssoc(Object *key, Object *value);
static size_t findIndex(RawArray *contents, DictComparator cmp, Value key, Value hash);


Dictionary *newDictionary(size_t size)
{
	Dictionary *dict = newObject(Handles.Dictionary, 0);
	// XXX: code bellow causes optimization bug in GCC:
	// dict->raw->contents = tagPtr(allocateObject(&CurrentThread.heap, Handles.Array->raw, size));
	RawObject *contents = allocateObject(&CurrentThread.heap, Handles.Array->raw, size);
	rawObjectStorePtr((RawObject *) dict->raw, &dict->raw->contents, contents);
	dict->raw->tally = 0;
	return dict;
}


Array *dictGetContents(Dictionary *dict)
{
	Array *contents = scopeHandle(asObject(dict->raw->contents));
	ASSERT(contents->raw->class == Handles.Array->raw);
	return contents;
}


size_t dictSize(Dictionary *dict)
{
	return asCInt(dict->raw->tally);
}


Association *dictAtPut(Dictionary *dict, DictComparator cmp, Object *key, Value hash, Value value)
{
	ASSERT(dict->raw->class == Handles.Dictionary->raw);
	Array *contents = dictGetContents(dict);
	ptrdiff_t index = findIndex(contents->raw, cmp, getTaggedPtr(key), hash);
	Association *assoc = scopeHandle(asObject(contents->raw->vars[index]));

	if (isNil(assoc)) {
		assoc = newObject(Handles.Association, 0);
		objectStorePtr((Object *) assoc, &assoc->raw->key, key);
		assoc->raw->value = value;

		arrayAtPutObject(contents, index, (Object *) assoc);
		dict->raw->tally += tagInt(1);
		if (dictSize(dict) == contents->raw->size) {
			growDictionary(dict);
		}
	} else {
		ASSERT(assoc->raw->class == Handles.Association->raw);
		assoc->raw->value = value;
	}
	return assoc;
}


Association *dictAtPutObject(Dictionary *dict, DictComparator cmp, Object *key, Value hash, Object *value)
{
	ASSERT(dict->raw->class == Handles.Dictionary->raw);
	Array *contents = dictGetContents(dict);
	ptrdiff_t index = findIndex(contents->raw, cmp, getTaggedPtr(key), hash);
	Association *assoc = scopeHandle(asObject(contents->raw->vars[index]));

	if (isNil(assoc)) {
		assoc = createAssoc(key, value);
		arrayAtPutObject(contents, index, (Object *) assoc);
		dict->raw->tally += tagInt(1);
		if (dictSize(dict) == contents->raw->size) {
			growDictionary(dict);
		}
	} else {
		ASSERT(assoc->raw->class == Handles.Association->raw);
		objectStorePtr((Object *) assoc, &assoc->raw->value, value);
	}
	return assoc;
}


static void growDictionary(Dictionary *dict)
{
	Array *contents = scopeHandle((RawArray *) asObject(dict->raw->contents));
	Array *newContents = newObject(Handles.Array, contents->raw->size * 2);
	objectStorePtr((Object *) dict,  &dict->raw->contents, (Object *) newContents);

	Iterator iterator;
	initArrayIterator(&iterator, contents, 0, 0);

	while (iteratorHasNext(&iterator)) {
		HandleScope scope;
		openHandleScope(&scope);

		Association *assoc = (Association *) iteratorNextObject(&iterator);
		if (assoc->raw->class == Handles.Association->raw) {
			RawClass *class = getClassOf(assoc->raw->key);
			if (class == Handles.String->raw) {
				stringDictAtPut(dict, scopeHandle(asObject(assoc->raw->key)), assoc->raw->value);
			} else if (class == Handles.Symbol->raw) {
				symbolDictAtPut(dict, scopeHandle(asObject(assoc->raw->key)), assoc->raw->value);
			} else {
				FAIL();
			}
		}

		closeHandleScope(&scope, NULL);
	}
}


static Association *createAssoc(Object *key, Object *value)
{
	Association *assoc = newObject(Handles.Association, 0);
	objectStorePtr((Object *) assoc, &assoc->raw->key, key);
	objectStorePtr((Object *) assoc, &assoc->raw->value, value);
	return assoc;
}


Value dictAt(Dictionary *dict, DictComparator cmp, Value key, Value hash)
{
	ASSERT(dict->raw->class == Handles.Dictionary->raw);
	RawArray *contents = (RawArray *) asObject(dict->raw->contents);
	ASSERT(contents->class == Handles.Array->raw);
	ptrdiff_t index = findIndex(contents, cmp, key, hash);
	RawAssociation *assoc = (RawAssociation *) asObject(contents->vars[index]);
	if (isRawNil(assoc)) {
		return getTaggedPtr(Handles.nil);
	}
	ASSERT(assoc->class == Handles.Association->raw);
	return assoc->value;
}


Association *dictAssocAt(Dictionary *dict, DictComparator cmp, Value key, Value hash)
{
	ASSERT(dict->raw->class == Handles.Dictionary->raw);
	RawArray *contents = (RawArray *) asObject(dict->raw->contents);
	ASSERT(contents->class == Handles.Array->raw);
	ptrdiff_t index = findIndex(contents, cmp, key, hash);
	return scopeHandle(asObject(contents->vars[index]));
}


static _Bool identityCmp(Value a, Value b)
{
	return a == b;
}


Association *symbolDictAtPut(Dictionary *dict, String *key, Value value)
{
	ASSERT(key->raw->class == Handles.Symbol->raw);
	return dictAtPut(dict, &identityCmp, (Object *) key, objectGetHash((Object *) key), value);
}


Association *symbolDictAtPutObject(Dictionary *dict, String *key, Object *value)
{
	ASSERT(key->raw->class == Handles.Symbol->raw);
	return dictAtPutObject(dict, &identityCmp, (Object *) key, objectGetHash((Object *) key), value);
}


Value symbolDictAt(Dictionary *dict, String *key)
{
	ASSERT(key->raw->class == Handles.Symbol->raw);
	return dictAt(dict, &identityCmp, getTaggedPtr(key), objectGetHash((Object *) key));
}


Object *symbolDictObjectAt(Dictionary *dict, String *key)
{
	return scopeHandle(asObject(symbolDictAt(dict, key)));
}


Association *symbolDictAssocAt(Dictionary *dict, String *key)
{
	ASSERT(key->raw->class == Handles.Symbol->raw);
	return dictAssocAt(dict, &identityCmp, getTaggedPtr(key), objectGetHash((Object *) key));
}


static _Bool stringCmp(Value a, Value b)
{
	return stringEquals(scopeHandle(asObject(a)), scopeHandle(asObject(b)));
}


Association *stringDictAtPut(Dictionary *dict, String *key, Value value)
{
	ASSERT(key->raw->class == Handles.String->raw);
	return dictAtPut(dict, &stringCmp, (Object *) key, computeStringHash(key), value);
}


Association *stringDictAtPutObject(Dictionary *dict, String *key, Object *value)
{
	ASSERT(key->raw->class == Handles.String->raw);
	return dictAtPutObject(dict, &stringCmp, (Object *) key, computeStringHash(key), value);
}


Value stringDictAt(Dictionary *dict, String *key)
{
	ASSERT(key->raw->class == Handles.String->raw);
	return dictAt(dict, &stringCmp, getTaggedPtr(key), computeStringHash(key));
}


Object *stringDictObjectAt(Dictionary *dict, String *key)
{
	return scopeHandle(asObject(stringDictAt(dict, key)));
}


Association *stringDictAssocAt(Dictionary *dict, String *key)
{
	ASSERT(key->raw->class == Handles.String->raw);
	return dictAssocAt(dict, &stringCmp, getTaggedPtr(key), computeStringHash(key));
}


static size_t findIndex(RawArray *contents, DictComparator cmp, Value key, Value hash)
{
	RawAssociation *assoc;
	ptrdiff_t index = hash & contents->size - 1;

	do {
		assoc = (RawAssociation *) asObject(contents->vars[index]);
		if (isRawNil(assoc) || cmp(assoc->key, key)) {
			return index;
		}
		index = index == contents->size - 1 ? 0 : index + 1;
	} while (1);
}
