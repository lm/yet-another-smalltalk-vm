#include "Optimizer.h"
#include "Bytecodes.h"
#include "Class.h"
#include "CodeDescriptors.h"
#include "Collection.h"
#include "Iterator.h"
#include "Assert.h"

#define MAX(a, b) (a > b ? a : b)

typedef struct {
	AssemblerBuffer buffer;
	OrderedCollection *literals;
	CompiledCodeHeader header;
} Optimizer;

static void inlineSend(Optimizer *optimizer, CompiledMethod *method, Operand *arguments, Operand result);
static void adjustOperand(Optimizer *optimizer, Array *literals, Operand *arguments, Operand *operand);


CompiledMethod *optimizeMethod(CompiledMethod *method)
{
	HandleScope scope;
	openHandleScope(&scope);

	Optimizer optimizer = {
		.literals = arrayAsOrdColl(compiledMethodGetLiterals(method)),
		.header = compiledMethodGetHeader(method),
	};
	CompiledCodeHeader newHeader = compiledMethodGetHeader(method);
	Array *descriptors = scopeHandle(compiledMethodGetNativeCode(method)->descriptors);
	OrderedCollection *typeFeedback = scopeHandle(compiledMethodGetNativeCode(method)->typeFeedback);
	BytecodesIterator iterator;

	bytecodeInitIterator(&iterator, compiledMethodGetBytes(method), method->raw->size);
	asmInitBuffer(&optimizer.buffer, 256);

	printBytecodes(method->raw->bytes, method->raw->size, compiledMethodGetLiterals(method)->raw);
	printf("\n");

	while (bytecodeHasNext(&iterator)) {
		Bytecode bytecode = bytecodeNext(&iterator);

		switch (bytecode) {
		case BYTECODE_COPY: {
			Operand src = bytecodeNextOperand(&iterator);
			Operand dst = bytecodeNextOperand(&iterator);
			bytecodeCopy(&optimizer.buffer, &src, &dst);
			break;
		}

		case BYTECODE_SEND:
		case BYTECODE_SEND_WITH_STORE: {
			ptrdiff_t bytecodeOff = bytecodeNumber(&iterator);
			//ptrdiff_t ic = descriptorGetPos(descriptorsAtBytecode(descriptors->raw, bytecodeNumber(&iterator)));
			uint8_t selectorIndex = bytecodeNextByte(&iterator);
			uint8_t argsSize = bytecodeNextByte(&iterator);
			Operand receiver = bytecodeNextOperand(&iterator);
			Operand result = { .isValid = 0 };
			String *selector = (String *) ordCollObjectAt(optimizer.literals, selectorIndex);
			AssemblerLabel exitLabels[16];
			ptrdiff_t exitLabelsSize = 0;

			Operand args[argsSize + 2];
			args[1] = receiver;
			for (uint8_t i = 0; i < argsSize; i++) {
				args[i + 2] = bytecodeNextOperand(&iterator);
			}

			if (bytecode == BYTECODE_SEND_WITH_STORE) {
				result = bytecodeNextOperand(&iterator);
			}

			Iterator typesIterator;
			initOrdCollIterator(&typesIterator, typeFeedback, 0, 0);
			while (iteratorHasNext(&typesIterator)) {
				TypeFeedback *feedback = (TypeFeedback *) iteratorNextObject(&typesIterator);
				Value descriptor = descriptorsAtPosition(descriptors->raw, asCInt(feedback->raw->ic));
				if (descriptorGetBytecode(descriptor) == bytecodeOff) {
					Class *class = typeFeedbackGetHintedClass(feedback);
					ptrdiff_t classIndex = ordCollAddObjectIfNotExists(optimizer.literals, (Object *) class);
					AssemblerLabel miss;

					asmInitLabel(&miss);
					bytecodeJumpNotMemberOf(&optimizer.buffer, &receiver, classIndex, &miss);

					CompiledMethod *callee = lookupSelector(class, selector);
					printBytecodes(callee->raw->bytes, callee->raw->size, compiledMethodGetLiterals(callee)->raw);
					printf("\n");

					CompiledCodeHeader calleeHeader = compiledMethodGetHeader(callee);
					newHeader.tempsSize = MAX(newHeader.tempsSize, optimizer.header.tempsSize + calleeHeader.tempsSize);
					newHeader.contextSize = MAX(newHeader.contextSize, optimizer.header.contextSize + calleeHeader.contextSize);
					inlineSend(&optimizer, callee, args, result);

					ASSERT(exitLabelsSize < 16);
					asmInitLabel(&exitLabels[exitLabelsSize]);
					bytecodeJump(&optimizer.buffer, &exitLabels[exitLabelsSize++]);

					asmLabelBind(&optimizer.buffer, &miss, asmOffset(&optimizer.buffer));
				}
			}

			if (bytecode == BYTECODE_SEND_WITH_STORE) {
				bytecodeSendWithStore(&optimizer.buffer, selectorIndex, &receiver, &result, args + 2, argsSize);
			} else {
				bytecodeSend(&optimizer.buffer, selectorIndex, &receiver, args + 2, argsSize);
			}

			for (ptrdiff_t i = 0; i < exitLabelsSize; i++) {
				asmLabelBind(&optimizer.buffer, &exitLabels[i], asmOffset(&optimizer.buffer));
			}
			break;
		}

		case BYTECODE_RETURN:
		case BYTECODE_OUTER_RETURN: {
			Operand operand = bytecodeNextOperand(&iterator);
			asmEmitUint8(&optimizer.buffer, bytecode);
			bytecodeOperand(&optimizer.buffer, &operand);
			break;
		}

		default:
			FAIL();
		}
	}

	printBytecodes(optimizer.buffer.buffer, asmOffset(&optimizer.buffer), ordCollAsArray(optimizer.literals)->raw);

	CompiledMethod *newMethod = newObject(Handles.CompiledMethod, asmOffset(&optimizer.buffer));
	asmCopyBuffer(&optimizer.buffer, newMethod->raw->bytes, newMethod->raw->size);
	compiledMethodSetHeader(newMethod, newHeader);
	compiledMethodSetLiterals(newMethod, ordCollAsArray(optimizer.literals));
	compiledMethodSetOwnerClass(newMethod, compiledMethodGetOwnerClass(method));

	return closeHandleScope(&scope, newMethod);
}


