#include "../tomb5/pch.h"
#include "box.h"
#include "lot.h"
#include "../specific/3dmath.h"
#include "lara_states.h"
#include "../specific/function_stubs.h"

void InitialiseCreature(short item_number)
{
	ITEM_INFO* item;

	item = &items[item_number];
	item->collidable = 1;
	item->data = 0;
	item->draw_room = ((item->pos.x_pos - room[item->room_number].x) >> 10 & 0xFF) << 8 | ((item->pos.z_pos - room[item->room_number].z) >> 10 & 0xFF);
	item->item_flags[2] = item->pos.y_pos - room[item->room_number].minfloor & 0xFF00 | item->room_number & 0xFF;
	item->TOSSPAD = item->pos.y_rot & 0xE000;
}

long CreatureActive(short item_number)
{
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->flags & IFL_CLEARBODY)
		return 0;

	if (item->status == 3)
	{
		if (!EnableBaddieAI(item_number, 0))
			return 0;

		item->status = 1;
	}

	return 1;
}

void CreatureAIInfo(ITEM_INFO* item, AI_INFO* info)
{
	CREATURE_INFO* creature;
	OBJECT_INFO* obj;
	ITEM_INFO* enemy;
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	short* zone;
	long x, y, z;
	short pivot, ang, state;

	creature = (CREATURE_INFO*)item->data;

	if (!creature)
		return;

	obj = &objects[item->object_number];
	enemy = creature->enemy;

	if (!enemy)
	{
		enemy = lara_item;
		creature->enemy = lara_item;
	}

	zone = ground_zone[creature->LOT.zone][flip_status];
	r = &room[item->room_number];
	floor = &r->floor[((item->pos.z_pos - r->z) >> 10) + r->x_size * ((item->pos.x_pos - r->x) >> 10)];
	item->box_number = floor->box;
	info->zone_number = zone[item->box_number];

	r = &room[enemy->room_number];
	floor = &r->floor[((enemy->pos.z_pos - r->z) >> 10) + r->x_size * ((enemy->pos.x_pos - r->x) >> 10)];
	enemy->box_number = floor->box;
	info->enemy_zone = zone[enemy->box_number];

	if (boxes[enemy->box_number].overlap_index & creature->LOT.block_mask ||
		creature->LOT.node[item->box_number].search_number == (creature->LOT.search_number | 0x8000))
		info->enemy_zone |= 0x4000;

	pivot = obj->pivot_length;

	if (enemy == lara_item)
		ang = lara.move_angle;
	else
		ang = enemy->pos.y_rot;

	x = enemy->pos.x_pos + (14 * enemy->speed * phd_sin(ang) >> 14) - (pivot * phd_sin(item->pos.y_rot) >> 14) - item->pos.x_pos;
	y = item->pos.y_pos - enemy->pos.y_pos;
	z = enemy->pos.z_pos + (14 * enemy->speed * phd_cos(ang) >> 14) - (pivot * phd_cos(item->pos.y_rot) >> 14) - item->pos.z_pos;

	ang = (short)phd_atan(z, x);

	if (z > 32000 || z < -32000 || x > 32000 || x < -32000)
		info->distance = 0x7FFFFFFF;
	else if (creature->enemy)
		info->distance = SQUARE(x) + SQUARE(z);
	else
		info->distance = 0x7FFFFFFF;

	info->angle = ang - item->pos.y_rot;
	info->enemy_facing = ang - enemy->pos.y_rot + 0x8000;

	x = abs(x);
	z = abs(z);

	if (enemy == lara_item)
	{
		state = lara_item->current_anim_state;

		if (state == AS_DUCK || state == AS_DUCKROLL || (state > AS_MONKEY180 && state < AS_HANG2DUCK) || state == AS_DUCKROTL || state == AS_DUCKROTR )
			y -= 384;
	}

	if (x > z)
		info->x_angle = (short)phd_atan(x + (z >> 1), y);
	else
		info->x_angle = (short)phd_atan(z + (x >> 1), y);

	info->ahead = info->angle > -0x4000 && info->angle < 0x4000;
	info->bite = info->ahead && enemy->hit_points > 0 && abs(enemy->pos.y_pos - item->pos.y_pos) <= 512;
}

