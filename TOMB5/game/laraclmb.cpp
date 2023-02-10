#include "../tomb5/pch.h"
#include "laraclmb.h"
#include "lara.h"
#include "lara_states.h"
#include "control.h"
#include "laramisc.h"

static long LaraCheckForLetGo(ITEM_INFO* item, COLL_INFO* coll)
{
	short room_number;

	item->gravity_status = 0;
	item->fallspeed = 0;
	room_number = item->room_number;
	GetHeight(GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number), item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	coll->trigger = trigger_index;

	if (input & IN_ACTION && item->hit_points > 0)
		return 0;

	lara.torso_y_rot = 0;
	lara.torso_x_rot = 0;
	lara.head_y_rot = 0;
	lara.head_x_rot = 0;
	item->goal_anim_state = AS_FORWARDJUMP;
	item->current_anim_state = AS_FORWARDJUMP;
	item->anim_number = ANIM_FALLDOWN;
	item->frame_number = anims[ANIM_FALLDOWN].frame_base;
	item->speed = 2;
	item->gravity_status = 1;
	item->fallspeed = 1;
	lara.gun_status = LG_NO_ARMS;
	return 1;
}

void lara_as_climbstnc(ITEM_INFO* item, COLL_INFO* coll)
{
	lara.IsClimbing = 1;
	coll->enable_spaz = 0;
	coll->enable_baddie_push = 0;
	camera.target_elevation = -3640;

	if (input & IN_LOOK)
		LookUpDown();

	if (input & IN_LEFT || input & IN_LSTEP)
	{
		item->goal_anim_state = AS_CLIMBLEFT;
		lara.move_angle = item->pos.y_rot - 16384;
	}
	else if (input & IN_RIGHT || input & IN_RSTEP)
	{
		item->goal_anim_state = AS_CLIMBRIGHT;
		lara.move_angle = item->pos.y_rot + 16384;
	}
	else if (input & IN_JUMP)
	{
		if (item->anim_number == ANIM_CLIMBSTNC)
		{
			item->goal_anim_state = AS_BACKJUMP;
			lara.gun_status = LG_NO_ARMS;
			lara.move_angle = item->pos.y_rot + 32768;
		}
	}
}

void lara_col_climbstnc(ITEM_INFO* item, COLL_INFO* coll)
{
	long result_r, result_l, shift_r, shift_l, ledge_r, ledge_l;

	if (LaraCheckForLetGo(item, coll))
		return;

	if (item->anim_number != ANIM_CLIMBSTNC)
		return;
	
	if (input & IN_FORWARD)
	{
		if (item->goal_anim_state == AS_NULL)
			return;

		item->goal_anim_state = AS_CLIMBSTNC;
		result_r = LaraTestClimbUpPos(item, coll->radius, coll->radius + 120, &shift_r, &ledge_r);
		result_l = LaraTestClimbUpPos(item, coll->radius, -(coll->radius + 120), &shift_l, &ledge_l);

		if (!result_r || !result_l)
			return;

		if (result_r < 0 || result_l < 0)
		{
			if (abs(ledge_l - ledge_r) > 120)
				return;

			if (result_r == -1 && result_l == -1)
			{
				item->goal_anim_state = AS_NULL;
				item->pos.y_pos += (ledge_l + ledge_r) / 2 - 256;
			}
			else
			{
				item->goal_anim_state = AS_CLIMB2DUCK;
				item->required_anim_state = AS_DUCK;
			}

			return;
		}

		if (shift_r)
		{
			if (shift_l)
			{

				if ((shift_r < 0) ^ (shift_l < 0))
					return;

				if (shift_r < 0 && shift_r < shift_l)
					shift_l = shift_r;
				else if (shift_r > 0 && shift_r > shift_l)
					shift_l = shift_r;
			}
			else
				shift_l = shift_r;
		}

		item->goal_anim_state = AS_CLIMBING;
		item->pos.y_pos += shift_l;
		return;
	}
	else if (input & IN_BACK)
	{
		if (item->goal_anim_state == AS_HANG)
			return;

		item->goal_anim_state = AS_CLIMBSTNC;
		item->pos.y_pos += 256;
		result_r = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -512, 512, &shift_r);
		result_l = LaraTestClimbPos(item, coll->radius, -(coll->radius + 120), -512, 512, &shift_l);
		item->pos.y_pos -= 256;

		if (result_r == 0 || result_l == 0 || result_l == -2 || result_r == -2)
			return;

		if (shift_r && shift_l)
		{
			if ((shift_r < 0) ^ (shift_l < 0))
				return;
			if (shift_r < 0 && shift_r < shift_l)
				shift_l = shift_r;
			else if (shift_r > 0 && shift_r > shift_l)
				shift_l = shift_r;
		}

		if (result_r != 1 || result_l != 1)
			item->goal_anim_state = AS_HANG;
		else
		{
			item->goal_anim_state = AS_CLIMBDOWN;
			item->pos.y_pos += shift_l;
		}
	}
}

