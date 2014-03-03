#include "Scavenger.h"
#include "Heap.h"
#include "StackFrame.h"
#include "CodeDescriptors.h"
#include "Thread.h"
#include "Exception.h"
#include <string.h>

#define SCAVENGER_ALIGN 8

static void iterateStack(Scavenger *scavenger);
static void iterateExceptionHandlers(Scavenger *scavenger);
static void iterateHandles(Scavenger *scavenger);
static void iterateRememberedSet(Scavenger *scavenger);
static void iterateNativeCode(Scavenger *scavenger);
static RawObject *processPointer(Scavenger *scavenger, RawObject **p);
static RawObject *processTaggedPointer(Scavenger *scavenger, Value *p);
static void forwardObject(Scavenger *scavenger, RawObject *object);
static void iterateObject(Scavenger *scavenger, RawObject *root);


void initScavenger(Scavenger *scavenger, Heap *heap, size_t size)
{
	scavenger->heap = heap;
	scavenger->page = mapHeapPage(size, 0);
	size_t semiSpaceSize = scavenger->page->bodySize / 2;
	uint8_t *start = scavenger->page->body;

	scavenger->size = semiSpaceSize;

	scavenger->fromSpace = start;
	scavenger->toSpace = start + semiSpaceSize;

	scavenger->top = (uint8_t *) ((uintptr_t) start | NEW_SPACE_TAG);
	scavenger->end = start + semiSpaceSize;
	scavenger->survivorEnd = scavenger->top;
}


void freeScavenger(Scavenger *scavenger)
{
	unmapHeapPage(scavenger->page);
}


uint8_t *scavengerTryAllocate(Scavenger *scavenger, size_t size)
{
	ASSERT(size % HEAP_OBJECT_ALIGN == 0);
	size_t available = scavenger->end - scavenger->top;
	if (size > available) {
		return NULL;
	}

	uint8_t *result = scavenger->top;
	ASSERT(((uintptr_t) result & SPACE_TAG) == NEW_SPACE_TAG);
	ASSERT(scavenger->fromSpace <= result && result <= (scavenger->fromSpace + scavenger->size));
	scavenger->top += size;
	return result;
}


void scavengerScavenge(Scavenger *scavenger)
{
	scavenger->hasPromotionFailure = 0;
	scavenger->top = (uint8_t *) ((uintptr_t) scavenger->toSpace | NEW_SPACE_TAG);
	scavenger->end = scavenger->toSpace + scavenger->size;

	uint8_t *fromSpace = scavenger->toSpace;
	uint8_t *toSpace = scavenger->fromSpace;
	scavenger->fromSpace = fromSpace;
	scavenger->toSpace = toSpace;

	iterateRememberedSet(scavenger);
	iterateStack(scavenger);
	iterateExceptionHandlers(scavenger);
	iterateHandles(scavenger);
	iterateNativeCode(scavenger);
	scavenger->survivorEnd = scavenger->top;
	memset(scavenger->toSpace, scavenger->size, 0);

#if VERIFY_HEAP_AFTER_GC
	verifyHeap();
#endif
}


_Bool scavengerIncludes(Scavenger *scavenger, uint8_t *addr)
{
	uint8_t *start = (uint8_t *) ((uintptr_t) scavenger->fromSpace | NEW_SPACE_TAG);
	return start <= addr && addr < scavenger->top;
}


