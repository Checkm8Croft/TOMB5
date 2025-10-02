#pragma once
#include "../global/types.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

extern int MusicVolume;

// Simuliamo i driver ACM con SDL2
typedef void* HACMDRIVER;
typedef int HACMDRIVERID;
typedef unsigned long DWORD_PTR;

// --- Funzioni principali (da rimappare) ---

void S_CDPlay(long track, long mode);
void S_CDStop();
void S_CDFade(long n);
void S_StartSyncedAudio(long track);

void ACMSetVolume();
void OpenStreamFile(char* name);
void GetADPCMData();
void ACMEmulateCDPlay(long track, long mode);

// Callback fittizia (non usata realmente in SDL2)
bool ACMEnumCallBack(HACMDRIVERID hadid, DWORD_PTR dwInstance);

long ACMSetupNotifications();
void FillADPCMBuffer(char* p, long track);
long ACMHandleNotifications();

bool ACMInit();
void ACMClose();

// --- Variabili globali ---

extern HACMDRIVER hACMDriver;
extern unsigned char* wav_file_buffer;
extern unsigned char* ADPCMBuffer;
extern bool acm_ready;
extern long XATrack;
extern long XAFlag;
extern const char* TrackFileNames[];
// --- Nuove strutture per SDL2 ---
extern Mix_Music* current_music;
extern Mix_Chunk* current_sound;
