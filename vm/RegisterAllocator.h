#ifndef REGISTER_ALLOCATOR_H
#define REGISTER_ALLOCATOR_H

#include "CompiledCode.h"
#include "Bytecodes.h"
#include "Assembler.h"

#define SPILLED_REG -1

typedef enum {
	VAR_CONTEXT = 0,
	VAR_CLASS = 1,
	VAR_ASSOC = 2,
	VAR_TMP = 3,
} VariableType;

typedef enum {
	VAR_DEFINED = 1,
	VAR_IN_REG = 1 << 1,
	VAR_ON_STACK = 1 << 2,
} VariableFlags;

typedef struct {
	uint8_t index;
	uint8_t type;
	VariableFlags flags;
	int8_t reg;
	size_t start;
	size_t end;
	ptrdiff_t frameOffset;
} Variable;

typedef struct {
	AvailableRegs *regs;
	uint8_t varsSize;
	Variable vars[256];
	Variable *specialVars[3][256];
	size_t frameSize;
	_Bool frameLess;
} RegsAlloc;

void computeRegsAlloc(RegsAlloc *alloc, AvailableRegs *regs, CompiledCode *code);
void invalidateRegs(RegsAlloc *alloc);

#endif
