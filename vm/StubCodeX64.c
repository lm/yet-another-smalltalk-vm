#include "StubCode.h"
#include "Heap.h"
#include "Lookup.h"
#include "CodeGenerator.h"
#include "CodeDescriptors.h"
#include "AssemblerX64.h"
#include "Thread.h"
#include "StackFrame.h"
#include "Handle.h"
#include "Smalltalk.h"

static void initCodeGenerator(CodeGenerator *generator);
static CompiledMethod *createDoesNotUnderstandCode(void);


NativeCode *getStubNativeCode(StubCode *stub)
{
	if (stub->nativeCode == NULL) {
		HandleScope scope;
		openHandleScope(&scope);

		CodeGenerator generator;
		initCodeGenerator(&generator);
		stub->generator(&generator);
		stub->nativeCode = buildNativeCode(&generator);
		if (generator.code.methodOrBlock != NULL) {
			compiledMethodSetNativeCode((CompiledMethod *) generator.code.methodOrBlock, stub->nativeCode);
		}
		asmFreeBuffer(&generator.buffer);

		closeHandleScope(&scope, NULL);
	}
	return stub->nativeCode;
}


static void initCodeGenerator(CodeGenerator *generator)
{
	asmInitBuffer(&generator->buffer, 256);
	generator->code.methodOrBlock = NULL;
	generator->regsAlloc.varsSize = 0;
	generator->regsAlloc.frameSize = generator->frameSize = 0;
	generator->frameRawAreaSize = 0;
	generator->tmpVar = 0;
	generator->bytecodeNumber = 0;
	generator->stackmaps = newOrdColl(8);
	generator->descriptors = NULL;
}


void generateStubCall(CodeGenerator *generator, StubCode *stubCode)
{
	AssemblerBuffer *buffer = &generator->buffer;
	asmMovqImm(buffer, (uint64_t) getStubNativeCode(stubCode)->insts, TMP);
	asmCallq(buffer, TMP);
	generateStackmap(generator);
	if (generator->descriptors != NULL) {
		ordCollAdd(generator->descriptors, createBytecodeDescriptor(asmOffset(buffer), generator->bytecodeNumber));
	}
}


// RDI: compiled method
// RSI: native code entry
// RDX: arguments
// RCX: thread
static void generateSmalltalkEntry(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel loop;
	ptrdiff_t argsOffset = offsetof(RawCompiledMethod, header) + offsetof(CompiledCodeHeader, argsSize);

	size_t savedRegsSize = 6 * sizeof(intptr_t);
	size_t frameSize = align(savedRegsSize + sizeof(EntryStackFrame), 16);
	frameSize -= savedRegsSize;

	asmInitLabel(&loop);

	asmPushq(buffer, RBP);
	asmPushq(buffer, RBX);
	asmPushq(buffer, R12);
	asmPushq(buffer, R13);
	asmPushq(buffer, R14);
	asmPushq(buffer, R15);

	asmMovq(buffer, RSP, RBP);
	asmSubqImm(buffer, RSP, frameSize);

	// load previous entry frame
	asmMovqMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, offsetof(Thread, stackFramesTail)), TMP);
	// setup it in current entry frame
	asmMovqToMem(buffer, TMP, asmMem(RSP, NO_REGISTER, SS_1, offsetof(EntryStackFrame, prev)));
	// setup RBP
	asmMovqToMem(buffer, RBP, asmMem(RSP, NO_REGISTER, SS_1, offsetof(EntryStackFrame, entry)));
	// zero exit frame
	asmXorq(buffer, TMP, TMP);
	asmMovqToMem(buffer, TMP, asmMem(RSP, NO_REGISTER, SS_1, offsetof(EntryStackFrame, exit)));
	// setup entry stack frame to thread
	asmMovqToMem(buffer, RSP, asmMem(RCX, NO_REGISTER, SS_1, offsetof(Thread, stackFramesTail)));

	// load method argumnets size
	asmMovzxbMemq(buffer, asmMem(RDI, NO_REGISTER, SS_1, argsOffset), RBX);
	asmIncq(buffer, RBX); // there is alway 'self' argument
	asmPushq(buffer, RBX);

	// push arguments on stack
	asmLabelBind(buffer, &loop, asmOffset(buffer));
	asmPushqMem(buffer, asmMem(RDX, RBX, SS_8, -sizeof(intptr_t)));
	asmDecq(buffer, RBX);
	asmJ(buffer, COND_NOT_ZERO, &loop);

	// load context
	asmMovqMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, offsetof(Thread, context)), CTX);

	// invoke method
	asmMovq(buffer, RSI, R11); // Smalltalk methods expects native code entry in R11
	asmCallq(buffer, RSI);

	// load thread
	asmMovqMem(buffer, asmMem(CTX, NO_REGISTER, SS_1, varOffset(RawContext, thread)), RCX);
	// load entry frame
	asmMovqMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, offsetof(Thread, stackFramesTail)), TMP);
	// load previous entry frame
	asmMovqMem(buffer, asmMem(TMP, NO_REGISTER, SS_1, offsetof(EntryStackFrame, prev)), TMP);
	// setup entry stack frame to thread
	asmMovqToMem(buffer, TMP, asmMem(RCX, NO_REGISTER, SS_1, offsetof(Thread, stackFramesTail)));

	// restore RSP
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, -sizeof(intptr_t) - frameSize), RBX);
	asmLeaq(buffer, asmMem(RSP, RBX, SS_8, sizeof(intptr_t) + frameSize), RSP);

	asmPopq(buffer, R15);
	asmPopq(buffer, R14);
	asmPopq(buffer, R13);
	asmPopq(buffer, R12);
	asmPopq(buffer, RBX);
	asmPopq(buffer, RBP);
	asmRet(buffer);
}
StubCode SmalltalkEntry = { .generator = generateSmalltalkEntry, .nativeCode = NULL };


