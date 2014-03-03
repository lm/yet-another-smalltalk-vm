#include "Bootstrap.h"
#include "Thread.h"
#include "Object.h"
#include "Smalltalk.h"
#include "Dictionary.h"
#include "Parser.h"
#include "Class.h"
#include "Compiler.h"
#include "Primitives.h"
#include "Heap.h"
#include "Thread.h"
#include "Entry.h"
#include "Handle.h"
#include "Iterator.h"
#include "Assert.h"
#include <errno.h>

static void initSmalltalkStubs(void);
static Class *newStubClass(Class *metaClass, InstanceShape shape, size_t instanceSize);
static Class *newStubMetaClass(InstanceShape shape, size_t instanceSize);
static Object *newStubObject(size_t size);
static _Bool parseKernelFiles(char *kernelDir);


_Bool bootstrap(char *kernelDir)
{
	initSmalltalkStubs();
	registerPrimitives();
	return parseKernelFiles(kernelDir);
}


static void initSmalltalkStubs(void)
{
	HandleScope scope;
	openHandleScope(&scope);

	Handles.nil = newStubObject(sizeof(struct { OBJECT_HEADER; }));
	Class *metaClass = newStubMetaClass(FixedShape, 9);

	Handles.MetaClass = newStubClass(metaClass, FixedShape, 6);
	Handles.UndefinedObject = newStubClass(metaClass, FixedShape, 0);
	Handles.True = newStubClass(metaClass, FixedShape, 0);
	Handles.False = newStubClass(metaClass, FixedShape, 0);
	Handles.SmallInteger = newStubClass(metaClass, FixedShape, 0);
	Handles.Symbol = newStubClass(metaClass, StringShape, 0);
	Handles.Character = newStubClass(metaClass, FixedShape, 0);
	// Handles.Float = newStubClass(metaClass, FixedShape, 0);
	Handles.String = newStubClass(metaClass, StringShape, 0);
	Handles.Array = newStubClass(metaClass, IndexedShape, 0);
	Handles.ByteArray = newStubClass(metaClass, BytesShape, 0);
	Handles.Association = newStubClass(metaClass, FixedShape, 2);
	Handles.Dictionary = newStubClass(metaClass, FixedShape, 2);
	Handles.OrderedCollection = newStubClass(metaClass, FixedShape, 3);
	Handles.Class = newStubClass(metaClass, FixedShape, 9);
	Handles.TypeFeedback = newStubClass(metaClass, FixedShape, 2);
	Handles.CompiledMethod = newStubClass(metaClass, CompiledCodeShape, 6);
	Handles.CompiledBlock = newStubClass(metaClass, CompiledCodeShape, 4);
	Handles.SourceCode = newStubClass(metaClass, FixedShape, 5);
	Handles.FileSourceCode = newStubClass(metaClass, FixedShape, 5);
	Handles.Block = newStubClass(metaClass, BlockShape, 3);
	Handles.Message = newStubClass(metaClass, FixedShape, 2);
	Handles.MethodContext = newStubClass(metaClass, ContextShape, 5);
	Handles.BlockContext = newStubClass(metaClass, ContextShape, 5);
	Handles.ExceptionHandler = newStubClass(metaClass, ExceptionHandlerShape, 2);
	Handles.ClassNode = newStubClass(metaClass, FixedShape, 6);
	Handles.MethodNode = newStubClass(metaClass, FixedShape, 5);
	Handles.BlockNode = newStubClass(metaClass, FixedShape, 5);
	Handles.BlockScope = newStubClass(metaClass, FixedShape, 6);
	Handles.ExpressionNode = newStubClass(metaClass, FixedShape, 5);
	Handles.MessageExpressionNode = newStubClass(metaClass, FixedShape, 3);
	Handles.NilNode = newStubClass(metaClass, FixedShape, 2);
	Handles.TrueNode = newStubClass(metaClass, FixedShape, 2);
	Handles.FalseNode = newStubClass(metaClass, FixedShape, 2);
	Handles.VariableNode = newStubClass(metaClass, FixedShape, 2);
	Handles.IntegerNode = newStubClass(metaClass, FixedShape, 2);
	Handles.CharacterNode = newStubClass(metaClass, FixedShape, 2);
	Handles.SymbolNode = newStubClass(metaClass, FixedShape, 2);
	Handles.StringNode = newStubClass(metaClass, FixedShape, 2);
	Handles.ArrayNode = newStubClass(metaClass, FixedShape, 2);
	Handles.ParseError = newStubClass(metaClass, FixedShape, 3);
	Handles.UndefinedVariableError = newStubClass(metaClass, FixedShape, 2);
	Handles.RedefinitionError = newStubClass(metaClass, FixedShape, 2);
	Handles.ReadonlyVariableError = newStubClass(metaClass, FixedShape, 2);
	Handles.InvalidPragmaError = newStubClass(metaClass, FixedShape, 2);
	Handles.IoError = newStubClass(metaClass, FixedShape, 1);

	Handles.nil->raw->class = Handles.UndefinedObject->raw;
	Handles.true = persistHandle(newObject(Handles.True, 0));
	Handles.false = persistHandle(newObject(Handles.False, 0));
	Handles.Smalltalk = persistHandle(newDictionary(256));
	Handles.SymbolTable = persistHandle(newArray(SYMBOL_TABLE_SIZE));
	Handles.initializeSymbol = persistHandle(getSymbol("initialize"));
	Handles.finalizeSymbol = persistHandle(getSymbol("finalize"));
	Handles.valueSymbol = persistHandle(getSymbol("value"));
	Handles.value_Symbol = persistHandle(getSymbol("value:"));
	Handles.valueValueSymbol = persistHandle(getSymbol("value:value:"));
	Handles.doesNotUnderstandSymbol = persistHandle(getSymbol("doesNotUnderstand:"));
	Handles.cannotReturnSymbol = persistHandle(getSymbol("cannotReturn:"));
	Handles.handlesSymbol = persistHandle(getSymbol("handles:"));
	Handles.generateBacktraceSymbol = persistHandle(getSymbol("generateBacktrace"));

	setGlobalObject("UndefinedObject", (Object *) Handles.UndefinedObject);
	setGlobalObject("nil", Handles.nil);
	setGlobalObject("True", (Object *) Handles.True);
	setGlobalObject("true", Handles.true);
	setGlobalObject("False", (Object *) Handles.False);
	setGlobalObject("false", Handles.false);
	setGlobalObject("SmallInteger", (Object *) Handles.SmallInteger);
	setGlobalObject("Character", (Object *) Handles.Character);

	setGlobal("FixedShape", *(Value *) &FixedShape);
	setGlobal("IndexedShape", *(Value *) &IndexedShape);
	setGlobal("StringShape", *(Value *) &StringShape);
	setGlobal("BytesShape", *(Value *) &BytesShape);
	setGlobal("CompiledCodeShape", *(Value *) &CompiledCodeShape);
	setGlobal("BlockShape", *(Value *) &BlockShape);
	setGlobal("ContextShape", *(Value *) &ContextShape);
	setGlobal("ExceptionHandlerShape", *(Value *) &ExceptionHandlerShape);

	setGlobalObject("Symbol", (Object *) Handles.Symbol);
	setGlobalObject("String", (Object *) Handles.String);
	setGlobalObject("Array", (Object *) Handles.Array);
	setGlobalObject("ByteArray", (Object *) Handles.ByteArray);
	setGlobalObject("Association", (Object *) Handles.Association);
	setGlobalObject("Dictionary", (Object *) Handles.Dictionary);
	setGlobalObject("OrderedCollection", (Object *) Handles.OrderedCollection);
	setGlobalObject("MetaClass", (Object *) Handles.MetaClass);
	setGlobalObject("Class", (Object *) Handles.Class);
	setGlobalObject("TypeFeedback", (Object *) Handles.TypeFeedback);
	setGlobalObject("CompiledMethod", (Object *) Handles.CompiledMethod);
	setGlobalObject("CompiledBlock", (Object *) Handles.CompiledBlock);
	setGlobalObject("SourceCode", (Object *) Handles.SourceCode);
	setGlobalObject("FileSourceCode", (Object *) Handles.FileSourceCode);
	setGlobalObject("Block", (Object *) Handles.Block);
	setGlobalObject("Message", (Object *) Handles.Message);
	setGlobalObject("MethodContext", (Object *) Handles.MethodContext);
	setGlobalObject("BlockContext", (Object *) Handles.BlockContext);
	setGlobalObject("ExceptionHandler", (Object *) Handles.ExceptionHandler);
	setGlobalObject("ClassNode", (Object *) Handles.ClassNode);
	setGlobalObject("MethodNode", (Object *) Handles.MethodNode);
	setGlobalObject("BlockNode", (Object *) Handles.BlockNode);
	setGlobalObject("BlockScope", (Object *) Handles.BlockScope);
	setGlobalObject("ExpressionNode", (Object *) Handles.ExpressionNode);
	setGlobalObject("MessageExpressionNode", (Object *) Handles.MessageExpressionNode);
	setGlobalObject("NilNode", (Object *) Handles.NilNode);
	setGlobalObject("TrueNode", (Object *) Handles.TrueNode);
	setGlobalObject("FalseNode", (Object *) Handles.FalseNode);
	setGlobalObject("VariableNode", (Object *) Handles.VariableNode);
	setGlobalObject("IntegerNode", (Object *) Handles.IntegerNode);
	setGlobalObject("CharacterNode", (Object *) Handles.CharacterNode);
	setGlobalObject("SymbolNode", (Object *) Handles.SymbolNode);
	setGlobalObject("StringNode", (Object *) Handles.StringNode);
	setGlobalObject("ArrayNode", (Object *) Handles.ArrayNode);
	setGlobalObject("ParseError", (Object *) Handles.ParseError);
	setGlobalObject("UndefinedVariableError", (Object *) Handles.UndefinedVariableError);
	setGlobalObject("RedefinitionError", (Object *) Handles.RedefinitionError);
	setGlobalObject("ReadonlyVariableError", (Object *) Handles.ReadonlyVariableError);
	setGlobalObject("InvalidPragmaError", (Object *) Handles.InvalidPragmaError);
	setGlobalObject("IoError", (Object *) Handles.IoError);
	setGlobalObject("SymbolTable", (Object *) Handles.SymbolTable);
	setGlobalObject("Smalltalk", (Object *) Handles.Smalltalk);

	closeHandleScope(&scope, NULL);
}


