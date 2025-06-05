#ifndef CUTILS_CUTILS_H
#define CUTILS_CUTILS_H

/* Includes every cutils module. Modules switched off in the build
   (CUTILS_WITH_<MODULE>=OFF) can still be included, their functions just
   aren't available to link against. */

#include "cutils/allocator.h"
#include "cutils/arena.h"
#include "cutils/config.h"
#include "cutils/expected.h"
#include "cutils/list.h"
#include "cutils/map.h"
#include "cutils/pool.h"
#include "cutils/pqueue.h"
#include "cutils/queue.h"
#include "cutils/stack.h"
#include "cutils/string.h"
#include "cutils/time.h"
#include "cutils/vector.h"

#endif /* CUTILS_CUTILS_H */