static void generateAllocate(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel noFreeSpace;
	AssemblerLabel bytes;
	AssemblerLabel align;
	AssemblerLabel notIndexed;
	AssemblerLabel noPayload;
	AssemblerLabel payloadLoop;
	AssemblerLabel noVars;
	AssemblerLabel varsLoop;
	AssemblerLabel noBytes;
	AssemblerLabel zeroBytes;
	AssemblerLabel zeroBytesLoop;

	ptrdiff_t sizeOffset = offsetof(RawClass, instanceShape) + offsetof(InstanceShape, size);
	ptrdiff_t varsOffset = offsetof(RawClass, instanceShape) + offsetof(InstanceShape, varsSize);
	ptrdiff_t isBytesOffset = offsetof(RawClass, instanceShape) + offsetof(InstanceShape, isBytes);
	ptrdiff_t scavengerOffset = offsetof(Thread, heap) + offsetof(Heap, newSpace);
	ptrdiff_t payloadOffset = offsetof(RawClass, instanceShape) + offsetof(InstanceShape, payloadSize);
	ptrdiff_t isIndexedOffset = offsetof(RawClass, instanceShape) + offsetof(InstanceShape, isIndexed);

	asmInitLabel(&noFreeSpace);
	asmInitLabel(&bytes);
	asmInitLabel(&align);
	asmInitLabel(&notIndexed);
	asmInitLabel(&noPayload);
	asmInitLabel(&payloadLoop);
	asmInitLabel(&noVars);
	asmInitLabel(&varsLoop);
	asmInitLabel(&noBytes);
	asmInitLabel(&zeroBytes);
	asmInitLabel(&zeroBytesLoop);

	// RSI: class
	// RDX: indexed variables size

	// load instance and variables size
	asmMovzxwMemq(buffer, asmMem(RSI, NO_REGISTER, SS_1, sizeOffset), RCX); // RCX: instance size
	asmMovzxbMemq(buffer, asmMem(RSI, NO_REGISTER, SS_1, varsOffset), RDI); // RDI: instance variables size

	// add indexed fields size
	asmTestbMemImm(buffer, asmMem(RSI, NO_REGISTER, SS_1, isBytesOffset), 1);
	asmJ(buffer, COND_NOT_ZERO, &bytes);

	// pointers shape
	asmLeaq(buffer, asmMem(RCX, RDX, SS_8, 0), RCX); // RCX: instance size
	asmAddq(buffer, RDX, RDI); // RDI: instance variables size
	asmJmpLabel(buffer, &align);

	// bytes shape
	asmLabelBind(buffer, &bytes, asmOffset(buffer));
	asmAddq(buffer, RDX, RCX); // RCX: instance size

	// align
	asmLabelBind(buffer, &align, asmOffset(buffer));
	asmAddqImm(buffer, RCX, HEAP_OBJECT_ALIGN - 1);
	asmAndqImm(buffer, RCX, -HEAP_OBJECT_ALIGN); // RCX: aligned size

	// check free space
	asmMovqMem(buffer, asmMem(CTX, NO_REGISTER, SS_1, varOffset(RawContext, thread)), RBX); // RBX: thread
	asmMovqMem(buffer, asmMem(RBX, NO_REGISTER, SS_1, scavengerOffset + offsetof(Scavenger, end)), TMP); // TMP: scavenger end
	asmMovqMem(buffer, asmMem(RBX, NO_REGISTER, SS_1, scavengerOffset + offsetof(Scavenger, top)), RAX); // RAX: new object
	asmSubq(buffer, RAX, TMP); // TMP: scavenger free space
	asmCmpq(buffer, RCX, TMP);
	asmJ(buffer, COND_ABOVE, &noFreeSpace);

	// move top cursor
	asmAddqToMem(buffer, RCX, asmMem(RBX, NO_REGISTER, SS_1, scavengerOffset + offsetof(Scavenger, top)));

	// class
	asmMovqToMem(buffer, RSI, asmMem(RAX, NO_REGISTER, SS_1, offsetof(RawObject, class)));
	// header
	asmMovq(buffer, RAX, TMP); // TMP: object hash
	asmShrqImm(buffer, TMP, 2);
	asmMovqImm(buffer, 0xFFFFFFFF, R8); // R8: mask
	asmAndq(buffer, R8, TMP);
	asmMovqToMem(buffer, TMP, asmMem(RAX, NO_REGISTER, SS_1, sizeof(Value)));
	asmMovq(buffer, RAX, R8);

	asmTestbMemImm(buffer, asmMem(RSI, NO_REGISTER, SS_1, isIndexedOffset), 1);
	asmJ(buffer, COND_ZERO, &notIndexed);
	asmMovqToMem(buffer, RDX, asmMem(RAX, NO_REGISTER, SS_1, offsetof(RawIndexedObject, size)));
	asmLeaq(buffer, asmMem(R8, NO_REGISTER, SS_1, sizeof(Value)), R8);

	asmLabelBind(buffer, &notIndexed, asmOffset(buffer));

	// zero payload
	asmMovzxbMemq(buffer, asmMem(RSI, NO_REGISTER, SS_1, payloadOffset), RBX); // RBX: payload size
	asmCmpqImm(buffer, RBX, 0);
	asmMovbToMem(buffer, BL, asmMem(RAX, NO_REGISTER, SS_1, offsetof(RawObject, payloadSize))); // store payload size in instance
	asmLeaq(buffer, asmMem(R8, RBX, SS_8, offsetof(RawObject, body)), RCX); // save pointer to inst vars
	asmJ(buffer, COND_EQUAL, &noPayload);

	asmLabelBind(buffer, &payloadLoop, asmOffset(buffer));
	asmMovqMemImm(buffer, 0, asmMem(R8, RBX, SS_8, offsetof(RawObject, body) - sizeof(Value)));
	asmDecq(buffer, RBX);
	asmJ(buffer, COND_NOT_ZERO, &payloadLoop);

	asmLabelBind(buffer, &noPayload, asmOffset(buffer));

	// nil variables
	asmCmpqImm(buffer, RDI, 0);
	asmJ(buffer, COND_EQUAL, &noVars);
	generateLoadObject(buffer, Handles.nil->raw, TMP, 1);
	asmMovq(buffer, RDI, RBX);
	asmMovbToMem(buffer, BL, asmMem(RAX, NO_REGISTER, SS_1, offsetof(RawObject, varsSize)));

	asmLabelBind(buffer, &varsLoop, asmOffset(buffer));
	asmMovqToMem(buffer, TMP, asmMem(RCX, RBX, SS_8, -sizeof(Value)));
	asmDecq(buffer, RBX);
	asmJ(buffer, COND_NOT_ZERO, &varsLoop);

	asmLabelBind(buffer, &noVars, asmOffset(buffer));

	// zero bytes
	asmTestbMemImm(buffer, asmMem(RSI, NO_REGISTER, SS_1, isBytesOffset), 1);
	asmJ(buffer, COND_ZERO, &noBytes);
	asmCmpqImm(buffer, RDX, 0);
	asmJ(buffer, COND_EQUAL, &zeroBytes);
	asmMovq(buffer, RDX, RBX);
	asmLeaq(buffer, asmMem(RCX, RDI, SS_8, 0), RCX);

	asmLabelBind(buffer, &zeroBytesLoop, asmOffset(buffer));
	asmMovbMemImm(buffer, 0, asmMem(RCX, RBX, SS_1, -1));
	asmDecq(buffer, RBX);
	asmJ(buffer, COND_NOT_ZERO, &zeroBytesLoop);

	asmLabelBind(buffer, &noBytes, asmOffset(buffer));
	asmLabelBind(buffer, &zeroBytes, asmOffset(buffer));

	asmIncq(buffer, RAX);
	asmRet(buffer);

	asmLabelBind(buffer, &noFreeSpace, asmOffset(buffer));
	asmLeaq(buffer, asmMem(RBX, NO_REGISTER, SS_1, offsetof(Thread, heap)), RDI);
	generateCCall(generator, (intptr_t) allocateObject, 3, 0);
	asmIncq(buffer, RAX);
	asmRet(buffer);
}
StubCode AllocateStub = { .generator = generateAllocate, .nativeCode = NULL };


