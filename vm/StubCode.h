#ifndef STUBCODE_H
#define STUBCODE_H

#include "CompiledCode.h"
#include "CodeGenerator.h"
#include "Assembler.h"

typedef struct {
	void (*generator)(CodeGenerator *generator);
	NativeCode *nativeCode;
} StubCode;

extern StubCode SmalltalkEntry;
extern StubCode AllocateStub;
extern StubCode LookupStub;
extern StubCode DoesNotUnderstandStub;

NativeCode *getStubNativeCode(StubCode *stub);
void generateStubCall(CodeGenerator *generator, StubCode *stubCode);

#endif
