#ifndef CUTILS_TIME_H
#define CUTILS_TIME_H

#include <stdbool.h>
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

/**
 * @brief Check whether more than timeout_ms have passed since start_ms
 *
 * Used by the *_timeout operations of every container. On bare metal builds
 * there is no clock, so this never reports a timeout.
 *
 * @param start_ms Value of cutils_get_current_time_ms when the operation began
 * @param timeout_ms Time the operation is allowed to take
 * @return true if the operation took longer than timeout_ms
 */
bool cutils_timeout_expired (uint64_t start_ms, uint32_t timeout_ms);

#endif // CUTILS_TIME_H
