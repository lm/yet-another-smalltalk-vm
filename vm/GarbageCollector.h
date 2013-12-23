#ifndef GARBAGECOLLECTOR_H
#define GARBAGECOLLECTOR_H

#include "Object.h"
#include "HeapPage.h"

typedef struct {
	size_t count;
	size_t total;
	size_t marked;
	size_t sweeped;
	size_t freed;
	size_t extended;
	int64_t time;
	int64_t totalTime;
} GCStats;

extern GCStats LastGCStats;

void gcMarkRoots(void);
void gcSweep(PageSpace *space);
void resetGcStats(void);
void printGcStats(void);

#endif
