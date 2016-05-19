#ifndef _cefexport_h
#define _cefexport_h


#ifndef _USE_WRAPCEF_LIB
#if defined(WRAPCEF_EXPORTS)
#  define SHARED_EXPORT_API extern "C" __declspec(dllexport)
#  define SHARED_EXPORT_CLASS __declspec(dllexport)
#else
#  define SHARED_EXPORT_API extern "C" __declspec(dllimport)
#  define SHARED_EXPORT_CLASS __declspec(dllimport)
#endif
#else
#  define SHARED_EXPORT_API
#  define SHARED_EXPORT_CLASS
#endif // !_USE_MIRAGE_LIB



#endif