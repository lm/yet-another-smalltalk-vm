#ifndef SCAVENGER_H
#define SCAVENGER_H

#include "HeapPage.h"

struct Heap;

typedef struct {
	struct Heap *heap;
	HeapPage *page;
	size_t size;
	_Bool hasPromotionFailure;
	uint8_t *fromSpace;
	uint8_t *toSpace;
	uint8_t *top;
	uint8_t *end;
	uint8_t *survivorEnd;
} Scavenger;

void initScavenger(Scavenger *scavenger, struct Heap *heap, size_t size);
void freeScavenger(Scavenger *scavenger);
uint8_t *scavengerTryAllocate(Scavenger *scavenger, size_t size);
void scavengerScavenge(Scavenger *scavenger);
_Bool scavengerIncludes(Scavenger *scavenger, uint8_t *addr);

#endif