static Class *newStubClass(Class *metaClass, InstanceShape shape, size_t instanceSize)
{
	Class *class = (Class *) newStubObject(sizeof(RawClass));
	class->raw->class = metaClass->raw;
	objectStorePtr((Object *) class,  &class->raw->superClass, (Object *) Handles.nil);
	objectStorePtr((Object *) class,  &class->raw->subClasses, (Object *) Handles.nil);
	objectStorePtr((Object *) class,  &class->raw->methodDictionary, (Object *) Handles.nil);
	shape.varsSize = instanceSize;
	shape.size += shape.varsSize * sizeof(Value);
	class->raw->instanceShape = shape;
	objectStorePtr((Object *) class,  &class->raw->instanceVariables, (Object *) Handles.nil);
	objectStorePtr((Object *) class,  &class->raw->name, (Object *) Handles.nil);
	objectStorePtr((Object *) class,  &class->raw->comment, (Object *) Handles.nil);
	objectStorePtr((Object *) class,  &class->raw->category, (Object *) Handles.nil);
	objectStorePtr((Object *) class,  &class->raw->classVariables, (Object *) Handles.nil);
	return class;
}


static Class *newStubMetaClass(InstanceShape shape, size_t instanceSize)
{
	RawObject *object = (RawObject *) allocate(&CurrentThread.heap, sizeof(RawMetaClass));
	object->hash = (Value) object >> 2; // XXX: replace with random hash generator
	object->payloadSize = 0;
	object->varsSize = 0;
	object->tags = 0;

	Class *metaClass = newStubClass(scopeHandle(NULL), FixedShape, 6);
	metaClass->raw->class = (RawClass *) object;

	MetaClass *class = (MetaClass *) scopeHandle(object);
	class->raw->class = metaClass->raw;
	objectStorePtr((Object *) class,  &class->raw->superClass, (Object *) Handles.nil);
	objectStorePtr((Object *) class,  &class->raw->subClasses, (Object *) Handles.nil);
	objectStorePtr((Object *) class,  &class->raw->methodDictionary, (Object *) Handles.nil);
	shape.varsSize = instanceSize;
	shape.size += shape.varsSize * sizeof(Value);
	class->raw->instanceShape = shape;
	objectStorePtr((Object *) class,  &class->raw->instanceVariables, (Object *) Handles.nil);
	objectStorePtr((Object *) class,  &class->raw->instanceClass, (Object *) Handles.nil);
	return (Class *) class;
}


