#include "Primitives.h"
#include "CodeGenerator.h"
#include "Object.h"
#include "Class.h"
#include "Heap.h"
#include "Exception.h"
#include "Smalltalk.h"
#include "Compiler.h"
#include "Stream.h"
#include "Socket.h"
#include "Parser.h"
#include "Lookup.h"
#include "StackFrame.h"
#include "Thread.h"
#include "Handle.h"
#include "GarbageCollector.h"
#include "Entry.h"
#include "Os.h"
#include "Assert.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
	Value value;
	intptr_t failed;
} PrimitiveResult;

typedef struct {
	char *name;
	enum { GEN, CCALL } type;
	union {
		void (*generate)(CodeGenerator *);
		PrimitiveResult (*cFunction)();
	};
	uint8_t numArgs;
} Primitive;

static PrimitiveResult primSuccess(Value resultValue);
static PrimitiveResult primFailed();
//static PrimitiveResult arrayEqualsPrimitive(Value receiver, Value operand, ErrorHandler handler);
static PrimitiveResult becomePrimitive(Value object, Value other);
static PrimitiveResult contextPositionDescriptorPrimitive(Value vContext);
static PrimitiveResult stringAsSymbolPrimitive(Value receiver);
static PrimitiveResult streamOpenPrimitive(Value fileStream, Value fileName, Value mode);
static PrimitiveResult streamClosePrimitive(Value fileStream, Value descriptor);
static PrimitiveResult streamReadPrimitive(Value vStream, Value descriptor, Value vSize, Value vBuffer, Value vStart);
static PrimitiveResult streamWritePrimitive(Value vStream, Value descriptor, Value vSize, Value vBuffer);
static PrimitiveResult streamFlushPrimitive(Value vStream, Value descriptor);
static PrimitiveResult streamGetPositionPrimitive(Value receiver, Value descriptor);
static PrimitiveResult streamSetPositionPrimitive(Value receiver, Value descriptor, Value position);
static PrimitiveResult streamAvailablePrimitive(Value receiver, Value descriptor);
static PrimitiveResult socketConnectPrimitive(Value socket, Value ip, Value port);
static PrimitiveResult socketBindPrimitive(Value socket, Value ip, Value port, Value queueSize);
static PrimitiveResult socketAcceptPrimitive(Value socket);
static PrimitiveResult socketHostLookupPrimitive(Value class, Value vHost);
static PrimitiveResult lastIoErrorPrimitive(Value receiver);
static PrimitiveResult currentMicroTimePrimitive(Value receiver);
static PrimitiveResult initParserPrimitive(Value receiver, Value string);
static PrimitiveResult initStreamParserPrimitive(Value receiver, Value string);
static PrimitiveResult freeParserPrimitive(Value receiver);
static PrimitiveResult parseClassPrimitive(Value receiver);
static PrimitiveResult parseMethodPrimitive(Value receiver);
static PrimitiveResult parseMethodOrBlockPrimitive(Value receiver);
static _Bool initParserFromParserObject(ParserObject *parserObj, Parser *parser);
static void freeParserWithinParserObject(ParserObject *parserObj, Parser *parser);
static ParseError *createParserError(Parser *parser);
static PrimitiveResult parserAtEndPrimitive(Value receiver);
static PrimitiveResult buildClassPrimitive(Value receiver, Value vNode);
static PrimitiveResult compileMethodPrimitive(Value receiver, Value vNode, Value class);
static PrimitiveResult collectGarbagePrimitive(Value receiver);
static PrimitiveResult printHeapPrimitive(Value receiver);
static PrimitiveResult lastGcStatsPrimitive(Value receiver);

#include "PrimitivesX64.c"

