#ifndef STACK_FRAME_H
#define STACK_FRAME_H

#define FRAME_VARS_OFFSET 2
#define FRAME_CODE_OFFSET 0
#define CONTEXT_SLOT 1

#include "Thread.h"
#include "Object.h"
#include "CompiledCode.h"
#include <stdint.h>

typedef struct StackFrame {
	// Value temps[];
	// RawContext *context;
	struct StackFrame *parent;
	uint8_t *parentIc;
	Value args[];
} StackFrame;

typedef struct EntryStackFrame {
	struct EntryStackFrame *prev;
	StackFrame *entry;
	StackFrame *exit;
} EntryStackFrame;

typedef struct {
	OBJECT_HEADER;
	Value size;
	Thread *thread;
	StackFrame *frame;
	Value ic;
	Value code;
	Value parent;
	Value outer;
	Value home;
	Value vars[];
} RawContext;
OBJECT_HANDLE(Context);

StackFrame *stackFrameGetParent(StackFrame *frame, EntryStackFrame *entryFrame);
RawContext *stackFrameGetContext(StackFrame *frame);
RawContext *stackFrameGetParentContext(StackFrame *frame);
void stackFrameSetArg(StackFrame *frame, ptrdiff_t index, Value value);
Value stackFrameGetArg(StackFrame *frame, ptrdiff_t index);
Value *stackFrameGetArgPtr(StackFrame *frame, ptrdiff_t index);
void stackFrameSetSlot(StackFrame *frame, ptrdiff_t index, Value value);
Value stackFrameGetSlot(StackFrame *frame, ptrdiff_t index);
Value *stackFrameGetSlotPtr(StackFrame *frame, ptrdiff_t index);
NativeCode *stackFrameGetNativeCode(StackFrame *frame);
_Bool contextHasValidFrame(RawContext *context);

#endif
