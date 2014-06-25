#ifndef _DEBUG_C
#define _DEBUG_C

#include "debug.h"

#define DEBUG(fmt, ...)\
do {\
    printDebug(fmt, __LINE__, __FILE__, ## __VA_ARGS__);\
}

void printDebug(const char *fmt, const int line, const char *file, ...) {
	return;
}

#endif /* _DEBUG_C */
