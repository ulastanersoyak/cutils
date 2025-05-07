#include "cutils/time.h"

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
  uint64_t time = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
  // Convert from 100-nanosecond intervals to milliseconds
  return time / 10000;
#else
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
#endif
}