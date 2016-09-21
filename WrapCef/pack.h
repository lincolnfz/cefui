#ifndef _PACK_H
#define _PACK_H

#ifdef __cplusplus
extern "C" {
#endif 

extern bool __stdcall zipFile2PackFile( const WCHAR* , const WCHAR* );

extern bool __stdcall exZipFile( const WCHAR* , const WCHAR* , unsigned char ** , unsigned long *);

extern void __stdcall freeExtfileBuf( const char* );

extern void __stdcall freeBuf(const unsigned char*);

#ifdef __cplusplus
}
#endif 


#endif