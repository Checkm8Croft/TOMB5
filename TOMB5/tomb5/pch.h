// pch.h: Precompiled header file per macOS/Linux
#ifndef PCH_H
#define PCH_H

// Standard C/C++ headers
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>

// Macros utili per compatibilità
#define _USE_MATH_DEFINES

// Stub per DirectX/Windows types
typedef int HRESULT;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef int BOOL;
typedef void* LPDIRECT3DDEVICE;
typedef void* LPDIRECTDRAW;
typedef void* LPDIRECTINPUTDEVICE;
typedef void* LPDIRECTSOUND;
typedef float D3DVALUE;

#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif

#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

// Stub structures Direct3D minimi
struct D3DVECTOR { float x, y, z; };
struct D3DMATRIX { float _11,_12,_13,_14; float _21,_22,_23,_24; float _31,_32,_33,_34; float _41,_42,_43,_44; };

// Stub per audio/macOS/Linux (da sostituire con SDL2/ALSA/OpenAL)
struct DSOUND_BUFFER {};



#endif // PCH_H