void lara_as_climbing(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_baddie_push = 0;
	coll->enable_spaz = 0;
	camera.target_elevation = 5460;
}

void lara_col_climbing(ITEM_INFO* item, COLL_INFO* coll)
{
	long frame, yshift, result_r, result_l, shift_r, shift_l, ledge_r, ledge_l;

	if (LaraCheckForLetGo(item, coll))
		return;

	if (item->anim_number == ANIM_CLIMBING)
	{
		frame = item->frame_number - anims[ANIM_CLIMBING].frame_base;
		
		if (frame == 0)
			yshift = 0;
		else if (frame == 28 || frame == 29)
			yshift = -256;
		else if (frame == 57)
			yshift = -512;
		else
			return;

		item->pos.y_pos += yshift - 256;
		result_r = LaraTestClimbUpPos(item, coll->radius, coll->radius + 120, &shift_r, &ledge_r);
		result_l = LaraTestClimbUpPos(item, coll->radius, -(coll->radius + 120), &shift_l, &ledge_l);
		item->pos.y_pos += 256;

		if (result_r && result_l && input & IN_FORWARD)
		{
			if (result_r < 0 || result_l < 0)
			{
				item->goal_anim_state = AS_CLIMBSTNC;
				AnimateLara(item);

				if (abs(ledge_r - ledge_l) <= 120)
				{
					if (result_r == -1 || result_l == -1)
					{
						item->goal_anim_state = AS_NULL;
						item->pos.y_pos += (ledge_r + ledge_l) / 2 - 256;
					}
					else
					{
						item->goal_anim_state = AS_CLIMB2DUCK;
						item->required_anim_state = AS_DUCK;
					}
				}
			}
			else
			{
				item->goal_anim_state = AS_CLIMBING;
				item->pos.y_pos -= yshift;
			}
		}
		else
		{
			item->goal_anim_state = AS_CLIMBSTNC;

			if (yshift)
				AnimateLara(item);
		}
	}
}

void lara_as_climbleft(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_baddie_push = 0;
	coll->enable_spaz = 0;
	camera.target_angle = -5460;
	camera.target_elevation = -2730;

	if (!(input & (IN_LEFT | IN_LSTEP)))
		item->goal_anim_state = AS_CLIMBSTNC;
}

void lara_col_climbleft(ITEM_INFO* item, COLL_INFO* coll)
{
	long shift, res;

	if (!LaraCheckForLetGo(item, coll))
	{
		lara.move_angle = item->pos.y_rot - 16384;
		res = LaraTestClimbPos(item, coll->radius, -(coll->radius + 120), -512, 512, &shift);
		LaraDoClimbLeftRight(item, coll, res, shift);
	}
}

void lara_as_climbright(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_baddie_push = 0;
	coll->enable_spaz = 0;
	camera.target_angle = 5460;
	camera.target_elevation = -2730;

	if (!(input & (IN_RIGHT | IN_RSTEP)))
		item->goal_anim_state = AS_CLIMBSTNC;
}

void lara_col_climbright(ITEM_INFO* item, COLL_INFO* coll)
{
	long shift, res;

	if (!LaraCheckForLetGo(item, coll))
	{
		lara.move_angle = item->pos.y_rot + 16384;
		res = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -512, 512, &shift);
		LaraDoClimbLeftRight(item, coll, res, shift);
	}
}

void lara_as_climbend(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_baddie_push = 0;
	coll->enable_spaz = 0;
	camera.flags = 1;
	camera.target_angle = -8190;
}

void lara_col_climbend(ITEM_INFO* item, COLL_INFO* coll)
{
	return;
}

void lara_as_climbdown(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enable_baddie_push = 0;
	coll->enable_spaz = 0;
	camera.target_elevation = -8190;
}

