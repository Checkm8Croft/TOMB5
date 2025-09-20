#pragma once
#include "../global/types.h"

void InitialiseFunctionTable();
void HWInitialise();
HRESULT HWBeginScene();
HRESULT HWEndScene();
bool _NVisible(GLVERTEX* v0, GLVERTEX* v1, GLVERTEX* v2);
bool _Visible(GLVERTEX* v0, GLVERTEX* v1, GLVERTEX* v2);
void SetCullCW();
void SetCullCCW();

extern void (*AddQuadSorted)(GLVERTEX* v, short v0, short v1, short v2, short v3, TEXTURESTRUCT* tex, long double_sided);
extern void (*AddTriSorted)(GLVERTEX* v, short v0, short v1, short v2, TEXTURESTRUCT* tex, long double_sided);
extern void (*AddQuadZBuffer)(GLVERTEX* v, short v0, short v1, short v2, short v3, TEXTURESTRUCT* tex, long double_sided);
extern void (*AddTriZBuffer)(GLVERTEX* v, short v0, short v1, short v2, TEXTURESTRUCT* tex, long double_sided);
extern void (*AddLineSorted)(GLVERTEX* v0, GLVERTEX* v1, short drawtype);
extern bool (*IsVisible)(GLVERTEX* v0, GLVERTEX* v1, GLVERTEX* v2);
extern HRESULT(*_BeginScene)();
extern HRESULT(*_EndScene)();
