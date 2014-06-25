//debuging functions header file
//Created by Nicholas Mykulowycz
//Created on June 24, 2014

//debug related functions were moved here to ensure drivers could operate properly outside of the min program

#ifndef DEBUG_H_
#define DEBUG_H_

// use this to provide dev-only feedback. See printDebug() too.
#define DEBUG(fmt, ...)\
do {\
    printDebug(fmt, __LINE__, __FILE__, ## __VA_ARGS__);\
} while(0)

// don't call this - call DEBUG() instead
void printDebug(const char *fmt, const int line, const char *file, ...);

// error print function, works like printf()
void eprintf(const char *fmt, ...);

#endif /* DEBUG_H_ */