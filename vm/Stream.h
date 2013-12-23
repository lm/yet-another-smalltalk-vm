#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include "Object.h"
#include "String.h"

#define EXTERNAL_STREAM_BODY \
	OBJECT_HEADER; \
	Value buffer; \
	Value position; \
	Value buffered; \
	Value atEnd; \
	Value descriptor

typedef struct {
	EXTERNAL_STREAM_BODY;
} RawExternalStream;
OBJECT_HANDLE(ExternalStream);

typedef struct {
	EXTERNAL_STREAM_BODY;
	Value name;
} RawFileStream;
OBJECT_HANDLE(FileStream);

typedef struct {
	OBJECT_HEADER;
	Value messageText;
} RawIoError;
OBJECT_HANDLE(IoError);

int streamOpen(RawString *fileName, intptr_t mode);
_Bool streamClose(int descriptor);
ptrdiff_t streamRead(int descriptor, void *buffer, size_t size);
ptrdiff_t streamWrite(int descriptor, void *buffer, size_t size);
_Bool streamFlush(int descriptor);
_Bool streamAtEnd(int descriptor);
ptrdiff_t streamGetPosition(int descriptor);
_Bool streamSetPosition(int descriptor, ptrdiff_t position);
intptr_t streamAvailable(int descriptor);
IoError *getLastIoError(void);

#endif

