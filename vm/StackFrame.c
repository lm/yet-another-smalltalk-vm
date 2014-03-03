#include "StackFrame.h"
#include "Thread.h"
#include "Smalltalk.h"
#include "CompiledCode.h"
#include "Heap.h"
#include "Handle.h"
#include "Assert.h"
#include <string.h>


StackFrame *stackFrameGetParent(StackFrame *frame, EntryStackFrame *entryFrame)
{
	if (frame->parent == entryFrame->entry) {
		return NULL;
	}
	return frame->parent;
}


RawContext *stackFrameGetContext(StackFrame *frame)
{
	return (RawContext *) asObject(stackFrameGetSlot(frame, CONTEXT_SLOT));
}


RawContext *stackFrameGetParentContext(StackFrame *frame)
{
	StackFrame *parent = stackFrameGetParent(frame, CurrentThread.stackFramesTail);
	if (parent == NULL) {
		return NULL;
	}

	Value contextSlotValue = stackFrameGetSlot(parent, CONTEXT_SLOT);
	RawContext *context;
	if (contextSlotValue == CurrentThread.context) {
		context = (RawContext *) allocateObject(&CurrentThread.heap, Handles.MethodContext->raw, 0);
		context->frame = parent;
		context->code = tagPtr(stackFrameGetNativeCode(parent)->compiledCode);
		stackFrameSetSlot(parent, CONTEXT_SLOT, tagPtr(context));
	} else {
		context = (RawContext *) asObject(contextSlotValue);
	}
	context->ic = tagInt((intptr_t) frame->parentIc/* - code->nativeCode->insts*/);
	return context;
}


void stackFrameSetArg(StackFrame *frame, ptrdiff_t index, Value value)
{
	frame->args[index] = value;
}


Value stackFrameGetArg(StackFrame *frame, ptrdiff_t index)
{
	return frame->args[index];
}


Value *stackFrameGetArgPtr(StackFrame *frame, ptrdiff_t index)
{
	return &frame->args[index];
}


void stackFrameSetSlot(StackFrame *frame, ptrdiff_t index, Value value)
{
	Value *slots = (Value *) frame - 1;
	slots[-index] = value;
}


Value stackFrameGetSlot(StackFrame *frame, ptrdiff_t index)
{
	Value *slots = (Value *) frame - 1;
	return slots[-index];
}


Value *stackFrameGetSlotPtr(StackFrame *frame, ptrdiff_t index)
{
	Value *slots = (Value *) frame - 1;
	return &slots[-index];
}


NativeCode *stackFrameGetNativeCode(StackFrame *frame)
{
	return (NativeCode *) (stackFrameGetSlot(frame, FRAME_CODE_OFFSET) - offsetof(NativeCode, insts));
}


_Bool contextHasValidFrame(RawContext *context)
{
	return stackFrameGetSlot(context->frame, CONTEXT_SLOT) == tagPtr(context);
}


void printBacktrace(void)
{
	EntryStackFrame *entryFrame = CurrentThread.stackFramesTail;
	while (entryFrame != NULL) {
		StackFrame *prev = entryFrame->exit;
		StackFrame *frame = stackFrameGetParent(prev, entryFrame);
		while (frame != NULL) {
			NativeCode *code = stackFrameGetNativeCode(frame);

			RawCompiledMethod *method = code->compiledCode;
			RawCompiledBlock *block = NULL;
			if (method->class == Handles.CompiledBlock->raw) {
				block = (RawCompiledBlock *) method;
				method = (RawCompiledMethod *) asObject(block->method);
			}

			RawClass *class = (RawClass *) asObject(method->ownerClass);
			if (class->class == Handles.MetaClass->raw) {
				class = (RawClass *) asObject(((RawMetaClass *) class)->instanceClass);
			}

			RawString *className = (RawString *) asObject(class->name);
			RawString *selector = (RawString *) asObject(method->selector);
			printf(
				"%p in %.*s#%.*s%s\n",
				(void *) prev->parentIc,
				(int) className->size, className->contents,
				(int) selector->size, selector->contents,
				block == NULL ? "" : "[]"
			);

			prev = frame;
			frame = stackFrameGetParent(frame, entryFrame);
		}
		entryFrame = entryFrame->prev;
	}
}
