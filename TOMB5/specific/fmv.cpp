#include "../tomb5/pch.h"
#include "fmv.h"
#include "dxshell.h"
#include "audio.h"
#include "function_stubs.h"
#include "lighting.h"
#include "function_table.h"
#include "winmain.h"
#include "input.h"
#include "3dmath.h"
#include "../game/text.h"
#include "d3dmatrix.h"
#include "dxsound.h"
#include "gamemain.h"
#include "LoadSave.h"
#include "setupdlg.h"
#include <SDL2/SDL.h>

SDL_Window* g_Window = nullptr;
int g_VideoWidth = 800;
int g_VideoHeight = 600;
bool g_Fullscreen = true;
bool fmvs_disabled= false;

long DXChangeVideoMode()
{
    if (!g_Window) return -1;

    Uint32 flags = g_Fullscreen ? SDL_WINDOW_FULLSCREEN : 0;

    // Cambia la risoluzione della finestra
    SDL_SetWindowSize(g_Window, g_VideoWidth, g_VideoHeight);

    // Imposta fullscreen/windowed
    if (SDL_SetWindowFullscreen(g_Window, flags) != 0) {
        SDL_Log("Errore nel cambiare fullscreen: %s", SDL_GetError());
        return -1;
    }

    // Aggiorna il renderer (se serve)
    SDL_Renderer* renderer = SDL_GetRenderer(g_Window);
    if (renderer) {
        SDL_RenderSetLogicalSize(renderer, g_VideoWidth, g_VideoHeight);
    }

    return 0; // successo
}


#define GET_DLL_PROC(dll, proc, n) \
{ \
	*(FARPROC *)&(proc) = GetProcAddress((dll), n); \
	if(!proc) throw #proc; \
}

static void(__stdcall* BinkCopyToBuffer)(BINK_STRUCT*, LPVOID, long, long, long, long, long);
static void(__stdcall* BinkOpenDirectSound)(ulong);
static void(__stdcall* BinkSetSoundSystem)(LPVOID, LPDIRECTSOUND);
static LPVOID(__stdcall* BinkOpen)(char*, ulong);
static long(__stdcall* BinkDDSurfaceType)(SurfaceDesc*);
static long(__stdcall* BinkDoFrame)(BINK_STRUCT*);
static void(__stdcall* BinkNextFrame)(BINK_STRUCT*);
static long(__stdcall* BinkWait)(BINK_STRUCT*);
static void(__stdcall* BinkClose)(BINK_STRUCT*);

static long nFmvFrames[9] = { 880, 1826, 3869, 3112, 1903, 1973, 3200, 2799, 1725 };

static BINK_STRUCT* Bink;
static SurfaceDesc BinkSurface;
static long BinkSurfaceType;


long PlayFmv(long num)
{
	DXDISPLAYMODE* modes;
	DXDISPLAYMODE* current;
	long dm, rm, ndms;
	char name[80];
	char path[80];

	if (MainThread.ended)
		return 0;

	/*
	if ((1 << num) & FmvSceneTriggered)
		return 1;

	FmvSceneTriggered |= 1 << num;
	*/
	S_CDStop();

	if (fmvs_disabled)
		return 0;

	sprintf(name, "movie\\fmv%01d.bik", num);
	memset(path, 0, sizeof(path));
	strcat(path, name);
	Log("PlayFMV %s", path);
	App.fmv = 1;
	rm = 0;
	dm = App.DXInfo.nDisplayMode;
	current = &modes[dm];

	if (current->bpp != 16 || current->w != 640 || current->h != 480)
	{

		for (int i = 0; i < ndms; i++, modes++)
		{
			if (modes->bpp == 16 && modes->w == 640 && modes->h == 480)
			{
				App.DXInfo.nDisplayMode = i;
				break;
			}
		}

		DXChangeVideoMode();
		HWInitialise();
		ClearSurfaces();
		rm = 1;
	}

}

long PlayFmvNow(long num, long u)
{
	switch (num)
	{
	case 0:
		PlayFmv(0);
		break;

	case 1:
		PlayFmv(1);
		break;

	case 2:
		PlayFmv(2);
		break;

	case 3:
		PlayFmv(3);
		PlayFmv(4);
		break;

	case 4:
		PlayFmv(5);
		PlayFmv(6);
		break;

	case 5:
		PlayFmv(7);
		break;

	case 6:
		PlayFmv(8);
		break;
	}

	return 0;
}
