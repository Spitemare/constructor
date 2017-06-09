#pragma once
#include <pebble.h>
#include "logging.h"
#include "enamel.h"

#ifdef PBL_PLATFORM_APLITE
#define PBL_IF_LOW_MEM_ELSE(if_true, if_false) (if_true)
#else
#define PBL_IF_LOW_MEM_ELSE(if_true, if_false) (if_false)
#endif

#ifdef PBL_PLATFORM_BASALT
#define PBL_QUICK_VIEW_ENABLED
#endif

#ifdef PBL_PLATFORM_DIORITE
#define PBL_QUICK_VIEW_ENABLED
#endif

typedef GColor (*EnamelGColorFunction)();
typedef uint32_t (*EnamelUint32Function)();
typedef int32_t (*EnamelInt32Function)();
typedef bool (*EnamelBoolFunction)();
typedef const char* (*EnamelStringFunction)();
