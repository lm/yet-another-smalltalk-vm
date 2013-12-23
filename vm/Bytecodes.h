#ifndef BYTECODES_H
#define BYTECODES_H

#include "Object.h"
#include "Assembler.h"
#include "Assert.h"
#include <stdlib.h>
#include <stdint.h>

typedef enum {
	BYTECODE_COPY, // destination:op, source:op
	BYTECODE_SEND, // selector:literal, noOfArgs:byte, receiver:op, arg:op[0..noOfArgs]
	BYTECODE_SEND_WITH_STORE, // selector:literal, noOfArgs:byte, receiver:op, arg:op[0..noOfArgs], result:op
	BYTECODE_RETURN, // source:op
	BYTECODE_OUTER_RETURN, // source:op
	BYTECODE_JUMP, // target:int32
	BYTECODE_JUMP_NOT_MEMBER_OF // class:literal, arg:op, target:int32
} Bytecode;

typedef enum {
	OPERAND_VALUE, // 64b int
	OPERAND_NIL,
	OPERAND_TRUE,
	OPERAND_FALSE,
	OPERAND_THIS_CONTEXT,
	OPERAND_TEMP_VAR, // index
	OPERAND_ARG_VAR, // index
	OPERAND_SUPER,
	OPERAND_CONTEXT_VAR, // index level
	OPERAND_INST_VAR, // index
	OPERAND_INST_VAR_OF, // index operandType index [level]
	OPERAND_LITERAL, // index
	OPERAND_ASSOC, // index
	OPERAND_BLOCK, // index
} OperandType;

typedef struct {
	_Bool isValid;
	OperandType type;
	union {
		struct {
			uint8_t index;
			uint8_t level;
			struct {
				OperandType type;
				uint8_t index;
				uint8_t level;
			} instance;
		};
		int64_t int64;
		uint64_t uint64;
		Value value;
		// double
	};
} Operand;

typedef struct {
	uint8_t *p;
	uint8_t *start;
	uint8_t *end;
	ptrdiff_t bytecodeNumber;
} BytecodesIterator;

static void bytecodeCopy(AssemblerBuffer *buffer, Operand *source, Operand *dest);
static void bytecodeSend(AssemblerBuffer *buffer, uint8_t selector, Operand *receiver, Operand *args, uint8_t numArgs);
static void bytecodeSendWithStore(AssemblerBuffer *buffer, uint8_t selector, Operand *receiver, Operand *result, Operand *args, uint8_t numArgs);
static void bytecodeReturn(AssemblerBuffer *buffer, Operand *operand, _Bool outer);
static void bytecodeJump(AssemblerBuffer *buffer, AssemblerLabel *label);
static void bytecodeJumpNotMemberOf(AssemblerBuffer *buffer, Operand *operand, uint8_t class, AssemblerLabel *label);
static void bytecodeOperand(AssemblerBuffer *buffer, Operand *operand);

static void bytecodeInitIterator(BytecodesIterator *iterator, uint8_t *bytecodes, size_t size);
static ptrdiff_t bytecodeNumber(BytecodesIterator *iterator);
static ptrdiff_t bytecodeOffset(BytecodesIterator *iterator);
static Bytecode bytecodeNext(BytecodesIterator *iterator);
static Operand bytecodeNextOperand(BytecodesIterator *iterator);
static uint8_t bytecodeNextByte(BytecodesIterator *iterator);
static _Bool bytecodeHasNext(BytecodesIterator *iterator);


static void bytecodeCopy(AssemblerBuffer *buffer, Operand *source, Operand *dest)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, BYTECODE_COPY);
	bytecodeOperand(buffer, source);
	ASSERT(dest->type == OPERAND_TEMP_VAR || dest->type == OPERAND_CONTEXT_VAR || dest->type == OPERAND_INST_VAR || dest->type == OPERAND_ASSOC);
	bytecodeOperand(buffer, dest);
	buffer->instOffset++;
}


static void bytecodeSend(AssemblerBuffer *buffer, uint8_t selector, Operand *receiver, Operand *args, uint8_t numArgs)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, BYTECODE_SEND);
	asmEmitUint8(buffer, selector);
	asmEmitUint8(buffer, numArgs);
	bytecodeOperand(buffer, receiver);
	for (ptrdiff_t i = numArgs - 1; i >= 0; i--) {
		bytecodeOperand(buffer, args + i);
	}
	buffer->instOffset++;
}


