// texture.cpp -- OpenGL-based texture handling (C-style as much as possible)

#include "../tomb5/pch.h"
#include "texture.h"
#include "function_stubs.h"
#include "dxshell.h"
#include "3dmath.h"
#include "winmain.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

long nTextures = 256;


// Assunzione: `Surface` è un tipo compatibile con un puntatore/integer.
// Per conservare compatibilità col resto del codice, ritorniamo l'ID OpenGL
// incapsulato in Surface (cast tramite uintptr_t).
// Se Surface è typedef void* o simile, questo mantiene un rappresentazione.
static inline Surface Surface_FromGLTex(GLuint id) {
    Surface s;
    s.gl_id = id;
    return s;
}

static inline GLuint Surface_ToGLTex(Surface s) {
    return s.gl_id;
}


// Funzione di utilità: logga errori GL (opzionale)
static void CheckGLError(const char* ctx) {
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) {
        Log("GL Error (%s): 0x%X", ctx ? ctx : "ctx", (unsigned)e);
    }
}

// Crea una texture OpenGL a partire dai dati pSrc.
// Restituisce Surface che contiene l'ID OpenGL (o 0 su errore).
// NOTE:
// - Questa implementazione crea una texture RGBA8 (32 bit).
// - mappatura da pSrc: replica la formula di campionamento originale
//   usando lS + x * 256 / w + y * 0x10000 / h come indice (come nel codice originale).
// - RGBM callback viene chiamato se fornita, per adattare r,g,b prima di upload.

Surface CreateTexturePage(long w, long h, long MipMapCount, long* pSrc, rgbfunc RGBM, long format)
{
    if (w <= 0 || h <= 0 || !pSrc) {
        return Surface_FromGLTex(0);
    }

    // Se dimens. troppo piccole, ignoriamo mipmap
    if (w < 32 || h < 32) {
        MipMapCount = 0;
    }

    // Allochiamo buffer RGBA (unsigned byte) per upload OpenGL
    const size_t pixelCount = (size_t)w * (size_t)h;
    const size_t bufSize = pixelCount * 4; // RGBA8
    unsigned char* pixels = (unsigned char*)malloc(bufSize);
    if (!pixels) {
        Log("CreateTexturePage: malloc failed (%ldx%ld)", w, h);
        return Surface_FromGLTex(0);
    }
    memset(pixels, 0, bufSize);

    // Copia / conversione dei pixel: manteniamo la stessa logica di campionamento
    // usata nel codice originale (sebbene le formule possano dipendere da dati esterni).
    for (unsigned long yy = 0; yy < (unsigned long)h; ++yy) {
        for (unsigned long xx = 0; xx < (unsigned long)w; ++xx) {
            // Indice sorgente calcolato come nell'implementazione originale:
            // lS + x * 256 / w + y * 0x10000 / h
            // useremo arithmetic con long per rimanere coerenti.
            long srcIndex = (long)(xx * 256 / w) + (long)((yy * 0x10000) / h);
            // Nota: il buffer pSrc deve essere abbastanza grande; non facciamo check oltre.
            unsigned long c = (unsigned long) *(pSrc + srcIndex);

            unsigned char r = (unsigned char)CLRR(c);
            unsigned char g = (unsigned char)CLRG(c);
            unsigned char b = (unsigned char)CLRB(c);
            unsigned char a = (unsigned char)CLRA(c);

            if (RGBM) {
                // RGBM può modificare r,g,b in place
                RGBM(&r, &g, &b);
            }

            // Posizione nel buffer RGBA (row-major)
            size_t dstPos = ((size_t)yy * (size_t)w + (size_t)xx) * 4;
            pixels[dstPos + 0] = r;
            pixels[dstPos + 1] = g;
            pixels[dstPos + 2] = b;
            pixels[dstPos + 3] = a;
        }
    }

    // Genera texture OpenGL
    GLuint texID = 0;
    glGenTextures(1, &texID);
    if (texID == 0) {
        Log("CreateTexturePage: glGenTextures failed");
        free(pixels);
        return Surface_FromGLTex(0);
    }

    glBindTexture(GL_TEXTURE_2D, texID);

    // Parametri texture (puoi adattare in base a App.Filtering o simili)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // considerare mipmaps se MipMapCount>0
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Upload dati (RGBA8)
    glTexImage2D(GL_TEXTURE_2D,
                 0,                 // livello base
                 GL_RGBA8,          // internal format
                 (GLsizei)w,
                 (GLsizei)h,
                 0,                 // border
                 GL_RGBA,           // formato dei dati
                 GL_UNSIGNED_BYTE,  // tipo dei dati
                 pixels);

    // Eventuale generazione mipmap se richiesta
    if (MipMapCount > 0) {
#ifdef GL_GENERATE_MIPMAP
        glGenerateMipmap(GL_TEXTURE_2D);
#else
        // per compatibilità: chiamata a glGenerateMipmap se disponibile
        #ifdef GL_FRAMEBUFFER_SRGB
        glGenerateMipmap(GL_TEXTURE_2D);
        #endif
#endif
    }

    CheckGLError("CreateTexturePage upload");

    // Liberiamo buffer client
    free(pixels);

    // Disaccoppiamo binding
    glBindTexture(GL_TEXTURE_2D, 0);

    // Ritorniamo la texture incapsulata in Surface
    return Surface_FromGLTex(texID);
}

// Rilascia tutte le texture gestite in Textures[]
void FreeTextures()
{
    TEXTURE* tex;
    Log(__FUNCTION__);

    // Disassocia texture da pipeline (se avevi chiamate DX equivalenti)
    // Con GL basta cancellare gli ID
    for (int i = 0; i < nTextures; ++i)
    {
        tex = &Textures[i];

        // Se tex->tex conteneva un puntatore COM, convertiamo via uintptr_t.
        // Se era già GLuint salvato, la conversione produce lo stesso ID.
        memset(&tex->tex, 0, sizeof(Surface));
		

        
        // tex->surface non usato in GL-version: azzeriamo se presente
        memset(&tex->tex, 0, sizeof(Surface));
		memset(&tex->surface, 0, sizeof(SurfaceDesc));
    }

    CheckGLError("FreeTextures");
}
