#include "FreeList.h"
#include "Object.h"
#include "HeapPage.h"
#include "Heap.h"
#include <string.h>
#include <stdio.h>

static ptrdiff_t indexForSize(size_t size);
static FreeSpace *popFreeSpace(FreeList *freeList, ptrdiff_t index);
static FreeSpace *popAndSplitFreeSpace(FreeList *freeList, ptrdiff_t index, size_t size);
static FreeSpace *splitFreeSpace(FreeList *freeList, FreeSpace *freeSpace, size_t size);
static ptrdiff_t freeMapNext(uint8_t *freeMap, ptrdiff_t index);
static FreeSpace *createInitialFreeSpace(struct HeapPage *page);
static void freeSpacePrint(FreeSpace *freeSpace);


void initFreeList(FreeList *freeList, struct HeapPage *page)
{
	for (size_t i = 0; i < FREE_LIST_SIZE; i++) {
		freeList->freeSpaces[i] = NULL;
	}
	freeList->freeSpaces[FREE_LIST_SIZE] = createInitialFreeSpace(page);
	memset(freeList->freeMap, 0, sizeof(freeList->freeMap));
#if FREE_LIST_COLLECT_STATS
	freeList->stats.exactAllocs = 0;
	freeList->stats.nextAllocs = 0;
	freeList->stats.fallbackAllocs = 0;
	freeList->stats.averageSize = 0;
	freeList->stats.addedFreeSpaces = 0;
	freeList->stats.averageAddedSpaceSize = 0;
	freeList->stats.expanded = 0;
#endif
}


void expandFreeList(FreeList *freeList, struct HeapPage *page)
{
#if FREE_LIST_COLLECT_STATS
	freeList->stats.expanded++;
#endif
	freeListAddFreeSpace(freeList, createInitialFreeSpace(page));
}


uint8_t *freeListTryAllocate(FreeList *freeList, size_t size)
{
#if FREE_LIST_COLLECT_STATS
	freeList->stats.averageSize = freeList->stats.averageSize == 0
		? size
		: ((freeList->stats.averageSize + size) / 2);
#endif

	ptrdiff_t index = indexForSize(size);
	if (index != FREE_LIST_SIZE && (freeList->freeMap[index / 8] & (1 << (index % 8))) != 0) {
#if FREE_LIST_COLLECT_STATS
		freeList->stats.exactAllocs++;
#endif
		return (uint8_t *) popFreeSpace(freeList, index);
	}

	if ((index + 1) < FREE_LIST_SIZE) {
		ptrdiff_t nextIndex = freeMapNext(freeList->freeMap, index + 1);
		if (nextIndex != -1) {
			ASSERT(nextIndex > index);
#if FREE_LIST_COLLECT_STATS
			freeList->stats.nextAllocs++;
#endif
			return (uint8_t *) popAndSplitFreeSpace(freeList, nextIndex, size);
		}
	}

	FreeSpace *freeSpace = freeList->freeSpaces[FREE_LIST_SIZE];
	while (freeSpace != NULL) {
		if (freeSpace->size >= size) {
#if FREE_LIST_COLLECT_STATS
			freeList->stats.fallbackAllocs++;
#endif
			return (uint8_t *) popAndSplitFreeSpace(freeList, FREE_LIST_SIZE, size);
		}
		freeSpace = freeSpace->next;
	}

	return NULL;
}


static ptrdiff_t indexForSize(size_t size)
{
	ptrdiff_t index = size / HEAP_OBJECT_ALIGN;
	if (index > FREE_LIST_SIZE) {
		index = FREE_LIST_SIZE;
	}
	return index;
}


void freeListAddFreeSpace(FreeList *freeList, FreeSpace *freeSpace)
{
	//ASSERT(pageSpaceIncludes(&_Heap.oldSpace, (uint8_t *) freeSpace)
	//	|| pageSpaceIncludes(&_Heap.execSpace, (uint8_t *) freeSpace));
	ptrdiff_t index = indexForSize(freeSpace->size);
	freeSpace->next = freeList->freeSpaces[index];
	freeList->freeSpaces[index] = freeSpace;
	freeList->freeMap[index / 8] |= 1 << (index % 8);
#if FREE_LIST_COLLECT_STATS
	if (index != FREE_LIST_SIZE) {
		freeList->stats.addedFreeSpaces++;
		freeList->stats.averageAddedSpaceSize = freeList->stats.averageAddedSpaceSize == 0
			? freeSpace->size
			: ((freeList->stats.averageAddedSpaceSize + freeSpace->size) / 2);
	}
#endif
}