Primitive Primitives[] = {
	{"AtPrimitive", GEN, generateAtPrimitive},
	{"AtPutPrimitive", GEN, generateAtPutPrimitive},
	{"SizePrimitive", GEN, generateSizePrimitive},
	{"InstVarAtPrimitive", GEN, generateInstVarAtPrimitive},
	{"InstVarAtPutPrimitive", GEN, generateInstVarAtPutPrimitive},
	{"BecomePrimitive", CCALL, .cFunction = becomePrimitive, 2},
	{"IdentityPrimitive", GEN, generateIdentityPrimitive},
	{"HashPrimitive", GEN, generateHashPrimitive},
	{"ClassPrimitive", GEN, generateClassPrimitive},

	{"BehaviorNewPrimitive", GEN, generateBehaviorNewPrimitive},
	{"BehaviorNewSizePrimitive", GEN, generateBehaviorNewSizePrimitive},

	{"CharacterNewPrimitive", GEN, generateCharacterNewPrimitive},
	{"CharacterCodePrimitive", GEN, generateCharacterCodePrimitive},

	{"BlockValuePrimitive", GEN, generateBlockValuePrimitive0Args},
	{"BlockValuePrimitive1", GEN, generateBlockValuePrimitive1Args},
	{"BlockValuePrimitive2", GEN, generateBlockValuePrimitive2Args},
	{"BlockValuePrimitive3", GEN, generateBlockValuePrimitive3Args},
	{"BlockValueArgsPrimitive", GEN, generateBlockValueArgsPrimitive},
	{"BlockWhileTruePrimitive", GEN, generateBlockWhileTrue},
	{"BlockWhileTrue2Primitive", GEN, generateBlockWhileTrue2},
	{"BlockOnExceptionPrimitive", GEN, generateBlockOnExceptionPrimitive},

	{"ContextParentPrimitive", CCALL, .cFunction = contextParentPrimitive, 1},
	{"ContextArgumentAt", CCALL, .cFunction = contextArgumentAt, 2},
	{"ContextTemporaryAt", CCALL, .cFunction = contextTemporaryAt, 2},
	{"ContextPositionDescriptorPrimitive", CCALL, .cFunction = contextPositionDescriptorPrimitive, 1},

	{"ExceptionSignal", GEN, generateExceptionSignalPrimitive},

	{"StringHashPrimitive", GEN, generateStringHashPrimitive},
	{"StringAsSymbolPrimitive", CCALL, .cFunction = stringAsSymbolPrimitive, 1},
	/*{"ArrayEqualsPrimitive", CCALL, .cFunction = arrayEqualsPrimitive, 3},*/

	{"IntLessThanPrimitive", GEN, generateIntLessThanPrimitive},
	{"IntAddPrimitive", GEN, generateIntAddPrimitive},
	{"IntSubPrimitive", GEN, generateIntSubPrimitive},
	{"IntMulPrimitive", GEN, generateIntMulPrimitive},
	{"IntQuoPrimitive", GEN, generateIntQuoPrimitive},
	{"IntModPrimitive", GEN, generateIntModPrimitive},
	{"IntRemPrimitive", GEN, generateIntRemPrimitive},
	{"IntNegPrimitive", GEN, generateIntNegPrimitive},
	{"IntAndPrimitive", GEN, generateIntAndPrimitive},
	{"IntOrPrimitive", GEN, generateIntOrPrimitive},
	{"IntXorPrimitive", GEN, generateIntXorPrimitive},
	{"IntShiftPrimitive", GEN, generateIntShiftPrimitive},
	{"IntAsObjectPrimitive", GEN, /*generateIntAsObjectPrimitive*/generateNotImplementedPrimitive},

	{"StreamOpenPrimitive", CCALL, .cFunction = streamOpenPrimitive, 3},
	{"StreamClosePrimitive", CCALL, .cFunction = streamClosePrimitive, 2},
	{"StreamReadPrimitive", CCALL, .cFunction = streamReadPrimitive, 5},
	{"StreamWritePrimitive", CCALL, .cFunction = streamWritePrimitive, 4},
	{"StreamFlushPrimitive", CCALL, .cFunction = streamFlushPrimitive, 2},
	{"StreamGetPositionPrimitive", CCALL, .cFunction = streamGetPositionPrimitive, 2},
	{"StreamSetPositionPrimitive", CCALL, .cFunction = streamSetPositionPrimitive, 3},
	{"StreamAvailablePrimitive", CCALL, .cFunction = streamAvailablePrimitive, 2},

	{"SocketConnectPrimitive", CCALL, .cFunction = socketConnectPrimitive, 3},
	{"SocketBindPrimitive", CCALL, .cFunction = socketBindPrimitive, 4},
	{"SocketAcceptPrimitive", CCALL, .cFunction = socketAcceptPrimitive, 1},
	{"SocketHostLookup", CCALL, .cFunction = socketHostLookupPrimitive, 2},

	{"LastIoErrorPrimitive", CCALL, .cFunction = lastIoErrorPrimitive, 1},

	{"CurrentMicroTimePrimitive", CCALL, .cFunction = currentMicroTimePrimitive, 1},

	{"GCPrimitive", CCALL, .cFunction = collectGarbagePrimitive, 1},
	{"LastGCStatsPrimitive", CCALL, .cFunction = lastGcStatsPrimitive, 1},
	{"PrintHeapPrimitive", CCALL, .cFunction = printHeapPrimitive, 1},
	{"InterruptPrimitive", GEN, generateInterruptPrimitive},
	{"ExitPrimitive", GEN, generateExitPrimitive}, // TODO: remove replace with process primitive

	{"ParseClassPrimitive", CCALL, .cFunction = parseClassPrimitive, 1},
	{"ParseMethodPrimitive", CCALL, .cFunction = parseMethodPrimitive, 1},
	{"ParseMethodOrBlockPrimitive", CCALL, .cFunction = parseMethodOrBlockPrimitive, 1},

	{"BuildClassPrimitive", CCALL, .cFunction = buildClassPrimitive, 2},
	{"CompileMethodPrimitive", CCALL, .cFunction = compileMethodPrimitive, 3},
	{"MethodSendPrimitive", GEN, generateMethodSendPrimitive},
	{"MethodSendArgsPrimitive", GEN, generateMethodSendArgsPrimitive},
};


