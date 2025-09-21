#ifndef COMPAT
#define COMPAT

// Tipi base
typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

typedef unsigned long DWORD;
typedef unsigned long long DWORD_PTR;
typedef int HRESULT;

// Handle ACM "finti"
typedef void* HACMDRIVERID;
typedef void* HACMDRIVER;

// LPCSTR, HBITMAP già definiti prima se servono
typedef const char* LPCSTR;
typedef void* HBITMAP;

// Convenzioni chiamata inutili su Linux/macOS
#ifndef __stdcall
#define __stdcall
#endif

#endif // WIN_COMPAT_H
