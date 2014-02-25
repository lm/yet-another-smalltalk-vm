#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "Object.h"
#include "CompiledCode.h"
#include "Assembler.h"
#include "AssemblerX64.h"
#include "Lookup.h"
#include "String.h"
#include "RegisterAllocator.h"

typedef struct {
	CompiledCode code;
	AssemblerBuffer buffer;
	size_t frameSize;
	size_t frameRawAreaSize;
	RegsAlloc regsAlloc;
	uint8_t tmpVar;
	ptrdiff_t bytecodeNumber;
	OrderedCollection *stackmaps;
	OrderedCollection *descriptors;
} CodeGenerator;

NativeCode *generateMethodCode(CompiledMethod *method);
void generateLoadObject(AssemblerBuffer *buffer, RawObject *object, Register dst, _Bool tag);
void generateLoadClass(AssemblerBuffer *buffer, Register src, Register dst);
void generateStoreCheck(CodeGenerator *generator, Register object, Register value);
void generateMethodLookup(CodeGenerator *generator);
void generateStackmap(CodeGenerator *generator);
void generateCCall(CodeGenerator *generator, intptr_t cFunction, size_t argsSize, _Bool storeIp);
void generateMethodContextAllocation(CodeGenerator *generator, size_t size);
void generateBlockContextAllocation(CodeGenerator *generator);
void generatePushDummyContext(AssemblerBuffer *buffer);
NativeCode *generateDoesNotUnderstand(String *selector);
NativeCode *buildNativeCode(CodeGenerator *generator);
NativeCode *buildNativeCodeFromAssembler(AssemblerBuffer *buffer);

#endif
