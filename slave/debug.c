//Debug functions c file
//created by Nicholas Mykulowycz
//created on june 24, 2014

#ifdef DEBUG_MAIN

/*	following includes need to be in parent file	*/ 
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include "debug.h"

int main(void){

	DEBUG("using the debug statement to check if the debug function is working - classic\n");
	
	//printf("debuging the debug function is super fun, everyone should try it\n");
	
	//remember to use the -DDEBUG_FLAG option when compiling kids!
}
#endif /* DEBUG_MAIN */


/*
 * Prettyful debug function 
 * (DO NOT CALL THIS DIRECTLY - use DEBUG() instead, or else tiny elves will feast on your toes)
 */
void printDebug(const char *fmt, const int line, const char *file, ...) {
#ifdef DEBUG_FLAG
    va_list list;
    va_start(list, file);

    fprintf(stderr, "Debug: on line %d of file %s: ", line, file);
    vfprintf(stderr, fmt, list);
    putc('\n',stderr);
    
    va_end(list);
#endif /* DEBUG_FLAG */
}


/*
 * Error function (called regardless of DEBUG flag to let user know they fucked up)
 */
void eprintf(const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    char *prefix = "ERROR: ";
    
    fprintf(stderr, prefix);
    vfprintf(stderr, fmt, list);
//#warning ignore the "format not a string literal" bullshit
    fputc('\n', stderr);
    
    va_end(list);
 }