long SearchLOT(LOT_INFO* LOT, long expansion)
{
	BOX_NODE* node;
	BOX_NODE* expand;
	BOX_INFO* box;
	short* zone;
	long index, done, box_number, overlap_flags, change;
	short search_zone;

	zone = ground_zone[LOT->zone][flip_status];
	search_zone = zone[LOT->head];

	for (int i = 0; i < expansion; i++)
	{
		if (LOT->head == 2047)
		{
			LOT->tail = 2047;
			return 0;
		}

		box = &boxes[LOT->head];
		node = &LOT->node[LOT->head];
		index = box->overlap_index & 0x3FFF;
		done = 0;

		do
		{
			box_number = overlap[index];
			index++;
			overlap_flags = box_number & ~2047;

			if (box_number & 0x8000)
				done = 1;

			box_number &= 2047;

			if (!LOT->fly && search_zone != zone[box_number])	//zone isn't in search area + can't fly
				continue;

			change = boxes[box_number].height - box->height;

			if ((change > LOT->step || change < LOT->drop) && (!(overlap_flags & 0x2000) || !LOT->can_monkey))	//can't traverse block + can't monkey
				continue;

			if (overlap_flags & 0x800 && !LOT->can_jump)	//can't jump
				continue;

			expand = &LOT->node[box_number];

			if ((node->search_number & 0x7FFF) < (expand->search_number & 0x7FFF))
				continue;

			if (node->search_number & 0x8000)
			{
				if ((node->search_number & 0x7FFF) == (expand->search_number & 0x7FFF))
					continue;

				expand->search_number = node->search_number;
			}
			else
			{
				if ((node->search_number & 0x7FFF) == (expand->search_number & 0x7FFF) && !(expand->search_number & 0x8000))
					continue;

				if (boxes[box_number].overlap_index & LOT->block_mask)
					expand->search_number = node->search_number | 0x8000;
				else
				{
					expand->search_number = node->search_number;
					expand->exit_box = LOT->head;
				}
			}

			if (expand->next_expansion == 2047 && box_number != LOT->tail)
			{
				LOT->node[LOT->tail].next_expansion = (short)box_number;
				LOT->tail = (short)box_number;
			}

		} while (!done);

		LOT->head = node->next_expansion;
		node->next_expansion = 2047;
	}

	return 1;
}

long UpdateLOT(LOT_INFO* LOT, long expansion)
{
	BOX_NODE* expand;

	if (LOT->required_box != 2047 && LOT->required_box != LOT->target_box)
	{
		LOT->target_box = LOT->required_box;
		expand = &LOT->node[LOT->required_box];

		if (expand->next_expansion == 2047 && LOT->tail != LOT->required_box)
		{
			expand->next_expansion = LOT->head;

			if (LOT->head == 2047)
				LOT->tail = LOT->target_box;

			LOT->head = LOT->target_box;
		}

		LOT->search_number++;
		expand->search_number = LOT->search_number;
		expand->exit_box = 2047;
	}

	return SearchLOT(LOT, expansion);
}

void TargetBox(LOT_INFO* LOT, short box_number)
{
	BOX_INFO* box;

	box = &boxes[box_number & 0x7FF];
	LOT->target.x = (((ulong)box->bottom - (ulong)box->top - 1) >> 5) * GetRandomControl() + ((ulong)box->top << 10) + 512;
	LOT->target.z = (((ulong)box->right - (ulong)box->left - 1) >> 5) * GetRandomControl() + ((ulong)box->left << 10) + 512;
	LOT->required_box = box_number & 0x7FF;

	if (LOT->fly)
		LOT->target.y = box->height - 384;
	else
		LOT->target.y = box->height;
}

long EscapeBox(ITEM_INFO* item, ITEM_INFO* enemy, short box_number)
{
	BOX_INFO* box;
	long x, z;

	box = &boxes[box_number];
	x = (((ulong)box->bottom + (ulong)box->top) << 9) - enemy->pos.x_pos;
	z = (((ulong)box->left + (ulong)box->right) << 9) - enemy->pos.z_pos;

	if (x > -5120 && x < 5120 && z > -5120 && z < 5120)
		return 0;

	return z > 0 == item->pos.z_pos > enemy->pos.z_pos || x > 0 == item->pos.x_pos > enemy->pos.x_pos;
}

