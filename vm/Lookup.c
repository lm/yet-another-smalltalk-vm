#include "Lookup.h"
#include "CodeGenerator.h"
#include "Object.h"
#include "Class.h"
#include "String.h"
#include "Smalltalk.h"
#include "Heap.h"
#include "Handle.h"
#include "CompiledCode.h"
#include "CodeDescriptors.h"
#include "Thread.h"
#include "StackFrame.h"

LookupTable LookupCache = {
	.classes = { NULL },
	.selectors = { NULL },
	.codes = { NULL },
};

static void feedbackType(Class *class);
static NativeCodeEntry doesNotUnderstand(Class *class, String *selector);


NativeCodeEntry lookupNativeCode(RawClass *class, RawString *selector)
{
	HandleScope scope;
	openHandleScope(&scope);

	Class *classHandle = scopeHandle(class);
	String *selectorHandle = scopeHandle(selector);
	CompiledMethod *method = lookupSelector(classHandle, selectorHandle);

	NativeCodeEntry entry;
	if (method == NULL) {
		entry = doesNotUnderstand(classHandle, selectorHandle);
	} else {
		entry = (NativeCodeEntry) getNativeCode(classHandle, method)->insts;
		feedbackType(classHandle);
	}

	intptr_t hash = lookupHash((intptr_t) classHandle->raw, (intptr_t) selectorHandle->raw);
	LookupCache.classes[hash] = classHandle->raw;
	LookupCache.selectors[hash] = selectorHandle->raw;
	LookupCache.codes[hash] = (uint8_t *) entry;

	closeHandleScope(&scope, NULL);
	return entry;
}


static void feedbackType(Class *class)
{
	EntryStackFrame *entryFrame = CurrentThread.stackFramesTail;
	if (entryFrame == NULL) {
		return;
	}

	StackFrame *frame = stackFrameGetParent(entryFrame->exit, entryFrame);
	NativeCode *code = stackFrameGetNativeCode(frame);
	OrderedCollection *typeFeedback;
	if (code->typeFeedback == NULL) {
		typeFeedback = newOrdColl(8);
		code->typeFeedback = typeFeedback->raw;
	} else {
		typeFeedback = scopeHandle(code->typeFeedback);
		if (ordCollSize(typeFeedback) > 16) {
			typeFeedback = newOrdColl(8);
			code->typeFeedback = typeFeedback->raw;
		}
	}

	TypeFeedback *type = newObject(Handles.TypeFeedback, 0);
	type->raw->ic = tagInt(entryFrame->exit->parentIc - code->insts);
	type->raw->hintedClass = getTaggedPtr(class);
	ordCollAddObject(typeFeedback, (Object *) type);
}


static NativeCodeEntry doesNotUnderstand(Class *class, String *selector)
{
	intptr_t hash = lookupHash((intptr_t) class->raw, (intptr_t) selector->raw);
	NativeCode *code = generateDoesNotUnderstand(selector);
	code->compiledCode = lookupSelector(class, Handles.doesNotUnderstandSymbol)->raw;
	return (NativeCodeEntry) code->insts;
}


NativeCode *getNativeCode(Class *class, CompiledMethod *method)
{
	NativeCode *code = compiledMethodGetNativeCode(method);
	if (code == NULL) {
		String *selector = compiledMethodGetSelector(method);
		code = generateMethodCode(method);
		compiledMethodSetNativeCode(method, code);
	}
	return code;
}
