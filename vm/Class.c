#include "Class.h"
#include "Heap.h"
#include "Handle.h"
#include "Smalltalk.h"
#include "Heap.h"
#include "Collection.h"
#include "Dictionary.h"
#include "Iterator.h"
#include "Compiler.h"
#include "Assert.h"
#include <string.h>
#include <stdarg.h>

static _Bool resolveSuperClass(ClassNode *node, MetaClass **superMetaClass, Class **superClass, _Bool *isRoot);
static Class *subClass(ClassNode *node, MetaClass *superMetaClass, Class *superClass);
static CompileError *processPragmas(Class *class, OrderedCollection *pragmas);
static CompileError *processShapePragma(MessageExpressionNode *pragma, Class *class);
static CompileError *declareVariables(Class *class, OrderedCollection *vars, Array *superInstVars);
static _Bool isClassVar(String *name);
static CompileError *compileMethods(Class *class, OrderedCollection *methods);
static CompileError *compileAndInstallMethod(MethodNode *node, Class *class, Dictionary *methodDict);
static CompileError *createMethodRedefinitionError(MethodNode *node);
static Class *installClass(Class *class);
static void addSubClass(Class *class, Class *subClass);


Object *buildClass(ClassNode *node)
{
	HandleScope handleScope;
	openHandleScope(&handleScope);

	CompileError *error;
	MetaClass *superMetaClass;
	Class *superClass;
	_Bool isRoot;
	if (!resolveSuperClass(node, &superMetaClass, &superClass, &isRoot)) {
		error = createUndefinedVariableError(classNodeGetSuperName(node));
		return (Object *) closeHandleScope(&handleScope, error);
	}

	Class *class = subClass(node, superMetaClass, superClass);
	MetaClass *metaClass = classGetMetaClass(class);

	OrderedCollection *pragmas = classNodeGetPragmas(node);
	error = processPragmas(class, pragmas);
	if (error != NULL) {
		return (Object *) closeHandleScope(&handleScope, error);
	}

	OrderedCollection *vars = classNodeGetVars(node);
	Array *superInstVars = classGetInstanceVariables(superClass);
	error = declareVariables(class, vars, superInstVars);
	if (error != NULL) {
		return (Object *) closeHandleScope(&handleScope, error);
	}

	OrderedCollection *methods = classNodeGetMethods(node);
	error = compileMethods(class, methods);
	if (error != NULL) {
		return (Object *) closeHandleScope(&handleScope, error);
	}

	class = installClass(class);
	if (isRoot) {
		classSetSuperClass(class, (Class *) Handles.nil);
	} else {
		addSubClass(superClass, class);
	}
	metaClassSetInstanceClass(metaClass, class);
	return (Object *) closeHandleScope(&handleScope, class);
}


static _Bool resolveSuperClass(ClassNode *node, MetaClass **superMetaClass, Class **superClass, _Bool *isRoot)
{
	String *name = literalNodeGetStringValue(classNodeGetSuperName(node));
	if (stringEqualsC(name, "nil")) {
		*superMetaClass = (MetaClass *) Handles.Class;
		*superClass = Handles.UndefinedObject;
		*isRoot = 1;
		return 1;
	}

	Class *super = (Class *) globalObjectAt(asSymbol(name));
	if (!isNil(super)) {
		*superMetaClass = classGetMetaClass(super);
		*superClass = super;
		*isRoot = 0;
		return 1;
	}

	return 0;
}


