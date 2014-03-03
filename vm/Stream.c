#include "Stream.h"
#include "Thread.h"
#include "Handle.h"
#include "Heap.h"
#include "Assert.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#if !defined(TEMP_FAILURE_RETRY)
	#define TEMP_FAILURE_RETRY(expression) \
		({ \
			int64_t __result; \
			do { \
				__result = (int64_t) expression; \
			} while (__result == -1L && errno == EINTR); \
			__result; \
		})
#endif


int streamOpen(RawString *fileName, intptr_t mode)
{
	HandleScope scope;
	openHandleScope(&scope);
	String *fileNameHandle = scopeHandle(fileName);
	char space[256];
	char *buffer = space;

	if (fileName->size > 256) {
		String *tmpString = (String *) copyResizedObject((Object *) fileNameHandle, fileName->size + 1);
		buffer = tmpString->raw->contents;
		buffer[fileName->size] = '\0';
	} else {
		stringPrintOn(fileNameHandle, buffer);
	}

	closeHandleScope(&scope, NULL);

	mode_t openMode = 0;
	switch (mode) {
	case 1:
		openMode = O_RDONLY;
		break;
	case 1 << 1:
		openMode = O_WRONLY;
		break;
	case 1 << 2:
		openMode = O_RDWR;
		break;
	default:
		return -1;
	}

	return TEMP_FAILURE_RETRY(open(buffer, openMode));
}


_Bool streamClose(int descriptor)
{
	return close(descriptor) == 0;
}


ptrdiff_t streamRead(int descriptor, void *buffer, size_t size)
{
	return TEMP_FAILURE_RETRY(read(descriptor, buffer, size));
}


ptrdiff_t streamWrite(int descriptor, void *buffer, size_t size)
{
	return TEMP_FAILURE_RETRY(write(descriptor, buffer, size));
}


_Bool streamFlush(int descriptor)
{
	return TEMP_FAILURE_RETRY(fsync(descriptor)) == 0;
}


/*_Bool streamAtEnd(RawFileStream *stream)
{
	return feof(stream->file);
}*/


ptrdiff_t streamGetPosition(int descriptor)
{
	return TEMP_FAILURE_RETRY(lseek(descriptor, 0, SEEK_CUR));
}


_Bool streamSetPosition(int descriptor, ptrdiff_t position)
{
	return TEMP_FAILURE_RETRY(lseek(descriptor, position, SEEK_SET)) != -1;
}


intptr_t streamAvailable(int descriptor)
{
	int available;
	int result = TEMP_FAILURE_RETRY(ioctl(descriptor, FIONREAD, &available));
	if (result < 0) {
		return result;
	}
	return available;
}


IoError *getLastIoError(void)
{
	HandleScope scope;
	openHandleScope(&scope);

	char msg[256] = "IoError: ";
	strerror_r(errno, msg + 9, 256 - 9);
	IoError *error = newObject(Handles.IoError, 0);
	objectStorePtr((Object *) error,  &error->raw->messageText, (Object *) asString(msg));

	return closeHandleScope(&scope, error);
}
