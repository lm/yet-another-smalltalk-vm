#ifndef HEAP_H
#define HEAP_H

#include "Object.h"
#include "HeapPage.h"
#include "Scavenger.h"

#define VERIFY_HEAP_AFTER_GC 0

typedef struct {
	Scavenger newSpace;
	PageSpace oldSpace;
	PageSpace execSpace;
	struct {
		Value objects[8192 * 16];
		Value *end;
	} rememberedSet;
} Heap;

struct NativeCode;

extern Heap _Heap;

void initHeap(void);
void freeHeap(void);
RawObject *allocateObject(RawClass *class, size_t size);
void freeObject(RawObject *object);
struct NativeCode *allocateNativeCode(size_t size, size_t pointersOffsetsSize);
uint8_t *allocate(size_t size);
uint8_t *tryAllocateOld(size_t size, _Bool grow);
void collectGarbage(void);
void markAndSweep(void);
void verifyHeap(void);
void printHeap(void);


static inline void rememberedSetAdd(RawObject *object)
{
	ASSERT(_Heap.rememberedSet.end < (_Heap.rememberedSet.objects + 8192 * 16));
	ASSERT((object->tags & TAG_REMEMBERED) == 0);
	object->tags |= TAG_REMEMBERED;
	*_Heap.rememberedSet.end++ = tagPtr(object);
}


static inline void rawObjectStorePtr(RawObject *object, Value *field, RawObject *value)
{
	if (isOldObject(object) && isNewObject(value) && (object->tags & TAG_REMEMBERED) == 0) {
		rememberedSetAdd(object);
	}
	*field = tagPtr(value);
}


static inline void objectStorePtr(Object *object, Value *field, Object *value)
{
	rawObjectStorePtr(object->raw, field, value->raw);
}

#endif
