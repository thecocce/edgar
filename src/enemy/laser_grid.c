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
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../geometry.h"

extern Entity entity[MAX_ENTITIES], *self;

static void init(void);
static void wait(void);
static void addLaser(void);
static void removeLaser(void);
static void laserWait(void);

Entity *addLaserGrid(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Laser Grid\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	if (self->active == TRUE)
	{
		addLaser();
	}

	self->action = &wait;

	self->health = self->active;
}

static void wait()
{
	if (self->maxThinkTime != 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->active = self->active == TRUE ? FALSE : TRUE;

			if (self->active == TRUE)
			{
				addLaser();
			}

			else
			{
				removeLaser();
			}

			self->thinkTime = self->maxThinkTime;
		}
	}

	else if (self->active != self->health)
	{
		if (self->active == TRUE)
		{
			addLaser();
		}

		else
		{
			removeLaser();
		}

		self->health = self->active;
	}
}

static void addLaser()
{
	int i;
	Entity *e;
	
	if (strcmpignorecase(self->name, "enemy/horizontal_laser_grid") == 0)
	{
		for (i=self->startX;i<self->endX;i+=32)
		{
			e = getFreeEntity();
	
			if (e == NULL)
			{
				printf("No free slots to add a Laser\n");
	
				exit(1);
			}
	
			loadProperties("enemy/horizontal_laser", e);
	
			e->x = i;
			e->y = self->y;
	
			e->action = &laserWait;
	
			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;
	
			e->head = self;
	
			e->currentFrame = prand() % 6;
	
			setEntityAnimation(e, STAND);
		}
	}
	
	else
	{
		for (i=self->startY;i<self->endY;i+=32)
		{
			e = getFreeEntity();
	
			if (e == NULL)
			{
				printf("No free slots to add a Laser\n");
	
				exit(1);
			}
	
			loadProperties("enemy/laser", e);
	
			e->x = self->x;
			e->y = i;
	
			e->action = &laserWait;
	
			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;
	
			e->head = self;
	
			e->currentFrame = prand() % 6;
	
			setEntityAnimation(e, STAND);
		}
	}

	playSoundToMap("sound/enemy/laser/zap.ogg", 7, self->x, self->y, 0);
}

static void removeLaser()
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && entity[i].head == self)
		{
			entity[i].inUse = FALSE;
		}
	}
}

static void laserWait()
{

}