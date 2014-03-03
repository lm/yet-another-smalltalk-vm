#include "Heap.h"
#include "Smalltalk.h"
#include "GarbageCollector.h"
#include "Assert.h"
#include "Os.h"
#include "CompiledCode.h"
#include "String.h"
#include <string.h>

#define KB 1024
#define MB (1024 * 1024)

#define SCAVENGE_EVERY_ALLOC 0
#define VERIFY_HEAP_AFTER_GC 0

static void nilVars(Value *vars, size_t count);
static uint8_t *pageSpaceAllocate(PageSpace *pageSpace, size_t size);
static void emptyRememberedSet(void);
static void verifyObject(Heap *heap, RawObject *object);
static void verifyPointer(Heap *heap, RawObject *object);
static void printHeapPage(HeapPage *page);
static void printFreeSpace(FreeSpace *freeSpace);
static void printPageSpace(PageSpace *space);


void initHeap(Heap *heap, struct Thread *thread)
{
	heap->thread = thread;
	initScavenger(&heap->newSpace, heap, 32 * MB);
	initPageSpace(&heap->oldSpace, 256 * KB, 0);
	initPageSpace(&heap->execSpace, 256 * KB, 1);
	initRememberedSet(&heap->rememberedSet);
}


void freeHeap(Heap *heap)
{
	freeScavenger(&heap->newSpace);
	freePageSpace(&heap->oldSpace);
	freePageSpace(&heap->execSpace);
}


RawObject *allocateObject(Heap *heap, RawClass *class, size_t size)
{
	HandleScope scope;
	openHandleScope(&scope);

	InstanceShape shape = class->instanceShape;
	size_t realSize = computeInstanceSize(class->instanceShape, size);
	Class *classHandle = scopeHandle(class);
#if SCAVENGE_EVERY_ALLOC
	scavengerScavenge(&heap->newSpace);
#endif
	RawObject *object = (RawObject *) allocate(heap, realSize);

	object->class = classHandle->raw;
	object->hash = (Value) object >> 2; // XXX: replace with random hash generator
	object->payloadSize = shape.payloadSize;
	object->varsSize = shape.varsSize;
	object->tags = 0;

	if (shape.isIndexed) {
		((RawIndexedObject *) object)->size = size;
		memset(((RawIndexedObject *) object)->body, 0, shape.payloadSize * sizeof(Value));
	} else {
		memset(object->body, 0, shape.payloadSize * sizeof(Value));
	}
	if (shape.isBytes) {
		nilVars(getRawObjectVars(object), class->instanceShape.varsSize);
		memset(getRawObjectIndexedVars(object), 0, size);
	} else {
		nilVars(getRawObjectVars(object), class->instanceShape.varsSize + size);
	}
	closeHandleScope(&scope, NULL);
	return object;
}


static void nilVars(Value *vars, size_t count)
{
	Value nil = getTaggedPtr(Handles.nil);
	Value *var = vars;
	Value *end = vars + count;

	for (; var < end; var++) {
		*var = nil;
	}
}


void freeObject(PageSpace *space, RawObject *object)
{
	FreeSpace *freeSpace = createFreeSpace((uint8_t *) object, align(computeRawObjectSize(object), HEAP_OBJECT_ALIGN));
	freeListAddFreeSpace(&space->freeList, freeSpace);
}


NativeCode *allocateNativeCode(Heap *heap, size_t size, size_t pointersOffsetsSize)
{
	size_t realSize = align(sizeof(NativeCode) + size + pointersOffsetsSize * sizeof(uint16_t), HEAP_OBJECT_ALIGN);
	NativeCode *code = (NativeCode *) pageSpaceAllocate(&heap->execSpace, realSize);
	code->size = size;
	code->pointersOffsetsSize = pointersOffsetsSize;
	code->tags = 0;
	return code;
}


static uint8_t *pageSpaceAllocate(PageSpace *pageSpace, size_t size)
{
	uint8_t *p = pageSpaceTryAllocate(pageSpace, size);
	if (p == NULL) {
		HeapPage *page = mapHeapPage(256 * KB, pageSpace->pagesTail->isExecutable);
		pageSpace->pagesTail->next = page;
		pageSpace->pagesTail = page;
		expandFreeList(&pageSpace->freeList, page);
		p = pageSpaceTryAllocate(pageSpace, size);
		ASSERT(p != NULL);
		return p;
	}
	return p;
}


