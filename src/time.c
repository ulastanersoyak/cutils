#include "cutils/time.h"
#include "cutils/config.h"

#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

uint64_t
cutils_get_current_time_ms (void)
{
#ifdef _WIN32
  FILETIME ft;
  GetSystemTimeAsFileTime (&ft);
  uint64_t intervals = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
  // Convert from 100-nanosecond intervals since 1601 to milliseconds since
  // the Unix epoch
  return (intervals - 116444736000000000ULL) / 10000;
#else
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}

bool
cutils_timeout_expired ([[maybe_unused]] uint64_t start_ms,
                        [[maybe_unused]] uint32_t timeout_ms)
{
#if CUTILS_PLATFORM_BARE_METAL
  return false;
#else
  return cutils_get_current_time_ms () - start_ms > timeout_ms;
#endif
}