void registerPrimitives(void)
{
	HandleScope scope;
	openHandleScope(&scope);

	Primitive *p = Primitives;
	Primitive *end = Primitives + sizeof(Primitives) / sizeof(Primitive);

	for (; p < end; p++) {
		setGlobal(p->name, tagInt(p - Primitives + 1));
	}

	closeHandleScope(&scope, NULL);
}


void generatePrimitive(CodeGenerator *generator, uint16_t primitive)
{
	Primitive *prim = Primitives + primitive - 1;
	if (prim->type == GEN) {
		prim->generate(generator);
	} else {
		generateCCallPrimitive(generator, prim->cFunction, prim->numArgs);
	}
}


static PrimitiveResult primSuccess(Value resultValue)
{
	PrimitiveResult result = { .value = resultValue, .failed = 0 };
	return result;
}


static PrimitiveResult primFailed()
{
	PrimitiveResult result = { .failed = 1 };
	return result;
}


/*static Value arrayEqualsPrimitive(Value receiver, Value operand)
{
	RawIndexedObject *obj1 = (RawIndexedObject *) asObject(receiver);
	RawIndexedObject *obj2 = (RawIndexedObject *) asObject(operand);

	if (obj1->class != obj2->class || obj1->size != obj2->size) {
		return getTaggedPtr(Handles.false);
	}

	InstanceShape shape = obj1->class->instanceShape;
	if (!shape.isIndexed) {
		handler(receiver, operand);
	}

	return asBool(memcmp(
		getObjectIndexedVars((Object *) obj1),
		getObjectIndexedVars((Object *) obj2),
		obj1->size * (shape.isBytes ? 1 : sizeof(Value))
	) == 0);
}*/


