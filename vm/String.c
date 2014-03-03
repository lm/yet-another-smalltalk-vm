#include "String.h"
#include "Thread.h"
#include "Heap.h"
#include "Smalltalk.h"
#include "Handle.h"
#include "../cityhash/city.h"
#include <string.h>


String *newString(size_t size)
{
	return (String *) newObject(Handles.String, size);
}


String *asString(char *buffer)
{
	size_t size = strlen(buffer);
	String *string = newString(size);
	memcpy(string->raw->contents, buffer, size);
	return string;
}


Value computeStringHash(String *string)
{
	return computeRawStringHash(string->raw);
}


Value computeRawStringHash(RawString *string)
{
	return CityHash64(string->contents, string->size) << 2 >> 2;
}


_Bool stringEquals(String *a, String *b)
{
	return a->raw->size == b->raw->size && memcmp(a->raw->contents, b->raw->contents, a->raw->size) == 0;
}


_Bool stringEqualsC(String *a, char *b)
{
	size_t size = strlen(b);
	return a->raw->size == size && memcmp(a->raw->contents, b, size) == 0;
}


void stringPrintOn(String *str, char *buffer)
{
	memcpy(buffer, str->raw->contents, str->raw->size);
	buffer[str->raw->size] = '\0';
}


size_t computeArguments(String *selector)
{
	size_t size = selector->raw->size;
	char *s = selector->raw->contents;
	char *end = s + size;
	size_t count = 0;

	if (s[size - 1] == ':') {
		while (s != end && (s = memchr(s, ':', end - s)) != NULL) {
			count++;
			s++;
		}
	} else if ((s[0] < 'A' || s[0] > 'Z') && (s[0] < 'a' || s[1] > 'z')) {
		count++;
	}
	return count;
}


void printValue(Value value)
{
	if (valueTypeOf(value, VALUE_CHAR)) {
		printf("%c", asCChar(value));
	} else if (valueTypeOf(value, VALUE_INT)) {
		printf("%zi", asCInt(value));
	} else {
		printRawObject(asObject(value));
	}
}


void printRawObject(RawObject *object)
{
	RawString *string;
	if (object->class == Handles.String->raw) {
		string = (RawString *) object;
	} else if (object->class == Handles.Symbol->raw) {
		printf("#");
		string = (RawString *) object;
	} else if (object->class->class == Handles.MetaClass->raw) {
		string = (RawString *) asObject(((RawClass *) object)->name);
	} else if (object->class == Handles.MetaClass->raw) {
		RawMetaClass *metaClass = (RawMetaClass *) object;
		RawClass *class = (RawClass *) asObject(metaClass->instanceClass);
		string = (RawString *) asObject(class->name);
	} else {
		string = (RawString *) asObject(object->class->name);
		printf("a ");
	}
	printRawString(string);
}


void printRawString(RawString *string)
{
	ASSERT(string->class == Handles.String->raw || string->class == Handles.Symbol->raw);
	printf("%.*s", (int) string->size, string->contents);
}
