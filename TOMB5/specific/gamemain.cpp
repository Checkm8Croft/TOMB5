#include "../tomb5/pch.h"
#include "gamemain.h"
#include "function_stubs.h"
#include "audio.h"
#include "file.h"
#include "function_table.h"
#include "3dmath.h"
#include "../game/text.h"
#include "time.h"
#include "winmain.h"
#include "../game/sound.h"
#include "../game/gameflow.h"
#include "dxshell.h"
#include "specificfx.h"
#include "../game/savegame.h"
#include "../tomb5/tomb5.h"

WATERTAB WaterTable[22][64];
THREAD MainThread;
float vert_wibble_table[32];
short* clipflags;
long SaveCounter;
unsigned char* ADPCMBuffer = nullptr;
unsigned char* wav_file_buffer = nullptr;

static ushort GetRandom(WATERTAB* wt, long lp)
{
	long loop;
	ushort ret;

	do
	{
		ret = rand() & 0xFC;
		
		for (loop = 0; loop < lp; loop++)
		{
			if (wt[loop].random == ret)
				break;
		}

	} while (loop != lp);

	return ret;
}

static void init_water_table()
{
	float fSin;
	short sSin;
	static short water_shimmer[4] = { 31, 63, 95, 127 };
	static short water_choppy[4] = { 16, 53, 90, 127 };
	static uchar water_abs[4] = { 4, 8, 12, 16 };

	srand(121197);

	for (int i = 0; i < 64; i++)
	{
		sSin = rcossin_tbl[i << 7];
		WaterTable[0][i].shimmer = (63 * sSin) >> 15;
		WaterTable[0][i].choppy = (16 * sSin) >> 12;
		WaterTable[0][i].random = (uchar)GetRandom(WaterTable[0], i);
		WaterTable[0][i].abs = 0;

		WaterTable[1][i].shimmer = (32 * sSin) >> 15;
		WaterTable[1][i].choppy = 0;
		WaterTable[1][i].random = (uchar)GetRandom(WaterTable[1], i);
		WaterTable[1][i].abs = -3;

		WaterTable[2][i].shimmer = (64 * sSin) >> 15;
		WaterTable[2][i].choppy = 0;
		WaterTable[2][i].random = (uchar)GetRandom(WaterTable[2], i);
		WaterTable[2][i].abs = 0;

		WaterTable[3][i].shimmer = (96 * sSin) >> 15;
		WaterTable[3][i].choppy = 0;
		WaterTable[3][i].random = (uchar)GetRandom(WaterTable[3], i);
		WaterTable[3][i].abs = 4;

		WaterTable[4][i].shimmer = (127 * sSin) >> 15;
		WaterTable[4][i].choppy = 0;
		WaterTable[4][i].random = (uchar)GetRandom(WaterTable[4], i);
		WaterTable[4][i].abs = 8;

		for (int j = 0, k = 5; j < 4; j++, k += 4)
		{
			for (int m = 0; m < 4; m++)
			{
				WaterTable[k + m][i].shimmer = -((sSin * water_shimmer[m]) >> 15);
				WaterTable[k + m][i].choppy = sSin * water_choppy[j] >> 12;
				WaterTable[k + m][i].random = (uchar)GetRandom(WaterTable[k + m], i);
				WaterTable[k + m][i].abs = water_abs[m];
			}
		}
	}

	for (int i = 0; i < 32; i++)
	{
		fSin = sin(i * 0.19634955F);
		vert_wibble_table[i] = fSin + fSin;
	}
}

long S_SaveGame(long slot_num)
{
	FILE* file;
	ulong bytes;
	long days, hours, minutes, seconds;
	char buffer[80], counter[16];

	for (int i = 0; i < 20; i++)
		buffer[i] = '\0';

	sprintf(buffer, "savegame.%d", slot_num);
	file = fopen(buffer, "wb");

	if (file != NULL)
	{
		for (int i = 0; i < 20; i++)
			buffer[i] = '\0';

		sprintf(buffer, "%s", SCRIPT_TEXT(gfLevelNames[gfCurrentLevel]));
		fwrite(buffer, 75, 1, file);
		fwrite(&SaveCounter, sizeof(long), 1, file);
		days = savegame.Game.Timer / 30 / 86400;
		hours = savegame.Game.Timer / 30 % 86400 / 3600;
		minutes = savegame.Game.Timer / 30 / 60 % 60;
		seconds = savegame.Game.Timer / 30 % 60;
		fwrite(&days, 2, 1, file);
		fwrite(&hours, 2, 1, file);
		fwrite(&minutes, 2, 1, file);
		fwrite(&seconds, 2, 1, file);
		fwrite(&savegame, sizeof(SAVEGAME_INFO), 1, file);
		fwrite(&tomb5_save, sizeof(tomb5_save_info), 1, file);
		fclose(file);
		sprintf(counter, "%d", SaveCounter);
		SaveCounter++;
		return 1;
	}

	return 0;
}

long S_LoadGame(long slot_num)
{
	FILE* file;
	ulong bytes;
	long value;
	char buffer[80];

	sprintf(buffer, "savegame.%d", slot_num);
	file = fopen(buffer, "rb");

	if (file != NULL)
	{
		fread(buffer, 75, 1, file);
		fread(&value, sizeof(long), 1, file);
		fread(&value, sizeof(long), 1, file);
		fread(&value, sizeof(long), 1, file);
		fread(&savegame, sizeof(SAVEGAME_INFO), 1, file);
		fread(&tomb5_save, sizeof(tomb5_save_info), 1, file);
		fclose(file);
		return 1;
	}

	return 0;
}

bool GameInitialise()
{
	Log(__FUNCTION__);
	init_game_malloc();
	clipflags = (short*)malloc(0x8000);
	init_water_table();
	aInitFX();
	return 1;
}

unsigned int __stdcall GameMain(void* ptr)
{
	Log(__FUNCTION__);

	if (GameInitialise())
	{
		InitialiseFunctionTable();
		HWInitialise();
		InitWindow(0, 0, App.dx.dwRenderWidth, App.dx.dwRenderHeight, 20, 20480, 80, App.dx.dwRenderWidth, App.dx.dwRenderHeight);
		InitFont();
		TIME_Init();
		App.SetupComplete = 1;
		S_CDStop();
		ClearSurfaces();

		if (!App.SoundDisabled)
			SOUND_Init();

		init_tomb5_stuff();
		DoGameflow();
		GameClose();
		S_CDStop();

SDL_Event event;
event.type = SDL_QUIT;
SDL_PushEvent(&event);
		MainThread.active = 0;
		pthread_exit(NULL);
	}

	return 1;
}

void GameClose()
{
	Log(__FUNCTION__);
	ACMClose();
	FreeLevel();
	free(clipflags);

	if (wav_file_buffer)
		free(wav_file_buffer);

	if (ADPCMBuffer)
		free(ADPCMBuffer);

	if (logF)
		fclose(logF);

	free(malloc_buffer);
	free(gfScriptFile);
	free(gfLanguageFile);
}