static void iterateStack(Scavenger *scavenger)
{
	EntryStackFrame *entryFrame = scavenger->heap->thread->stackFramesTail;
	while (entryFrame != NULL) {
		StackFrame *prev = entryFrame->exit;
		StackFrame *frame = stackFrameGetParent(prev, entryFrame);
		processTaggedPointer(scavenger, stackFrameGetSlotPtr(prev, 0));

		while (frame != NULL) {
			NativeCode *code = stackFrameGetNativeCode(frame);
			ASSERT(code->insts <= prev->parentIc && prev->parentIc <= (code->insts + code->size));

			size_t argsSize = code->argsSize + 1;
			for (ptrdiff_t i = 0; i < argsSize; i++) {
				Value *value = stackFrameGetArgPtr(frame, i);
				if (valueTypeOf(*value, VALUE_POINTER)) {
					processTaggedPointer(scavenger, value);
				}
			}

			RawStackmap *stackmap = findStackmap(code, (ptrdiff_t) prev->parentIc);
			ASSERT(stackmap != NULL);
			size_t frameSize = (stackmap->size - sizeof(Value)) * 8;
			for (size_t i = 0; i < frameSize; i++) {
				//ASSERT(i != 0 || stackmapIncludes(stackmap, i));
				if (stackmapIncludes(stackmap, i)) {
					Value *value = stackFrameGetSlotPtr(frame, i);
					if (valueTypeOf(*value, VALUE_POINTER)) {
						//ASSERT(pageSpaceIncludes(&_Heap.space, (uint8_t *) asObject(value)));
						processTaggedPointer(scavenger, value);
					}
				}
			}

			prev = frame;
			frame = stackFrameGetParent(frame, entryFrame);
		}
		entryFrame = entryFrame->prev;
	}
}


static void iterateExceptionHandlers(Scavenger *scavenger)
{
	Value handlerValue = CurrentExceptionHandler;

	while (handlerValue != 0) {
		RawExceptionHandler *handler = (RawExceptionHandler *) asObject(handlerValue);
		RawContext *context = (RawContext *) asObject(handler->context);
		if (contextHasValidFrame(context)) {
			CurrentExceptionHandler = handlerValue;
			break;
		}
		handlerValue = handler->parent;
	}

	if (CurrentExceptionHandler != 0) {
		processTaggedPointer(scavenger, &CurrentExceptionHandler);
	}
}


static void iterateHandles(Scavenger *scavenger)
{
	Thread *thread = scavenger->heap->thread;
	HandlesIterator handlesIterator;
	initHandlesIterator(&handlesIterator, thread->handles);
	while (handlesIteratorHasNext(&handlesIterator)) {
		processPointer(scavenger, &handlesIteratorNext(&handlesIterator)->raw);
	}

	HandleScopeIterator handleScopeIterator;
	initHandleScopeIterator(&handleScopeIterator, thread->handleScopes);
	while (handleScopeIteratorHasNext(&handleScopeIterator)) {
		HandleScope *scope = handleScopeIteratorNext(&handleScopeIterator);
		for (ptrdiff_t i = 0; i < scope->size; i++) {
			processPointer(scavenger, &scope->handles[i].raw);
		}
	}

	if (thread->context != 0) {
		processTaggedPointer(scavenger, &thread->context);
	}
}


static void iterateRememberedSet(Scavenger *scavenger)
{
	RememberedSetIterator iterator;
	initRememberedSetIterator(&iterator, &scavenger->heap->rememberedSet);
	while (rememberedSetIteratorHasNext(&iterator)) {
		iterateObject(scavenger, rememberedSetIteratorNext(&iterator));
	}
}


static void iterateNativeCode(Scavenger *scavenger)
{
	PageSpaceIterator iterator;
	pageSpaceIteratorInit(&iterator, &scavenger->heap->execSpace);
	NativeCode *code = (NativeCode *) pageSpaceIteratorNext(&iterator);

	while (code != NULL) {
		if ((code->tags & TAG_FREESPACE) == 0) {
			if (code->compiledCode != NULL) {
				processPointer(scavenger, (RawObject **) &code->compiledCode);
			}
			if (code->stackmaps != NULL) {
				processPointer(scavenger, (RawObject **) &code->stackmaps);
			}
			if (code->descriptors != NULL) {
				processPointer(scavenger, (RawObject **) &code->descriptors);
			}
			if (code->typeFeedback != NULL) {
				processPointer(scavenger, (RawObject **) &code->typeFeedback);
			}
			for (size_t i = 0; i < code->pointersOffsetsSize; i++) {
				uint16_t offset = ((uint16_t *) (code->insts + code->size))[i];
				Value *ptr = (Value *) (code->insts + offset);
				if (valueTypeOf(*ptr, VALUE_POINTER)) {
					processTaggedPointer(scavenger, ptr);
				} else {
					processPointer(scavenger, (RawObject **) ptr);
				}
			}
		}
		code = (NativeCode *) pageSpaceIteratorNext(&iterator);
	}
}


