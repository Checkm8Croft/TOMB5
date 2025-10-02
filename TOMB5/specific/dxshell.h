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
    typedef struct { unsigned int dummy;
    uint32_t dwSize;
    uint32_t dwFlags;           // 👈 usato ovunque
    uint32_t dwFourCC;
    uint32_t dwRGBBitCount;     // 👈 usato come "bpp"
    uint32_t dwRBitMask;        // 👈 maschera rossa
    uint32_t dwGBitMask;        // 👈 maschera verde
    uint32_t dwBBitMask;        // 👈 maschera blu
    uint32_t dwRGBAlphaBitMask; // 👈 maschera alpha
} DDPIXELFORMAT;
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
HRESULT __stdcall DXEnumDisplayModes(Surface* lpDDSurfaceDesc2, LPVOID lpContext);
HRESULT __stdcall DXEnumZBufferFormats(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext);
HRESULT __stdcall DXEnumTextureFormats(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext);
long BPPToDDBD(long BPP);
long DXCreateD3DDevice(D3DInterface d3d, GUID guid, Surface surf, GLuint device);
long DXSetVideoMode(DrawDevice dd, long dwWidth, long dwHeight, long dwBPP);
long DXCreateSurface(DrawDevice dd, SurfaceDesc desc, Surface* surf);
long DXCreateViewport(D3DInterface d3d, GLuint device, long w, long h);
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
extern DrawDevice G_ddraw;
extern D3DInterface G_d3d;
extern HWindow G_hwnd;
extern const Uint8* keymap;
static inline int Surface_Lock(Surface* s, int a, void* desc, int flags, int b) {
    (void)s; (void)a; (void)desc; (void)flags; (void)b;
    return 0; // success
}

static inline int Surface_Unlock(Surface* s, int a) {
    (void)s; (void)a;
    return 0;
}

static inline int Surface_Release(Surface* s) {
    (void)s;
    return 0;
}
/* HRESULT-like macros */
#ifndef DD_OK
#define DD_OK 0
#endif
#ifndef DI_OK
#define DI_OK 0
#endif
#ifndef DD_FALSE
#define DD_FALSE 0
#endif

/* IID placeholders */
#ifndef IID_IDirectDraw4
#define IID_IDirectDraw4 ((void*)0)
#endif
#ifndef IID_IDirect3D3
#define IID_IDirect3D3 ((void*)0)
#endif

/* DirectDraw flags stubs (examples) */
#define DDSCL_ALLOWMODEX          0x00000001
#define DDSCL_ALLOWREBOOT         0x00000002
#define DDSCL_CREATEDEVICEWINDOW  0x00000004
#define DDSCL_EXCLUSIVE           0x00000008
#define DDSCL_FPUSETUP            0x00000010
#define DDSCL_FULLSCREEN          0x00000020
#define DDSCL_MULTITHREADED       0x00000040
#define DDSCL_NORMAL              0x00000080
#define DDSCL_NOWINDOWCHANGES     0x00000100
#define DDSCL_SETDEVICEWINDOW     0x00000200
#define DDSCL_SETFOCUSWINDOW      0x00000400

/* generic no-op macros to silence FAILED(...) on non-HRESULT values */
#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

/* define common DirectInput/DirectDraw return macros if needed */
#define DDENUMRET_CANCEL 0
#define DDENUMRET_OK     1

#define D3DENUMRET_CANCEL 0
#define D3DENUMRET_OK     1
#define DDPF_ALPHAPIXELS    0x00000001
#define DDPF_ALPHA          0x00000002
#define DDPF_FOURCC         0x00000004
#define DDPF_RGB            0x00000040
#define DDPF_PALETTEINDEXED4 0x00000008
#define DDPF_PALETTEINDEXED8 0x00000020
#define DDBD_1               0x00004000
#define DDBD_2               0x00002000
#define DDBD_4               0x00001000
#define DDBD_8               0x00000800
#define DDBD_16              0x00000400
#define DDBD_24              0x00000200
#define DDBD_32              0x00000100