static void inlineSend(Optimizer *optimizer, CompiledMethod *method, Operand *arguments, Operand result)
{
	Array *literals = compiledMethodGetLiterals(method);
	AssemblerBuffer *buffer = &optimizer->buffer;
	BytecodesIterator iterator;
	bytecodeInitIterator(&iterator, method->raw->bytes, method->raw->size);

	while (bytecodeHasNext(&iterator)) {
		Bytecode bytecode = bytecodeNext(&iterator);
		switch (bytecode) {
		case BYTECODE_COPY: {
			Operand src = bytecodeNextOperand(&iterator);
			Operand dst = bytecodeNextOperand(&iterator);
			adjustOperand(optimizer, literals, arguments, &src);
			adjustOperand(optimizer, literals, arguments, &dst);
			bytecodeCopy(buffer, &src, &dst);
			break;
		}

		case BYTECODE_SEND:
		case BYTECODE_SEND_WITH_STORE: {
			uint8_t selectorIndex = bytecodeNextByte(&iterator);
			uint8_t argsSize = bytecodeNextByte(&iterator);
			Operand receiver = bytecodeNextOperand(&iterator);
			adjustOperand(optimizer, literals, arguments, &receiver);

			Object *selector = arrayObjectAt(literals, selectorIndex);
			selectorIndex = ordCollAddObjectIfNotExists(optimizer->literals, selector);

			Operand args[argsSize];
			for (uint8_t i = 0; i < argsSize; i++) {
				args[i] = bytecodeNextOperand(&iterator);
				adjustOperand(optimizer, literals, arguments, &args[i]);
			}
			if (bytecode == BYTECODE_SEND_WITH_STORE) {
				Operand result = bytecodeNextOperand(&iterator);
				adjustOperand(optimizer, literals, arguments, &result);
				bytecodeSendWithStore(buffer, selectorIndex, &receiver, &result, args, argsSize);
			} else {
				bytecodeSend(buffer, selectorIndex, &receiver, args, argsSize);
			}
			break;
		}

		case BYTECODE_RETURN: {
			Operand source = bytecodeNextOperand(&iterator);
			bytecodeCopy(buffer, &source, &result);
			break;
		}

		case BYTECODE_OUTER_RETURN:
		default:
			FAIL();
		}
	}
}


static void adjustOperand(Optimizer *optimizer, Array *literals, Operand *arguments, Operand *operand)
{
	switch (operand->type) {
	case OPERAND_VALUE:
	case OPERAND_NIL:
	case OPERAND_TRUE:
	case OPERAND_FALSE:
	case OPERAND_THIS_CONTEXT:
		break;

	case OPERAND_TEMP_VAR:
		operand->index += optimizer->header.tempsSize;
		break;

	case OPERAND_ARG_VAR:
		*operand = arguments[operand->index];
		break;

	case OPERAND_SUPER:
		// TODO:
		FAIL();
		break;

	case OPERAND_CONTEXT_VAR:
		operand->index += optimizer->header.contextSize;
		break;

	case OPERAND_INST_VAR:
		operand->type = OPERAND_INST_VAR_OF;
		// TODO:
		break;

	case OPERAND_LITERAL:
	case OPERAND_ASSOC:
	case OPERAND_BLOCK: {
		Object *object = arrayObjectAt(literals, operand->index);
		operand->index = ordCollAddObjectIfNotExists(optimizer->literals, object);
		break;
	}

	default:
		FAIL();
	}
}


/*

(receiver isKindOf: SmallInteger)
	ifTrue: [ ... ]
	ifFalse: [
		(receiver isKindOf: LargeInteger) ifTrue: [ ... ]]


:smi
	cmp r10, $SmallInteger		# test if receiver is SmallInteger
	jne	largeInt
		...
	jmp sendExit
:largeInt
	cmp r10, $LargeInteger		# test if receiver is LargeInteger
	jne char
		...
	jmp sendExit
:char
	cmp r10, $Character			# test if receiver is Character
	jne lookup
		...
	jmp sendExit
:lookup
	...							# do normal lookup here
:sendExit						# message was send continue

*/
