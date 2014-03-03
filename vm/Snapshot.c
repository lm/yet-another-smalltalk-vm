#include "Snapshot.h"
#include "Thread.h"
#include "Heap.h"
#include "Object.h"
#include "Heap.h"
#include "Handle.h"
#include "Smalltalk.h"
#include "Assert.h"
#include <stdlib.h>
#include <string.h>

#define OBJECT_FIELD_MASK 7
#define OBJECT_INLINE 1
#define OBJECT_POINTER 5

enum {
	SS_ASSOC_DEFINED = 1,
	SS_ASSOC_WRITTEN = 1 << 1,
};

typedef struct {
	uint8_t flags;
	intptr_t key;
	intptr_t value;
} SnapshotAssoc;

typedef struct {
	size_t size;
	size_t tally;
	SnapshotAssoc *array;
} SnapshotDictionary;

typedef struct {
	FILE *file;
	SnapshotDictionary dict;
} Snapshot;

static void iterateHandles(Snapshot *snapshot);
static void writeNewObject(Snapshot *snapshot, RawObject *object);
static void writeObject(Snapshot *snapshot, RawObject *object);
static void writeField(Snapshot *snapshot, Value value);
static void writeFieldObject(Snapshot *snapshot, RawObject *object);
static void writeInt64(Snapshot *snapshot, int64_t value);
static void registerBuiltinObjects(Snapshot *snapshot);
static SnapshotAssoc *registerObject(Snapshot *snapshot, RawObject *object, _Bool written);
static Value readField(Snapshot *snapshot);
static Value readObject(int64_t field, Snapshot *snapshot);
static int64_t readInt64(Snapshot *snapshot);
static void createBuiltinObjectsHandles(Snapshot *snapshot);
static void initDicitonary(SnapshotDictionary *dict);
static void freeDictionary(SnapshotDictionary *dict);
static SnapshotAssoc *snapshotDictAtPut(SnapshotDictionary *dict, intptr_t key, intptr_t value);
static void snapshotGrowDict(SnapshotDictionary *dict);
static SnapshotAssoc *snapshotDictAt(SnapshotDictionary *dict, intptr_t key);
static ptrdiff_t findIndex(SnapshotDictionary *dict, intptr_t key);


void snapshotWrite(FILE *file)
{
	Snapshot snapshot;
	snapshot.file = file;
	initDicitonary(&snapshot.dict);
	registerBuiltinObjects(&snapshot);
	iterateHandles(&snapshot);
	freeDictionary(&snapshot.dict);
}


static void iterateHandles(Snapshot *snapshot)
{
	HandlesIterator handlesIterator;
	initHandlesIterator(&handlesIterator, CurrentThread.handles);
	while (handlesIteratorHasNext(&handlesIterator)) {
		writeNewObject(snapshot, handlesIteratorNext(&handlesIterator)->raw);
	}

	HandleScopeIterator handleScopeIterator;
	initHandleScopeIterator(&handleScopeIterator, CurrentThread.handleScopes);
	while (handleScopeIteratorHasNext(&handleScopeIterator)) {
		HandleScope *scope = handleScopeIteratorNext(&handleScopeIterator);
		for (ptrdiff_t i = 0; i < scope->size; i++) {
			writeNewObject(snapshot, scope->handles[i].raw);
		}
	}
}


static void writeNewObject(Snapshot *snapshot, RawObject *object)
{
	SnapshotAssoc *id = snapshotDictAt(&snapshot->dict, (intptr_t) object);
	if (id == NULL || (id->flags & SS_ASSOC_WRITTEN) == 0) {
		writeObject(snapshot, object);
	}
}


