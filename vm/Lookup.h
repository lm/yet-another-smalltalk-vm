#ifndef MESSAGE_H
#define MESSAGE_H

#include "Object.h"
#include "CompiledCode.h"
#include "String.h"
#include <stdint.h>

#define LOOKUP_CACHE_SIZE 4096

typedef struct {
	OBJECT_HEADER;
	Value selector;
	Value arguments;
} RawMessage;
OBJECT_HANDLE(Message);

typedef struct {
	RawClass *classes[LOOKUP_CACHE_SIZE];
	RawString *selectors[LOOKUP_CACHE_SIZE];
	uint8_t *codes[LOOKUP_CACHE_SIZE];
} LookupTable;

extern LookupTable LookupCache;

NativeCodeEntry lookupNativeCode(RawClass *class, RawString *selector);
NativeCode *getNativeCode(Class *class, CompiledMethod *method);


static intptr_t lookupHash(intptr_t classHash, intptr_t selectorHash)
{
	return (classHash ^ selectorHash) & LOOKUP_CACHE_SIZE - 1;
}


static void flushLookupCache(void)
{
	memset(&LookupCache, 0, sizeof(LookupCache.classes) + sizeof(LookupCache.selectors));
}


static NativeCodeEntry cachedLookupNativeCode(RawClass *class, RawString *selector)
{
	intptr_t hash = lookupHash((intptr_t) class, (intptr_t) selector);
	if (LookupCache.classes[hash] == class && LookupCache.selectors[hash] == selector) {
		return (NativeCodeEntry) LookupCache.codes[hash];
	}
	return lookupNativeCode(class, selector);
}

#endif
