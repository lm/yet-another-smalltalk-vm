#ifndef HEAP_H
#define HEAP_H

#include "Object.h"
#include "HeapPage.h"
#include "Scavenger.h"
#include "RememberedSet.h"

struct Thread;
struct NativeCode;

typedef struct Heap {
	struct Thread *thread;
	Scavenger newSpace;
	PageSpace oldSpace;
	PageSpace execSpace;
	RememberedSet rememberedSet;
} Heap;

void initHeap(Heap *heap, struct Thread *thread);
void freeHeap(Heap *heap);
RawObject *allocateObject(Heap *heap, RawClass *class, size_t size);
void freeObject(PageSpace *space, RawObject *object);
struct NativeCode *allocateNativeCode(Heap *heap, size_t size, size_t pointersOffsetsSize);
uint8_t *allocate(Heap *heap, size_t size);
uint8_t *tryAllocateOld(Heap *heap, size_t size, _Bool grow);
void collectGarbage(struct Thread *thread);
void markAndSweep(struct Thread *thread);
void verifyHeap(Heap *heap);
void printHeap(Heap *heap);

#endif