static PrimitiveResult becomePrimitive(Value object, Value other)
{
	HandleScope scope;
	openHandleScope(&scope);
	objectBecome(scopeHandle(asObject(object)), scopeHandle(asObject(other)));
	closeHandleScope(&scope, NULL);
	return primSuccess(other);
}


static PrimitiveResult contextPositionDescriptorPrimitive(Value vContext)
{
	RawContext *context = (RawContext *) asObject(vContext);
	intptr_t ic = isTaggedNil(context->ic) ? 0 : asCInt(context->ic);
	return primSuccess(findSourceCode(asObject(context->code), ic));
}


static PrimitiveResult stringAsSymbolPrimitive(Value receiver)
{
	HandleScope scope;
	openHandleScope(&scope);
	String *string = scopeHandle(asObject(receiver));
	Value result = getTaggedPtr(asSymbol(string));
	closeHandleScope(&scope, NULL);
	return primSuccess(result);
}


static PrimitiveResult streamOpenPrimitive(Value receiver, Value fileName, Value mode)
{
	int descriptor = streamOpen((RawString *) asObject(fileName), asCInt(mode));
	return descriptor < 0 ? primFailed() : primSuccess(tagInt(descriptor));
}


static PrimitiveResult streamClosePrimitive(Value stream, Value descriptor)
{
	return streamClose(asCInt(descriptor)) ? primSuccess(stream) : primFailed();
}


static PrimitiveResult streamReadPrimitive(Value vStream, Value descriptor, Value vSize, Value vBuffer, Value vStart)
{
	intptr_t size = asCInt(vSize);
	RawString *buffer = (RawString *) asObject(vBuffer);
	intptr_t start = asCInt(vStart) - 1;

	if (size > buffer->size || start < 0 || start >= buffer->size) {
		return primFailed();
	}

	ptrdiff_t read = streamRead(asCInt(descriptor), buffer->contents + start, size);
	if (read < 0) {
		return primFailed();
	}

	return primSuccess(tagInt(read));
}


static PrimitiveResult streamWritePrimitive(Value vStream, Value descriptor, Value vSize, Value vBuffer)
{
	intptr_t size = asCInt(vSize);
	RawString *buffer = (RawString *) asObject(vBuffer);

	if (size > buffer->size) {
		return primFailed();
	}

	ptrdiff_t written = streamWrite(asCInt(descriptor), buffer->contents, size);
	if (written < 0) {
		return primFailed();
	}

	return primSuccess(tagInt(written));
}


static PrimitiveResult streamFlushPrimitive(Value vStream, Value descriptor)
{
	return streamFlush(asCInt(descriptor)) ? primSuccess(vStream) : primFailed();
}


static PrimitiveResult streamGetPositionPrimitive(Value receiver, Value descriptor)
{
	ptrdiff_t position = streamGetPosition(asCInt(descriptor));
	return position < 0 ? primFailed() : primSuccess(tagInt(position));
}


static PrimitiveResult streamSetPositionPrimitive(Value receiver, Value descriptor, Value position)
{
	return streamSetPosition(asCInt(descriptor), asCInt(position))
		? primSuccess(receiver)
		: primFailed();
}


static PrimitiveResult streamAvailablePrimitive(Value receiver, Value descriptor)
{
	intptr_t available = streamAvailable(asCInt(descriptor));
	return available < 0 ? primFailed() : primSuccess(tagInt(available));
}


static PrimitiveResult socketConnectPrimitive(Value socket, Value vAddr, Value port)
{
	RawInternetAddress *addr = (RawInternetAddress *) asObject(vAddr);
	int descriptor = socketConnect(asCInt(addr->address), asCInt(port));
	return descriptor < 0 ? primFailed() : primSuccess(tagInt(descriptor));
}


static PrimitiveResult socketBindPrimitive(Value socket, Value vAddr, Value port, Value queueSize)
{
	RawInternetAddress *addr = (RawInternetAddress *) asObject(vAddr);
	int descriptor = socketBind(asCInt(addr->address), asCInt(port), asCInt(queueSize));
	return descriptor < 0 ? primFailed() : primSuccess(tagInt(descriptor));
}


