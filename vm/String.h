#ifndef STRING_H
#define STRING_H

#include "Object.h"

typedef struct {
	OBJECT_HEADER;
	Value size;
	char contents[];
} RawString;
OBJECT_HANDLE(String);

String *newString(size_t size);
String *asString(char *buffer);

Value computeStringHash(String *string);
Value computeRawStringHash(RawString *string);
_Bool stringEquals(String *a, String *b);
_Bool stringEqualsC(String *a, char *b);
void stringPrintOn(String *str, char *buffer);
size_t computeArguments(String *selector);
void printValue(Value value);
void printRawObject(RawObject *object);
void printRawString(RawString *string);

#endif