static Class *subClass(ClassNode *node, MetaClass *superMetaClass, Class *superClass)
{
	MetaClass *metaClass = (MetaClass *) newObject(Handles.MetaClass, 0);
	metaClassSetSuperClass(metaClass, superMetaClass);
	metaClassSetSubClasses(metaClass, newOrdColl(16));
	metaClassSetInstanceShape(metaClass, metaClassGetInstanceShape(superMetaClass));
	metaClassSetInstanceVariables(metaClass, metaClassGetInstanceVariables(superMetaClass));

	Class *class = (Class *) newObject((Class *) metaClass, 0);
	classSetSuperClass(class, superClass);
	classSetSubClasses(class, newOrdColl(16));
	classSetName(class, asSymbol(literalNodeGetStringValue(classNodeGetName(node))));
	classSetInstanceShape(class, classGetInstanceShape(superClass));

	metaClassSetInstanceClass(metaClass, class);
	return class;
}


static CompileError *processPragmas(Class *class, OrderedCollection *pragmas)
{
	MessageExpressionNode *pragma;
	CompileError *error;
	Iterator iterator;

	initOrdCollIterator(&iterator, pragmas, 0, 0);
	while (iteratorHasNext(&iterator)) {
		MessageExpressionNode *pragma = (MessageExpressionNode *) iteratorNextObject(&iterator);
		String *selector = messageExpressionNodeGetSelector(pragma);
		if (stringEqualsC(selector, "shape:")) {
			error = processShapePragma(pragma, class);
			if (error != NULL) {
				return error;
			}
		}
	}

	return NULL;
}


static CompileError *processShapePragma(MessageExpressionNode *pragma, Class *class)
{
	OrderedCollection *args = messageExpressionNodeGetArgs(pragma);
	ASSERT(ordCollSize(args) == 1);
	LiteralNode *arg = (LiteralNode *) ordCollObjectAt(args, 0);
	Value value;
	CompileError *error;

	if (arg->raw->class == Handles.VariableNode->raw) {
		value = globalAt(asSymbol(literalNodeGetStringValue(arg)));
	} else if (arg->raw->class == Handles.IntegerNode->raw) {
		value = literalNodeGetValue(arg);
	} else {
		goto error;
	}

	classSetInstanceShape(class, *(InstanceShape *) &value);
	return NULL;

error:
	error = (CompileError *) newObject(Handles.InvalidPragmaError, 0);
	compileErrorSetVariable(error, (LiteralNode *) pragma);
	return error;
}


static CompileError *declareVariables(Class *class, OrderedCollection *vars, Array *superInstVars)
{
	OrderedCollection *tmpInstVars = newOrdColl(8);
	Dictionary *classVars = newDictionary(8);
	Iterator iterator;

	initOrdCollIterator(&iterator, vars, 0, 0);
	while (iteratorHasNext(&iterator)) {
		String *name = literalNodeGetStringValue((LiteralNode *) iteratorNextObject(&iterator));
		if (isClassVar(name)) {
			symbolDictAtPutObject(classVars, asSymbol(name), Handles.nil);
		} else {
			ordCollAddObject(tmpInstVars, (Object *) name);
		}
	}

	Array *instVars;
	size_t size = objectSize((Object *) superInstVars);
	if (size == 0) {
		instVars = ordCollAsArray(tmpInstVars);
	} else {
		instVars = (Array *) copyResizedObject((Object *) superInstVars, size + ordCollSize(tmpInstVars));
		initOrdCollIterator(&iterator, tmpInstVars, 0, 0);
		while (iteratorHasNext(&iterator)) {
			ptrdiff_t index = size + iteratorIndex(&iterator);
			arrayAtPutObject(instVars, index, iteratorNextObject(&iterator));
		}
	}

	InstanceShape shape = classGetInstanceShape(class);
	shape.varsSize = instVars->raw->size;
	shape.size = COMPUTE_INST_SHAPE_SIZE(shape.payloadSize, shape.varsSize, shape.isIndexed);
	classSetInstanceShape(class, shape);

	classSetInstanceVariables(class, instVars);
	classSetClassVariables(class, classVars);
	return NULL;
}


static _Bool isClassVar(String *name)
{
	return name->raw->contents[0] >= 'A' && name->raw->contents[0] <= 'Z';
}


