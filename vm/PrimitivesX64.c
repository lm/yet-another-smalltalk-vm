#include "AssemblerX64.h"
#include "StubCode.h"
#include "CodeDescriptors.h"

static void generateBlockValuePrimitive(CodeGenerator *generator, uint8_t args);
static void movArg(AssemblerBuffer *buffer, ptrdiff_t index, Register dst);
static MemoryOperand arg(ptrdiff_t index);


static void generateCCallPrimitive(CodeGenerator *generator, PrimitiveResult (*cFunction)(), size_t argsSize)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel failed;
	asmInitLabel(&failed);

	for (size_t i = 0; i < argsSize; i++) {
		movArg(buffer, i, ArgumentsRegisters[i]);
	}
	asmMovq(buffer, R11, R13);
	generateCCall(generator, (intptr_t) cFunction, argsSize, 0);
	asmTestq(buffer, RDX, RDX);
	asmJ(buffer, COND_NOT_ZERO, &failed);
	asmRet(buffer);
	asmLabelBind(buffer, &failed, asmOffset(buffer));
	asmMovq(buffer, R13, R11);
}


static void loadClass(CodeGenerator *generator, Register src, Register dst)
{
	asmMovqMem(&generator->buffer, asmMem(src, NO_REGISTER, SS_1, -1), dst);
}


static void testInt(CodeGenerator *generator, ByteRegister reg)
{
	asmTestbImm(&generator->buffer, reg, 3);
}


static void primitveNotImplemented(void)
{
	printf("Error: Primitive is not implemented\n");
	exit(EXIT_FAILURE);
}


static void generateNotImplementedPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	asmMovqImm(buffer, (uint64_t) primitveNotImplemented, TMP);
	asmCallq(buffer, TMP);
}


static void generateInstVarAtPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	AssemblerLabel outOfBounds;
	ptrdiff_t offset = offsetof(RawClass, instanceShape);
	ptrdiff_t sizeOffset = offset + offsetof(InstanceShape, varsSize);
	ptrdiff_t isIndexedOffset = offset + offsetof(InstanceShape, isIndexed);
	ptrdiff_t payloadSizeOffset = offset + offsetof(InstanceShape, payloadSize);

	asmInitLabel(&notInt);
	asmInitLabel(&outOfBounds);

	movArg(buffer, 0, RDI);
	movArg(buffer, 1, RSI);

	testInt(generator, SIL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	loadClass(generator, RDI, RCX);
	asmShrqImm(buffer, RSI, 2);
	asmDecq(buffer, RSI);
	asmCmpbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, sizeOffset), SIL);
	asmJ(buffer, COND_ABOVE_EQUAL, &outOfBounds);

	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, isIndexedOffset), SIL);
	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, payloadSizeOffset), SIL);
	asmMovqMem(buffer, asmMem(RDI, RSI, SS_8, HEADER_SIZE - 1), RAX);
	asmRet(buffer);

	asmLabelBind(buffer, &outOfBounds, asmOffset(buffer));
	asmIncq(buffer, RSI);
	asmShlqImm(buffer, RSI, 2);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
}


static void generateInstVarAtPutPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	AssemblerLabel outOfBounds;
	ptrdiff_t offset = offsetof(RawClass, instanceShape);
	ptrdiff_t sizeOffset = offset + offsetof(InstanceShape, varsSize);
	ptrdiff_t isIndexedOffset = offset + offsetof(InstanceShape, isIndexed);
	ptrdiff_t payloadSizeOffset = offset + offsetof(InstanceShape, payloadSize);

	asmInitLabel(&notInt);
	asmInitLabel(&outOfBounds);

	movArg(buffer, 0, RDI);
	movArg(buffer, 1, RSI);
	movArg(buffer, 2, RAX);

	testInt(generator, SIL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	loadClass(generator, RDI, RCX);
	asmShrqImm(buffer, RSI, 2);
	asmDecq(buffer, RSI);
	asmCmpbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, sizeOffset), SIL);
	asmJ(buffer, COND_ABOVE_EQUAL, &outOfBounds);

	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, isIndexedOffset), SIL);
	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, payloadSizeOffset), SIL);
	asmMovqToMem(buffer, RAX, asmMem(RDI, RSI, SS_8, HEADER_SIZE - 1));
	generateStoreCheck(generator, RDI, RAX);
	asmRet(buffer);

	asmLabelBind(buffer, &outOfBounds, asmOffset(buffer));
	asmIncq(buffer, RSI);
	asmShlqImm(buffer, RSI, 2);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
}


