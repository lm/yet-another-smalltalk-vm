#include "HeapPage.h"
#include "CompiledCode.h"
#include "Assert.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define PRINT_PAGE_ALLOC 0


void initPageSpace(PageSpace *pageSpace, size_t size, _Bool executable)
{
	HeapPage *page = mapHeapPage(size, executable);
	pageSpace->pages = pageSpace->pagesTail = page;
	pageSpace->spaces = pageSpace->spacesTail = createInitialFreeSpace(page);
}


void freePageSpace(PageSpace *pageSpace)
{
	HeapPage *page = pageSpace->pages;
	while (page != NULL) {
		HeapPage *next = page->next;
		unmapHeapPage(page);
		page = next;
	}
}


HeapPage *mapHeapPage(size_t size, _Bool executable)
{
	size_t alignedSize = align(size, getpagesize());
	int protection = PROT_READ | PROT_WRITE | (executable ? PROT_EXEC : 0);
	HeapPage *page = mmap(NULL, alignedSize, protection, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (page == MAP_FAILED) {
		FAIL();
	}

	page->next = NULL;
	page->isExecutable = executable;
	page->size = alignedSize;
	page->bodySize = alignedSize - sizeof(*page);
	page->body = (uint8_t *) page + sizeof(*page);
	memset(page->body, executable ? 0xCC : 0, page->bodySize);
	page->bodySize -= page->bodySize % HEAP_OBJECT_ALIGN;
#if PRINT_PAGE_ALLOC
	printf("Page %p %zu%s\n", page, size, executable ? " executable" : "");
#endif
	return page;
}


void unmapHeapPage(HeapPage *page)
{
	if (munmap(page, page->size) == -1) {
		FAIL();
	}
}


_Bool heapPageIncludes(HeapPage *page, uint8_t *addr)
{
	return page->body <= addr && addr < page->body + page->bodySize;
}


FreeSpace *createFreeSpace(uint8_t *p, size_t size)
{
	ASSERT((uintptr_t) p % HEAP_OBJECT_ALIGN == 0);
	ASSERT((uintptr_t) size % HEAP_OBJECT_ALIGN == 0);
	FreeSpace *space = (FreeSpace *) p;
	// TODO: zeroing class here can broke interpreting of next objects in GC sweep
	// memset(p, 0, size);
	space->next = NULL;
	space->size = size;
	space->tags = TAG_FREESPACE;
	return space;
}


FreeSpace *createInitialFreeSpace(HeapPage *page)
{
	uint8_t *start = (uint8_t *) align((uintptr_t) page->body, HEAP_OBJECT_ALIGN);
	return createFreeSpace(start, align(page->bodySize - HEAP_OBJECT_ALIGN / 2, HEAP_OBJECT_ALIGN));
}


void extendFreeSpace(FreeSpace *space, size_t size)
{
	//HeapPage *page1 = pageSpaceFindPage(&_Heap.space, (uint8_t *) space);
	//ASSERT(((uint8_t *) space + space->size + size) <= ((uint8_t *) page1 + page1->size));
	// TODO: zeroing class here can broke interpreting of next objects in GC sweep
	// memset((uint8_t *) space + space->size, 0, size);
	space->size += size;
}


uint8_t *pageSpaceTryAllocate(PageSpace *pageSpace, size_t size)
{
	ASSERT(size % HEAP_OBJECT_ALIGN == 0);

	FreeSpace *freeSpace = pageSpace->spaces;
	FreeSpace *prev = NULL;

	while (freeSpace != NULL) {
		if (freeSpace->size >= size) {
			ASSERT(freeSpace->size % HEAP_OBJECT_ALIGN == 0);
			freeSpace->size -= size;
			ASSERT(freeSpace->size % HEAP_OBJECT_ALIGN == 0);
			if (freeSpace->size < HEAP_OBJECT_ALIGN) {
				if (prev == NULL) {
					pageSpace->spaces = freeSpace->next;
				} else {
					prev->next = freeSpace->next;
				}
				if (freeSpace->next == NULL) {
					pageSpace->spacesTail = prev;
				}
			}
			uint8_t *p = (uint8_t *) freeSpace + freeSpace->size;
			//HeapPage *page = pageSpaceFindPage(pageSpace, p);
			//ASSERT(p + size <= (uint8_t *) page + page->size);
			ASSERT(((intptr_t) p & SPACE_TAG) == OLD_SPACE_TAG);
			ASSERT((intptr_t) freeSpace->size % HEAP_OBJECT_ALIGN == 0);
			return p;
		}
		prev = freeSpace;
		freeSpace = freeSpace->next;
	}

	return NULL;
}


void pageSpaceFree(PageSpace *pageSpace, uint8_t *p, size_t size)
{
	FreeSpace *space = createFreeSpace(p, size);
	if (pageSpace->spaces == NULL) {
		pageSpace->spaces = space;
	} else {
		pageSpace->spaces->next = space;
	}
	pageSpace->spacesTail = space;
}


HeapPage *pageSpaceFindPage(PageSpace *pageSpace, uint8_t *addr)
{
	HeapPage *page = pageSpace->pages;

	while (page != NULL) {
		if (page->body <= addr && addr < page->body + page->bodySize) {
			return page;
		}
		page = page->next;
	}
	return NULL;
}


_Bool pageSpaceIncludes(PageSpace *PageSpace, uint8_t *addr)
{
	HeapPage *page = PageSpace->pages;

	while (page != NULL) {
		if (page->body <= addr && addr < page->body + page->bodySize) {
			return 1;
		}
		page = page->next;
	}
	return 0;
}


void pageSpaceIteratorInit(PageSpaceIterator *iterator, PageSpace *space)
{
	iterator->page = space->pages;
	iterator->current = (FreeSpace *) align((uintptr_t) iterator->page->body, HEAP_OBJECT_ALIGN);
}


RawObject *pageSpaceIteratorNext(PageSpaceIterator *iterator)
{
	FreeSpace *object = iterator->current;
	if ((uint8_t *) object >= iterator->page->body + iterator->page->bodySize) {
		if (iterator->page->next == NULL) {
			return NULL;
		}
		iterator->page = iterator->page->next;
		object = (FreeSpace *) align((uintptr_t) iterator->page->body, HEAP_OBJECT_ALIGN);
	}

	size_t size;
	if (object->tags & TAG_FREESPACE) {
		size = object->size;
	} else if (iterator->page->isExecutable) {
		size = computeNativeCodeSize((NativeCode *) object);
	} else {
		size = computeRawObjectSize((RawObject *) object);
	}
	iterator->current = (FreeSpace *) ((uint8_t *) object + align(size, HEAP_OBJECT_ALIGN));
	return (RawObject *) object;
}
