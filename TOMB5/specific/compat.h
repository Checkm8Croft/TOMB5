#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <GL/gl.h>
#include "../global/types.h"
// --- Forward struct "handle" types ---
typedef struct IDirect3D3         IDirect3D3;
typedef struct IDirect3DDevice3   IDirect3DDevice3;
typedef struct IDirectDraw4       IDirectDraw4;
typedef struct D3DVIEWPORT2       D3DVIEWPORT2;
typedef struct LPDIRECT3DVIEWPORT3   LPDIRECT3DVIEWPORT3;

// --- Punteri stile COM ---
typedef IDirect3D3*       LPDIRECT3D3;
typedef IDirect3DDevice3* LPDIRECT3DDEVICE3;
typedef IDirectDraw4*     LPDIRECTDRAW4;
typedef D3DVIEWPORT2*     LPD3DVIEWPORT2;
typedef uint32_t        DWORD;
typedef uint8_t         BYTE;
typedef uint16_t        WORD;
typedef uint32_t        ULONG;
typedef uint32_t        ULONG_PTR;
// --- D3DVIEWPORT2 compat ---
struct D3DVIEWPORT2 {
    uint32_t dwSize;
    uint32_t dwX, dwY, dwWidth, dwHeight;
    float dvMinZ, dvMaxZ;
};

// -------------------
// Fake OpenGL-backed interfaces
// -------------------
struct IDirect3D3 {
    // Funzioni simulate (array di puntatori a funzione)
    void (*CreateDevice)(LPDIRECT3DDEVICE3* outDevice);
    void (*EnumDevices)(void* lpContext);
    void (*GetDeviceCaps)(LPDIRECT3DDEVICE3 device, void* lpCaps);
    void (*CreateViewport)(LPDIRECT3DVIEWPORT3* outViewport, void* pUnk);
    
};

struct IDirect3DDevice3 {
    void (*SetTransform)(int state, const void* matrix);
    void (*DrawPrimitive)(int primType, void* vertices, int count);
    void (*SetRenderState)(int state, int value);
    void (*AddViewport)(LPDIRECT3DDEVICE3 device, LPDIRECT3DVIEWPORT3 viewport);
};

struct IDirectDraw4 {
    void (*SetCooperativeLevel)(void* hwnd, uint32_t flags);
    void (*SetDisplayMode)(int w, int h, int bpp, int refresh, uint32_t flags);
};

struct LPDIRECT3DVIEWPORT3{

};
// -------------------
// Esempi di "implementazioni" fittizie con OpenGL
// -------------------
static void GL_SetTransform(int state, const void* matrix) {
    // qui potresti fare glLoadMatrixf(...) ecc.
}

static void GL_DrawPrimitive(int primType, void* vertices, int count) {
    // qui potresti tradurre in glDrawArrays o simile
}

static void GL_SetRenderState(int state, int value) {
    // qui potresti usare glEnable/glDisable a seconda dello stato
}

// -------------------
// Costruttori fake per gli oggetti
// -------------------
static inline LPDIRECT3DDEVICE3 CreateFakeDevice3() {
    static IDirect3DDevice3 dev = {
        .SetTransform = GL_SetTransform,
        .DrawPrimitive = GL_DrawPrimitive,
        .SetRenderState = GL_SetRenderState
    };
    return &dev;
}

static inline LPDIRECT3D3 CreateFakeD3D3() {
    static IDirect3D3 d3d = {
        .CreateDevice = (void (*)(LPDIRECT3DDEVICE3*)) [](LPDIRECT3DDEVICE3* out) {
            *out = CreateFakeDevice3();
        }
    };
    return &d3d;
    LPDIRECT3D3 d3d;
}

static inline LPDIRECTDRAW4 CreateFakeDDraw4() {
    static IDirectDraw4 dd = {
        .SetCooperativeLevel = (void (*)(void*, uint32_t)) [](void*, uint32_t){},
        .SetDisplayMode = (void (*)(int,int,int,int,uint32_t)) [](int,int,int,int,uint32_t){}
    };
    return &dd;
    long dwBPP;
}

struct D3DVIEWPORT2 {
    uint32_t dwSize;
    uint32_t dwX, dwY, dwWidth, dwHeight;
    float dvMinZ, dvMaxZ;
    uint32_t dvClipX, dvClipY, dvClipWidth, dvClipHeight;
};
// Add these OpenGL-related structs and types
typedef struct {
    GLuint fbo;           // Framebuffer object
    GLuint colorBuffer;   // Color render target
    GLuint depthBuffer;   // Depth/stencil buffer
    int width;
    int height;
} GLSurface;

// Add viewport structure compatible with both DX and GL
struct D3DVIEWPORT2 {
    uint32_t dwSize;
    uint32_t dwX;
    uint32_t dwY; 
    uint32_t dwWidth;
    uint32_t dwHeight;
    float dvMinZ;
    float dvMaxZ;
    float dvClipX;
    float dvClipY; 
    float dvClipWidth;
    float dvClipHeight;
};

// Add these OpenGL helper functions
static inline GLSurface* CreateGLSurface(int width, int height) {
    GLSurface* surf = (GLSurface*)malloc(sizeof(GLSurface));
    
    // Generate FBO and attachments
    glGenFramebuffers(1, &surf->fbo);
    glGenTextures(1, &surf->colorBuffer);
    glGenRenderbuffers(1, &surf->depthBuffer);
    
    surf->width = width;
    surf->height = height;
    
    return surf;
}

static inline void DeleteGLSurface(GLSurface* surf) {
    if (!surf) return;
    
    glDeleteFramebuffers(1, &surf->fbo);
    glDeleteTextures(1, &surf->colorBuffer); 
    glDeleteRenderbuffers(1, &surf->depthBuffer);
    
    free(surf);
}

// Add viewport setup function
static inline void SetGLViewport(const D3DVIEWPORT2* vp) {
    glViewport(vp->dwX, vp->dwY, vp->dwWidth, vp->dwHeight);
    glDepthRange(vp->dvMinZ, vp->dvMaxZ);
}

// Replace DirectX device with OpenGL context wrapper
typedef struct {
    SDL_GLContext glContext;
    SDL_Window* window;
    int width;
    int height;
} GLDevice;

// Add these function replacements
#define DXCreateD3DDevice(d3d, guid, surface, device) CreateGLDevice(device)
#define DXCreateViewport(d3d, device, w, h, viewport) CreateGLViewport(w, h, viewport)
#define DXSetVideoMode(dd, w, h, bpp) SetGLVideoMode(w, h)
#define DXCreateSurface(dd, desc, surf) CreateGLSurface(desc.dwWidth, desc.dwHeight)