static void generateAtPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	AssemblerLabel notIndexable;
	AssemblerLabel outOfBounds;
	AssemblerLabel bytes;
	ptrdiff_t offset = offsetof(RawClass, instanceShape);
	ptrdiff_t payloadSizeOffset = offset + offsetof(InstanceShape, payloadSize);
	ptrdiff_t sizeOffset = offset + offsetof(InstanceShape, varsSize);
	ptrdiff_t isIndexedOffset = offset + offsetof(InstanceShape, isIndexed);
	ptrdiff_t isBytesOffset = offset + offsetof(InstanceShape, isBytes);
	ptrdiff_t valueTypeOffset = offset + offsetof(InstanceShape, valueType);

	asmInitLabel(&notInt);
	asmInitLabel(&notIndexable);
	asmInitLabel(&outOfBounds);
	asmInitLabel(&bytes);

	movArg(buffer, 0, RDI);
	movArg(buffer, 1, RSI);

	testInt(generator, SIL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	loadClass(generator, RDI, RCX);
	asmTestbMemImm(buffer, asmMem(RCX, NO_REGISTER, SS_1, isIndexedOffset), 1);
	asmJ(buffer, COND_ZERO, &notIndexable);

	asmShrqImm(buffer, RSI, 2);
	asmDecq(buffer, RSI);
	asmCmpqMem(buffer, asmMem(RDI, NO_REGISTER, SS_1, offsetof(RawIndexedObject, size) - 1), RSI);
	asmJ(buffer, COND_ABOVE_EQUAL, &outOfBounds);

	asmTestbMemImm(buffer, asmMem(RCX, NO_REGISTER, SS_1, isBytesOffset), 1);
	asmJ(buffer, COND_NOT_ZERO, &bytes);

	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, payloadSizeOffset), SIL);
	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, sizeOffset), SIL);
	asmMovqMem(buffer, asmMem(RDI, RSI, SS_8, HEADER_SIZE + sizeof(Value) - 1), RAX);
	asmRet(buffer);

	asmLabelBind(buffer, &bytes, asmOffset(buffer));
	asmXorq(buffer, RAX, RAX);
	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, payloadSizeOffset), AL);
	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, sizeOffset), AL);
	asmLeaq(buffer, asmMem(RSI, RAX, SS_8, sizeof(Value)), RAX);
	asmMovzxbMemq(buffer, asmMem(RDI, RAX, SS_1, HEADER_SIZE - 1), RAX);
	asmShlqImm(buffer, RAX, 2);
	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, valueTypeOffset), AL);
	asmRet(buffer);

	asmLabelBind(buffer, &outOfBounds, asmOffset(buffer));
	asmIncq(buffer, RSI);
	asmShlqImm(buffer, RSI, 2);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
	asmLabelBind(buffer, &notIndexable, asmOffset(buffer));
}


static void generateAtPutPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	AssemblerLabel notIndexable;
	AssemblerLabel outOfBounds;
	AssemblerLabel bytes;
	ptrdiff_t offset = offsetof(RawClass, instanceShape);
	ptrdiff_t payloadSizeOffset = offset + offsetof(InstanceShape, payloadSize);
	ptrdiff_t sizeOffset = offset + offsetof(InstanceShape, varsSize);
	ptrdiff_t isIndexedOffset = offset + offsetof(InstanceShape, isIndexed);
	ptrdiff_t isBytesOffset = offset + offsetof(InstanceShape, isBytes);
	ptrdiff_t valueTypeOffset = offset + offsetof(InstanceShape, valueType);

	asmInitLabel(&notInt);
	asmInitLabel(&notIndexable);
	asmInitLabel(&outOfBounds);
	asmInitLabel(&bytes);

	// load arguments
	movArg(buffer, 0, RDI);
	movArg(buffer, 1, RSI);
	movArg(buffer, 2, RDX);

	// value is result
	asmMovq(buffer, RDX, RAX);

	// check if index is int
	testInt(generator, SIL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	// check if object is indexed
	loadClass(generator, RDI, RCX);
	asmTestbMemImm(buffer, asmMem(RCX, NO_REGISTER, SS_1, isIndexedOffset), 1);
	asmJ(buffer, COND_ZERO, &notIndexable);

	// untag and decrement index
	asmShrqImm(buffer, RSI, 2);
	asmDecq(buffer, RSI);

	// check bounds
	asmCmpqMem(buffer, asmMem(RDI, NO_REGISTER, SS_1, offsetof(RawIndexedObject, size) - 1), RSI);
	asmJ(buffer, COND_ABOVE_EQUAL, &outOfBounds);

	// check if object is bytes
	asmTestbMemImm(buffer, asmMem(RCX, NO_REGISTER, SS_1, isBytesOffset), 1);
	asmJ(buffer, COND_NOT_ZERO, &bytes);

	// compute offset
	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, payloadSizeOffset), SIL);
	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, sizeOffset), SIL);

	// set the value
	asmMovqToMem(buffer, RDX, asmMem(RDI, RSI, SS_8, HEADER_SIZE + sizeof(Value) - 1));
	generateStoreCheck(generator, RDI, RDX);
	asmRet(buffer);

	// bytes
	asmLabelBind(buffer, &bytes, asmOffset(buffer));

	// compute offset
	asmXorq(buffer, RBX, RBX);
	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, payloadSizeOffset), AL);
	asmAddbMem(buffer, asmMem(RCX, NO_REGISTER, SS_1, sizeOffset), AL);
	asmLeaq(buffer, asmMem(RSI, RBX, SS_8, sizeof(Value)), RBX);

	// TODO: valueTypeOf(RDX) == RCX->instanceShape.valueType
	// untag and set the value
	asmShrqImm(buffer, RDX, 2);
	asmMovbToMem(buffer, DL, asmMem(RDI, RBX, SS_1, HEADER_SIZE - 1));
	asmRet(buffer);

	asmLabelBind(buffer, &outOfBounds, asmOffset(buffer));
	asmIncq(buffer, RSI);
	asmShlqImm(buffer, RSI, 2);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
	asmLabelBind(buffer, &notIndexable, asmOffset(buffer));
}


static void generateSizePrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notPointer;
	AssemblerLabel notIndexable;
	ptrdiff_t offset = offsetof(RawClass, instanceShape);
	ptrdiff_t isIndexedOffset = offset + offsetof(InstanceShape, isIndexed);

	asmInitLabel(&notPointer);
	asmInitLabel(&notIndexable);

	movArg(buffer, 0, RDI);

	asmMovb(buffer, DIL, SIL);
	asmAndbImm(buffer, SIL, 3);
	asmCmpbImm(buffer, SIL, VALUE_POINTER);
	asmJ(buffer, COND_NOT_EQUAL, &notPointer);

	loadClass(generator, RDI, RDX);
	asmTestbMemImm(buffer, asmMem(RDX, NO_REGISTER, SS_1, isIndexedOffset), 1);
	asmJ(buffer, COND_ZERO, &notIndexable);

	asmMovqMem(buffer, asmMem(RDI, NO_REGISTER, SS_1, offsetof(RawIndexedObject, size) - 1), RAX);
	asmShlqImm(buffer, RAX, 2);
	asmRet(buffer);

	asmLabelBind(buffer, &notPointer, asmOffset(buffer));
	asmLabelBind(buffer, &notIndexable, asmOffset(buffer));
	asmXorq(buffer, RAX, RAX);
	asmRet(buffer);
}


