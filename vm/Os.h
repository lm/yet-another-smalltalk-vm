#ifndef OS_H
#define OS_H

#include <stdint.h>
#include <pthread.h>

typedef pthread_t OsThread;
typedef pthread_mutex_t OsMutex;

int64_t osCurrentMicroTime(void);
void osCreateThread(OsThread *thread, void *(*cb) (void *), void *arg);
void osExitThread(void *result);
void osMutexInit(OsMutex *mutex);
void osMutexFree(OsMutex *mutex);
void osMutexLock(OsMutex *mutex);
void osMutexUnlock(OsMutex *mutex);
void osSleep(uint64_t microseconds);

#endif