static void generateLookup(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	generateCCall(generator, (intptr_t) lookupNativeCode, 2, 0);
	asmMovq(buffer, RAX, R11);
	asmRet(buffer);
}
StubCode LookupStub = { .generator = generateLookup, .nativeCode = NULL };


static void generateDoesNotUnderstandStub(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel loop;
	AssemblerLabel zeroArgs;
	asmInitLabel(&loop);
	asmInitLabel(&zeroArgs);

	generator->code.methodOrBlock = createDoesNotUnderstandCode();
	generator->frameSize = 3;

	asmPushq(buffer, RBP);
	asmMovq(buffer, RSP, RBP);

	// save native code
	asmPushq(buffer, R11);
	generatePushDummyContext(buffer);

	// spill selector
	asmPushq(buffer, RDI);

	// allocate arguments array
	generateLoadObject(buffer, (RawObject *) Handles.Array->raw, RSI, 0);
	generateStubCall(generator, &AllocateStub);

	// fill arguments array from stack
	asmMovqMem(buffer, asmMem(RAX, NO_REGISTER, SS_1, varOffset(RawArray, size)), RBX);
	asmTestq(buffer, RBX, RBX);
	asmJ(buffer, COND_ZERO, &zeroArgs);
	asmLabelBind(buffer, &loop, asmOffset(buffer));
	asmMovqMem(buffer, asmMem(RBP, RBX, SS_8, 3 * sizeof(intptr_t)), TMP);
	asmMovqToMem(buffer, TMP, asmMem(RAX, RBX, SS_8, varOffset(RawArray, vars) - sizeof(intptr_t)));
	asmDecq(buffer, RBX);
	asmJ(buffer, COND_NOT_ZERO, &loop);
	asmLabelBind(buffer, &zeroArgs, asmOffset(buffer));

	// spill arguments array
	asmPushq(buffer, RAX);
	generator->frameSize++;

	// allocate message
	generateLoadObject(buffer, (RawObject *) Handles.Message->raw, RSI, 0);
	asmXorq(buffer, RDX, RDX);
	generateStubCall(generator, &AllocateStub);

	// fill message
	asmPopqMem(buffer, asmMem(RAX, NO_REGISTER, SS_1, varOffset(RawMessage, arguments)));
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, -3 * sizeof(intptr_t)), TMP);
	asmMovqToMem(buffer, TMP, asmMem(RAX, NO_REGISTER, SS_1, varOffset(RawMessage, selector)));
	asmPushq(buffer, RAX);

	// lookup #doesNotUnderstand:
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), TMP);
	asmPushq(buffer, TMP);
	generator->frameSize++;
	generateLoadClass(buffer, TMP, RDI);
	generateLoadObject(buffer, (RawObject *) Handles.doesNotUnderstandSymbol->raw, RSI, 0);
	generateMethodLookup(generator);

	// call #doesNotUnderstand:
	generator->frameSize -= 2;
	asmCallq(buffer, R11);
	generateStackmap(generator);

	// epilogue
	asmAddqImm(buffer, RSP, 5 * sizeof(intptr_t));
	asmPopq(buffer, RBP);
	asmRet(buffer);
}
StubCode DoesNotUnderstandStub = { .generator = generateDoesNotUnderstandStub, .nativeCode = NULL };


static CompiledMethod *createDoesNotUnderstandCode(void)
{
	CompiledCodeHeader header = { 0 };
	CompiledMethod *method = handle(allocateObject(&CurrentThread.heap, Handles.CompiledMethod->raw, 0));
	SourceCode *source = newObject(Handles.SourceCode, 0);
	sourceCodeSetSourceOrFileName(source, asString("_doesNotUnderstand []"));
	sourceCodeSetPosition(source, 0);
	sourceCodeSetSourceSize(source, 0);
	sourceCodeSetLine(source, 0);
	sourceCodeSetColumn(source, 0);
	compiledMethodSetOwnerClass(method, Handles.UndefinedObject);
	compiledMethodSetSelector(method, getSymbol("_doesNotUnderstand"));
	compiledMethodSetSourceCode(method, source);
	method->raw->header = header;
	return method;
}