static void generateIdentityPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel equal;
	asmInitLabel(&equal);

	movArg(buffer, 0, RDI);
	asmCmpqMem(buffer, arg(1), RDI);
	asmJ(buffer, COND_EQUAL, &equal);
	generateLoadObject(buffer, Handles.false->raw, RAX, 1);
	asmRet(buffer);

	asmLabelBind(buffer, &equal, asmOffset(buffer));
	generateLoadObject(buffer, Handles.true->raw, RAX, 1);
	asmRet(buffer);
}


static void generateHashPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	movArg(buffer, 0, RDI);
	asmMovqMem(buffer, asmMem(RDI, NO_REGISTER, SS_1, /*offsetof(Object, hash)*/ sizeof(Value) - 1), RAX);
	asmMovqImm(buffer, 0xFFFFFFFF, TMP);
	asmAndq(buffer, TMP, RAX);
	asmShlqImm(buffer, RAX, 2);
	asmRet(buffer);
}


static void generateClassPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	movArg(buffer, 0, RDI);
	generateLoadClass(buffer, RDI, RAX);
	asmRet(buffer);
}


static void generateBehaviorNewPrimitive(CodeGenerator *generator)
{
	ptrdiff_t compiledCodeOffset = offsetof(NativeCode, compiledCode) - offsetof(NativeCode, insts);
	AssemblerBuffer *buffer = &generator->buffer;

	asmPushq(buffer, RBP);
	asmMovq(buffer, RSP, RBP);

	// save native code
	asmPushq(buffer, R11);
	generatePushDummyContext(buffer);

	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), RSI);
	asmDecq(buffer, RSI);
	asmXorq(buffer, RDX, RDX);
	generateStubCall(generator, &AllocateStub);

	asmMovq(buffer, RBP, RSP);
	asmPopq(buffer, RBP);
	asmRet(buffer);
}


static void generateBehaviorNewSizePrimitive(CodeGenerator *generator)
{
	ptrdiff_t compiledCodeOffset = offsetof(NativeCode, compiledCode) - offsetof(NativeCode, insts);
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	asmInitLabel(&notInt);

	asmPushq(buffer, RBP);
	asmMovq(buffer, RSP, RBP);

	// save native code
	asmPushq(buffer, R11);
	generatePushDummyContext(buffer);

	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 3 * sizeof(intptr_t)), RDX);

	// check if size is int
	testInt(generator, DL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), RSI);
	asmDecq(buffer, RSI);
	asmShrqImm(buffer, RDX, 2);
	generateStubCall(generator, &AllocateStub);

	asmMovq(buffer, RBP, RSP);
	asmPopq(buffer, RBP);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
	asmMovq(buffer, RBP, RSP);
	asmPopq(buffer, RBP);
}


static void generateCharacterNewPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	asmInitLabel(&notInt);

	movArg(buffer, 1, RAX);
	testInt(generator, AL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	asmAddqImm(buffer, RAX, VALUE_CHAR);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
}


static void generateCharacterCodePrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	movArg(buffer, 0, RAX);
	asmSubqImm(buffer, RAX, VALUE_CHAR);
	asmRet(buffer);
}


static void generateStringHashPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	movArg(buffer, 0, RDI);
	asmDecq(buffer, RDI);
	asmMovqImm(buffer, (uint64_t) computeRawStringHash, TMP);
	asmCallq(buffer, TMP);
	asmShlqImm(buffer, RAX, 2);
	asmRet(buffer);
}


static void generateInterruptPrimitive(CodeGenerator *generator)
{
	asmInt3(&generator->buffer);
}


static void generateExitPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	asmMovqImm(buffer, 1, RDI);
	asmMovqImm(buffer, (uint64_t) exit, TMP);
	asmCallq(buffer, TMP);
}


static NativeCodeEntry scopedGetNativeCode(Value vReceiver, Value vMethod)
{
	HandleScope scope;
	openHandleScope(&scope);

	Class *class = scopeHandle(getClassOf(vReceiver));
	CompiledMethod *method = scopeHandle(asObject(vMethod));
	NativeCodeEntry entry = (NativeCodeEntry) getNativeCode(class, method)->insts;

	closeHandleScope(&scope, NULL);
	return entry;
}


static void generateMethodSendPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;

	// prologue
	asmPushq(buffer, RBP);
	asmMovq(buffer, RSP, RBP);

	// save native code
	asmPushq(buffer, R11);
	generatePushDummyContext(buffer);

	// get native code for given method
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), RSI);
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 3 * sizeof(intptr_t)), RDI);
	generateCCall(generator, (intptr_t) scopedGetNativeCode, 2, 1);
	// push receiver
	asmPushqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 3 * sizeof(intptr_t)));
	// invoke method
	asmMovq(buffer, RAX, R11);
	asmCallq(buffer, R11);
	generateStackmap(generator);

	// epilogue
	asmAddqImm(buffer, RSP, 3 * sizeof(intptr_t));
	asmPopq(buffer, RBP);
	asmRet(buffer);
}


