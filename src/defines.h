#pragma once

#ifdef _MSC_VER
#define POSIDEON_PLATFORM_WINDOWS
#endif

#ifdef POSIDEON_ASSERTS
#ifdef POSIDEON_PLATFORM_WINDOWS
#define POSIDEON_ASSERT(expr) if (!(expr)) { __debugbreak(); }
#endif
#endif