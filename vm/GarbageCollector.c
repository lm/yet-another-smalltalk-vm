#include "GarbageCollector.h"
#include "Heap.h"
#include "Object.h"
#include "Handle.h"
#include "Class.h"
#include "CodeDescriptors.h"
#include "Entry.h"
#include "StackFrame.h"
#include "Thread.h"
#include "Assert.h"
#include <stdio.h>
#include <inttypes.h>

#define QUEUE_INIT_SIZE 1024

typedef struct {
	size_t size;
	RawObject **objects;
	ptrdiff_t index;
} MarkingQueue;

static void iterateStack(MarkingQueue *queue, Thread *thread);
static void iterateHandles(MarkingQueue *queue, Thread *thread);
static void iterateNativeCode(MarkingQueue *queue, Thread *thread);
static void iterateObject(MarkingQueue *queue, Thread *thread, RawObject *root);
static void markObject(MarkingQueue *queue, Thread *thread, RawObject *object);
static void markingQueueAdd(MarkingQueue *queue, RawObject *object);
static _Bool markingQueueIsEmpty(MarkingQueue *queue);
static RawObject *markingQueuePop(MarkingQueue *queue);
static _Bool hasFinalizer(RawObject *object);

GCStats LastGCStats = { 0 };


void gcMarkRoots(Thread *thread)
{
	MarkingQueue queue = {
		.size = QUEUE_INIT_SIZE,
		.objects = malloc(QUEUE_INIT_SIZE * sizeof(RawObject *)),
		.index = 0,
	};
	iterateStack(&queue, thread);
	iterateHandles(&queue, thread);
	iterateNativeCode(&queue, thread);

	while (!markingQueueIsEmpty(&queue)) {
		iterateObject(&queue, thread, markingQueuePop(&queue));
	}

	free(queue.objects);
}


static void iterateStack(MarkingQueue *queue, Thread *thread)
{
	EntryStackFrame *entryFrame = thread->stackFramesTail;
	while (entryFrame != NULL) {
		StackFrame *prev = entryFrame->exit;
		StackFrame *frame = stackFrameGetParent(prev, entryFrame);

		Value value = stackFrameGetSlot	(prev, 0);
		if (valueTypeOf(value, VALUE_POINTER)) {
			markObject(queue, thread, asObject(value));
		}

		while (frame != NULL) {
			NativeCode *code = stackFrameGetNativeCode(frame);
			size_t argsSize = code->argsSize + 1;
			for (ptrdiff_t i = 0; i < argsSize; i++) {
				Value value = stackFrameGetArg(frame, i);
				if (valueTypeOf(value, VALUE_POINTER)) {
					markObject(queue, thread, asObject(value));
				}
			}

			RawStackmap *stackmap = findStackmap(code, (ptrdiff_t) prev->parentIc);
			ASSERT(stackmap != NULL);
			size_t frameSize = (stackmap->size - sizeof(Value)) * 8;
			for (size_t i = 0; i < frameSize; i++) {
				if (stackmapIncludes(stackmap, i)) {
					Value value = stackFrameGetSlot(frame, i);
					if (valueTypeOf(value, VALUE_POINTER)) {
						markObject(queue, thread, asObject(value));
					}
				}
			}

			prev = frame;
			frame = stackFrameGetParent(frame, entryFrame);
		}
		entryFrame = entryFrame->prev;
	}
}


static void iterateHandles(MarkingQueue *queue, Thread *thread)
{
	HandlesIterator handlesIterator;
	initHandlesIterator(&handlesIterator, thread->handles);
	while (handlesIteratorHasNext(&handlesIterator)) {
		markObject(queue, thread, handlesIteratorNext(&handlesIterator)->raw);
	}

	HandleScopeIterator handleScopeIterator;
	initHandleScopeIterator(&handleScopeIterator, thread->handleScopes);
	while (handleScopeIteratorHasNext(&handleScopeIterator)) {
		HandleScope *scope = handleScopeIteratorNext(&handleScopeIterator);
		for (ptrdiff_t i = 0; i < scope->size; i++) {
			markObject(queue, thread, scope->handles[i].raw);
		}
	}

	if (CurrentThread.context != 0) {
		markObject(queue, thread, asObject(CurrentThread.context));
	}
}


static void iterateNativeCode(MarkingQueue *queue, Thread *thread)
{
	PageSpaceIterator iterator;
	pageSpaceIteratorInit(&iterator, &thread->heap.execSpace);
	NativeCode *code = (NativeCode *) pageSpaceIteratorNext(&iterator);

	while (code != NULL) {
		if ((code->tags & TAG_FREESPACE) == 0) {
			if (code->compiledCode != NULL) {
				markObject(queue, thread, (RawObject *) code->compiledCode);
			}
			if (code->stackmaps != NULL) {
				markObject(queue, thread, (RawObject *) code->stackmaps);
			}
			if (code->descriptors != NULL) {
				markObject(queue, thread, (RawObject *) code->descriptors);
			}
			if (code->typeFeedback != NULL) {
				markObject(queue, thread, (RawObject *) code->typeFeedback);
			}
			for (size_t i = 0; i < code->pointersOffsetsSize; i++) {
				uint16_t offset = ((uint16_t *) (code->insts + code->size))[i];
				Value value = *(Value *) (code->insts + offset);
				if (valueTypeOf(value, VALUE_POINTER)) {
					markObject(queue, thread, asObject(value));
				} else {
					markObject(queue, thread, (RawObject *) value);
				}
			}
		}
		code = (NativeCode *) pageSpaceIteratorNext(&iterator);
	}
}


