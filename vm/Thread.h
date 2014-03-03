#ifndef THREAD_H
#define THREAD_H

#include "Heap.h"

struct HandleScope;
struct StackFrame;
struct EntryStackFrame;

typedef struct Thread {
	Heap heap;
	struct Handle *handles;
	struct HandleScope *handleScopes;
	Value context;
	struct EntryStackFrame *stackFramesTail;
} Thread;

extern __thread Thread CurrentThread;

void initThread(Thread *thread);
void initThreadContext(Thread *thread);
void freeThread(Thread *thread);
void threadSetExitFrame(struct StackFrame *stackFrame);


static inline void rawObjectStorePtr(RawObject *object, Value *field, RawObject *value)
{
	if (isOldObject(object) && isNewObject(value) && (object->tags & TAG_REMEMBERED) == 0) {
		rememberedSetAdd(&CurrentThread.heap.rememberedSet, object);
	}
	*field = tagPtr(value);
}


static inline void objectStorePtr(Object *object, Value *field, Object *value)
{
	rawObjectStorePtr(object->raw, field, value->raw);
}

#endif
