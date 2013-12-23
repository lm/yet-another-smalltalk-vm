#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "Assert.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#define ASM_BUFFER_GAP 32

typedef struct {
	uint8_t argRegsSize;
	uint8_t *argRegs;
	uint8_t regsSize;
	uint8_t *regs;
} AvailableRegs;

typedef struct {
	_Bool isBound;
	_Bool isResolved;
	ptrdiff_t offset;
	size_t size;
} AssemblerLabel;

typedef enum {
	ASM_FIXUP_IP,
} AssemblerFixupType;

typedef struct {
	ptrdiff_t offset;
	size_t size;
	AssemblerFixupType type;
	int64_t value;
} AssemblerFixup;

typedef struct {
	uint8_t *buffer;
	uint8_t *end;
	uint8_t *p;
	ptrdiff_t instOffset;
	AssemblerFixup fixups[8];
	uint8_t fixupsSize;
	uint16_t pointersOffsets[1024]; // TODO: get rid of fixed size buffer?
	size_t pointersOffsetsSize;
} AssemblerBuffer;

static void asmInitBuffer(AssemblerBuffer *buffer, size_t size);
static void asmFreeBuffer(AssemblerBuffer *buffer);
static void asmCopyBuffer(AssemblerBuffer *buffer, uint8_t *dest, size_t size);
static ptrdiff_t asmOffset(AssemblerBuffer *buffer);
static void asmInitLabel(AssemblerLabel *label);
static void asmEmitLabel32(AssemblerBuffer *buffer, AssemblerLabel *label);
static void asmLabelBind(AssemblerBuffer *buffer, AssemblerLabel *label, ptrdiff_t offset);
static AssemblerFixup *asmEmitFixup(AssemblerBuffer *buffer, AssemblerFixupType type, size_t size, ptrdiff_t offset);
static void asmBindFixups(AssemblerBuffer *buffer, uint8_t *p);
static void asmAddPointerOffset(AssemblerBuffer *buffer, ptrdiff_t offset);
static void asmCopyPointersOffsets(AssemblerBuffer *buffer, uint16_t *dest);
static void asmEmitInt8(AssemblerBuffer *buffer, int8_t v);
static void asmEmitUint8(AssemblerBuffer *buffer, uint8_t v);
static void asmEmitInt32(AssemblerBuffer *buffer, int32_t v);
static void asmEmitUint64(AssemblerBuffer *buffer, uint64_t v);

static void asmEnsureCapacity(AssemblerBuffer *buffer);
static void asmBindFixup(AssemblerBuffer *buffer, AssemblerFixup *fixup, int64_t value);


static void asmInitBuffer(AssemblerBuffer *buffer, size_t size)
{
	buffer->buffer = malloc(size + ASM_BUFFER_GAP);
	buffer->end = buffer->buffer + size;
	buffer->p = buffer->buffer;
	buffer->instOffset = 0;
	buffer->fixupsSize = 0;
	buffer->pointersOffsetsSize = 0;
}


static void asmFreeBuffer(AssemblerBuffer *buffer)
{
	free(buffer->buffer);
}


static void asmCopyBuffer(AssemblerBuffer *buffer, uint8_t *dest, size_t size)
{
	memcpy(dest, buffer->buffer, size);
}


static ptrdiff_t asmOffset(AssemblerBuffer *buffer)
{
	return buffer->p - buffer->buffer;
}


static void asmInitLabel(AssemblerLabel *label)
{
	label->isBound = 0;
	label->isResolved = 0;
}


static void asmEmitLabel32(AssemblerBuffer *buffer, AssemblerLabel *label)
{
	ASSERT(!label->isResolved);
	asmEnsureCapacity(buffer);
	label->isResolved = 1;
	if (label->isBound) {
		asmEmitInt32(buffer, label->offset - asmOffset(buffer) - sizeof(int32_t));
	} else {
		label->offset = asmOffset(buffer);
		label->size = sizeof(int32_t);
		asmEmitInt32(buffer, 0);
	}
}


