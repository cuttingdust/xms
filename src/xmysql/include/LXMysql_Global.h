#ifndef LXMYSQL_GLOBAL_H
#define LXMYSQL_GLOBAL_H

#ifdef _WIN32

#ifdef xmysql_STATIC
#define LXM_EXPORT
#else
#ifdef xmysql_EXPORTS
#define LXM_EXPORT __declspec(dllexport)
#else
#define LXM_EXPORT __declspec(dllimport)
#endif
#endif
#else
#define LXM_EXPORT
#endif

#endif // LXMYSQL_GLOBAL_H
