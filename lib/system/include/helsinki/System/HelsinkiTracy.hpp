#pragma once


#ifdef HELSINKI_TRACY_ENABLE
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>
#else
#define ZoneScoped
#define ZoneScopedN(x)
#define ZoneNameF(x,...)
#endif