static void bytecodeSendWithStore(AssemblerBuffer *buffer, uint8_t selector, Operand *receiver, Operand *result, Operand *args, uint8_t numArgs)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, BYTECODE_SEND_WITH_STORE);
	asmEmitUint8(buffer, selector);
	asmEmitUint8(buffer, numArgs);
	bytecodeOperand(buffer, receiver);
	for (ptrdiff_t i = numArgs - 1; i >= 0; i--) {
		bytecodeOperand(buffer, args + i);
	}
	bytecodeOperand(buffer, result);
	buffer->instOffset++;
}


static void bytecodeReturn(AssemblerBuffer *buffer, Operand *operand, _Bool outer)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, outer ? BYTECODE_OUTER_RETURN : BYTECODE_RETURN);
	bytecodeOperand(buffer, operand);
	buffer->instOffset++;
}


static void bytecodeJump(AssemblerBuffer *buffer, AssemblerLabel *label)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, BYTECODE_JUMP);
	asmEmitLabel32(buffer, label);
	buffer->instOffset++;
}


static void bytecodeJumpNotMemberOf(AssemblerBuffer *buffer, Operand *operand, uint8_t class, AssemblerLabel *label)
{
	asmEnsureCapacity(buffer);
	asmEmitUint8(buffer, BYTECODE_JUMP_NOT_MEMBER_OF);
	asmEmitUint8(buffer, class);
	bytecodeOperand(buffer, operand);
	asmEmitLabel32(buffer, label);
	buffer->instOffset++;
}


static void bytecodeOperand(AssemblerBuffer *buffer, Operand *operand)
{
	switch (operand->type) {
	case OPERAND_VALUE:
		asmEmitUint8(buffer, operand->type);
		*((Value *) buffer->p) = operand->value;
		buffer->p += sizeof(operand->value);
		break;

	case OPERAND_TEMP_VAR:
	case OPERAND_ARG_VAR:
	case OPERAND_INST_VAR:
	case OPERAND_LITERAL:
	case OPERAND_ASSOC:
	case OPERAND_BLOCK:
		asmEmitUint8(buffer, operand->type);
		asmEmitUint8(buffer, operand->index);
		break;

	case OPERAND_INST_VAR_OF:
		asmEmitUint8(buffer, operand->type);
		asmEmitUint8(buffer, operand->index);
		ASSERT(operand->instance.type != OPERAND_VALUE)
		ASSERT(operand->instance.type != OPERAND_INST_VAR);
		ASSERT(operand->instance.type != OPERAND_INST_VAR_OF);
		bytecodeOperand(buffer, (Operand *) &operand->instance);
		break;

	case OPERAND_CONTEXT_VAR:
		asmEmitUint8(buffer, operand->type);
		asmEmitUint8(buffer, operand->index);
		asmEmitUint8(buffer, operand->level);
		break;

	default:
		asmEmitUint8(buffer, operand->type);
	}
}


static void bytecodeInitIterator(BytecodesIterator *iterator, uint8_t *bytecodes, size_t size)
{
	iterator->p = bytecodes;
	iterator->start = bytecodes;
	iterator->end = bytecodes + size;
	iterator->bytecodeNumber = -1;
}


static ptrdiff_t bytecodeNumber(BytecodesIterator *iterator)
{
	return iterator->bytecodeNumber;
}


static ptrdiff_t bytecodeOffset(BytecodesIterator *iterator)
{
	return iterator->p - iterator->start;
}


static Bytecode bytecodeNext(BytecodesIterator *iterator)
{
	iterator->bytecodeNumber++;
	return (Bytecode) bytecodeNextByte(iterator);
}


static Operand bytecodeNextOperand(BytecodesIterator *iterator)
{
	Operand operand = { .isValid = 1, .type = bytecodeNextByte(iterator) };
	switch (operand.type) {
	case OPERAND_VALUE:
		operand.value = *(Value *) iterator->p;
		iterator->p += sizeof(Value);
		break;
	case OPERAND_TEMP_VAR:
	case OPERAND_ARG_VAR:
	case OPERAND_INST_VAR:
	case OPERAND_LITERAL:
	case OPERAND_ASSOC:
	case OPERAND_BLOCK:
		operand.index = bytecodeNextByte(iterator);
		break;
	case OPERAND_INST_VAR_OF:
		operand.index = bytecodeNextByte(iterator);
		Operand instance = bytecodeNextOperand(iterator);
		ASSERT(instance.type != OPERAND_VALUE)
		ASSERT(instance.type != OPERAND_INST_VAR);
		ASSERT(instance.type != OPERAND_INST_VAR_OF);
		operand.instance.type = instance.type;
		operand.instance.index = instance.index;
		operand.instance.level = instance.level;
		break;
	case OPERAND_CONTEXT_VAR:
		operand.index = bytecodeNextByte(iterator);
		operand.level = bytecodeNextByte(iterator);
		break;
	default:;
	}
	return operand;
}