static PrimitiveResult socketAcceptPrimitive(Value socket)
{
	RawServerSocket *server = (RawServerSocket *) asObject(socket);
	int descriptor = socketAccept(asCInt(server->descriptor));
	return descriptor < 0 ? primFailed() : primSuccess(tagInt(descriptor));
}


static PrimitiveResult socketHostLookupPrimitive(Value class, Value vHost)
{
	HandleScope scope;
	openHandleScope(&scope);

	InternetAddress *addr = newObject(scopeHandle(asObject(class)), 0);
	String *host = (String *) scopeHandle(asObject(vHost));

	char space[256];
	char *buffer = space;
	if (host->raw->size > 256) {
		String *tmpString = (String *) copyResizedObject((Object *) host, host->raw->size + 1);
		buffer = tmpString->raw->contents;
		buffer[host->raw->size] = '\0';
	} else {
		stringPrintOn(host, buffer);
	}

	const char *error;
	addr->raw->address = tagInt(socketHostLookup(buffer, &error));

	Value result = getTaggedPtr(addr);
	closeHandleScope(&scope, NULL);
	return error == NULL ? primSuccess(result) : primFailed();
}


static PrimitiveResult lastIoErrorPrimitive(Value receiver)
{
	HandleScope scope;
	openHandleScope(&scope);
	Value error = getTaggedPtr(getLastIoError());
	closeHandleScope(&scope, NULL);
	return primSuccess(error);
}


static PrimitiveResult currentMicroTimePrimitive(Value receiver)
{
	return primSuccess(tagInt(osCurrentMicroTime()));
}


static PrimitiveResult parseClassPrimitive(Value receiver)
{
	HandleScope scope;
	openHandleScope(&scope);

	ParserObject *parserObj = scopeHandle(asObject(receiver));
	Parser parser;
	if (!initParserFromParserObject(parserObj, &parser)) {
		closeHandleScope(&scope, NULL);
		return primFailed();
	}

	ClassNode *node = parseClass(&parser);
	Value result = getTaggedPtr(node == NULL ? (Object *) createParserError(&parser) : (Object *) node);

	objectStorePtr((Object *) parserObj, &parserObj->raw->atEnd, asBool(parserAtEnd(&parser)));
	freeParserWithinParserObject(parserObj, &parser);

	closeHandleScope(&scope, NULL);
	return primSuccess(result);
}


static PrimitiveResult parseMethodPrimitive(Value receiver)
{
	HandleScope scope;
	openHandleScope(&scope);

	ParserObject *parserObj = scopeHandle(asObject(receiver));
	Parser parser;
	if (!initParserFromParserObject(parserObj, &parser)) {
		closeHandleScope(&scope, NULL);
		return primFailed();
	}

	MethodNode *node = parseMethod(&parser);
	Value result = getTaggedPtr(node == NULL ? (Object *) createParserError(&parser) : (Object *) node);

	objectStorePtr((Object *) parserObj, &parserObj->raw->atEnd, asBool(parserAtEnd(&parser)));
	freeParser(&parser);

	closeHandleScope(&scope, NULL);
	return primSuccess(result);
}


static PrimitiveResult parseMethodOrBlockPrimitive(Value receiver)
{
	HandleScope scope;
	openHandleScope(&scope);

	ParserObject *parserObj = scopeHandle(asObject(receiver));
	Parser parser;
	if (!initParserFromParserObject(parserObj, &parser)) {
		closeHandleScope(&scope, NULL);
		return primFailed();
	}

	Object *node;
	if (currentToken(&parser.tokenizer)->type == TOKEN_OPEN_SQUARE_BRACKET) {
		node = (Object *) parseBlock(&parser);
	} else {
		node = (Object *) parseMethod(&parser);
	}
	Value result = getTaggedPtr(node == NULL ? (Object *) createParserError(&parser) : node);

	objectStorePtr((Object *) parserObj, &parserObj->raw->atEnd, asBool(parserAtEnd(&parser)));
	freeParser(&parser);

	closeHandleScope(&scope, NULL);
	return primSuccess(result);
}


