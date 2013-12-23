#ifndef SMALLTALK_H
#define SMALLTALK_H

#include "Object.h"
#include "String.h"
#include "Dictionary.h"
#include "Handle.h"

#define SYMBOL_TABLE_SIZE 1024

String *asSymbol(String *string);
String *getSymbol(char *s);
void setGlobal(char *key, Value value);
void setGlobalObject(char *key, Object *value);
Value getGlobal(char *key);
Object *getGlobalObject(char *key);
void globalAtPut(String *key, Value value);
Value globalAt(String *key);
Object *globalObjectAt(String *key);
Class *getClass(char *key);
void objectBecome(Object *object, Object *other);


static _Bool isNil(void *p)
{
	return ((Object *) p)->raw == Handles.nil->raw;
}


static _Bool isRawNil(void *p)
{
	return (RawObject *) p == Handles.nil->raw;
}


static _Bool isTaggedNil(Value value)
{
	return value == getTaggedPtr(Handles.nil);
}


static _Bool isTaggedTrue(Value value)
{
	return value == getTaggedPtr(Handles.true);
}


static Object *asBool(_Bool bool)
{
	return bool ? Handles.true : Handles.false;
}

#endif
