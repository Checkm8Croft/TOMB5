#include "../tomb5/pch.h"
#include "time.h"
#include "function_stubs.h"

#include <stdint.h>
#include <sys/time.h>   // gettimeofday
#include <stdbool.h>    // per bool

// -----------------------------------------------------
// Replacement di QueryPerformanceCounter / Frequency
// -----------------------------------------------------

// Ritorna "ticks" in microsecondi
static inline uint64_t GetPerformanceCounter(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec;
}

// Ritorna "frequenza" → 1 MHz (microsecondi)
static inline uint64_t GetPerformanceFrequency(void)
{
    return 1000000ULL;
}

// -----------------------------------------------------
// Variabili globali (simulano la logica originale)
// -----------------------------------------------------
static int64_t frequency = 0;
static int64_t counter   = 0;

// -----------------------------------------------------
// Funzioni originali riscritte
// -----------------------------------------------------
long Sync()
{
    int64_t PerformanceCount, f;
    long n;

    PerformanceCount = GetPerformanceCounter();
    f = (PerformanceCount - counter) / frequency;
    counter += frequency * f;
    n = (long)f;
    return n;
}

void TIME_Reset()
{
    counter = GetPerformanceCounter();
}

bool TIME_Init()
{
    int64_t pfq;

    Log(__FUNCTION__);

    pfq = GetPerformanceFrequency();
    if (!pfq)
        return 0;

    frequency = pfq / 60;  // 60 Hz come originale
    TIME_Reset();
    return 1;
}
