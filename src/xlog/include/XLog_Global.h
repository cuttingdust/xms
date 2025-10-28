#ifndef XLOG_GLOBAL_H
#define XLOG_GLOBAL_H

#ifdef _WIN32

#ifdef xlog_STATIC
#define XLOG_EXPORT
#else
#ifdef xlog_EXPORTS
#define XLOG_EXPORT __declspec(dllexport)
#else
#define XLOG_EXPORT __declspec(dllimport)
#endif
#endif
#else
#define XPLATFROM_EXPORT
#endif

#endif // XLOG_GLOBAL_H
