#ifndef OS_H
#define OS_H

#include <stdint.h>
#include <pthread.h>

typedef pthread_t OsThread;

int64_t osCurrentMicroTime(void);
void osCreateThread(OsThread *thread, void *(*cb) (void *), void *arg);
void osExitThread(void *result);
void osSleep(uint64_t microseconds);

#endif
