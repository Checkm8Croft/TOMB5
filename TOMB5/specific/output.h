#pragma once
#include "../global/types.h"
#include <SDL2/SDL.h>
void S_DrawPickup(short object_number);
void aTransformLightClipMesh(MESH_DATA* mesh);
void aTransformLightPrelightClipMesh(MESH_DATA* mesh);
void phd_PutPolygons_train(short* lol, long x);
void RenderLoadPic(long unused);
long S_GetObjectBounds(short* bounds);
void S_AnimateTextures(long n);
long aCheckMeshClip(MESH_DATA* mesh);
HRESULT DDCopyBitmap(Surface surf, HBITMAP hbm, long x, long y, long dx, long dy);
int _LoadBitmap(SDL_Surface* surf, const char* name);
void do_boot_screen(long language);
void aCalcColorSplit(long col, long* pC, long* pS);
HRESULT S_DumpScreen();
HRESULT S_DumpScreenFrame();
void StashSkinVertices(long node);
void SkinVerticesToScratch(long node);
void StashSkinNormals(long node);
void SkinNormalsToScratch(long node);
void S_InitialisePolyList();
void S_OutputPolyList();
void S_InsertRoom(ROOM_INFO* r, long a);
void phd_PutPolygons(short* objptr, long clipstatus);
void phd_PutPolygonsSkyMesh(short* objptr, long clipstatus);
void phd_PutPolygonsPickup(short* objptr, float x, float y, long color);
void phd_PutPolygons_seethrough(short* objptr, long fade);
void phd_PutPolygonsSpcXLU(short* objptr, long clipstatus);
void phd_PutPolygonsSpcEnvmap(short* objptr, long clipstatus);
long GetFixedScale(long unit);

extern GLVERTEX aVertexBuffer[1024];
extern long aGlobalSkinMesh;
extern long GlobalAlpha;
extern long GlobalAmbient;
extern float AnimatingTexturesV[16][8][3];
extern float aBoundingBox[24];
