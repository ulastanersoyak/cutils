#ifndef CUTILS_TIME_H
#define CUTILS_TIME_H

#include <stdint.h>

/**
 * @brief Get the current time in milliseconds since the epoch
 *
 * This function returns the current time in milliseconds since the Unix epoch
 * (January 1, 1970, 00:00:00 UTC). The implementation is platform-independent
 * and works on both Windows and Unix-like systems.
 *
 * @return uint64_t The current time in milliseconds
 */
uint64_t cutils_get_current_time_ms (void);

#endif // CUTILS_TIME_H