static Object *newStubObject(size_t size)
{
	RawObject *object = (RawObject *) allocate(&CurrentThread.heap, size);
	object->hash = (Value) object >> 2; // XXX: replace with random hash generator
	object->payloadSize = 0;
	object->varsSize = 0;
	object->tags = 0;
	return handle(object);
}


static _Bool parseKernelFiles(char *kernelDir)
{
	char *kernelFiles[] = {
		"Object.st",
		"UndefinedObject.st",
		"Behavior.st",
		"ClassDescription.st",
		"MetaClass.st",
		"Class.st",
		"Boolean.st",
		"True.st",
		"False.st",
		"TypeFeedback.st",
		"Magnitudes/Magnitude.st",
		"Magnitudes/Number.st",
		"Magnitudes/Integer.st",
		"Magnitudes/SmallInteger.st",
		"Magnitudes/Character.st",
		"Magnitudes/DateTime.st",
		"Iterator.st",
		"Collections/Association.st",
		"Collections/Collection.st",
		"Collections/SequenceableCollection.st",
		"Collections/ArrayedCollection.st",
		"Collections/Array.st",
		"Collections/String.st",
		"Collections/Symbol.st",
		"Collections/HashedCollection.st",
		"Collections/Dictionary.st",
		"Collections/Set.st",
		"Collections/ByteArray.st",
		"Collections/Bag.st",
		"Collections/OrderedCollection.st",
		"Collections/Interval.st",
		"CompiledCode.st",
		"CompiledBlock.st",
		"CompiledMethod.st",
		"Block.st",
		"Context.st",
		"ContextCopy.st",
		"MethodContext.st",
		"BlockContext.st",
		"ExceptionHandler.st",
		"Message.st",
		"SourceCode.st",
		"FileSourceCode.st",
		"Streams/Stream.st",
		"Streams/PositionableStream.st",
		"Streams/StreamView.st",
		"Streams/CollectionStream.st",
		"Streams/BufferedStream.st",
		"Streams/ExternalStream.st",
		"Streams/FileStream.st",
		"Streams/Socket.st",
		"Streams/ServerSocket.st",
		"Streams/InternetAddress.st",

		"GarbageCollector.st",
		"Processes/ProcessorScheduler.st",
		"Processes/Process.st",
		// "Processes/Delay.st",

		"Exception.st",
		"Error.st",
		"MessageNotUnderstood.st",
		"OutOfRangeError.st",
		"NotFoundError.st",
		"ShouldNotImplement.st",
		"SubClassResponsibility.st",
		"IoError.st",

		"Parser/ParseError.st",
		"Parser/Parser.st",
		"Parser/LiteralNode.st",
		"Parser/ClassNode.st",
		"Parser/MethodNode.st",
		"Parser/BlockNode.st",
		"Parser/ExpressionNode.st",
		"Parser/MessageExpressionNode.st",
		"Parser/VariableNode.st",
		"Parser/NilNode.st",
		"Parser/TrueNode.st",
		"Parser/FalseNode.st",
		"Parser/IntegerNode.st",
		"Parser/CharacterNode.st",
		"Parser/StringNode.st",
		"Parser/SymbolNode.st",
		"Parser/ArrayNode.st",
		"Parser/BlockScope.st",

		"Compiler/CompileError.st",
		"Compiler/Compiler.st",
		"Compiler/InvalidPragmaError.st",
		"Compiler/UndefinedVariableError.st",
		"Compiler/RedefinitionError.st",
		"Compiler/ReadonlyVariableError.st",

		"Debugger.st",
		"Repl.st",
		"Assert.st",
		"AssertError.st",
	};

	HandleScope scope;
	openHandleScope(&scope);

	Array *classInstanceVariables = newArray(classGetInstanceShape(Handles.Class).varsSize);
	arrayAtPutObject(classInstanceVariables, 0, (Object *) asString("superClass"));
	arrayAtPutObject(classInstanceVariables, 1, (Object *) asString("subClasses"));
	arrayAtPutObject(classInstanceVariables, 2, (Object *) asString("methodDictionary"));
	arrayAtPutObject(classInstanceVariables, 3, (Object *) asString("instanceShape"));
	arrayAtPutObject(classInstanceVariables, 4, (Object *) asString("instanceVariables"));
	arrayAtPutObject(classInstanceVariables, 5, (Object *) asString("name"));
	arrayAtPutObject(classInstanceVariables, 6, (Object *) asString("comment"));
	arrayAtPutObject(classInstanceVariables, 7, (Object *) asString("category"));
	arrayAtPutObject(classInstanceVariables, 8, (Object *) asString("classVariables"));
	classSetInstanceVariables(Handles.Class, classInstanceVariables);

	size_t kernelDirNameSize = strlen(kernelDir);
	for (size_t i = 0; i < sizeof(kernelFiles) / sizeof(char *); i++) {
		size_t fileNameSize = strlen(kernelFiles[i]);
		char fileName[kernelDirNameSize + fileNameSize + 1];
		memcpy(fileName, kernelDir, kernelDirNameSize);
		fileName[kernelDirNameSize] = '/';
		memcpy(fileName + kernelDirNameSize + 1, kernelFiles[i], fileNameSize + 1);
		if (!parseFile(fileName, NULL, NULL)) {
			closeHandleScope(&scope, NULL);
			return 0;
		}
	}

	Iterator iterator;
	initDictIterator(&iterator, Handles.Smalltalk);
	while (iteratorHasNext(&iterator)) {
		HandleScope scope2;
		openHandleScope(&scope2);
		Association *assoc = (Association *) iteratorNextObject(&iterator);
		if (!isNil(assoc) && valueTypeOf(assoc->raw->value, VALUE_POINTER)) {
			Object *object = scopeHandle(asObject(assoc->raw->value));
			ASSERT(object->raw->class != NULL);
			if (object->raw->class->class == Handles.MetaClass->raw) {
				invokeInititalize(object);
			}
		}
		closeHandleScope(&scope2, NULL);
	}

	closeHandleScope(&scope, NULL);
	return 1;
}
