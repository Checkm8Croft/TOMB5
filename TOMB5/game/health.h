#pragma once
#include "../global/vars.h"


void inject_health(bool replace);


long FlashIt();
void DrawGameInfo(long timed);
void DrawHealthBar(long flash_state);
void DrawAirBar(long flash_state);
void MakeAmmoString(char* string);
void InitialisePickUpDisplay();
void DrawPickups(long timed);
void AddDisplayPickup(short object_number);