static void generateMethodSendArgsPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel invalidArgs;
	AssemblerLabel loop;
	AssemblerLabel zeroArgs;
	ptrdiff_t argsOffset = offsetof(RawCompiledMethod, header) + offsetof(CompiledCodeHeader, argsSize) - 1;

	asmInitLabel(&invalidArgs);
	asmInitLabel(&loop);
	asmInitLabel(&zeroArgs);

	// prologue
	asmPushq(buffer, RBP);
	asmMovq(buffer, RSP, RBP);

	// save native code
	asmPushq(buffer, R11);
	generatePushDummyContext(buffer);

	// get native code for given method
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), RSI);
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 3 * sizeof(intptr_t)), RDI);
	generateCCall(generator, (intptr_t) scopedGetNativeCode, 2, 1);

	// load arguments array
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 4 * sizeof(intptr_t)), RDI);
	// load arguments array size
	asmMovqMem(buffer, asmMem(RDI, NO_REGISTER, SS_1, varOffset(RawArray, size)), RBX);
	// load compiled method
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), TMP);
	// check arguments size
	asmCmpbMem(buffer, asmMem(TMP, NO_REGISTER, SS_1, argsOffset), BL);
	asmJ(buffer, COND_NOT_EQUAL, &invalidArgs);

	// push arguments on stack
	asmTestq(buffer, RBX, RBX);
	asmJ(buffer, COND_ZERO, &zeroArgs);
	asmLabelBind(buffer, &loop, asmOffset(buffer));
	asmDecq(buffer, RBX);
	asmPushqMem(buffer, asmMem(RDI, RBX, SS_8, varOffset(RawArray, vars)));
	asmJ(buffer, COND_NOT_ZERO, &loop);
	asmLabelBind(buffer, &zeroArgs, asmOffset(buffer));

	// push receiver
	asmPushqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 3 * sizeof(intptr_t)));

	// invoke method
	asmMovq(buffer, RAX, R11);
	asmCallq(buffer, R11);
	generateStackmap(generator);

	// epilogue
	asmMovq(buffer, RBP, RSP);
	asmPopq(buffer, RBP);
	asmRet(buffer);

	asmLabelBind(buffer, &invalidArgs, asmOffset(buffer));
	asmMovq(buffer, RBP, RSP);
	asmPopq(buffer, RBP);
}


/*static void generateSmallIntCheck(buffer)
{
	asmMovq(buffer, RDI, TMP);
	asmOr(buffer, RSI, TMP);
	asmTestqRegImm(buffer, TMP, VALUE_INT);
	asmJ(buffer, COND_NO_ZERO, )
}*/


static void generateIntLessThanPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel less;
	AssemblerLabel notInt;
	asmInitLabel(&less);
	asmInitLabel(&notInt);

	movArg(buffer, 1, RSI);
	testInt(generator, SIL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	movArg(buffer, 0, RAX);
	asmCmpq(buffer, RAX, RSI);

	asmJ(buffer, COND_LESS, &less);
	generateLoadObject(buffer, Handles.false->raw, RAX, 1);
	asmRet(buffer);

	asmLabelBind(buffer, &less, asmOffset(buffer));
	generateLoadObject(buffer, Handles.true->raw, RAX, 1);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
}


static void generateIntAddPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	AssemblerLabel overflow;
	asmInitLabel(&notInt);
	asmInitLabel(&overflow);

	movArg(buffer, 1, RSI);
	testInt(generator, SIL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	movArg(buffer, 0, RAX);
	asmAddq(buffer, RSI, RAX);
	asmJ(buffer, COND_OVERFLOW, &overflow);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
	asmLabelBind(buffer, &overflow, asmOffset(buffer));
}


static void generateIntSubPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	AssemblerLabel overflow;
	asmInitLabel(&notInt);
	asmInitLabel(&overflow);

	movArg(buffer, 1, RSI);
	testInt(generator, SIL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	movArg(buffer, 0, RAX);
	asmSubq(buffer, RSI, RAX);
	asmJ(buffer, COND_OVERFLOW, &overflow);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
	asmLabelBind(buffer, &overflow, asmOffset(buffer));
}


static void generateIntMulPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	AssemblerLabel overflow;
	asmInitLabel(&notInt);
	asmInitLabel(&overflow);

	movArg(buffer, 1, RAX);
	testInt(generator, AL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	asmShrqImm(buffer, RAX, 2);
	asmImulqMem(buffer, arg(0), RAX);
	asmJ(buffer, COND_OVERFLOW, &overflow);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
	asmLabelBind(buffer, &overflow, asmOffset(buffer));
}


static void generateIntQuoPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	asmInitLabel(&notInt);

	movArg(buffer, 1, RSI);
	testInt(generator, SIL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	movArg(buffer, 0, RAX);
	asmCqo(buffer);
	asmIdivq(buffer, RSI);
	asmShlqImm(buffer, RAX, 2);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
}


static void generateIntModPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	AssemblerLabel negativeResult;
	AssemblerLabel negativeDivisor;
	asmInitLabel(&notInt);
	asmInitLabel(&negativeResult);
	asmInitLabel(&negativeDivisor);

	movArg(buffer, 1, RDI);
	testInt(generator, DIL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	movArg(buffer, 0, RAX);
	asmCqo(buffer);
	asmIdivq(buffer, RDI);
	asmMovq(buffer, RDX, RAX);

	asmXorq(buffer, RDI, RDX);
	asmCmpqImm(buffer, RDX, 0);
	asmJ(buffer, COND_LESS, &negativeResult);
	asmRet(buffer);

	// negative
	asmLabelBind(buffer, &negativeResult, asmOffset(buffer));
	asmLabelBind(buffer, &negativeDivisor, asmOffset(buffer));
	asmAddq(buffer, RDI, RAX);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
}


static void generateIntRemPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	asmInitLabel(&notInt);

	movArg(buffer, 1, RDI);
	testInt(generator, DIL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	movArg(buffer, 0, RAX);
	asmCqo(buffer);
	asmIdivq(buffer, RDI);
	asmMovq(buffer, RDX, RAX);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
}


