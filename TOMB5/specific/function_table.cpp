#include "../tomb5/pch.h"
#include "function_table.h"
#include "polyinsert.h"
#include "function_stubs.h"
#include "dxshell.h"
#include "winmain.h"
#include "3dmath.h"

void (*AddQuadSorted)(D3DTLVERTEX* v, short v0, short v1, short v2, short v3, TEXTURESTRUCT* tex, long double_sided);
void (*AddTriSorted)(D3DTLVERTEX* v, short v0, short v1, short v2, TEXTURESTRUCT* tex, long double_sided);
void (*AddQuadZBuffer)(D3DTLVERTEX* v, short v0, short v1, short v2, short v3, TEXTURESTRUCT* tex, long double_sided);
void (*AddTriZBuffer)(D3DTLVERTEX* v, short v0, short v1, short v2, TEXTURESTRUCT* tex, long double_sided);
void (*AddLineSorted)(D3DTLVERTEX* v0, D3DTLVERTEX* v1, short drawtype);
bool (*IsVisible)(D3DTLVERTEX* v0, D3DTLVERTEX* v1, D3DTLVERTEX* v2);
HRESULT(*_BeginScene)();
HRESULT(*_EndScene)();

void InitialiseFunctionTable()
{
	_BeginScene = HWBeginScene;
	_EndScene = HWEndScene;
	IsVisible = _NVisible;
	AddQuadZBuffer = AddQuadSubdivide;
	AddTriZBuffer = AddTriSubdivide;
	AddQuadSorted = AddQuadSubdivide;
	AddTriSorted = AddTriSubdivide;		
	AddLineSorted = AddLineClippedSorted;
}
#define D3DTSS_COLOROP 0
#define D3DTOP_DISABLE 0
#define D3DTOP_MODULATE 1
#define D3DTSS_ALPHAOP 2
#define D3DTOP_SELECTARG1 1
#define D3DTSS_COLORARG1 3
#define D3DTSS_COLORARG2 4
#define D3DTSS_ALPHAARG1 5
#define D3DTSS_ALPHAARG2 6
#define D3DTA_TEXTURE 1
#define D3DTA_DIFFUSE 2
#define D3DTA_TFACTOR 3
#define D3DTSS_TEXCOORDINDEX 7
#define D3DTSS_TCI_PASSTHRU 0

// filters (dummy)
#define D3DTSS_MINFILTER 0
#define D3DTSS_MAGFILTER 0
#define D3DTSS_MIPFILTER 0
#define D3DTFN_LINEAR 1
#define D3DTFP_NONE 0
#define D3DTFG_LINEAR 1
#define D3DTFG_POINT 0

// stub DXAttempt
#define DXAttempt(x) x
static inline void GL_SetTextureStageState(GLDevice* dev, int stage, int state, int value) {
    // Ignora D3DTSS_COLOROP / D3DTOP_DISABLE
    if(state == D3DTSS_COLOROP && value == D3DTOP_DISABLE) {
        glActiveTexture(GL_TEXTURE0 + stage);
        glBindTexture(GL_TEXTURE_2D, 0); // disabilita la texture stage
    }

    // Ignora TEXCOORDINDEX per ora, puoi implementare se servono più coordinate
    if(state == D3DTSS_TEXCOORDINDEX && value == D3DTSS_TCI_PASSTHRU) {
        // pass-through: non serve fare nulla per OpenGL fixed function
    }
}

void HWInitialise()
{
	Log(__FUNCTION__);



    // tutte le altre texture/alpha arg ignorate, perché OpenGL fixed function li usa direttamente

    // render states (dummy o equivalenti OpenGL)
    glDisable(GL_BLEND);       // D3DRENDERSTATE_ALPHABLENDENABLE = 0
    glEnable(GL_LIGHTING);     // speculare abilitato
    glDisable(GL_CULL_FACE);   // no culling
    glEnable(GL_DEPTH_TEST);   // z-buffer
    glDepthMask(GL_TRUE);      // write-enable
    glEnable(GL_TEXTURE_2D);   // texture perspective / perspective correction
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // D3DFILL_SOLID
    glEnable(GL_DITHER);       // dithering

    // alpha test
    glAlphaFunc(GL_NOTEQUAL, 0.0f);
    glDisable(GL_ALPHA_TEST);

    // luci (stub)
    // DXAttempt(App.dx.lpD3DDevice->SetLightState(...));

    // fog
    glDisable(GL_FOG);
}

HRESULT HWBeginScene()
{
	if (App.dx.InScene)
		Log("Already In Scene");

	App.dx.InScene = 1;
	App.dx.DoneBlit = 0;
	while (App.dx.WaitAtBeginScene) {};
}

HRESULT HWEndScene()
{
	App.dx.InScene = 0;
}

bool _NVisible(D3DTLVERTEX* v0, D3DTLVERTEX* v1, D3DTLVERTEX* v2)
{
	return (v0->sy - v1->sy) * (v2->sx - v1->sx) - (v2->sy - v1->sy) * (v0->sx - v1->sx) < 0;
}

bool _Visible(D3DTLVERTEX* v0, D3DTLVERTEX* v1, D3DTLVERTEX* v2)
{
	return (v0->sy - v1->sy) * (v2->sx - v1->sx) - (v2->sy - v1->sy) * (v0->sx - v1->sx) > 0;
}

void SetCullCW()
{
	IsVisible = _Visible;
}

void SetCullCCW()
{
	IsVisible = _NVisible;
}
