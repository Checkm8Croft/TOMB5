#pragma once
#include "../global/types.h"
#include <stdint.h>

extern bool start_setup;
extern bool fmvs_disabled;

extern bool SetupDialog;
typedef struct {
    int graphicsAdapter;
    int d3dDevice;
    int resolutionWidth;
    int resolutionHeight;
    int textureFormat;
    int filter;      // 1 = bilinear, 0 = nearest
    int textLow;     // 1 = low quality
    int windowed;    // 1 = windowed, 0 = fullscreen
    int disableSound;
    int noFMV;
} SetupSettings;

extern SetupSettings settings;

void InitDefaultSettings();
int ShowSetupDialog();

