#ifndef XCOM_GLOBAL_H
#define XCOM_GLOBAL_H

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

#endif // XCOM_GLOBAL_H