long ValidBox(ITEM_INFO* item, short zone_number, short box_number)
{
	CREATURE_INFO* creature;
	BOX_INFO* box;

	creature = (CREATURE_INFO*)item->data;

	if (!creature->LOT.fly && ground_zone[creature->LOT.zone][flip_status][box_number] != zone_number)
		return 0;

	box = &boxes[box_number];

	if (creature->LOT.block_mask & box->overlap_index)
		return 0;

	if (item->pos.z_pos > box->left << 10 && item->pos.z_pos < box->right << 10 &&
		item->pos.x_pos > box->top << 10 && item->pos.x_pos < box->bottom << 10)
		return 0;

	return 1;
}

long StalkBox(ITEM_INFO* item, ITEM_INFO* enemy, short box_number)
{
	BOX_INFO* box;
	long x, z, xrange, zrange, enemy_quad, box_quad, baddie_quad;

	if (!enemy)
		return 0;

	box = &boxes[box_number];
	x = (((ulong)box->bottom + (ulong)box->top) << 9) - enemy->pos.x_pos;
	z = (((ulong)box->left + (ulong)box->right) << 9) - enemy->pos.z_pos;
	xrange = ((ulong)box->bottom - (ulong)box->top + 3) << 10;	//3 is the # of blocks
	zrange = ((ulong)box->right - (ulong)box->left + 3) << 10;

	if (x > xrange || x < -xrange || z > zrange || z < -zrange)
		return 0;

	enemy_quad = (enemy->pos.y_rot >> 14) + 2;
	box_quad = z <= 0 ? (x <= 0 ? 0 : 3) : (x > 0) + 1;

	if (enemy_quad == box_quad)
		return 0;

	baddie_quad = item->pos.z_pos <= enemy->pos.z_pos ? (item->pos.x_pos <= enemy->pos.x_pos ? 0 : 3) : (item->pos.x_pos > enemy->pos.x_pos) + 1;
	return enemy_quad != baddie_quad || abs(enemy_quad - box_quad) != 2;
}