static void generateIntNegPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel overflow;
	asmInitLabel(&overflow);

	movArg(buffer, 0, RAX);
	asmNegq(buffer, RAX);
	asmJ(buffer, COND_OVERFLOW, &overflow);
	asmRet(buffer);

	asmLabelBind(buffer, &overflow, asmOffset(buffer));
}


static void generateIntAndPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	asmInitLabel(&notInt);

	movArg(buffer, 1, RAX);
	testInt(generator, AL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	asmAndqMem(buffer, arg(0), RAX);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
}


static void generateIntOrPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	asmInitLabel(&notInt);

	movArg(buffer, 1, RAX);
	testInt(generator, AL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	asmOrqMem(buffer, arg(0), RAX);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
}


static void generateIntXorPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	asmInitLabel(&notInt);

	movArg(buffer, 1, RAX);
	testInt(generator, AL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	asmXorqMem(buffer, arg(0), RAX);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
}


static void generateIntShiftPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel notInt;
	AssemblerLabel rightShift;
	asmInitLabel(&notInt);
	asmInitLabel(&rightShift);

	movArg(buffer, 1, RSI);
	testInt(generator, SIL);
	asmJ(buffer, COND_NOT_ZERO, &notInt);

	movArg(buffer, 0, RAX);
	asmSarqImm(buffer, RSI, 2);
	asmMovb(buffer, SIL, CL);
	asmTestq(buffer, RSI, RSI);
	asmJ(buffer, COND_SIGN, &rightShift);
	asmShlq(buffer, RAX);
	asmRet(buffer);

	asmLabelBind(buffer, &rightShift, asmOffset(buffer));
	asmNegb(buffer, CL);
	asmShrq(buffer, RAX);
	asmAndqImm(buffer, RAX, ~3);
	asmRet(buffer);

	asmLabelBind(buffer, &notInt, asmOffset(buffer));
}


static void generateBlockValuePrimitive0Args(CodeGenerator *generator)
{
	generateBlockValuePrimitive(generator, 0);
}


static void generateBlockValuePrimitive1Args(CodeGenerator *generator)
{
	generateBlockValuePrimitive(generator, 1);
}


static void generateBlockValuePrimitive2Args(CodeGenerator *generator)
{
	generateBlockValuePrimitive(generator, 2);
}


static void generateBlockValuePrimitive3Args(CodeGenerator *generator)
{
	generateBlockValuePrimitive(generator, 3);
}


static void generateBlockValuePrimitive(CodeGenerator *generator, uint8_t args)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel invalidArgs;
	ptrdiff_t compiledCodeOffset = offsetof(NativeCode, compiledCode) - offsetof(NativeCode, insts);
	ptrdiff_t argsOffset = offsetof(RawCompiledBlock, header) + offsetof(CompiledCodeHeader, argsSize) - 1;
	uint8_t i;

	asmInitLabel(&invalidArgs);

	asmPushq(buffer, RBP);
	asmMovq(buffer, RSP, RBP);

	// save native code
	asmPushq(buffer, R11);
	// push current context
	asmPushq(buffer, CTX);

	// load compiled block
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), TMP);
	asmMovqMem(buffer, asmMem(TMP, NO_REGISTER, SS_1, varOffset(RawBlock, compiledBlock)), TMP);

	// check arguments size
	asmCmpbMemImm(buffer, asmMem(TMP, NO_REGISTER, SS_1, argsOffset), args);
	asmJ(buffer, COND_NOT_EQUAL, &invalidArgs);

	generateBlockContextAllocation(generator);

	// epilogue
	asmMovq(buffer, RBP, RSP);
	asmPopq(buffer, RBP);

	// replace receiver on stack
	asmMovqMem(buffer, asmMem(RDI, NO_REGISTER, SS_1, varOffset(RawBlock, receiver)), TMP);
	asmMovqToMem(buffer, TMP, arg(0));

	// call block
	asmMovqMem(buffer, asmMem(RDI, NO_REGISTER, SS_1, varOffset(RawBlock, nativeCode)), R11);
	asmAddqImm(buffer, R11, offsetof(NativeCode, insts));
	asmJmpq(buffer, R11);

	asmLabelBind(buffer, &invalidArgs, asmOffset(buffer));

	// epilogue
	asmMovq(buffer, RBP, RSP);
	asmPopq(buffer, RBP);
}


static void generateBlockValueArgsPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel invalidArgs;
	AssemblerLabel loop;
	AssemblerLabel zeroArgs;
	ptrdiff_t compiledCodeOffset = offsetof(NativeCode, compiledCode) - offsetof(NativeCode, insts);
	ptrdiff_t argsOffset = offsetof(RawCompiledBlock, header) + offsetof(CompiledCodeHeader, argsSize) - 1;
	uint8_t i;

	asmInitLabel(&invalidArgs);
	asmInitLabel(&loop);
	asmInitLabel(&zeroArgs);

	asmPushq(buffer, RBP);
	asmMovq(buffer, RSP, RBP);

	// save native code
	asmPushq(buffer, R11);
	// push current context
	asmPushq(buffer, CTX);

	// load block and arguments array
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), R13);
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 3 * sizeof(intptr_t)), RSI);

	// load compiled block
	asmMovqMem(buffer, asmMem(R13, NO_REGISTER, SS_1, varOffset(RawBlock, compiledBlock)), TMP);

	// load array arguments size
	asmMovqMem(buffer, asmMem(RSI, NO_REGISTER, SS_1, varOffset(RawArray, size)), RBX);

	// check arguments size
	asmCmpbMem(buffer, asmMem(TMP, NO_REGISTER, SS_1, argsOffset), BL);
	asmJ(buffer, COND_NOT_EQUAL, &invalidArgs);

	// push arguments on stack
	asmTestq(buffer, RBX, RBX);
	asmJ(buffer, COND_ZERO, &zeroArgs);
	asmLabelBind(buffer, &loop, asmOffset(buffer));
	asmDecq(buffer, RBX);
	asmPushqMem(buffer, asmMem(RSI, RBX, SS_8, varOffset(RawArray, vars)));
	asmJ(buffer, COND_NOT_ZERO, &loop);
	asmLabelBind(buffer, &zeroArgs, asmOffset(buffer));

	// push receiver
	asmPushqMem(buffer, asmMem(R13, NO_REGISTER, SS_1, varOffset(RawBlock, receiver)));

	generateBlockContextAllocation(generator);

	// call block
	asmMovqMem(buffer, asmMem(R13, NO_REGISTER, SS_1, varOffset(RawBlock, nativeCode)), RAX);
	asmAddqImm(buffer, RAX, offsetof(NativeCode, insts));
	asmCallq(buffer, RAX);

	// epilogue
	asmMovq(buffer, RBP, RSP);
	asmPopq(buffer, RBP);
	asmRet(buffer);

	asmLabelBind(buffer, &invalidArgs, asmOffset(buffer));
	asmMovq(buffer, RBP, RSP);
	asmPopq(buffer, RBP);
}


