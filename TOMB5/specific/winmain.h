#pragma once
#include "../global/types.h"
#include <SDL2/SDL.h>

void ClearSurfaces();
bool WinRunCheck(LPSTR WindowName, LPSTR ClassName, void* mutex);
float WinFrameRate();
void WinProcMsg();
void WinProcessCommands(long cmd);
void CLSetup(char* cmd);
void CLNoFMV(char* cmd);
void WinProcessCommandLine(LPSTR cmd);
long WinMainWndProc(SDL_Window*, uint32_t uMsg, uint32_t wParam, uint32_t lParam);
void WinClose();
bool WinRegisterWindow(HInstance hinstance);
bool WinCreateWindow();
void WinSetStyle(bool fullscreen, ulong& set);
int __stdcall WinMain(HInstance hInstance, HInstance hPrevInstance, char* lpCmdLine, int nShowCmd);

extern WINAPP App;
extern long resChangeCounter;
