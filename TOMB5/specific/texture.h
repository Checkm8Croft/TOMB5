#pragma once
#include "../global/types.h"
typedef void (*rgbfunc)(unsigned char* r, unsigned char* g, unsigned char* b);
Surface CreateTexturePage(long w, long h, long MipMapCount, long* pSrc, rgbfunc RGBM, long format);
void FreeTextures();

extern TEXTURE Textures[64];
extern long nTextures;
