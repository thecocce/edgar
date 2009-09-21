/*
Copyright (C) 2009 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

#include "../graphics/animation.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"

extern Entity *self;

static void floatUp(void);
static void die(void);
static void touch(Entity *);

Entity *addBubble(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Bubble\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &floatUp;
	e->touch = &touch;
	e->die = &die;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void touch(Entity *other)
{
	if (other->y < self->y)
	{
		pushEntity(other);

		if (self->dirY == 0 && self->dirX == 0)
		{
			self->die();
		}
	}

	else
	{
		self->die();
	}
}

static void floatUp()
{
	float dirY;

	self->dirX *= 0.95;

	if (fabs(self->dirX) <= 0.05)
	{
		self->dirX = 0;

		self->dirY = -self->speed;
	}

	dirY = self->dirY;

	checkToMap(self);

	if (self->dirY == 0 && dirY < 0)
	{
		self->die();
	}
}

static void die()
{
	playSoundToMap("sound/common/pop.ogg", -1, self->x, self->y, 0);

	self->inUse = FALSE;
}