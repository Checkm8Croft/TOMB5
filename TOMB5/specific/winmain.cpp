#include "../tomb5/pch.h"
#include "winmain.h"
#include "dxshell.h"
#include "function_stubs.h"
#include "output.h"
#include "../game/text.h"
#include "lighting.h"
#include "function_table.h"
#include "d3dmatrix.h"
#include "3dmath.h"
#include "audio.h"
#include "registry.h"
#include "../game/gameflow.h"
#include "texture.h"
#include "gamemain.h"
#include "file.h"
#include "fmv.h"
#include "setupdlg.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/file.h>   // LOCK_EX, LOCK_NB, LOCK_UN
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define KA_ALTENTER 1
#define KA_ALTP     2
#define KA_ALTM     3
WINAPP App;
long resChangeCounter;

static COMMANDLINES commandlines[] =
{
    { "SETUP", 0, &CLSetup },
    { "NOFMV", 0, &CLNoFMV }
};

// ------------------ SINGLE INSTANCE ------------------
int WinRunCheck(const char* WindowName, int* mutex)
{
    if (!WindowName) WindowName = "tomb5";

    const char* tmpdir = getenv("TMPDIR");
    if (!tmpdir) tmpdir = "/tmp";

    char lockpath[PATH_MAX];
    snprintf(lockpath, sizeof(lockpath), "%s/tomb5_%s.lock", tmpdir, WindowName);

    int fd = open(lockpath, O_RDWR | O_CREAT, 0600);
    if (fd < 0) {
        if (mutex) *mutex = 0;
        return 0;
    }


    // scriviamo PID nel lockfile
    char pidbuf[32];
    int plen = snprintf(pidbuf, sizeof(pidbuf), "%ld\n", (long)getpid());
    if (plen > 0) {
        ftruncate(fd, 0);
        lseek(fd, 0, SEEK_SET);
        write(fd, pidbuf, (size_t)plen);
        fsync(fd);
    }

    if (mutex) *mutex = fd;
    return 0;
}

void WinRunRelease(int mutex)
{
    if (mutex <= 0) return;
    close(mutex);
}

// ------------------ CLEAR SURFACES ------------------
void ClearSurfaces()
{
    int x = (int)App.dx.rViewport.left;
    int y = (int)App.dx.rViewport.top;
    int w = (int)App.dx.rViewport.right;
    int h = (int)App.dx.rViewport.bottom;

    if (w <= 0 || h <= 0) return;

    int winH = (App.dx.dwRenderHeight > 0) ? (int)App.dx.dwRenderHeight
                                           : (int)(App.dx.rScreen.top + App.dx.rScreen.bottom);

    int scissorY = winH - (y + h);
    if (scissorY < 0) scissorY = 0;

    glEnable(GL_SCISSOR_TEST);
    glScissor(x, scissorY, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    S_DumpScreen();
}

// ------------------ FPS / TIMER ------------------
float WinFrameRate()
{
    static uint64_t last_time = 0;
    static float fps = 0;
    static int counter = 0;

    counter++;
    if (!last_time) last_time = SDL_GetPerformanceCounter();

    if (counter >= 10)
    {
        uint64_t now = SDL_GetPerformanceCounter();
        double t = (double)(now - last_time) / (double)SDL_GetPerformanceFrequency();
        fps = (float)(counter / t);
        counter = 0;
        last_time = now;
    }

    App.fps = fps;
    return fps;
}


// ------------------ COMMAND LINE ------------------
void CLSetup(char* cmd) { start_setup = (cmd) ? 0 : 1; }
void CLNoFMV(char* cmd) { fmvs_disabled = (cmd) ? 0 : 1; }

void WinProcessCommandLine(const char* cmdline)
{
    if (!cmdline) return;

    char cmd[1024];
    strncpy(cmd, cmdline, sizeof(cmd));
    cmd[sizeof(cmd)-1] = 0;

    // converti tutto in maiuscolo
    for (int i = 0; cmd[i]; i++) cmd[i] = toupper(cmd[i]);

    size_t num = sizeof(commandlines) / sizeof(commandlines[0]);
    for (size_t i = 0; i < num; i++)
    {
        COMMANDLINES* command = &commandlines[i];
        char* p = strstr(cmd, command->command);
        if (p)
            command->code(command->needs_parameter ? p+strlen(command->command) : 0);
    }
}


void* GameThreadFunc(void*)
{
    MainThread.active = 1;
    MainThread.ended = 0;

    GameMain(0);

    MainThread.ended = 1;
    MainThread.active = 0;
    return nullptr;
}

// ------------------ SDL EVENT LOOP ------------------
void WinProcMsg()
{
    SDL_Event event;
    while (!MainThread.ended)
    {
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    MainThread.ended = 1;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_RETURN &&
                        (event.key.keysym.mod & KMOD_ALT))
                    {
                        WinProcessCommands(KA_ALTENTER);
                    }
                    break;

                default:
                    break;
            }
        }
        SDL_Delay(1);
    }
}

// ------------------ PROCESS COMMANDS ------------------
void WinProcessCommands(long cmd)
{
    if (cmd == KA_ALTENTER)
    {
        // gestione fullscreen toggle SDL/OpenGL
        // TODO: implementare il toggle video mode via SDL
    }
}

// ------------------ WINDOW INIT ------------------
SDL_Window* gWindow = nullptr;
SDL_GLContext gGLContext = nullptr;

bool WinCreateWindow(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    gWindow = SDL_CreateWindow("Tomb Raider Chronicles",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               width, height,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!gWindow)
    {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    gGLContext = SDL_GL_CreateContext(gWindow);
    if (!gGLContext)
    {
        printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetSwapInterval(1); // VSync
    return true;
}

void WinClose()
{
    if (gGLContext) SDL_GL_DeleteContext(gGLContext);
    if (gWindow) SDL_DestroyWindow(gWindow);
    SDL_Quit();
}

// ------------------ MAIN ------------------
int main(int argc, char** argv)
{
    int mutex_fd;
    if (WinRunCheck("tomb5", &mutex_fd)) return 1;

    LoadGameflow();
    WinProcessCommandLine((argc > 1) ? argv[1] : nullptr);

    if (!WinCreateWindow(1024, 768))
    {
        WinRunRelease(mutex_fd);
        return 1;
    }

    if (start_setup || !LoadSettings())
    {
        if (!SetupDialog())
        {
            free(gfScriptFile);
            free(gfLanguageFile);
            WinClose();
            WinRunRelease(mutex_fd);
            return 1;
        }
        LoadSettings();
    }

    if (!fmvs_disabled && !LoadBinkStuff())
    {
        printf("Failed to load Bink, disabling FMVs.\n");
        fmvs_disabled = 1;
    }

    pthread_t thread;
    pthread_create(&thread, nullptr, GameThreadFunc, nullptr);

    WinProcMsg();

    pthread_join(thread, nullptr);

    WinClose();
    WinRunRelease(mutex_fd);
    return 0;
}