uint8_t *allocate(Heap *heap, size_t size)
{
	size_t realSize = align(size, HEAP_OBJECT_ALIGN);
	uint8_t *p = scavengerTryAllocate(&heap->newSpace, realSize);
	if (p == NULL) {
		scavengerScavenge(&heap->newSpace);
		if (heap->newSpace.hasPromotionFailure) {
			markAndSweep(&CurrentThread);
		}
		p = scavengerTryAllocate(&heap->newSpace, realSize);
	}
	if (p == NULL) {
		p = tryAllocateOld(heap, realSize, 1);
	}
	return p;
}


uint8_t *tryAllocateOld(Heap *heap, size_t size, _Bool grow)
{
	size_t realSize = align(size, HEAP_OBJECT_ALIGN);
	uint8_t *p = pageSpaceTryAllocate(&heap->oldSpace, realSize);
	if (p == NULL && grow) {
		p = pageSpaceAllocate(&heap->oldSpace, realSize);
	}
	ASSERT(p == NULL || isOldObject((RawObject *) p));
	return p;
}


void collectGarbage(Thread *thread)
{
	scavengerScavenge(&thread->heap.newSpace);
	markAndSweep(thread);
}


void markAndSweep(Thread *thread)
{
	resetGcStats();
	LastGCStats.count++;
	int64_t startTime = osCurrentMicroTime();

	rememberedSetReset(&thread->heap.rememberedSet);
	gcMarkRoots(thread);
	gcSweep(&thread->heap.oldSpace);

	LastGCStats.time = osCurrentMicroTime() - startTime;
	LastGCStats.totalTime += LastGCStats.time;

#if VERIFY_HEAP_AFTER_GC
	verifyHeap(&thread->heap);
#endif
}


void verifyHeap(Heap *heap)
{
	RawObject *object = (RawObject *) ((uintptr_t) heap->newSpace.fromSpace | NEW_SPACE_TAG);

	while ((uint8_t *) object < heap->newSpace.top) {
		verifyObject(heap, object);
		object = (RawObject *) ((uint8_t *) object + align(computeRawObjectSize(object), HEAP_OBJECT_ALIGN));
	}

	PageSpaceIterator iterator;
	pageSpaceIteratorInit(&iterator, &heap->oldSpace);
	object = pageSpaceIteratorNext(&iterator);

	while (object != NULL) {
		if ((object->tags & TAG_FREESPACE) == 0) {
			verifyObject(heap, object);
		}
		object = pageSpaceIteratorNext(&iterator);
	}
}


static void verifyObject(Heap *heap, RawObject *object)
{
	verifyPointer(heap, (RawObject *) object->class);

	Value *vars = getRawObjectVars(object);
	size_t size = object->class->instanceShape.varsSize;
	if (object->class->instanceShape.isIndexed && !object->class->instanceShape.isBytes) {
		size += rawObjectSize(object);
	}

	for (size_t i = 0; i < size; i++) {
		if (valueTypeOf(vars[i], VALUE_POINTER)) {
			verifyPointer(heap, asObject(vars[i]));
		}
	}
}


static void verifyPointer(Heap *heap, RawObject *object)
{
	if (scavengerIncludes(&heap->newSpace, (uint8_t *) object)) {
		return;
	}
	if (pageSpaceIncludes(&heap->oldSpace, (uint8_t *) object)) {
		return;
	}
	FAIL();
}


void printHeap(Heap *heap)
{
	printf("Scavenger\n\t");
	printHeapPage(heap->newSpace.page);
	printf("\tfree space: %zi\n", heap->newSpace.top - heap->newSpace.fromSpace);

	printf("Old space\n");
	printPageSpace(&heap->oldSpace);

	printf("Executable space\n");
	printPageSpace(&heap->execSpace);
}


static void printHeapPage(HeapPage *page)
{
	printf("page %p size %zi%s\n", page, page->size, page->isExecutable ? " executable" : "");
}


static void printFreeSpace(FreeSpace *freeSpace)
{
	printf("free space %p size %zi\n", freeSpace, freeSpace->size);
}


static void printPageSpace(PageSpace *space)
{
	/*for (HeapPage *page = space->pages; page != NULL; page = page->next) {
		printf("\t");
		printHeapPage(page);
	}
	for (FreeSpace *freeSpace = space->spaces; freeSpace != NULL; freeSpace = freeSpace->next) {
		printf("\t");
		printFreeSpace(freeSpace);
	}*/
}