static void generateBlockWhileTrue(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel loop;
	AssemblerLabel notBoolean;
	asmInitLabel(&loop);
	asmInitLabel(&notBoolean);

	asmPushq(buffer, RBP);
	asmMovq(buffer, RSP, RBP);

	// save native code
	asmPushq(buffer, R11);
	generatePushDummyContext(buffer);

	generateLoadObject(buffer, (RawObject *) Handles.Block->raw, RDI, 1);
	generateLoadObject(buffer, (RawObject *) Handles.valueSymbol->raw, RSI, 0);
	generateMethodLookup(generator);

	asmPushq(buffer, R11);
	asmSubqImm(buffer, RSP, sizeof(intptr_t));

	// value block
	asmLabelBind(buffer, &loop, asmOffset(buffer));
	// copy receiver in loop again as #blockValue replaces it
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), TMP);
	asmMovqToMem(buffer, TMP, asmMem(RSP, NO_REGISTER, SS_1, 0));
	// call block native code
	asmMovqMem(buffer, asmMem(RSP, NO_REGISTER, SS_1, sizeof(intptr_t)), R11);
	asmCallq(buffer, R11);
	generateStackmap(generator);

	// repeat if result is true
	generateLoadObject(buffer, Handles.true->raw, TMP, 1);
	asmCmpq(buffer, RAX, TMP);
	asmJ(buffer, COND_EQUAL, &loop);

	// check if result is false
	generateLoadObject(buffer, Handles.false->raw, TMP, 1);
	asmCmpq(buffer, RAX, TMP);
	asmJ(buffer, COND_NOT_EQUAL, &notBoolean);

	// return receiver
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), RAX);
	asmAddqImm(buffer, RSP, 4 * sizeof(intptr_t));
	asmPopq(buffer, RBP);
	asmRet(buffer);

	// not boolean
	asmLabelBind(buffer, &notBoolean, asmOffset(buffer));
	asmAddqImm(buffer, RSP, 3 * sizeof(intptr_t));
	asmPopq(buffer, R11);
	asmPopq(buffer, RBP);
}


static void generateBlockWhileTrue2(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	ptrdiff_t compiledCodeOffset = offsetof(NativeCode, compiledCode) - offsetof(NativeCode, insts);
	AssemblerLabel loop;
	AssemblerLabel end;
	AssemblerLabel notBoolean;
	asmInitLabel(&loop);
	asmInitLabel(&end);
	asmInitLabel(&notBoolean);

	asmPushq(buffer, RBP);
	asmMovq(buffer, RSP, RBP);

	// save native code
	asmPushq(buffer, R11);
	generatePushDummyContext(buffer);

	generateLoadObject(buffer, (RawObject *) Handles.Block->raw, RDI, 1);
	generateLoadObject(buffer, (RawObject *) Handles.valueSymbol->raw, RSI, 0);
	generateMethodLookup(generator);
	asmPushq(buffer, R11);

	// value block
	asmLabelBind(buffer, &loop, asmOffset(buffer));
	asmPushqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)));
	asmMovqMem(buffer, asmMem(RSP, NO_REGISTER, SS_1, sizeof(intptr_t)), R11);
	asmCallq(buffer, R11);
	generateStackmap(generator);
	asmAddqImm(buffer, RSP, sizeof(intptr_t));

	// repeat if result is true
	generateLoadObject(buffer, Handles.true->raw, TMP, 1);
	asmCmpq(buffer, RAX, TMP);
	asmJ(buffer, COND_NOT_EQUAL, &end);

	// value second block
	asmPushqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 3 * sizeof(intptr_t)));
	asmMovqMem(buffer, asmMem(RSP, NO_REGISTER, SS_1, sizeof(intptr_t)), R11);
	asmCallq(buffer, R11);
	generateStackmap(generator);
	asmAddqImm(buffer, RSP, sizeof(intptr_t));
	asmJmpLabel(buffer, &loop);

	// result is not true
	asmLabelBind(buffer, &end, asmOffset(buffer));

	// check if result is false
	generateLoadObject(buffer, Handles.false->raw, TMP, 1);
	asmCmpq(buffer, RAX, TMP);
	asmJ(buffer, COND_NOT_EQUAL, &notBoolean);

	// return receiver
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), RAX);
	asmAddqImm(buffer, RSP, 3 * sizeof(intptr_t));
	asmPopq(buffer, RBP);
	asmRet(buffer);

	// not boolean
	asmLabelBind(buffer, &notBoolean, asmOffset(buffer));
	asmAddqImm(buffer, RSP, 2 * sizeof(intptr_t));
	asmPopq(buffer, R11);
	asmPopq(buffer, RBP);
}


