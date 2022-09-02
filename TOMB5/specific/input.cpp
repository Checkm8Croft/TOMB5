#include "../tomb5/pch.h"
#include "input.h"
#include "dxshell.h"
#include "../game/lara_states.h"
#include "../game/sound.h"
#ifdef GENERAL_FIXES
#include "../game/gameflow.h"
#include "../game/newinv2.h"
#include "../tomb5/tomb5.h"
#endif

#ifdef GENERAL_FIXES
short ammo_change_timer = 0;
char ammo_change_buf[12];

static void DoWeaponHotkey()
{
	short state;
	bool goin;

	if (!lara_item)
		goin = 0;
	else
	{
		state = lara_item->current_anim_state;
		goin = !(gfLevelFlags & GF_YOUNGLARA) && (lara.water_status == LW_ABOVE_WATER || lara.water_status == LW_WADE) && !bDisableLaraControl &&
			(state != AS_ALL4S && state != AS_CRAWL && state != AS_ALL4TURNL && state != AS_ALL4TURNR && state != AS_CRAWLBACK &&
				state != AS_CRAWL2HANG && state != AS_DUCK && state != AS_DUCKROTL && state != AS_DUCKROTR);
	}

	if (!goin)
		return;
	
	if (keymap[DIK_1])
	{
		if (gfCurrentLevel < LVL5_THIRTEENTH_FLOOR)
		{
			if (!(lara.pistols_type_carried & WTYPE_PRESENT))
				return;

			lara.request_gun_type = WEAPON_PISTOLS;

			if (lara.gun_status == LG_NO_ARMS && lara.gun_type == WEAPON_PISTOLS)
				lara.gun_status = LG_DRAW_GUNS;
		}
		else
		{
			if (!(lara.hk_type_carried & WTYPE_PRESENT))
				return;

			lara.request_gun_type = WEAPON_HK;

			if (lara.gun_type == WEAPON_HK)
			{
				if (lara.gun_status == LG_NO_ARMS)
					lara.gun_status = LG_DRAW_GUNS;
				else if (lara.gun_status == LG_READY && !ammo_change_timer)
				{
					if (!tomb5.ammotype_hotkeys)
						return;

					memset(ammo_change_buf, 0, sizeof(ammo_change_buf));

					if (lara.hk_type_carried & WTYPE_AMMO_3)
					{
						lara.hk_type_carried &= ~WTYPE_AMMO_3;
						lara.hk_type_carried |= WTYPE_AMMO_2;
						ammo_change_timer = 30;
						sprintf(ammo_change_buf, "Burst");
					}
					else if (lara.hk_type_carried & WTYPE_AMMO_2)
					{
						lara.hk_type_carried &= ~WTYPE_AMMO_2;
						lara.hk_type_carried |= WTYPE_AMMO_1;
						ammo_change_timer = 30;
						sprintf(ammo_change_buf, "Sniper");
					}
					else if (lara.hk_type_carried & WTYPE_AMMO_1)
					{
						lara.hk_type_carried &= ~WTYPE_AMMO_1;
						lara.hk_type_carried |= WTYPE_AMMO_3;
						ammo_change_timer = 30;
						sprintf(ammo_change_buf, "Rapid");
					}
				}
			}
		}
	}
	else if (keymap[DIK_2])
	{
		if (gfCurrentLevel < LVL5_THIRTEENTH_FLOOR)
		{
			if (!(lara.shotgun_type_carried & WTYPE_PRESENT))
				return;

			lara.request_gun_type = WEAPON_SHOTGUN;

			if (lara.gun_type == WEAPON_SHOTGUN)
			{
				if (lara.gun_status == LG_NO_ARMS)
					lara.gun_status = LG_DRAW_GUNS;
				else if (lara.gun_status == LG_READY && !ammo_change_timer)
				{
					if (!tomb5.ammotype_hotkeys)
						return;

					memset(ammo_change_buf, 0, sizeof(ammo_change_buf));

					if (lara.shotgun_type_carried & WTYPE_AMMO_2)
					{
						lara.shotgun_type_carried &= ~WTYPE_AMMO_2;
						lara.shotgun_type_carried |= WTYPE_AMMO_1;
						ammo_change_timer = 30;
						sprintf(ammo_change_buf, "Normal");
					}
					else if (lara.shotgun_type_carried & WTYPE_AMMO_1)
					{
						lara.shotgun_type_carried &= ~WTYPE_AMMO_1;
						lara.shotgun_type_carried |= WTYPE_AMMO_2;
						ammo_change_timer = 30;
						sprintf(ammo_change_buf, "Wideshot");
					}
				}
			}
		}
		else
		{
			if (!(lara.crossbow_type_carried & WTYPE_PRESENT))
				return;

			lara.request_gun_type = WEAPON_CROSSBOW;

			if (lara.gun_status == LG_NO_ARMS && lara.gun_type == WEAPON_CROSSBOW)
				lara.gun_status = LG_DRAW_GUNS;
		}
	}
	else if (keymap[DIK_3])
	{
		if (!(lara.uzis_type_carried & WTYPE_PRESENT))
			return;

		lara.request_gun_type = WEAPON_UZI;

		if (lara.gun_status == LG_NO_ARMS && lara.gun_type == WEAPON_UZI)
			lara.gun_status = LG_DRAW_GUNS;
	}
	else if (keymap[DIK_4])
	{
		if (!(lara.sixshooter_type_carried & WTYPE_PRESENT))
			return;

		lara.request_gun_type = WEAPON_REVOLVER;

		if (lara.gun_status == LG_NO_ARMS && lara.gun_type == WEAPON_REVOLVER)
			lara.gun_status = LG_DRAW_GUNS;
	}
}
#endif

