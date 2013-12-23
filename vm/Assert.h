#ifndef ASSERT_H
#define ASSERT_H

#include <stdlib.h>
#include <stdio.h>

#ifdef NDEBUG
#define ASSERT(cond) while(0 && (cond)) {}
#define FAIL() { \
		printf("Fatal error\n"); \
		exit(EXIT_FAILURE); \
	}
#else
#define ASSERT(cond) \
	if (!(cond)) { \
		printf("Assertion '%s' failed in %s:%u\n", #cond, __FILE__, __LINE__); \
		abort(); \
	}
#define FAIL() { \
		printf("Fatal error\n"); \
		abort(); \
	}
#endif

#endif