static void generateBlockOnExceptionPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	ptrdiff_t compiledCodeOffset = offsetof(NativeCode, compiledCode) - offsetof(NativeCode, insts);
	AssemblerFixup *ip;
	generator->frameSize = 2;

	// prologue
	asmPushq(buffer, RBP);
	asmMovq(buffer, RSP, RBP);
	asmSubqImm(buffer, RSP, generator->frameSize * sizeof(intptr_t));

	// save native code
	asmMovqToMem(buffer, R11, asmMem(RBP, NO_REGISTER, SS_1, -sizeof(intptr_t)));
	generateMethodContextAllocation(generator, 0);

	// allocate exception handler
	generateLoadObject(buffer, (RawObject *) Handles.ExceptionHandler->raw, RSI, 0);
	asmXorq(buffer, RDX, RDX);
	generateStubCall(generator, &AllocateStub);
	// setup return IP
	asmMovqImm(buffer, 0, TMP);
	ip = asmEmitFixup(buffer, ASM_FIXUP_IP, sizeof(intptr_t), asmOffset(buffer) - sizeof(intptr_t));
	asmMovqToMem(buffer, TMP, asmMem(RAX, NO_REGISTER, SS_1, varOffset(RawExceptionHandler, ip)));
	// setup context exception handler
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, -2 * sizeof(intptr_t)), R9);
	asmMovqToMem(buffer, R9, asmMem(RAX, NO_REGISTER, SS_1, varOffset(RawExceptionHandler, context)));
	// spill exception handler
	asmPushq(buffer, RAX);
	generator->frameSize++;

	// install exception handler
	asmMovqImm(buffer, (int64_t) &CurrentExceptionHandler, RDI);
	asmMovqMem(buffer, asmMem(RDI, NO_REGISTER, SS_1, 0), TMP);
	asmMovqToMem(buffer, TMP, asmMem(RAX, NO_REGISTER, SS_1, varOffset(RawExceptionHandler, parent)));
	asmMovqToMem(buffer, RAX, asmMem(RDI, NO_REGISTER, SS_1, 0));

	// value block
	generateLoadObject(buffer, (RawObject *) Handles.Block->raw, RDI, 1);
	generateLoadObject(buffer, (RawObject *) Handles.valueSymbol->raw, RSI, 0);
	asmPushqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)));
	generator->frameSize++;
	generateMethodLookup(generator);
	generator->frameSize--;
	asmCallq(buffer, R11);
	generateStackmap(generator);

	// unregister exception handler
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, -3 * sizeof(intptr_t)), TMP);
	asmMovqMem(buffer, asmMem(TMP, NO_REGISTER, SS_1, varOffset(RawExceptionHandler, parent)), TMP);
	asmMovqImm(buffer, (int64_t) &CurrentExceptionHandler, RDI);
	asmMovqToMem(buffer, TMP, asmMem(RDI, NO_REGISTER, SS_1, 0));

	// epilogue
	asmMovqMem(&generator->buffer, asmMem(CTX, NO_REGISTER, SS_1, varOffset(RawContext, parent)), CTX);
	asmAddqImm(buffer, RSP, 4 * sizeof(intptr_t));
	asmPopq(buffer, RBP);
	asmRet(buffer);

	// jumped from exception signal
	// RBP, RSP are restored by exception signal
	ip->value = asmOffset(buffer) - ip->offset;
	generator->frameSize = 5; // native code + context + backtrace + exception + block

	// restore context
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, -2 * sizeof(intptr_t)), CTX);

	// value exception block
	asmPushqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 4 * sizeof(intptr_t)));
	generateLoadObject(buffer, (RawObject *) Handles.Block->raw, RDI, 1);
	generateMethodLookup(generator);
	generator->frameSize -= 3;
	asmCallq(buffer, R11);
	generateStackmap(generator);

	// epilogue
	asmMovqMem(&generator->buffer, asmMem(CTX, NO_REGISTER, SS_1, varOffset(RawContext, parent)), CTX);
	asmAddqImm(buffer, RSP, 5 * sizeof(intptr_t));
	asmPopq(buffer, RBP);
	asmRet(buffer);
}