static FreeSpace *popFreeSpace(FreeList *freeList, ptrdiff_t index)
{
	FreeSpace *result = freeList->freeSpaces[index];
	FreeSpace *next = result->next;
	if (next == NULL) {
		freeList->freeMap[index / 8] &= ~(1 << (index % 8));
	}
	freeList->freeSpaces[index] = next;
	return result;
}


static FreeSpace *popAndSplitFreeSpace(FreeList *freeList, ptrdiff_t index, size_t size)
{
	FreeSpace *freeSpace = popFreeSpace(freeList, index);
	return splitFreeSpace(freeList, freeSpace, size);
}


static FreeSpace *splitFreeSpace(FreeList *freeList, FreeSpace *freeSpace, size_t size)
{
	ASSERT(freeSpace->size > size);
	//ASSERT(pageSpaceIncludes(&_Heap.oldSpace, (uint8_t *) freeSpace)
	//	|| pageSpaceIncludes(&_Heap.execSpace, (uint8_t *) freeSpace));
	size_t newSize = freeSpace->size - size;
	FreeSpace *newFreeSpace = createFreeSpace((uint8_t *) freeSpace + size, newSize);
	freeListAddFreeSpace(freeList, newFreeSpace);
	return freeSpace;
}


static ptrdiff_t freeMapNext(uint8_t *freeMap, ptrdiff_t index)
{
	ASSERT(index < FREE_LIST_SIZE);
	uint8_t element = index / 8;
	uint8_t v = freeMap[element] & ~((1 << index) - 1);

	if (v != 0) {
		ASSERT((element * 8 + __builtin_ctz(v)) < FREE_LIST_SIZE);
		return element * 8 + __builtin_ctz(v);
	}

	do {
		element++;
		v = freeMap[element];
		if (v != 0) {
			ptrdiff_t nextIndex = element * 8 + __builtin_ctz(v);
			return nextIndex == FREE_LIST_SIZE ? -1 : nextIndex;
		}
	} while (element < FREE_MAP_SIZE);

	return -1;
}


FreeSpace *createFreeSpace(uint8_t *p, size_t size)
{
	ASSERT((uintptr_t) p % HEAP_OBJECT_ALIGN == 0);
	ASSERT((uintptr_t) size % HEAP_OBJECT_ALIGN == 0);
	ASSERT((uintptr_t) size >= HEAP_OBJECT_ALIGN);
	FreeSpace *space = (FreeSpace *) p;
	space->next = NULL;
	space->size = size;
	space->tags = TAG_FREESPACE;
	return space;
}


static FreeSpace *createInitialFreeSpace(struct HeapPage *page)
{
	uint8_t *start = (uint8_t *) align((uintptr_t) page->body, HEAP_OBJECT_ALIGN);
	return createFreeSpace(start, align(page->bodySize - HEAP_OBJECT_ALIGN / 2, HEAP_OBJECT_ALIGN));
}


void freeListPrint(FreeList *freeList)
{
#if FREE_LIST_COLLECT_STATS
	printf(
		"Free list #%p (alloc exact: %zu next: %zu fallback: %zu average size: %zu B, added spaces: %zu average size: %zu B, expanded: %zu)\n",
		freeList,
		freeList->stats.exactAllocs,
		freeList->stats.nextAllocs,
		freeList->stats.fallbackAllocs,
		freeList->stats.averageSize,
		freeList->stats.addedFreeSpaces,
		freeList->stats.averageAddedSpaceSize,
		freeList->stats.expanded
	);
#else
	printf("Free list #%p\n", freeList);
#endif

	for (size_t i = 0; i <= FREE_LIST_SIZE; i++) {
		_Bool hasSpace = (freeList->freeMap[i / 8] & (1 << (i % 8))) != 0;
		if (hasSpace) {
			printf("%zu.%s\n", i, hasSpace ? " [has space]" : "");
			freeSpacePrint(freeList->freeSpaces[i]);
		}
	}
}


static void freeSpacePrint(FreeSpace *freeSpace)
{
	if (freeSpace == NULL) return;

	ASSERT((freeSpace->tags & TAG_FREESPACE) != 0);

	size_t count = 1;
	FreeSpace *next = freeSpace->next;
	while (next != NULL && next->size == freeSpace->size) {
		ASSERT((next->tags & TAG_FREESPACE) != 0);
		count++;
		next = next->next;
	}

	if (count > 1) {
		printf("\t%zu B * %zu\n", freeSpace->size, count);
	} else {
		printf("\t%zu B\n", freeSpace->size);
	}
	freeSpacePrint(next);
}