static _Bool initParserFromParserObject(ParserObject *parserObj, Parser *parser)
{
	String *source = scopeHandle(asObject(parserObj->raw->source));
	if (!isTaggedNil(parserObj->raw->stream)) {
		FileStream *stream = scopeHandle(asObject(parserObj->raw->stream));
		FILE *file = fdopen(dup(asCInt(stream->raw->descriptor)), "r");
		if (file == NULL) {
			return 0;
		}
		initFileParser(parser, file, source);
	} else {
		initParser(parser, source);
		// TODO: preserve position in source string or use CollectionStream?
	}
	return 1;
}


static void freeParserWithinParserObject(ParserObject *parserObj, Parser *parser)
{
	String *source = scopeHandle(asObject(parserObj->raw->source));
	if (!isTaggedNil(parserObj->raw->stream)) {
		ASSERT(parser->tokenizer.isFile);
		FileStream *stream = scopeHandle(asObject(parserObj->raw->stream));
		FILE *file = parser->tokenizer.source.file;
		streamSetPosition(asCInt(stream->raw->descriptor), currentToken(&parser->tokenizer)->position - 1);
		fclose(file);
	}
	freeParser(parser);
}


static ParseError *createParserError(Parser *parser)
{
	Token *token = currentToken(&parser->tokenizer);
	ParseError *error = newObject(Handles.ParseError, 0);
	objectStorePtr((Object *) error, &error->raw->token, (Object *) asString(token->content));
	objectStorePtr((Object *) error, &error->raw->sourceCode, (Object *) createSourceCode(parser, 1));
	return error;
}


static PrimitiveResult buildClassPrimitive(Value receiver, Value vNode)
{
	HandleScope scope;
	openHandleScope(&scope);

	ClassNode *node = scopeHandle(asObject(vNode));
	if (node->raw->class != Handles.ClassNode->raw) {
		closeHandleScope(&scope, NULL);
		return primFailed();
	}
	Object *class = buildClass(node);
	Value result = getTaggedPtr(class);
	closeHandleScope(&scope, NULL);
	return primSuccess(result);
}


static PrimitiveResult compileMethodPrimitive(Value receiver, Value vNode, Value class)
{
	HandleScope scope;
	openHandleScope(&scope);

	MethodNode *node = scopeHandle(asObject(vNode));
	if (node->raw->class != Handles.MethodNode->raw) {
		closeHandleScope(&scope, NULL);
		return primFailed();
	}
	Value result = getTaggedPtr(compileMethod(node, scopeHandle(asObject(class))));
	closeHandleScope(&scope, NULL);
	return primSuccess(result);
}


static PrimitiveResult collectGarbagePrimitive(Value receiver)
{
	collectGarbage(&CurrentThread);
	return primSuccess(receiver);
}


static PrimitiveResult printHeapPrimitive(Value receiver)
{
	printHeap(&CurrentThread.heap);
	return primSuccess(receiver);
}


static PrimitiveResult lastGcStatsPrimitive(Value receiver)
{
	HandleScope scope;
	openHandleScope(&scope);
	Dictionary *stats = newDictionary(8);
	stringDictAtPut(stats, asString("count"), tagInt(LastGCStats.count));
	stringDictAtPut(stats, asString("total"), tagInt(LastGCStats.total));
	stringDictAtPut(stats, asString("marked"), tagInt(LastGCStats.marked));
	stringDictAtPut(stats, asString("sweeped"), tagInt(LastGCStats.sweeped));
	stringDictAtPut(stats, asString("freed"), tagInt(LastGCStats.freed));
	stringDictAtPut(stats, asString("extended"), tagInt(LastGCStats.extended));
	Value result = getTaggedPtr(stats);
	closeHandleScope(&scope, NULL);
	return primSuccess(result);
}