void lara_col_climbdown(ITEM_INFO* item, COLL_INFO* coll)
{
	long frame, yshift, result_r, result_l, shift_r, shift_l;

	if (LaraCheckForLetGo(item, coll) || item->anim_number != ANIM_CLIMBDOWN)
		return;

	frame = item->frame_number - anims[ANIM_CLIMBDOWN].frame_base;

	if (frame == 0)
		yshift = 0;
	else if (frame == 28 || frame == 29)
		yshift = 256;
	else if (frame == 57)
		yshift = 512;
	else
		return;

	item->pos.y_pos += yshift + 256;
	result_r = LaraTestClimbPos(item, coll->radius, coll->radius + 120, -512, 512, &shift_r);
	result_l = LaraTestClimbPos(item, coll->radius, -(coll->radius + 120), -512, 512, &shift_l);
	item->pos.y_pos -= 256;

	if (result_r != 0 && result_l != 0 && result_r != -2 && result_l != -2 && input & IN_BACK)
	{
		if (shift_r && shift_l)
		{
			if (shift_r < 0 != shift_l < 0)
			{
				item->goal_anim_state = AS_CLIMBSTNC;
				AnimateLara(item);
				return;
			}

			if (shift_r < 0 && shift_r < shift_l ||
				shift_r > 0 && shift_r > shift_l)
				shift_l = shift_r;
		}

		if (result_r == -1 || result_l == -1)
		{
			item->anim_number = ANIM_CLIMBSTNC;
			item->frame_number = anims[ANIM_CLIMBSTNC].frame_base;
			item->current_anim_state = AS_CLIMBSTNC;
			item->goal_anim_state = AS_HANG;
			AnimateLara(item);
		}
		else
		{
			item->goal_anim_state = AS_CLIMBDOWN;
			item->pos.y_pos -= yshift;
		}

		return;
	}

	item->goal_anim_state = AS_CLIMBSTNC;

	if (yshift)
		AnimateLara(item);
}

short GetClimbTrigger(long x, long y, long z, short room_number)
{
	FLOOR_INFO* floor;
	short* data;

	floor = GetFloor(x, y, z, &room_number);
	GetHeight(floor, x, y, z);

	if (!trigger_index)
		return 0;

	data = trigger_index;

	if ((*data & 0x1F) == LAVA_TYPE)
	{
		if (*data & 0x8000)
			return 0;

		data++;
	}

	if ((*data & 0x1F) == CLIMB_TYPE)
		return *data;

	return 0;
}

long LaraTestClimbUpPos(ITEM_INFO* item, long front, long right, long* shift, long* ledge)
{
	FLOOR_INFO* floor;
	long angle, x, y, z, xfront, zfront, h, c;
	short room_number;

	xfront = 0;
	zfront = 0;
	y = item->pos.y_pos - 768;
	angle = ushort(item->pos.y_rot + 0x2000) / 0x4000;

	switch (angle)
	{
	case NORTH:
		x = right + item->pos.x_pos;
		z = front + item->pos.z_pos;
		zfront = 4;
		break;

	case EAST:
		x = front + item->pos.x_pos;
		z = item->pos.z_pos - right;
		xfront = 4;
		break;

	case SOUTH:
		x = item->pos.x_pos - right;
		z = item->pos.z_pos - front;
		zfront = -4;
		break;

	default:
		x = item->pos.x_pos - front;
		z = right + item->pos.z_pos;
		xfront = -4;
		break;
	}

	*shift = 0;
	room_number = item->room_number;
	floor = GetFloor(x, y, z, &room_number);
	c = 256 - y + GetCeiling(floor, x, y, z);

	if (c > 70)
		return 0;

	if (c > 0)
		*shift = c;

	floor = GetFloor(x + xfront, y, z + zfront, &room_number);
	h = GetHeight(floor, x + xfront, y, z + zfront);

	if (h == NO_HEIGHT)
	{
		*ledge = NO_HEIGHT;
		return 1;
	}

	h -= y;
	*ledge = h;

	if (h > 128)
	{
		c = GetCeiling(floor, x + xfront, y, z + zfront) - y;

		if (c >= 512)
			return 1;

		if (h - c > 762)
		{
			*shift = h;
			return -1;
		}

		if (h - c >= 512)
		{
			*shift = h;
			return -2;
		}
	}
	else
	{
		if (h > 0 && h > *shift)
			*shift = h;

		room_number = item->room_number;
		GetFloor(x, y + 512, z, &room_number);
		floor = GetFloor(x + xfront, y + 512, z + zfront, &room_number);
		c = GetCeiling(floor, x + xfront, y + 512, z + zfront) - y;

		if (c <= h || c >= 512)
			return 1;
	}

	return 0;
}

