#ifndef OBJECT_H
#define OBJECT_H

#include "Assert.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define OBJECT_HANDLE(name) \
	typedef union name { \
		Raw##name *raw; \
		uintptr_t ptr; \
	} name

#define HEADER_SIZE (2 * sizeof(Value))
#define OBJECT_HEADER \
	struct RawClass *class; \
	uint32_t hash; \
	uint8_t unused; \
	uint8_t payloadSize; \
	uint8_t varsSize; \
	uint8_t tags

enum {
	SPACE_TAG = 1 << 3,
	NEW_SPACE_TAG = SPACE_TAG,
	OLD_SPACE_TAG = 0,
	HEAP_OBJECT_ALIGN = 16,
};

enum {
	TAG_FREESPACE = 1,
	TAG_MARKED = 1 << 2,
	TAG_FORWARDED = 1 << 3,
	TAG_FINALIZED = 1 << 4,
	TAG_REMEMBERED = 1 << 5,
} ObjectTag;

typedef enum {
	VALUE_INT = 0,
	VALUE_POINTER = 1,
	VALUE_CHAR = 2,
} ValueType;

typedef uintptr_t Value;
typedef intptr_t SignedValue;

typedef struct {
	uint8_t tag;
	uint8_t payloadSize;
	uint8_t varsSize;
	uint8_t isIndexed;
	uint8_t isBytes;
	uint8_t valueType;
	uint16_t size;
} InstanceShape;

typedef struct RawClass {
	OBJECT_HEADER;
	Value superClass;
	Value subClasses;
	Value methodDictionary;
	InstanceShape instanceShape;
	Value instanceVariables;
	Value name;
	Value comment;
	Value category;
	Value classVariables;
} RawClass;
OBJECT_HANDLE(Class);

typedef struct {
	OBJECT_HEADER;
	Value superClass;
	Value subClasses;
	Value methodDictionary;
	InstanceShape instanceShape;
	Value instanceVariables;
	Value instanceClass;
} RawMetaClass;
OBJECT_HANDLE(MetaClass);

typedef struct {
	OBJECT_HEADER;
	uint8_t body[];
} RawObject;
OBJECT_HANDLE(Object);

typedef struct {
	OBJECT_HEADER;
	Value size;
	uint8_t body[];
} RawIndexedObject;
OBJECT_HANDLE(IndexedObject);

typedef struct {
	OBJECT_HEADER;
	Value size;
	Value vars[];
} RawArray;
OBJECT_HANDLE(Array);

typedef struct {
	OBJECT_HEADER;
	Value key;
	Value value;
} RawAssociation;
OBJECT_HANDLE(Association);

#define COMPUTE_INST_SHAPE_SIZE(aPayloadSize, aVarsSize, aIsIndexed) \
	HEADER_SIZE + ((aIsIndexed) + (aPayloadSize) + (aVarsSize)) * sizeof(Value)
#define DEFINE_INST_SHAPE(aPayloadSize, aVarsSize, aIsIndexed, aIsBytes, aValueType) { \
	.tag = 0, \
	.payloadSize = (aPayloadSize), \
	.varsSize = (aVarsSize), \
	.isIndexed = (aIsIndexed), \
	.isBytes = (aIsBytes), \
	.valueType = (aValueType), \
	.size = COMPUTE_INST_SHAPE_SIZE(aPayloadSize, aVarsSize, aIsIndexed), \
}

static InstanceShape FixedShape = DEFINE_INST_SHAPE(0, 0, 0, 0, 0);
static InstanceShape IndexedShape = DEFINE_INST_SHAPE(0, 0, 1, 0, VALUE_POINTER);
static InstanceShape StringShape = DEFINE_INST_SHAPE(0, 0, 1, 1, VALUE_CHAR);
static InstanceShape BytesShape = DEFINE_INST_SHAPE(0, 0, 1, 1, VALUE_INT);
static InstanceShape CompiledCodeShape = DEFINE_INST_SHAPE(1, 0, 1, 1, VALUE_INT);
static InstanceShape BlockShape = DEFINE_INST_SHAPE(1, 0, 0, 0, 0);
static InstanceShape ContextShape = DEFINE_INST_SHAPE(2, 0, 1, 0, VALUE_POINTER);
static InstanceShape ExceptionHandlerShape = DEFINE_INST_SHAPE(1, 2, 0, 0, VALUE_POINTER);

