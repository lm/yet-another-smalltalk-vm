#include "Thread.h"
#include "Heap.h"
#include "Handle.h"
#include "Assert.h"

/*__thread*/ Thread CurrentThread = { 0 };


void initThread(Thread *thread)
{
	RawContext *context = (RawContext *) allocateObject(Handles.MethodContext->raw, 0);
	thread->context = tagPtr(context);
	thread->stackFramesTail = NULL;
	context->thread = thread;
}


void threadSetExitFrame(StackFrame *stackFrame)
{
	CurrentThread.stackFramesTail->exit = stackFrame;
}
