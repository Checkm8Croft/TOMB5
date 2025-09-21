#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "audio.h"

static Mix_Music* currentMusic = nullptr;
static bool acm_ready = false;

bool ACMInit()
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL_Init audio failed: %s", SDL_GetError());
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("Mix_OpenAudio failed: %s", Mix_GetError());
        return false;
    }
    acm_ready = true;
    return true;
}

void ACMClose()
{
    if (!acm_ready) return;
    Mix_HaltMusic();
    Mix_FreeMusic(currentMusic);
    currentMusic = nullptr;
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    acm_ready = false;
}

void S_CDPlay(long track, long mode)
{
    if (!acm_ready) return;

    char filename[256];
    snprintf(filename, sizeof(filename), "audio/%s", TrackFileNames[track]);

    if (currentMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(currentMusic);
    }

    currentMusic = Mix_LoadMUS(filename);
    if (!currentMusic) {
        SDL_Log("Cannot load music %s: %s", filename, Mix_GetError());
        return;
    }

    int loops = (mode == 1) ? -1 : 0; // 1 = loop, 2 = once
    Mix_PlayMusic(currentMusic, loops);
}

void S_CDStop()
{
    if (!acm_ready) return;
    Mix_HaltMusic();
}

void S_CDFade(long ms)
{
    if (!acm_ready) return;
    Mix_FadeOutMusic(ms);
}

void ACMSetVolume()
{
    Mix_VolumeMusic(MusicVolume * MIX_MAX_VOLUME / 100);
}
