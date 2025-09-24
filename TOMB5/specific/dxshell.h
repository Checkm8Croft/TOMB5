#pragma once
#include "../global/types.h"

// Fix DirectX/Win32 su macOS/Linux
#ifndef _WIN32
    typedef void* LPVOID;
    typedef char* LPSTR;
    typedef const char* LPCSTR;
    typedef int BOOL;
    typedef int HRESULT;

    typedef struct { unsigned int dummy; } DDSURFACEDESC2;
    typedef struct { unsigned int dummy; } DDPIXELFORMAT;
    typedef DDPIXELFORMAT* LPDDPIXELFORMAT;

    typedef struct { unsigned int dummy; } D3DDEVICEDESC;
    typedef D3DDEVICEDESC* LPD3DDEVICEDESC;

    #ifndef __stdcall
    #define __stdcall
    #endif
#endif

void DXReadKeyboard(char* KeyMap);
void DXBitMask2ShiftCnt(ulong mask, uchar* shift, uchar* count);
long DXAttempt(HRESULT r);
long DIAttempt(HRESULT r);
void* AddStruct(void* p, long num, long size);
long DXDDCreate(GUID* pGuid, void** pDD4);
long DXD3DCreate(DrawDevice pDD4, void** pD3D);
long DXSetCooperativeLevel(DrawDevice pDD4, HWindow hwnd, long flags);
HRESULT __stdcall DXEnumDisplayModes(DDSURFACEDESC2* lpDDSurfaceDesc2, LPVOID lpContext);
HRESULT __stdcall DXEnumZBufferFormats(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext);
HRESULT __stdcall DXEnumTextureFormats(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext);
long BPPToDDBD(long BPP);
long DXCreateD3DDevice(D3DInterface d3d, GUID guid, Surface surf, GLuint device);
long DXSetVideoMode(DrawDevice dd, long dwWidth, long dwHeight, long dwBPP);
long DXCreateSurface(DrawDevice dd, SurfaceDesc desc, Surface* surf);
long DXCreateViewport(D3DInterface d3d, GLuint device, long w, long h, Viewport* viewport);
void DXSaveScreen(Surface surf, const char* name);
HRESULT DXShowFrame();
void DXMove(long x, long y);
void DXClose();
long DXCreate(long w, long h, long bpp, long Flags, DXPTR* dxptr, HWindow hWnd, long WindowStyle);
long DXChangeVideoMode();
long DXToggleFullScreen();
HRESULT __stdcall DXEnumDirect3D(GUID* pGUID, LPSTR lpDeviceDescription, LPSTR lpDeviceName,
                                 LPD3DDEVICEDESC lpHWDesc, LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext);
BOOL __stdcall DXEnumDirectDraw(GUID* pGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext);
BOOL __stdcall DXEnumDirectSound(GUID* pGUID, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);
long DXGetInfo(DXINFO* dxinfo, HWindow hwnd);
void DXFreeInfo(DXINFO* dxinfo);
long DXUpdateJoystick();
void DXInitInput(HWindow hwnd, HInstance hinstance);
const char* DXGetErrorString(HRESULT hr);
const char* DIGetErrorString(HRESULT hr);

extern DXPTR* G_dxptr;
extern DXINFO* G_dxinfo;
extern DrawDevice G_ddraw;
extern D3DInterface G_d3d;
extern HWindow G_hwnd;
extern const Uint8* keymap;  // in dxshell.h
