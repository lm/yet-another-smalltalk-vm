#include "Heap.h"
#include "Smalltalk.h"
#include "GarbageCollector.h"
#include "Assert.h"
#include "Os.h"
#include "CompiledCode.h"
#include <string.h>

#define KB 1024
#define MB (1024 * 1024)

#define SCAVENGE_EVERY_ALLOC 0

Heap _Heap = { .rememberedSet = { 0 } };

static void nilVars(Value *vars, size_t count);
static uint8_t *pageSpaceAllocate(PageSpace *pageSpace, size_t size);
static void emptyRememberedSet(void);
static void verifyObject(RawObject *object);
static void verifyPointer(RawObject *object);
static void printHeapPage(HeapPage *page);
static void printFreeSpace(FreeSpace *freeSpace);
static void printPageSpace(PageSpace *space);


void initHeap(void)
{
	initScavenger(&_Heap.newSpace, 32 * MB);
	initPageSpace(&_Heap.oldSpace, 256 * KB, 0);
	initPageSpace(&_Heap.execSpace, 256 * KB, 1);
	_Heap.rememberedSet.end = _Heap.rememberedSet.objects;
}


void freeHeap(void)
{
	freeScavenger(&_Heap.newSpace);
	freePageSpace(&_Heap.oldSpace);
	freePageSpace(&_Heap.execSpace);
}


RawObject *allocateObject(RawClass *class, size_t size)
{
	HandleScope scope;
	openHandleScope(&scope);

	InstanceShape shape = class->instanceShape;
	size_t realSize = computeInstanceSize(class->instanceShape, size);
	Class *classHandle = scopeHandle(class);
#if SCAVENGE_EVERY_ALLOC
	scavengerScavenge(&_Heap.newSpace);
#endif
	RawObject *object = (RawObject *) allocate(realSize);

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


void freeObject(RawObject *object)
{
	pageSpaceFree(&_Heap.oldSpace, (uint8_t *) object, align(computeRawObjectSize(object), HEAP_OBJECT_ALIGN));
}


NativeCode *allocateNativeCode(size_t size, size_t pointersOffsetsSize)
{
	size_t realSize = align(sizeof(NativeCode) + size + pointersOffsetsSize * sizeof(uint16_t), HEAP_OBJECT_ALIGN);
	NativeCode *code = (NativeCode *) pageSpaceAllocate(&_Heap.execSpace, realSize);
	code->size = size;
	code->pointersOffsetsSize = pointersOffsetsSize;
	code->tags = 0;
	return code;
}


static uint8_t *pageSpaceAllocate(PageSpace *pageSpace, size_t size)
{
	uint8_t *p = pageSpaceTryAllocate(pageSpace, size);
	if (p == NULL) {
		HeapPage *page = mapHeapPage(256 * 1024, pageSpace->pagesTail->isExecutable);
		FreeSpace *freeSpace = createInitialFreeSpace(page);
		ASSERT(freeSpace->size >= size);

		pageSpace->pagesTail->next = page;
		pageSpace->pagesTail = page;
		if (pageSpace->spaces == NULL) {
			pageSpace->spaces = freeSpace;
		} else {
			pageSpace->spaces->next = freeSpace;
		}
		pageSpace->spacesTail = freeSpace;

		freeSpace->size -= size;
		ASSERT(((intptr_t) p & SPACE_TAG) == OLD_SPACE_TAG);
		p = (uint8_t *) freeSpace + freeSpace->size;
	}
	return p;
}


uint8_t *allocate(size_t size)
{
	size_t realSize = align(size, HEAP_OBJECT_ALIGN);
	uint8_t *p = scavengerTryAllocate(&_Heap.newSpace, realSize);
	if (p == NULL) {
		scavengerScavenge(&_Heap.newSpace);
		if (_Heap.newSpace.hasPromotionFailure) {
			markAndSweep();
		}
		p = scavengerTryAllocate(&_Heap.newSpace, realSize);
	}
	if (p == NULL) {
		p = tryAllocateOld(realSize, 1);
	}
	return p;
}


uint8_t *tryAllocateOld(size_t size, _Bool grow)
{
	size_t realSize = align(size, HEAP_OBJECT_ALIGN);
	uint8_t *p = pageSpaceTryAllocate(&_Heap.oldSpace, realSize);
	if (p == NULL && grow) {
		p = pageSpaceAllocate(&_Heap.oldSpace, realSize);
	}
	return p;
}


void collectGarbage(void)
{
	scavengerScavenge(&_Heap.newSpace);
	markAndSweep();
}


void markAndSweep(void)
{
	resetGcStats();
	LastGCStats.count++;
	int64_t startTime = osCurrentMicroTime();

	emptyRememberedSet();
	gcMarkRoots();
	gcSweep(&_Heap.oldSpace);

	LastGCStats.time = osCurrentMicroTime() - startTime;
	LastGCStats.totalTime += LastGCStats.time;

#if VERIFY_HEAP_AFTER_GC
	verifyHeap();
#endif
}


static void emptyRememberedSet(void)
{
	_Heap.rememberedSet.end = _Heap.rememberedSet.objects;
}


void verifyHeap(void)
{
	RawObject *object = (RawObject *) ((uintptr_t) _Heap.newSpace.fromSpace | NEW_SPACE_TAG);

	while ((uint8_t *) object < _Heap.newSpace.top) {
		verifyObject(object);
		object = (RawObject *) ((uint8_t *) object + align(computeRawObjectSize(object), HEAP_OBJECT_ALIGN));
	}

	PageSpaceIterator iterator;
	pageSpaceIteratorInit(&iterator, &_Heap.oldSpace);
	object = pageSpaceIteratorNext(&iterator);

	while (object != NULL) {
		if ((object->tags & TAG_FREESPACE) == 0) {
			verifyObject(object);
		}
		object = pageSpaceIteratorNext(&iterator);
	}
}


static void verifyObject(RawObject *object)
{
	verifyPointer((RawObject *) object->class);

	Value *vars = getRawObjectVars(object);
	size_t size = object->class->instanceShape.varsSize;
	if (object->class->instanceShape.isIndexed && !object->class->instanceShape.isBytes) {
		size += rawObjectSize(object);
	}

	for (size_t i = 0; i < size; i++) {
		if (valueTypeOf(vars[i], VALUE_POINTER)) {
			verifyPointer(asObject(vars[i]));
		}
	}
}


static void verifyPointer(RawObject *object)
{
	if (scavengerIncludes(&_Heap.newSpace, (uint8_t *) object)) {
		return;
	}
	if (pageSpaceIncludes(&_Heap.oldSpace, (uint8_t *) object)) {
		return;
	}
	FAIL();
}


void printHeap(void)
{
	printf("Scavenger\n\t");
	printHeapPage(_Heap.newSpace.page);
	printf("\tfree space: %zi\n", _Heap.newSpace.top - _Heap.newSpace.fromSpace);

	printf("Old space\n");
	printPageSpace(&_Heap.oldSpace);

	printf("Executable space\n");
	printPageSpace(&_Heap.execSpace);
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
	for (HeapPage *page = space->pages; page != NULL; page = page->next) {
		printf("\t");
		printHeapPage(page);
	}
	for (FreeSpace *freeSpace = space->spaces; freeSpace != NULL; freeSpace = freeSpace->next) {
		printf("\t");
		printFreeSpace(freeSpace);
	}
}
