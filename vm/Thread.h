#ifndef THREAD_H
#define THREAD_H

#include "StackFrame.h"

typedef struct Thread {
	Value context;
	EntryStackFrame *stackFramesTail;
} Thread;

extern /*__thread*/ Thread CurrentThread;

void initThread(Thread *thread);
void threadSetExitFrame(StackFrame *stackFrame);

#endif