static RawObject *processPointer(Scavenger *scavenger, RawObject **p)
{
	RawObject *object = *p;
	if (((uintptr_t) object & SPACE_TAG) == OLD_SPACE_TAG) {
		return object;
	}
	if (object->tags & TAG_FORWARDED) {
		ASSERT(isOldObject((RawObject *) object->class) || scavenger->fromSpace <= (uint8_t *) object->class && (uint8_t *) object->class <= scavenger->top);
		ASSERT((object->class->tags & TAG_FORWARDED) == 0);
		*p = (RawObject *) object->class;
		return (RawObject *) object->class;
	}

	forwardObject(scavenger, object);
	ASSERT((object->class->tags & TAG_FORWARDED) == 0);
	object = (RawObject *) object->class;
	*p = object;
	iterateObject(scavenger, object);
	return object;
}


static RawObject *processTaggedPointer(Scavenger *scavenger, Value *p)
{
	RawObject *object = asObject(*p);
	if (((uintptr_t) object & SPACE_TAG) == OLD_SPACE_TAG) {
		return object;
	}
	if (object->tags & TAG_FORWARDED) {
		ASSERT(isOldObject((RawObject *) object->class) || scavenger->fromSpace <= (uint8_t *) object->class && (uint8_t *) object->class < scavenger->top);
		*p = tagPtr(object->class);
		return (RawObject *) object->class;
	}

	forwardObject(scavenger, object);
	object = (RawObject *) object->class;
	*p = tagPtr(object);
	iterateObject(scavenger, object);
	return object;
}


static void forwardObject(Scavenger *scavenger, RawObject *object)
{
	ASSERT((object->tags & TAG_FORWARDED) == 0);
	ASSERT(scavenger->toSpace <= (uint8_t *) object && (uint8_t *) object <= (scavenger->toSpace + scavenger->size));

	size_t size = align(computeRawObjectSize(object), HEAP_OBJECT_ALIGN);
	RawObject *newObject;

	object->tags &= ~TAG_MARKED;

	if ((uint8_t *) object < scavenger->survivorEnd) {
		newObject = (RawObject *) tryAllocateOld(scavenger->heap, size, scavenger->hasPromotionFailure);
		if (newObject == NULL) {
			scavenger->hasPromotionFailure = 1;
			newObject = (RawObject *) tryAllocateOld(scavenger->heap, size, 1);
		}
	} else {
		newObject = (RawObject *) scavengerTryAllocate(scavenger, size);
		ASSERT(scavenger->fromSpace <= (uint8_t *) newObject && (uint8_t *) newObject <= (scavenger->fromSpace + scavenger->size));
	}

	ASSERT(newObject != NULL);
	memcpy(newObject, object, size);
	object->tags |= TAG_FORWARDED;
	object->class = (RawClass *) newObject;
}


static void iterateObject(Scavenger *scavenger, RawObject *root)
{
	RawObject *object = processPointer(scavenger, (RawObject **) &root->class);
	_Bool remember = isNewObject(object);

	Value *vars = getRawObjectVars(root);
	size_t size = root->class->instanceShape.varsSize;
	if (root->class->instanceShape.isIndexed && !root->class->instanceShape.isBytes) {
		size += rawObjectSize(root);
	}

	for (size_t i = 0; i < size; i++) {
		if (valueTypeOf(vars[i], VALUE_POINTER)) {
			RawObject *object = processTaggedPointer(scavenger, &vars[i]);
			remember = remember || isNewObject(object);
		}
	}

	if (remember && isOldObject(root) && (root->tags & TAG_REMEMBERED) == 0) {
		rememberedSetAdd(&scavenger->heap->rememberedSet, root);
	}
}
