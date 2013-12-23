#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "Object.h"
#include "Collection.h"
#include "String.h"

typedef struct {
	OBJECT_HEADER;
	Value contents;
	Value tally;
} RawDictionary;
OBJECT_HANDLE(Dictionary);

typedef _Bool (*DictComparator)(Value, Value);

Dictionary *newDictionary(size_t size);
Array *dictGetContents(Dictionary *dict);
size_t dictSize(Dictionary *dict);

Association *dictAtPut(Dictionary *dict, DictComparator cmp, Object *key, Value hash, Value value);
Association *dictAtPutObject(Dictionary *dict, DictComparator cmp, Object *key, Value hash, Object *value);
Value dictAt(Dictionary *dict, DictComparator cmp, Value key, Value hash);
Association *dictAssocAt(Dictionary *dict, DictComparator cmp, Value key, Value hash);

Association *symbolDictAtPut(Dictionary *dictionary, String *key, Value value);
Association *symbolDictAtPutObject(Dictionary *dictionary, String *key, Object *object);
Value symbolDictAt(Dictionary *dictionary, String *key);
Object *symbolDictObjectAt(Dictionary *dictionary, String *key);
Association *symbolDictAssocAt(Dictionary *dictionary, String *key);

Association *stringDictAtPut(Dictionary *dictionary, String *key, Value value);
Association *stringDictAtPutObject(Dictionary *dictionary, String *key, Object *object);
Value stringDictAt(Dictionary *dictionary, String *key);
Object *stringDictObjectAt(Dictionary *dictionary, String *key);
Association *stringDictAssocAt(Dictionary *dictionary, String *key);

#endif