target_type CalculateTarget(PHD_VECTOR* target, ITEM_INFO* item, LOT_INFO* LOT)
{
	BOX_INFO* box;
	long box_number, box_left, box_right, box_top, box_bottom;
	long left, right, top, bottom, prime_free;
	static ushort loops;

	UpdateLOT(LOT, 5);
	target->x = item->pos.x_pos;
	target->y = item->pos.y_pos;
	target->z = item->pos.z_pos;
	box_number = item->box_number;

	if (box_number == 2047)
		return NO_TARGET;

	box = &boxes[box_number];
	left = box->left << 10;
	right = (box->right << 10) - 1;
	top = box->top << 10;
	bottom = (box->bottom << 10) - 1;
	loops = 0;
	prime_free = 15;

	do
	{
		loops++;
		box = &boxes[box_number];

		if (LOT->fly)
		{
			if (target->y > box->height - 1024)
				target->y = box->height - 1024;
		}
		else if (target->y > box->height)
			target->y = box->height;

		box_left = box->left << 10;
		box_right = (box->right << 10) - 1;
		box_top = box->top << 10;
		box_bottom = (box->bottom << 10) - 1;

		if (item->pos.z_pos >= box_left && item->pos.z_pos <= box_right && item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom)
		{
			left = box->left << 10;
			right = (box->right << 10) - 1;
			top = box->top << 10;
			bottom = (box->bottom << 10) - 1;
		}
		else
		{
			if (item->pos.z_pos < box_left && prime_free != 2)
			{
				if (prime_free & 1 && item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom)
				{
					if (target->z < box_left + 512)
						target->z = box_left + 512;

					if (prime_free & 16)
						return SECONDARY_TARGET;

					if (box_top > top)
						top = box_top;

					if (box_bottom < bottom)
						bottom = box_bottom;

					prime_free = 1;
				}
				else if (prime_free != 1)
				{
					target->z = right - 512;

					if (prime_free != 15)
						return SECONDARY_TARGET;

					prime_free = 31;
				}
			}
			else if (item->pos.z_pos > box_right && prime_free != 1)
			{
				if (prime_free & 2 && item->pos.x_pos >= box_top && item->pos.x_pos <= box_bottom)
				{
					if (target->z > box_right - 512)
						target->z = box_right - 512;

					if (prime_free & 16)
						return SECONDARY_TARGET;

					if (box_top > top)
						top = box_top;

					if (box_bottom < bottom)
						bottom = box_bottom;

					prime_free = 2;
				}
				else if (prime_free != 2)
				{
					target->z = left + 512;

					if (prime_free != 15)
						return SECONDARY_TARGET;

					prime_free = 31;
				}
			}

			if (item->pos.x_pos < box_top && prime_free != 8)
			{
				if (prime_free & 4 && item->pos.z_pos >= box_left && item->pos.z_pos <= box_right)
				{
					if (target->x < box_top + 512)
						target->x = box_top + 512;

					if (prime_free & 16)
						return SECONDARY_TARGET;

					if (box_left > left)
						left = box_left;

					if (box_right < right)
						right = box_right;

					prime_free = 4;
				}
				else if (prime_free != 4)
				{
					target->x = bottom - 512;

					if (prime_free != 15)
						return SECONDARY_TARGET;

					prime_free = 31;
				}
			}
			else if (item->pos.x_pos > box_bottom && prime_free != 4)
			{
				if (prime_free & 8 && item->pos.z_pos >= box_left && item->pos.z_pos <= box_right)
				{
					if (target->x > box_bottom - 512)
						target->x = box_bottom - 512;

					if (prime_free & 16)
						return SECONDARY_TARGET;

					if (box_left > left)
						left = box_left;

					if (box_right < right)
						right = box_right;

					prime_free = 8;
				}
				else if (prime_free != 8)
				{
					target->x = top + 512;

					if (prime_free != 15)
						return SECONDARY_TARGET;

					prime_free = 31;
				}
			}
		}

		if (box_number == LOT->target_box)
		{
			if (prime_free & 3)
				target->z = LOT->target.z;
			else if (!(prime_free & 16))
			{
				if (target->z < box_left + 512)
					target->z = box_left + 512;
				else if (target->z > box_right - 512)
					target->z = box_right - 512;
			}

			if (prime_free & 12)
				target->x = LOT->target.x;
			else if (!(prime_free & 16))
			{
				if (target->x < box_top + 512)
					target->x = box_top + 512;
				else if (target->x > box_bottom - 512)
					target->x = box_bottom - 512;
			}

			target->y = LOT->target.y;
			return PRIME_TARGET;
		}

		box_number = LOT->node[box_number].exit_box;

		if (box_number != 2047 && boxes[box_number].overlap_index & LOT->block_mask)
			break;
	}
	while (box_number != 2047);

	if (!(prime_free & 16))
	{
		if (target->z < box_left + 512)
			target->z = box_left + 512;
		else if (target->z > box_right - 512)
			target->z = box_right - 521;
	}

	if (!(prime_free & 16))	//wut
	{
		if (target->x < box_top + 512)
			target->x = box_top + 512;
		else if (target->x > box_bottom - 512)
			target->x = box_bottom - 512;
	}

	if (LOT->fly)
		target->y = box->height - 320;
	else
		target->y = box->height;

	return NO_TARGET;
}

void inject_box(bool replace)
{
	INJECT(0x00408550, InitialiseCreature, replace);
	INJECT(0x00408630, CreatureActive, replace);
	INJECT(0x004086C0, CreatureAIInfo, replace);
	INJECT(0x00408BA0, SearchLOT, replace);
	INJECT(0x00408B00, UpdateLOT, replace);
	INJECT(0x00408E20, TargetBox, replace);
	INJECT(0x00408EF0, EscapeBox, replace);
	INJECT(0x00408FD0, ValidBox, replace);
	INJECT(0x00409770, StalkBox, replace);
	INJECT(0x004098B0, CalculateTarget, replace);
}