// +------------------------------------+
// | ID | pointer or inline | value tag |
// | instance shape                     |
// | object header                      |
// | indexed size (optional)            |
// | class                              |
// +------------------------------------+
static void writeObject(Snapshot *snapshot, RawObject *object)
{
	SnapshotAssoc *id = registerObject(snapshot, object, 1);
	Value *vars = getRawObjectVars(object);
	size_t size = object->class->instanceShape.varsSize;
	if (object->class->instanceShape.isIndexed && !object->class->instanceShape.isBytes) {
		size += rawObjectSize(object);
	}

	writeInt64(snapshot, (id->value << 3) | OBJECT_INLINE);
	writeInt64(snapshot, *(int64_t *) &object->class->instanceShape);
	writeInt64(snapshot, ((Value *) object)[1]); // header
	if (object->class->instanceShape.isIndexed) {
		writeInt64(snapshot, rawObjectSize(object));
	}
	writeFieldObject(snapshot, (RawObject *) object->class);
	for (size_t i = 0; i < size; i++) {
		writeField(snapshot, vars[i]);
	}
	if (object->class->instanceShape.isIndexed && object->class->instanceShape.isBytes) {
		size_t written = fwrite(getRawObjectIndexedVars(object), sizeof(uint8_t), rawObjectSize(object), snapshot->file);
		ASSERT(written == rawObjectSize(object));
	}
}


static void writeField(Snapshot *snapshot, Value value)
{
	if (valueTypeOf(value, VALUE_POINTER)) {
		writeFieldObject(snapshot, asObject(value));
	} else {
		writeInt64(snapshot, value);
	}
}


static void writeFieldObject(Snapshot *snapshot, RawObject *object)
{
	SnapshotAssoc *id = snapshotDictAt(&snapshot->dict, (intptr_t) object);
	if (id == NULL || (id->flags & SS_ASSOC_WRITTEN) == 0) {
		writeObject(snapshot, object);
	} else {
		writeInt64(snapshot, (id->value << 3) | OBJECT_POINTER);
	}
}


static void writeInt64(Snapshot *snapshot, int64_t value)
{
	size_t written = fwrite(&value, sizeof(int64_t), 1, snapshot->file);
	ASSERT(written == 1);
	fflush(snapshot->file);
}


static void registerBuiltinObjects(Snapshot *snapshot)
{
	Object **handle = (Object **) &Handles.nil;
	Object **end = handle + sizeof(Handles) / sizeof(*handle);

	while (handle < end) {
		registerObject(snapshot, (*handle)->raw, 0);
		handle++;
	}
}


static SnapshotAssoc *registerObject(Snapshot *snapshot, RawObject *object, _Bool written)
{
	SnapshotDictionary *dict = &snapshot->dict;
	ptrdiff_t index = findIndex(dict, (intptr_t) object);
	SnapshotAssoc *assoc = &dict->array[index];
	if ((assoc->flags & SS_ASSOC_DEFINED) == 0) {
		assoc->key = (intptr_t) object;
		assoc->value = dict->tally++;
		assoc->flags |= SS_ASSOC_DEFINED;
		if (dict->tally == dict->size) {
			snapshotGrowDict(dict);
		}
	}
	if (written) {
		assoc->flags |= SS_ASSOC_WRITTEN;
	}
	return assoc;
}


void snapshotRead(FILE *file)
{
	Snapshot snapshot;
	snapshot.file = file;
	initDicitonary(&snapshot.dict);

	do {
		int64_t field;
		if (fread(&field, sizeof(field), 1, file) != 1) {
			break;
		}
		readObject(field, &snapshot);
	} while (1);

	createBuiltinObjectsHandles(&snapshot);
	freeDictionary(&snapshot.dict);
}


static Value readField(Snapshot *snapshot)
{
	int64_t field = readInt64(snapshot);
	SnapshotAssoc *assoc;
	switch (field & OBJECT_FIELD_MASK) {
	case OBJECT_INLINE:
		return readObject(field, snapshot);
	case OBJECT_POINTER:
		assoc = snapshotDictAt(&snapshot->dict, field >> 3);
		ASSERT(assoc != NULL);
		return assoc->value;
	default:
		return field;
	}
}