static void iterateObject(MarkingQueue *queue, Thread *thread, RawObject *root)
{
	_Bool remember = isNewObject((RawObject *) root->class);
	markObject(queue, thread, (RawObject *) root->class);

	Value *vars = getRawObjectVars(root);
	size_t size = root->class->instanceShape.varsSize;
	if (root->class->instanceShape.isIndexed && !root->class->instanceShape.isBytes) {
		size += rawObjectSize(root);
	}

	for (size_t i = 0; i < size; i++) {
		if (valueTypeOf(vars[i], VALUE_POINTER)) {
			RawObject *object = asObject(vars[i]);
			remember = remember || isNewObject(object);
			markObject(queue, thread, object);
		}
	}

	root->tags = root->tags & ~TAG_REMEMBERED;
	if (remember && isOldObject(root)) {
		rememberedSetAdd(&thread->heap.rememberedSet, root);
	}
}


static void markObject(MarkingQueue *queue, Thread *thread, RawObject *object)
{
	ASSERT(isOldObject(object) || (thread->heap.newSpace.fromSpace <= (uint8_t *) object && (uint8_t *) object <= (thread->heap.newSpace.fromSpace + thread->heap.newSpace.size)));
	if (object->tags & TAG_MARKED) {
		return;
	}
	// TODO: for now scavenge has to be called before mark&sweep to clear
	// marked tag of new objects
	object->tags |= TAG_MARKED;
	markingQueueAdd(queue, object);
	LastGCStats.marked++;
}


static void markingQueueAdd(MarkingQueue *queue, RawObject *object)
{
	if (queue->size == queue->index) {
		queue->size += QUEUE_INIT_SIZE;
		queue->objects = realloc(queue->objects, queue->size * sizeof(RawObject *));
	}
	queue->objects[queue->index++] = object;
}


static RawObject *markingQueuePop(MarkingQueue *queue)
{
	queue->index--;
	return queue->objects[queue->index];
}


static _Bool markingQueueIsEmpty(MarkingQueue *queue)
{
	return queue->index == 0;
}


void gcSweep(PageSpace *space)
{
	PageSpaceIterator iterator;
	pageSpaceIteratorInit(&iterator, space);
	RawObject *object = pageSpaceIteratorNext(&iterator);
	RawObject *prev = NULL;

	RawObject *finalize[256] = { NULL };
	size_t finalizeSize = 0;

	while (object != NULL) {
		LastGCStats.total++;
		if ((object->tags & (TAG_MARKED | TAG_FREESPACE)) == 0) {
			if ((object->tags & TAG_FINALIZED) == 0 && hasFinalizer(object)) {
				ASSERT(finalizeSize < 256); // TODO: realloc instead
				finalize[finalizeSize++] = object;
				object->tags = (object->tags ^ TAG_MARKED) | TAG_FINALIZED;
				prev = object;
			/*} else if (prev != NULL && prev->tags & TAG_FREESPACE && heapPageIncludes(iterator.page, (uint8_t *) prev)) {
				extendFreeSpace((FreeSpace *) prev, align(computeRawObjectSize(object), HEAP_OBJECT_ALIGN));
				LastGCStats.extended++;
				LastGCStats.sweeped++;*/
			} else {
				freeObject(space, object);
				prev = object;
				LastGCStats.freed++;
				LastGCStats.sweeped++;
			}
		} else {
			object->tags = object->tags ^ TAG_MARKED;
			prev = object;
		}
		object = pageSpaceIteratorNext(&iterator);
	}

	for (size_t i = 0; i < finalizeSize; i++) {
		HandleScope scope;
		openHandleScope(&scope);
		EntryArgs args = { .size = 0 };
		entryArgsAddObject(&args, scopeHandle(finalize[i]));
		sendMessage(Handles.finalizeSymbol, &args);
		closeHandleScope(&scope, NULL);
	}
}


static _Bool hasFinalizer(RawObject *object)
{
	HandleScope scope;
	openHandleScope(&scope);
	// TODO: maybe add empty implementation in Object and check if #finalize was overwritten in subclass
	_Bool hasFinalizer = lookupSelector(scopeHandle(object->class), Handles.finalizeSymbol) != NULL;
	closeHandleScope(&scope, NULL);
	return hasFinalizer;
}


void resetGcStats(void)
{
	LastGCStats.total = 0;
	LastGCStats.marked = 0;
	LastGCStats.sweeped = 0;
	LastGCStats.freed = 0;
	LastGCStats.extended = 0;
}


void printGcStats(void)
{
	printf(
		"GC total: %zu"
		" marked: %zu"
		" sweeped: %zu"
		" freed: %zu"
		" extended: %zu"
		" time: %" PRIu64
		" count: %zu\n",
		LastGCStats.total,
		LastGCStats.marked,
		LastGCStats.sweeped,
		LastGCStats.freed,
		LastGCStats.extended,
		LastGCStats.totalTime,
		LastGCStats.count
	);
}
