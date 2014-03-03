#include "Os.h"
#include "Assert.h"
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <stddef.h>


int64_t osCurrentMicroTime(void)
{
	struct timeval time;
	int result = gettimeofday(&time, NULL);
	if (result != 0) {
		FAIL();
	}
	return time.tv_sec * 1000000 + time.tv_usec;
}


void osCreateThread(OsThread *thread, void *(*cb) (void *), void *arg)
{
	pthread_create(thread, NULL, cb, arg);
}


void osExitThread(void *result)
{
	pthread_exit(result);
}


void osSleep(uint64_t microseconds)
{
	struct timespec toSleep = {
		.tv_sec = microseconds / 1000000,
		.tv_nsec = microseconds % 1000000,
	};
	if (nanosleep(&toSleep, NULL) != 0) {
		FAIL()
	};
}
