#ifndef XPLATFROM_GLOBAL_H
#define XPLATFROM_GLOBAL_H

#ifdef _WIN32

#ifdef xplatfrom_STATIC
#define XPLATFROM_EXPORT
#else
#ifdef xplatfrom_EXPORTS
#define XPLATFROM_EXPORT __declspec(dllexport)
#else
#define XPLATFROM_EXPORT __declspec(dllimport)
#endif
#endif
#else
#define XPLATFROM_EXPORT
#endif

#include <memory>

#endif // XPLATFROM_GLOBAL_H