long LaraTestClimb(long x, long y, long z, long xfront, long zfront, long item_height, short item_room, long* shift)
{
	FLOOR_INFO* floor;
	long hang, h, c;
	short room_number;

	*shift = 0;
	hang = 1;

	if (!lara.climb_status)
		return 0;

	room_number = item_room;
	floor = GetFloor(x, y - 128, z, &room_number);
	h = GetHeight(floor, x, y, z);

	if (h == NO_HEIGHT)
		return 0;

	h -= 128 + y + item_height;

	if (h < -70)
		return 0;

	if (h < 0)
		*shift = h;

	c = GetCeiling(floor, x, y, z) - y;

	if (c > 70)
		return 0;

	if (c > 0)
	{
		if (*shift)
			return 0;

		*shift = c;
	}

	if (item_height + h < 900)
		hang = 0;

	floor = GetFloor(xfront + x, y, zfront + z, &room_number);
	h = GetHeight(floor, xfront + x, y, zfront + z);

	if (h != NO_HEIGHT)
		h -= y;

	if (h > 70)
	{
		c = GetCeiling(floor, xfront + x, y, zfront + z) - y;

		if (c >= 512)
			return 1;

		if (c > 442)
		{
			if (*shift > 0)
			{
				if (hang)
					return -1;
				else
					return 0;
			}

			*shift = c - 512;
			return 1;
		}

		if (c > 0)
		{
			if (hang)
				return -1;
			else
				return 0;
		}

		if (c <= -70 || !hang || *shift > 0)
			return 0;

		if (*shift > c)
			*shift = c;

		return -1;
	}

	if (h > 0)
	{
		if (*shift < 0)
			return 0;

		if (h > *shift)
			*shift = h;
	}

	room_number = item_room;
	GetFloor(x, y + item_height, z, &room_number);
	floor = GetFloor(xfront + x, y + item_height, zfront + z, &room_number);
	c = GetCeiling(floor, xfront + x, y + item_height, zfront + z);

	if (c == NO_HEIGHT)
		return 1;

	c -= y;

	if (c <= h || c >= 512)
		return 1;

	if (c <= 442)
	{
		if (hang)
			return -1;
		else
			return 0;
	}

	if (*shift > 0)
	{
		if (hang)
			return -1;
		else
			return 0;
	}

	*shift = c - 512;
	return 1;
}

long LaraTestClimbPos(ITEM_INFO* item, long front, long right, long origin, long height, long* shift)
{
	long angle, x, z, xfront, zfront;

	xfront = 0;
	zfront = 0;
	angle = ushort(item->pos.y_rot + 0x2000) / 0x4000;

	switch (angle)
	{
	case NORTH:
		x = right + item->pos.x_pos;
		z = front + item->pos.z_pos;
		zfront = 256;
		break;

	case EAST:
		x = front + item->pos.x_pos;
		z = item->pos.z_pos - right;
		xfront = 256;
		break;

	case SOUTH:
		x = item->pos.x_pos - right;
		z = item->pos.z_pos - front;
		zfront = -256;
		break;

	default:
		x = item->pos.x_pos - front;
		z = right + item->pos.z_pos;
		xfront = -256;
		break;
	}

	return LaraTestClimb(x, origin + item->pos.y_pos, z, xfront, zfront, height, item->room_number, shift);
}

void inject_laraclmb(bool replace)
{
	INJECT(0x00450D40, lara_as_climbstnc, replace);
	INJECT(0x00450E20, lara_col_climbstnc, replace);
	INJECT(0x004510E0, LaraCheckForLetGo, replace);
	INJECT(0x00451ED0, lara_as_climbing, replace);
	INJECT(0x00451F00, lara_col_climbing, replace);
	INJECT(0x004514D0, lara_as_climbleft, replace);
	INJECT(0x00451520, lara_col_climbleft, replace);
	INJECT(0x00451E00, lara_as_climbright, replace);
	INJECT(0x00451E50, lara_col_climbright, replace);
	INJECT(0x00452340, lara_as_climbend, replace);
	INJECT(0x00452380, lara_col_climbend, replace);
	INJECT(0x004520E0, lara_as_climbdown, replace);
	INJECT(0x00452110, lara_col_climbdown, replace);
	INJECT(0x004523A0, GetClimbTrigger, replace);
	INJECT(0x00451200, LaraTestClimbUpPos, replace);
	INJECT(0x00450A50, LaraTestClimb, replace);
	INJECT(0x00450970, LaraTestClimbPos, replace);
}
