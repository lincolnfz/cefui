#ifndef _defexport_h
#define _defexport_h

/*
#ifdef _USE_MIRAGE_DLL
#ifdef MIRAGE_EXPORTS
#define MIRAGE_API extern "C" __declspec(dllexport)
#define MIRAGE_CLASS __declspec(dllexport)
#else
#define MIRAGE_API extern "C" __declspec(dllimport)
#define MIRAGE_CLASS __declspec(dllimport)
#endif
#else
#define MIRAGE_API
#define MIRAGE_CLASS
#endif
*/

#if defined(WRAPCEF_EXPORTS)
#  define SHARED_EXPORT_API extern "C" __declspec(dllexport)
#  define SHARED_EXPORT_CLASS __declspec(dllexport)
#else
#  define SHARED_EXPORT_API extern "C" __declspec(dllimport)
#  define SHARED_EXPORT_CLASS __declspec(dllimport)
#endif

#endif