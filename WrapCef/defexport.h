#ifndef _defexport_h
#define _defexport_h

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

#endif