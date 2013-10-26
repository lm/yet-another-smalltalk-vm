#ifndef FREE_LIST_H
#define FREE_LIST_H

#include <stddef.h>
#include <stdint.h>

#define FREE_LIST_SIZE 128
#define FREE_MAP_SIZE (FREE_LIST_SIZE / 8 + 1)
#define FREE_LIST_COLLECT_STATS 0

typedef struct FreeSpace {
	struct FreeSpace *next;
	uintptr_t size:56;
	uint8_t tags;
} FreeSpace;

typedef struct {
	FreeSpace *freeSpaces[FREE_LIST_SIZE + 1];
	uint8_t freeMap[FREE_MAP_SIZE];
#if FREE_LIST_COLLECT_STATS
	struct {
		size_t exactAllocs;
		size_t nextAllocs;
		size_t fallbackAllocs;
		size_t averageSize;
		size_t addedFreeSpaces;
		size_t averageAddedSpaceSize;
		size_t expanded;
	} stats;
#endif
} FreeList;

struct HeapPage;

void initFreeList(FreeList *freeList, struct HeapPage *page);
void expandFreeList(FreeList *freeList, struct HeapPage *page);
FreeSpace *createFreeSpace(uint8_t *p, size_t size);
void freeListAddFreeSpace(FreeList *freeList, FreeSpace *freeSpace);
uint8_t *freeListTryAllocate(FreeList *freeList, size_t size);
void freeListPrint(FreeList *freeList);

#endif