long Key(long number)
{
	short key = layout[1][number];

	if (number >= 256)
		return joy_fire & (1 << number);

	if (keymap[key])
		return 1;

	switch (key)
	{
	case DIK_RCONTROL:
		return keymap[DIK_LCONTROL];
	case DIK_LCONTROL:
		return keymap[DIK_RCONTROL];
	case DIK_RSHIFT:
		return keymap[DIK_LSHIFT];
	case DIK_LSHIFT:
		return keymap[DIK_RSHIFT];
	case DIK_RMENU:
		return keymap[DIK_LMENU];
	case DIK_LMENU:
		return keymap[DIK_RMENU];
	}

	if (conflict[number])
		return 0;

	key = layout[0][number];

	if (keymap[key])
		return 1;

	switch (key)
	{
	case DIK_RCONTROL:
		return keymap[DIK_LCONTROL];
	case DIK_LCONTROL:
		return keymap[DIK_RCONTROL];
	case DIK_RSHIFT:
		return keymap[DIK_LSHIFT];
	case DIK_LSHIFT:
		return keymap[DIK_RSHIFT];
	case DIK_RMENU:
		return keymap[DIK_LMENU];
	case DIK_LMENU:
		return keymap[DIK_RMENU];
	}

	return 0;
}

long S_UpdateInput()
{
	static long linput;
	static long LookCnt;
	static long med_hotkey_timer;
	static long cheat_code;
	static long weird = 0;
	long flag;
	short state;
	static bool flare_no_db = 0;
	bool debounce;

	flag = 0;
	debounce = SetDebounce;

	DXUpdateJoystick();
	DXReadKeyboard(keymap);

	if (ControlMethod == 1)
		joy_fire = ReadJoystick(joy_x, joy_y);

	linput = 0;

	if (ControlMethod == 1)
	{
		if (joy_x < -8)
			linput = IN_LEFT;
		else if (joy_x > 8)
			linput = IN_RIGHT;

		if (joy_y > 8)
			linput |= IN_BACK;
		else if (joy_y < -8)
			linput |= IN_FORWARD;
	}

	if (Key(0))
		linput |= IN_FORWARD;

	if (Key(1))
		linput |= IN_BACK;

	if (Key(2))
		linput |= IN_LEFT;

	if (Key(3))
		linput |= IN_RIGHT;

	if (Key(4))
		linput |= IN_DUCK;

	if (Key(5))
		linput |= IN_SPRINT;

	if (Key(6))
		linput |= IN_WALK;

	if (Key(7))
		linput |= IN_JUMP;

	if (Key(8))
		linput |= IN_SELECT | IN_ACTION;

	if (Key(9))
		linput |= IN_DRAW;

	if (ControlMethod == 1)
	{
		if (Key(MappedControls[3] + 256))
			linput |= IN_JUMP;

		if (Key(MappedControls[4] + 256))
			linput |= IN_SELECT | IN_ACTION;

		if (Key(MappedControls[5] + 256))
			linput |= IN_DRAW;

		if (Key(MappedControls[1] + 256))
			linput |= IN_SPRINT;

		if (Key(MappedControls[2] + 256))
			linput |= IN_WALK;

		if (Key(MappedControls[0] + 256))
			linput |= IN_DUCK;

		if (Key(MappedControls[6] + 256))
			flag = 1;
	}

	if (Key(10) || flag)
	{
		if (!flare_no_db)
		{
			state = lara_item->current_anim_state;

			if (state == AS_ALL4S || state == AS_CRAWL || state == AS_ALL4TURNL ||
				state == AS_ALL4TURNR || state == AS_CRAWLBACK || state == AS_CRAWL2HANG)
			{
				SoundEffect(SFX_LARA_NO, 0, SFX_ALWAYS);
				flare_no_db = 1;
			}
			else
			{
				flare_no_db = 0;
				linput |= IN_FLARE;
			}
		}
	}
	else
		flare_no_db = 0;

	if (Key(11))
		linput |= IN_LOOK;

	if (Key(12))
		linput |= IN_ROLL;

	if (Key(13))
		linput |= IN_OPTION;

	if (Key(14))
		linput |= IN_WALK | IN_LEFT;

	if (Key(15))
		linput |= IN_WALK | IN_RIGHT;

	if (Key(16))
		linput |= IN_PAUSE;

	if (Key(17))
		linput |= IN_SELECT;

	if (ControlMethod == 1)
	{
		if (Key(MappedControls[7] + 256))
			linput |= IN_LOOK;

		if (Key(MappedControls[8] + 256))
			linput |= IN_ROLL;

		if (Key(MappedControls[9] + 256))
			linput |= IN_OPTION;
	}

	if (keymap[DIK_ESCAPE])
		linput |= IN_DESELECT | IN_OPTION;

	if (lara.gun_status == LG_READY)
	{
		savegame.AutoTarget = (uchar)App.AutoTarget;

		if (linput & IN_LOOK)
		{
			if (LookCnt >= 6)
				LookCnt = 100;
			else
			{
				linput &= ~IN_LOOK;
				LookCnt++;
			}
		}
		else
		{
			if (LookCnt && LookCnt != 100)
				linput |= IN_TARGET;

			LookCnt = 0;
		}
	}

#ifdef GENERAL_FIXES
	DoWeaponHotkey();
#endif

	if (keymap[DIK_0])
	{
		if (!med_hotkey_timer)
		{
			if (lara_item->hit_points > 0 && lara_item->hit_points < 1000 || lara.poisoned)
			{
#ifdef GENERAL_FIXES
				if (lara.num_small_medipack)
				{
					if (lara.num_small_medipack != -1)
						lara.num_small_medipack--;

					lara.dpoisoned = 0;
					lara_item->hit_points += 500;
					SoundEffect(SFX_MENU_MEDI, 0, SFX_ALWAYS);
					savegame.Game.HealthUsed++;

					if (lara_item->hit_points > 1000)
						lara_item->hit_points = 1000;

					if (InventoryActive && !lara.num_small_medipack)
					{
						construct_object_list();
						setup_objectlist_startposition(INV_COMPASS_ITEM);
					}

					med_hotkey_timer = 15;
				}
#else
				if (lara.num_small_medipack && lara.num_small_medipack != -1)
					lara.num_small_medipack--;

				if (lara.num_small_medipack)
				{
					lara.dpoisoned = 0;
					lara_item->hit_points += 500;

					if (lara_item->hit_points > 1000)
					{
						lara_item->hit_points = 1000;
						SoundEffect(SFX_MENU_MEDI, 0, SFX_ALWAYS);
						savegame.Game.HealthUsed++;
					}
				}

				med_hotkey_timer = 15;
#endif
			}
		}
	}
	else if (keymap[DIK_9])
	{
		if (!med_hotkey_timer)
		{
			if (lara_item->hit_points > 0 && lara_item->hit_points < 1000 || lara.poisoned)
			{
#ifdef GENERAL_FIXES
				if (lara.num_large_medipack)
				{
					if (lara.num_large_medipack != -1)
						lara.num_large_medipack--;

					lara.dpoisoned = 0;
					lara_item->hit_points = 1000;
					SoundEffect(SFX_MENU_MEDI, 0, SFX_ALWAYS);
					savegame.Game.HealthUsed++;
					med_hotkey_timer = 15;

					if (InventoryActive && !lara.num_large_medipack)
					{
						construct_object_list();

						if (lara.num_small_medipack)
							setup_objectlist_startposition(INV_SMALLMEDI_ITEM);
						else
							setup_objectlist_startposition(INV_COMPASS_ITEM);
					}
				}
#else
				if (lara.num_large_medipack && lara.num_large_medipack != -1)
					lara.num_large_medipack--;

				if (lara.num_large_medipack)
				{
					lara.dpoisoned = 0;
					lara_item->hit_points += 1000;

					if (lara_item->hit_points > 1000)
					{
						lara_item->hit_points = 1000;
						SoundEffect(SFX_MENU_MEDI, 0, SFX_ALWAYS);
						savegame.Game.HealthUsed++;
					}
				}

				med_hotkey_timer = 15;
#endif
			}
		}
	}
	else if (med_hotkey_timer)
		med_hotkey_timer--;

	if (keymap[DIK_F10])
		linput |= IN_E;

	if (linput & IN_WALK && !(linput & (IN_FORWARD | IN_BACK)))
	{
		if (linput & IN_LEFT)
			linput = (linput & ~IN_LEFT) | IN_LSTEP;
		else if (linput & IN_RIGHT)
			linput = (linput & ~IN_RIGHT) | IN_RSTEP;
	}

	if (linput & IN_FORWARD && linput & IN_BACK)
		linput |= IN_ROLL;

	if (linput & IN_ROLL && BinocularRange)
		linput &= ~IN_ROLL;

	if ((linput & (IN_RIGHT | IN_LEFT)) == (IN_RIGHT | IN_LEFT))
		linput -= IN_RIGHT | IN_LEFT;

	if (debounce)
		dbinput = inputBusy;

	if (!gfGameMode && Gameflow->LoadSaveEnabled)
	{
		if (keymap[DIK_F5])
			linput |= IN_SAVE;

		if (keymap[DIK_F6])
			linput |= IN_LOAD;
	}

	if (Gameflow->CheatEnabled)
	{
		if (linput)
			cheat_code = 0;

		switch (cheat_code)
		{
			case 0:

				if (keymap[DIK_D])
					cheat_code = 1;

				break;

			case 1:

				if (keymap[DIK_O])
					cheat_code = 2;

				break;

			case 2:

				if (keymap[DIK_Z])
					cheat_code = 3;

				break;

			case 3:

				if (keymap[DIK_Y])
					linput = IN_CHEAT;

				break;
		}
	}

	inputBusy = linput;

	if (lara.Busy)
	{
		linput &= IN_PAUSE | IN_LOOK | IN_OPTION | IN_RIGHT | IN_LEFT | IN_BACK | IN_FORWARD;

		if (linput & IN_FORWARD && linput & IN_BACK)
			linput ^= IN_BACK;
	}

	if (debounce)
		dbinput = inputBusy & (dbinput ^ inputBusy);

	input = linput;
	linput = weird;

	if (keymap[DIK_F])
		linput = 1;

	weird &= weird ^ linput;

	return 1;
}

void inject_input(bool replace)
{
	INJECT(0x004A9110, Key, replace);
	INJECT(0x004A92D0, S_UpdateInput, replace);
}