static Value readObject(int64_t field, Snapshot *snapshot)
{
	ASSERT((field & OBJECT_FIELD_MASK) == OBJECT_INLINE);

	int64_t header[2];
	size_t read = fread(header, sizeof(int64_t), 2, snapshot->file);
	ASSERT(read == 2);

	uint64_t id = field >> 3;
	InstanceShape shape = *(InstanceShape *) &header[0];
	size_t size = shape.varsSize;
	size_t indexedSize = shape.isIndexed ? readInt64(snapshot) : 0;

	RawObject *object = (RawObject *) allocate(&CurrentThread.heap, computeInstanceSize(shape, indexedSize));
	snapshotDictAtPut(&snapshot->dict, id, tagPtr(object));

	if (shape.isIndexed) {
		((RawIndexedObject *) object)->size = indexedSize;
		if (!shape.isBytes) {
			size += indexedSize;
		}
	}

	((Value *) object)[1] = header[1]; // object header
	object->tags = 0;
	object->class = (RawClass *) asObject(readField(snapshot));
	ASSERT(object->class != NULL);

	Value *vars = getRawObjectVarsFromShape(object, shape);
	for (size_t i = 0; i < size; i++) {
		vars[i] = readField(snapshot);
	}

	if (shape.isIndexed && shape.isBytes) {
		fread(getRawObjectIndexedVarsFromShape(object, shape), sizeof(uint8_t), indexedSize, snapshot->file);
	}

	return tagPtr(object);
}


static int64_t readInt64(Snapshot *snapshot)
{
	int64_t value;
	size_t read = fread(&value, sizeof(value), 1, snapshot->file);
	ASSERT(read == 1);
	return value;
}


static void createBuiltinObjectsHandles(Snapshot *snapshot)
{
	Object **object = (Object **) &Handles.nil;
	Object **end = object + sizeof(Handles) / sizeof(*object);
	size_t i = 0;

	while (object < end) {
		*object = handle(asObject(snapshotDictAt(&snapshot->dict, i++)->value));
		object++;
	}
}


static void initDicitonary(SnapshotDictionary *dict)
{
	dict->size = 1024 * 8;
	dict->tally = 0;
	dict->array = malloc(dict->size * sizeof(*dict->array));
	ASSERT(dict->array != NULL);
	memset(dict->array, 0, dict->size * sizeof(*dict->array));
}


static void freeDictionary(SnapshotDictionary *dict)
{
	free(dict->array);
}


static SnapshotAssoc *snapshotDictAtPut(SnapshotDictionary *dict, intptr_t key, intptr_t value)
{
	ASSERT(dict->tally < dict->size);
	ptrdiff_t index = findIndex(dict, key);
	SnapshotAssoc *assoc = &dict->array[index];
	assoc->key = key;
	assoc->value = value;
	assoc->flags |= SS_ASSOC_DEFINED;
	dict->tally++;
	if (dict->tally == dict->size) {
		snapshotGrowDict(dict);
	}
	return assoc;
}


static void snapshotGrowDict(SnapshotDictionary *dict)
{
	size_t newSize = dict->size * 2;
	SnapshotAssoc *newArray = malloc(newSize * sizeof(*dict->array));
	ASSERT(newArray != NULL);
	memset(newArray, 0, newSize * sizeof(*dict->array));

	SnapshotAssoc *array = dict->array;
	dict->array = newArray;
	dict->size = newSize;

	for (size_t i = 0; i < dict->tally; i++) {
		if (array[i].flags & SS_ASSOC_DEFINED) {
			memcpy(&newArray[findIndex(dict, array[i].key)], &array[i], sizeof(*array));
		}
	}

	free(array);
}


static SnapshotAssoc *snapshotDictAt(SnapshotDictionary *dict, intptr_t key)
{
	ASSERT(dict->tally < dict->size);
	ptrdiff_t index = findIndex(dict, key);
	SnapshotAssoc *assoc = &dict->array[index];
	return (assoc->flags & SS_ASSOC_DEFINED) == 0 ? NULL : assoc;
}


static ptrdiff_t findIndex(SnapshotDictionary *dict, intptr_t key)
{
	ASSERT(dict->tally < dict->size);
	ptrdiff_t index = key & (dict->size - 1);

	do {
		SnapshotAssoc *assoc = &dict->array[index];
		if ((assoc->flags & SS_ASSOC_DEFINED) == 0 || assoc->key == key) {
			return index;
		}
		index = index == dict->size - 1 ? 0 : index + 1;
	} while(1);
}
