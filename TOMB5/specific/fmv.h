#pragma once
#include "../global/types.h"
#include <SDL2/SDL.h>

long PlayFmv(long num);
long PlayFmvNow(long num, long u);
extern int g_VideoWidth;
extern int g_VideoHeight;
extern bool g_Fullscreen;
extern SDL_Window* g_Window;