static void asmLabelBind(AssemblerBuffer *buffer, AssemblerLabel *label, ptrdiff_t offset)
{
	ASSERT(!label->isBound);
	label->isBound = 1;

	if (label->isResolved) {
		switch (label->size) {
		case 1: {
			uint8_t *p = buffer->buffer + label->offset;
			*p = offset - label->offset - label->size;
			break;
		}
		case 4: {
			uint32_t *p = (uint32_t *) (buffer->buffer + label->offset);
			*p = offset - label->offset - label->size;
			break;
		}
		case 8: {
			uint64_t *p = (uint64_t *) (buffer->buffer + label->offset);
			*p = offset - label->offset - label->size;
			break;
		}
		default:
			FAIL();
		}
	} else {
		label->offset = offset;
	}
}


static AssemblerFixup *asmEmitFixup(AssemblerBuffer *buffer, AssemblerFixupType type, size_t size, ptrdiff_t offset)
{
	ASSERT(buffer->fixupsSize < 8);
	AssemblerFixup *fixup = buffer->fixups + buffer->fixupsSize++;
	fixup->type = type;
	fixup->offset = offset;
	fixup->size = size;
	return fixup;
}


static void asmBindFixups(AssemblerBuffer *buffer, uint8_t *p)
{
	AssemblerFixup *fixup;
	for (uint8_t i = 0; i < buffer->fixupsSize; i++) {
		fixup = buffer->fixups + i;
		switch (fixup->type) {
		case ASM_FIXUP_IP:
			asmBindFixup(buffer, fixup, (intptr_t) p + fixup->offset + fixup->value);
			break;
		default:
			FAIL();
		}
	}
}


static void asmBindFixup(AssemblerBuffer *buffer, AssemblerFixup *fixup, int64_t value)
{
	int8_t *p = (int8_t *) buffer->buffer + fixup->offset;
	switch (fixup->size) {
	case 1:
		ASSERT(INT8_MIN <= value && value <= INT8_MAX);
		*p += value;
		break;
	case 4:
		ASSERT(INT32_MIN <= value && value <= INT32_MAX);
		*(int32_t *) p = value;
		break;
	case 8:
		ASSERT(INT64_MIN <= value && value <= INT64_MAX);
		*(int64_t *) p = value;
		break;
	default:
		FAIL();
	}
}


static void asmAddPointerOffset(AssemblerBuffer *buffer, ptrdiff_t offset)
{
	ASSERT(INT16_MIN <= offset && offset <= INT16_MAX);
	ASSERT(buffer->pointersOffsetsSize < 1024);
	buffer->pointersOffsets[buffer->pointersOffsetsSize++] = offset;
}


static void asmCopyPointersOffsets(AssemblerBuffer *buffer, uint16_t *dest)
{
	memcpy(dest, buffer->pointersOffsets, buffer->pointersOffsetsSize * sizeof(*dest));
}


static void asmEmitInt8(AssemblerBuffer *buffer, int8_t v)
{
	*(int8_t *) buffer->p = v;
	buffer->p++;
}


static void asmEmitUint8(AssemblerBuffer *buffer, uint8_t v)
{
	*(uint8_t *) buffer->p = v;
	buffer->p++;
}


static void asmEmitInt32(AssemblerBuffer *buffer, int32_t v)
{
	*(int32_t *) buffer->p = v;
	buffer->p += sizeof(int32_t);
}


static void asmEmitUint64(AssemblerBuffer *buffer, uint64_t v)
{
	*(uint64_t *) buffer->p = v;
	buffer->p += sizeof(uint64_t);
}


static void asmEnsureCapacity(AssemblerBuffer *buffer)
{
	if (buffer->p >= buffer->end) {
		size_t size = (buffer->end - buffer->buffer + ASM_BUFFER_GAP) * 2;
		size_t pos = buffer->p - buffer->buffer;

		buffer->buffer = realloc(buffer->buffer, size);
		if (buffer->buffer == NULL) {
			FAIL();
		}
		buffer->end = buffer->buffer + size - ASM_BUFFER_GAP;
		buffer->p = buffer->buffer + pos;
	}
}

#endif