static CompileError *compileMethods(Class *class, OrderedCollection *methods)
{
	HandleScope scope;
	openHandleScope(&scope);

	MetaClass *metaClass = classGetMetaClass(class);
	Dictionary *methodDict = newDictionary(64);
	Dictionary *classMethodDict = newDictionary(32);
	CompileError *error;
	Iterator iterator;

	initOrdCollIterator(&iterator, methods, 0, 0);
	while (iteratorHasNext(&iterator)) {
		HandleScope scope2;
		openHandleScope(&scope2);

		MethodNode *methodNode = (MethodNode *) iteratorNextObject(&iterator);
		if (isNil(methodNodeGetClassName(methodNode))) {
			error = compileAndInstallMethod(methodNode, class, methodDict);
		} else { // TODO: check for class name: methodNode->className == "class"
			error = compileAndInstallMethod(methodNode, (Class *) metaClass, classMethodDict);
		}
		if (error != NULL) {
			return closeHandleScope(&scope, closeHandleScope(&scope2, error));
		}

		closeHandleScope(&scope2, NULL);
	}

	classSetMethodDictionary(class, methodDict);
	metaClassSetMethodDictionary(metaClass, classMethodDict);
	closeHandleScope(&scope, NULL);
	return NULL;
}


static CompileError *compileAndInstallMethod(MethodNode *node, Class *class, Dictionary *methodDict)
{
	if (!isNil(symbolDictObjectAt(methodDict, asSymbol(methodNodeGetSelector(node))))) {
		return createMethodRedefinitionError(node);
	}

	Object *method = compileMethod(node, class);
	if (method->raw->class == Handles.CompiledMethod->raw) {
		symbolDictAtPutObject(methodDict, compiledMethodGetSelector((CompiledMethod *) method), method);
		return NULL;
	} else {
		return (CompileError *) method;
	}
}


static CompileError *createMethodRedefinitionError(MethodNode *node)
{
	LiteralNode *literal = newObject(Handles.VariableNode, 0);
	literalNodeSetValue(literal, (Object *) methodNodeGetSelector(node));
	literalNodeSetSourceCode(literal, methodNodeGetSourceCode(node));
	return createRedefinitionError(literal)	;
}


static Class *installClass(Class *class)
{
	String *name = classGetName(class);
	Object *currentClass = globalObjectAt(name);
	if (isNil(currentClass)) {
		globalAtPut(name, getTaggedPtr(class));
		return class;
	} else {
		// TODO: temporarily do memcpy() instead of #become:
		memcpy(currentClass->raw, class->raw, sizeof(*class->raw));
		return (Class *) currentClass;
	}
}


static void addSubClass(Class *class, Class *subClass)
{
	ordCollAddObject(classGetSubClasses(class), (Object *) subClass);
	ordCollAddObject(metaClassGetSubClasses(classGetMetaClass(class)), (Object *) classGetMetaClass(subClass));
}


CompiledMethod *lookupSelector(Class *startClass, String *selector)
{
	HandleScope scope;
	openHandleScope(&scope);

	Class *class = startClass;
	CompiledMethod *method = (CompiledMethod *) symbolDictObjectAt(classGetMethodDictionary(class), selector);

	while (isNil(method)) {
		class = classGetSuperClass(class);
		if (isNil(class)) {
			return closeHandleScope(&scope, NULL);
		}
		method = (CompiledMethod *) symbolDictObjectAt(classGetMethodDictionary(class), selector);
	}

	return closeHandleScope(&scope, method);
}


void printClassName(RawClass *class)
{
	_Bool isMetaClass = class->class == Handles.MetaClass->raw;
	if (isMetaClass) {
		class = (RawClass *) asObject(((RawMetaClass *) class)->instanceClass);
	}
	RawString *className = (RawString *) asObject(class->name);
	printf("%.*s", (int) className->size, className->contents);
	if (isMetaClass) {
		printf(" class");
	}
}
