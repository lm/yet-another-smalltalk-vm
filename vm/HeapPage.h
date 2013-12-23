#ifndef PAGE_H
#define PAGE_H

#include "Object.h"
#include <stddef.h>
#include <stdint.h>

typedef struct HeapPage {
	struct HeapPage *next;
	_Bool isExecutable;
	size_t size;
	size_t bodySize;
	uint8_t *body;
} HeapPage;

typedef struct FreeSpace {
	struct FreeSpace *next;
	uintptr_t size:56;
	uint8_t tags;
} FreeSpace;

typedef struct {
	HeapPage *pages;
	HeapPage *pagesTail;
	FreeSpace *spaces;
	FreeSpace *spacesTail;
} PageSpace;

typedef struct {
	HeapPage *page;
	FreeSpace *current;
} PageSpaceIterator;

void initPageSpace(PageSpace *pageSpace, size_t size, _Bool executable);
void freePageSpace(PageSpace *pageSpace);
HeapPage *mapHeapPage(size_t size, _Bool executable);
void unmapHeapPage(HeapPage *page);
_Bool heapPageIncludes(HeapPage *page, uint8_t *addr);
FreeSpace *createFreeSpace(uint8_t *p, size_t size);
FreeSpace *createInitialFreeSpace(HeapPage *page);
void extendFreeSpace(FreeSpace *space, size_t size);
uint8_t *pageSpaceTryAllocate(PageSpace *pageSpace, size_t size);
void pageSpaceFree(PageSpace *pageSpace, uint8_t *p, size_t size);
HeapPage *pageSpaceFindPage(PageSpace *PageSpace, uint8_t *addr);
_Bool pageSpaceIncludes(PageSpace *PageSpace, uint8_t *addr);
void pageSpaceIteratorInit(PageSpaceIterator *iterator, PageSpace *space);
RawObject *pageSpaceIteratorNext(PageSpaceIterator *iterator);


static uintptr_t align(uintptr_t v, size_t align)
{
	return (v + (align - 1)) & -align;
}

#endif
