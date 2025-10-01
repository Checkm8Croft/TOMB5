#include <stddef.h>
#include <sys/types.h>
#include <pthread.h>

#include "dxsound.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
// ====== Definizioni concrete ======
struct ACMSTREAMHEADER {
    unsigned long cbStruct;
    unsigned long cbSrcLength;
    unsigned long cbDstLength;
    unsigned char* pbSrc;
    unsigned char* pbDst;
};

struct DSBUFFERDESC {
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwBufferBytes;
    unsigned long dwReserved;
    void* lpwfxFormat;
};

// ====== Globali ======
char* samples_buffer = nullptr;
LPDIRECTSOUNDBUFFER DSPrimary = nullptr;

// ====== Dummy fake DirectSound buffer ======
struct FakeDSBuffer {
    int size;
    char* data;
    bool playing;
};

// ====== Esempio funzione minima ======
static ACMSTREAMHEADER ACMStreamHeader;

static int SetCooperativeLevelLocal(void* ds, void* hwnd, unsigned long flags) {
    (void)ds; (void)hwnd; (void)flags;
    return DS_OK;
}
static int CreateSoundBufferLocal(void* ds, DSBUFFERDESC* desc, FakeDSBuffer** buffer, void* unused) {
	(void)ds; (void)unused;
	if (!desc || desc->dwSize != sizeof(DSBUFFERDESC) || !desc->lpwfxFormat || desc->dwBufferBytes <= 0)
		return -1; // errore
	FakeDSBuffer* buf = (FakeDSBuffer*)malloc(sizeof(FakeDSBuffer));
	if (!buf) return -1; // errore
	buf->size = desc->dwBufferBytes;
	buf->data = (char*)malloc(buf->size);
	if (!buf->data) { free(buf); return -1; } // errore
	memset(buf->data, 0, buf->size);
	buf->playing = false;
	*buffer = buf;
	return DS_OK;
}