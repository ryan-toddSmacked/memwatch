#ifndef __MEMWATCH_C_DECL_H
#define __MEMWATCH_C_DECL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stddef.h>
#include <inttypes.h>


typedef enum watch_dtypes
{
    watch_asChar = 0,
    watch_asInt8,
    watch_asInt16,
    watch_asInt32,
    watch_asInt64,
    watch_asUint8,
    watch_asUint16,
    watch_asUint32,
    watch_asUint64,
    watch_asFloat,
    watch_asDouble,
    watch_asPtr
} dtype_t;



/**
 * Wrapper for printf from stdio.h.
 * Please call this instead of printf directly.
 * Undefined behavior if printf is called directly.
 */
void watch_printf (const char* fmt, ...);


/**
 * Initialize memory watching, takes over terminal.
 */
void watch_init (void);

/**
 * Always call when done watching.
 * Un-allocates stuff.
 */
void watch_end (void);


/**
 * Returns how many addresses can be watched.
 */
size_t watch_howMany (void);


/**
 * Sets the specified row to watch over the passed address.
 */
void watch_this (const void* addr, size_t specificRow, dtype_t type);


/**
 * Remove row from watch list.
 */
void watch_notThis (size_t specificRow);


/**
 * Ncurses does not handle threading as of yet..
 * Call this function and the memory window will refresh.
 */
void watch_refresh (void);



#ifdef __cplusplus
}
#endif

#endif
