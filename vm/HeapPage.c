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
	initFreeList(&pageSpace->freeList, page);
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


uint8_t *pageSpaceTryAllocate(PageSpace *pageSpace, size_t size)
{
	ASSERT(size % HEAP_OBJECT_ALIGN == 0);
	uint8_t *p = freeListTryAllocate(&pageSpace->freeList, size);
	ASSERT(p == NULL || pageSpaceIncludes(pageSpace, p));
	return p;
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