static void generateExceptionSignalPrimitive(CodeGenerator *generator)
{
	AssemblerBuffer *buffer = &generator->buffer;
	AssemblerLabel handlerNotFound;
	AssemblerLabel skipBacktrace;
	asmInitLabel(&handlerNotFound);
	asmInitLabel(&skipBacktrace);

	// prologue
	asmPushq(buffer, RBP);
	asmMovq(buffer, RSP, RBP);

	// save native code
	asmPushq(buffer, R11);
	generatePushDummyContext(buffer);

	asmMovq(buffer, R11, R13);

	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), RDI);
	asmDecq(buffer, RDI);

	generateCCall(generator, (intptr_t) unwindExceptionHandler, 1, 1);
	asmTestq(buffer, RAX, RAX);
	asmJ(buffer, COND_ZERO, &handlerNotFound);

	// load context
	asmMovqMem(buffer, asmMem(RAX, NO_REGISTER, SS_1, varOffset(RawExceptionHandler, context)), CTX);
	// load handler frame
	asmMovqMem(buffer, asmMem(CTX, NO_REGISTER, SS_1, varOffset(RawContext, frame)), TMP);
	// load handler block
	asmMovqMem(buffer, asmMem(TMP, NO_REGISTER, SS_1, 4 * sizeof(intptr_t)), TMP);
	// load compiled block
	asmMovqMem(buffer, asmMem(TMP, NO_REGISTER, SS_1, varOffset(RawBlock, compiledBlock)), TMP);
	// check if block accepts backtrace in second argument
	ptrdiff_t argsSizeOffset = varOffset(RawCompiledBlock, header) + offsetof(CompiledCodeHeader, argsSize);
	asmCmpbMemImm(buffer, asmMem(TMP, NO_REGISTER, SS_1, argsSizeOffset), 2);
	asmJ(buffer, COND_LESS, &skipBacktrace);

	asmPushq(buffer, RAX);
	generator->frameSize++;

	// generate backtrace
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), TMP);
	asmPushq(buffer, TMP);
	generator->frameSize++;
	generateLoadClass(buffer, TMP, RDI);
	generateLoadObject(buffer, (RawObject *) Handles.generateBacktraceSymbol->raw, RSI, 0);
	generateMethodLookup(generator);
	generator->frameSize--;
	asmCallq(buffer, R11);
	generateStackmap(generator);

	asmAddqImm(buffer, RSP, sizeof(intptr_t));
	asmPopq(buffer, RDI);
	generator->frameSize--;

	// load signaled exception as this stack frame is later destroyed
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), R9);
	// load context
	asmMovqMem(buffer, asmMem(RDI, NO_REGISTER, SS_1, varOffset(RawExceptionHandler, context)), CTX);
	// restore SP and BP
	asmMovqMem(buffer, asmMem(CTX, NO_REGISTER, SS_1, varOffset(RawContext, frame)), RBP);
	asmLeaq(buffer, asmMem(RBP, NO_REGISTER, SS_1, -2 * sizeof(intptr_t)), RSP);
	asmPushq(buffer, RAX); // push generated backtrace
	asmPushq(buffer, R9); // push signaled exception
	// load #value:value: as backtrace is not passed to the handler block
	generateLoadObject(buffer, (RawObject *) Handles.valueValueSymbol->raw, RSI, 0);
	// jump to handler
	asmMovqMem(buffer, asmMem(RDI, NO_REGISTER, SS_1, varOffset(RawExceptionHandler, ip)), TMP);
	asmJmpq(buffer, TMP);

	asmLabelBind(buffer, &skipBacktrace, asmOffset(buffer));

	// load signaled exception as this stack frame is later destroyed
	asmMovqMem(buffer, asmMem(RBP, NO_REGISTER, SS_1, 2 * sizeof(intptr_t)), R9);
	// restore SP and BP
	asmMovqMem(buffer, asmMem(CTX, NO_REGISTER, SS_1, varOffset(RawContext, frame)), RBP);
	asmLeaq(buffer, asmMem(RBP, NO_REGISTER, SS_1, -2 * sizeof(intptr_t)), RSP);
	asmPushq(buffer, R9); // not used argument
	asmPushq(buffer, R9); // push signaled exception
	// load #value: as backtrace is not passed to the handler block
	generateLoadObject(buffer, (RawObject *) Handles.value_Symbol->raw, RSI, 0);
	// jump to handler
	asmMovqMem(buffer, asmMem(RAX, NO_REGISTER, SS_1, varOffset(RawExceptionHandler, ip)), TMP);
	asmJmpq(buffer, TMP);

	// return to smalltalk code
	asmLabelBind(buffer, &handlerNotFound, asmOffset(buffer));
	asmAddqImm(buffer, RSP, 2 * sizeof(intptr_t));
	asmPopq(buffer, RBP);
	asmMovq(buffer, R13, R11);
}


static PrimitiveResult contextParentPrimitive(Value vContext)
{
	RawContext *context = (RawContext *) asObject(vContext);
	if (!contextHasValidFrame(context)) {
		return primSuccess(getTaggedPtr(Handles.nil));
	}

	StackFrame *frame = context->frame;
	RawContext *parent = stackFrameGetParentContext(frame);

	if (parent == NULL) {
		return primSuccess(getTaggedPtr(Handles.nil));
	} else if (parent->class == Handles.MethodContext->raw || parent->class == Handles.BlockContext->raw) {
		return primSuccess(tagPtr(parent));
	} else {
		return primFailed();
	}
}


static PrimitiveResult contextArgumentAt(Value vContext, Value vIndex)
{
	RawContext *context = (RawContext *) asObject(vContext);
	if (!contextHasValidFrame(context)) {
		return primSuccess(getTaggedPtr(Handles.nil));
	}
	RawCompiledMethod *code = (RawCompiledMethod *) asObject(context->code);
	intptr_t index = asCInt(vIndex);
	if (index < 0 || index > code->header.argsSize) {
		return primFailed();
	}
	return primSuccess(stackFrameGetArg(context->frame, index));
}


static PrimitiveResult contextTemporaryAt(Value vContext, Value vIndex)
{
	RawContext *context = (RawContext *) asObject(vContext);
	if (!contextHasValidFrame(context)) {
		return primSuccess(getTaggedPtr(Handles.nil));
	}
	RawCompiledMethod *code = (RawCompiledMethod *) asObject(context->code);
	intptr_t index = asCInt(vIndex);
	if (index < context->size) {
		return primSuccess(context->vars[index]);
	} else if (context->ic == getTaggedPtr(Handles.nil)) {
		return primSuccess(getTaggedPtr(Handles.nil));
	} else {
		RawStackmap *stackmap = findStackmap(code->nativeCode, (ptrdiff_t) asCInt(context->ic));
		index = index - context->size + FRAME_VARS_OFFSET - 1;
		if (stackmapIncludes(stackmap, index)) {
			return primSuccess(stackFrameGetSlot(context->frame, index));
		} else {
			return primSuccess(getTaggedPtr(Handles.nil));
		}
	}
	return primFailed();
}


static void movArg(AssemblerBuffer *buffer, ptrdiff_t index, Register dst)
{
	asmMovqMem(buffer, arg(index), dst);
}


static MemoryOperand arg(ptrdiff_t index)
{
	return asmMem(RSP, NO_REGISTER, SS_1, (index + 1) * sizeof(intptr_t));
}