#define varOffset(type, member) (offsetof(type, member) - 1)

static inline size_t computeInstanceSize(InstanceShape shape, size_t size);
static inline size_t computeObjectSize(Object *object);
static inline size_t computeRawObjectSize(RawObject *object);
static inline size_t objectSize(Object *object);
static inline size_t rawObjectSize(RawObject *object);
static inline Value *getObjectVars(Object *object);
static inline Value *getRawObjectVars(RawObject *object);
static inline Value *getRawObjectVarsFromShape(RawObject *object, InstanceShape shape);
static inline uint8_t *getObjectIndexedVars(Object *object);
static inline uint8_t *getRawObjectIndexedVars(RawObject *object);
static inline uint8_t *getRawObjectIndexedVarsFromShape(RawObject *object, InstanceShape shape);
static inline uintptr_t objectGetHash(Object *object);
static inline _Bool isNewObject(RawObject *object);
static inline _Bool isOldObject(RawObject *object);

static inline intptr_t asCInt(Value value);
static inline char asCChar(Value value);
static inline RawObject *asObject(Value value);

static inline Value tagInt(intptr_t i);
static inline Value tagChar(char ch);
static inline Value tagPtr(void *object);

static inline _Bool valueTypeOf(Value value, ValueType type);


static inline size_t computeInstanceSize(InstanceShape shape, size_t size)
{
	size_t varSize = shape.isBytes ? 1 : sizeof(Value);
	return shape.size + size * varSize;
}


static inline size_t computeObjectSize(Object *object)
{
	return computeRawObjectSize(object->raw);
}


static inline size_t computeRawObjectSize(RawObject *object)
{
	return computeInstanceSize(object->class->instanceShape, rawObjectSize(object));
}


static inline size_t objectSize(Object *object)
{
	return rawObjectSize(object->raw);
}


static inline size_t rawObjectSize(RawObject *object)
{
	return object->class->instanceShape.isIndexed ? ((RawIndexedObject *) object)->size : 0;
}


static inline Value *getObjectVars(Object *object)
{
	return getRawObjectVars(object->raw);
}


static inline Value *getRawObjectVars(RawObject *object)
{
	return getRawObjectVarsFromShape(object, object->class->instanceShape);
}


static inline Value *getRawObjectVarsFromShape(RawObject *object, InstanceShape shape)
{
	return (Value *) (object->body + (shape.isIndexed + shape.payloadSize) * sizeof(Value));
}


static inline uint8_t *getObjectIndexedVars(Object *object)
{
	return getRawObjectIndexedVars(object->raw);
}


static inline uint8_t *getRawObjectIndexedVars(RawObject *object)
{
	return getRawObjectIndexedVarsFromShape(object, object->class->instanceShape);
}


static inline uint8_t *getRawObjectIndexedVarsFromShape(RawObject *object, InstanceShape shape)
{
	return (uint8_t *) (getRawObjectVarsFromShape(object, shape) + shape.varsSize);
}


static inline uintptr_t objectGetHash(Object *object)
{
	return object->raw->hash;
}


static inline _Bool isNewObject(RawObject *object)
{
	return ((uintptr_t) object & SPACE_TAG) == NEW_SPACE_TAG;
}


static inline _Bool isOldObject(RawObject *object)
{
	return ((uintptr_t) object & SPACE_TAG) == OLD_SPACE_TAG;
}


static inline intptr_t asCInt(Value value)
{
	ASSERT((value & 3) == VALUE_INT);
	return (SignedValue) value >> 2;
}


static inline char asCChar(Value value)
{
	ASSERT((value & 3) == VALUE_CHAR);
	return (char) (value >> 2);
}


static inline RawObject *asObject(Value value)
{
	ASSERT((value & 3) == VALUE_POINTER);
	return (RawObject *) (value - 1);
}


static inline Value tagInt(intptr_t i)
{
	int64_t max = (int64_t) (UINT64_MAX >> 2);
	int64_t min = -max;
	ASSERT(min <= i && i <= max);
	return i << 2;
}


static inline Value tagChar(char ch)
{
	return (Value) (ch << 2) + VALUE_CHAR;
}


static inline Value tagPtr(void *object)
{
	return (Value) object + VALUE_POINTER;
}


static inline _Bool valueTypeOf(Value value, ValueType type)
{
	return (value & 3) == type;
}

#endif
