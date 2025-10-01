// setupdlg_portable.cpp
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int graphics_adapter;
    int d3d_device;
    int resolution_w;
    int resolution_h;
    int bilinear_filter;
    int low_quality_tex;
} SetupDialogState;

// stub per mostrare dialog
void ShowDialog(SetupDialogState* state) {
    printf("=== Setup Dialog ===\n");
    printf("Graphics Adapter (0-%d): ", 2);
    scanf("%d", &state->graphics_adapter);

    printf("D3D Device (0-%d): ", 1);
    scanf("%d", &state->d3d_device);

    printf("Resolution Width: ");
    scanf("%d", &state->resolution_w);
    printf("Resolution Height: ");
    scanf("%d", &state->resolution_h);

    printf("Bilinear Filter (0/1): ");
    scanf("%d", &state->bilinear_filter);

    printf("Low Quality Textures (0/1): ");
    scanf("%d", &state->low_quality_tex);

    printf("\nSetup completed!\n");
}
