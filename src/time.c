#include "cutils/time.h"
#include "cutils/config.h"

#include <stddef.h>

#if !CUTILS_PLATFORM_BARE_METAL
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
#endif

static cutils_time_source_t g_time_source = NULL;

void
cutils_set_time_source (cutils_time_source_t now_ms)
{
  g_time_source = now_ms;
}

#if !CUTILS_PLATFORM_BARE_METAL
static uint64_t
system_time_ms (void)
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
#endif

uint64_t
cutils_get_current_time_ms (void)
{
  if (g_time_source != NULL)
    {
      return g_time_source ();
    }
#if CUTILS_PLATFORM_BARE_METAL
  return 0;
#else
  return system_time_ms ();
#endif
}

bool
cutils_timeout_expired (uint64_t start_ms, uint32_t timeout_ms)
{
#if CUTILS_PLATFORM_BARE_METAL
  // without a clock there is nothing to measure against
  if (g_time_source == NULL)
    {
      return false;
    }
#endif
  return cutils_get_current_time_ms () - start_ms > timeout_ms;
}