static uint8_t bytecodeNextByte(BytecodesIterator *iterator)
{
	return *iterator->p++;
}


static int32_t bytecodeNextInt32(BytecodesIterator *iterator)
{
	int32_t *p = (int32_t *) iterator->p;
	int32_t result = *p++;
	iterator->p = (uint8_t *) p;
	return result;
}


static _Bool bytecodeHasNext(BytecodesIterator *iterator)
{
	return iterator->p < iterator->end;
}


static void printOperand(Operand operand, RawArray *literals)
{
	switch (operand.type) {
	case OPERAND_VALUE:
		printf(" %zx", operand.value);
		break;
	case OPERAND_NIL:
		printf(" nil");
		break;
	case OPERAND_TRUE:
		printf(" true");
		break;
	case OPERAND_FALSE:
		printf(" false");
		break;
	case OPERAND_THIS_CONTEXT:
		printf(" thisContext");
		break;
	case OPERAND_TEMP_VAR:
		printf(" temp#%i", operand.index);
		break;
	case OPERAND_ARG_VAR:
		printf(" arg#%i", operand.index);
		break;
	case OPERAND_SUPER:
		printf(" super");
		break;
	case OPERAND_CONTEXT_VAR:
		printf(" context#%i.%i", operand.level, operand.index);
		break;
	case OPERAND_INST_VAR:
		printf(" instVar#%i", operand.index);
		break;
	case OPERAND_INST_VAR_OF:
		printf(" instVar#%i of", operand.index);
		printOperand(*(Operand *) &operand.instance, literals);
		break;
	case OPERAND_LITERAL:
	case OPERAND_ASSOC:
	case OPERAND_BLOCK:
		printf(" ");
		printValue(literals->vars[operand.index]);
		printf("#%i", operand.index);
		break;
	}
}


static void printBytecodes(uint8_t *bytecodes, size_t size, RawArray *literals)
{
	BytecodesIterator iterator;
	bytecodeInitIterator(&iterator, bytecodes, size);

	while (bytecodeHasNext(&iterator)) {
		printf("<%zX>\t", iterator.p - bytecodes);
		Bytecode bytecode = bytecodeNext(&iterator);

		switch (bytecode) {
		case BYTECODE_COPY:
			printf("COPY");
			printOperand(bytecodeNextOperand(&iterator), literals);
			printOperand(bytecodeNextOperand(&iterator), literals);
			break;
		case BYTECODE_SEND:
		case BYTECODE_SEND_WITH_STORE:
			printf("SEND #");
			printRawString((RawString *) asObject(literals->vars[bytecodeNextByte(&iterator)]));
			uint8_t argsSize = bytecodeNextByte(&iterator);
			printf(" TO");
			printOperand(bytecodeNextOperand(&iterator), literals);
			if (argsSize > 0) {
				printf(" ARGUMENTS");
			}
			for (uint8_t i = 0; i < argsSize; i++) {
				printOperand(bytecodeNextOperand(&iterator), literals);
				printf(",");
			}
			if (bytecode == BYTECODE_SEND_WITH_STORE) {
				printf(" STORE IN");
				printOperand(bytecodeNextOperand(&iterator), literals);
			}
			break;
		case BYTECODE_RETURN:
			printf("RETURN");
			printOperand(bytecodeNextOperand(&iterator), literals);
			break;
		case BYTECODE_OUTER_RETURN:
			printf("OUTER RETURN");
			printOperand(bytecodeNextOperand(&iterator), literals);
			break;
		case BYTECODE_JUMP:
			printf("JUMP 0x%X", bytecodeNextInt32(&iterator));
			break;
		case BYTECODE_JUMP_NOT_MEMBER_OF: {
			RawClass *class = (RawClass *) asObject(literals->vars[bytecodeNextByte(&iterator)]);
			Operand operand = bytecodeNextOperand(&iterator);
			printf("JUMP 0x%X IF", bytecodeNextInt32(&iterator));
			printOperand(operand, literals);
			printf(" IS NOT MEMBER OF ");
			printClassName(class);
			break;
		}
		default:
			FAIL();
		}

		printf("\n");
	}
}

#endif
