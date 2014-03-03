#include "Thread.h"
#include "StackFrame.h"
#include "Heap.h"
#include "Handle.h"
#include "Assert.h"

__thread Thread CurrentThread = { 0 };


void initThread(Thread *thread)
{
	initHeap(&thread->heap, thread);
	thread->stackFramesTail = NULL;
}


void initThreadContext(Thread *thread)
{
	if (thread->context == 0) {
		RawContext *context = (RawContext *) allocateObject(&CurrentThread.heap, Handles.MethodContext->raw, 0);
		context->thread = thread;
		thread->context = tagPtr(context);
	}
}


void freeThread(Thread *thread)
{
	thread->context = 0;
	freeHeap(&thread->heap);
}


void threadSetExitFrame(StackFrame *stackFrame)
{
	CurrentThread.stackFramesTail->exit = stackFrame;
}
