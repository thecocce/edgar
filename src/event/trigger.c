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

#include "../entity.h"
#include "objective.h"
#include "trigger.h"
#include "../event/script.h"

static Trigger trigger[MAX_TRIGGERS];
static Type type[] = {
					{UPDATE_OBJECTIVE, "UPDATE_OBJECTIVE"},
					{ACTIVATE_ENTITY, "ACTIVATE_ENTITY"},
					{RUN_SCRIPT, "RUN_SCRIPT"}
					};
static int length = sizeof(type) / sizeof(Type);

static void addTrigger(char *, int, int, char *);

void freeTriggers()
{
	/* Clear the list */

	memset(trigger, 0, sizeof(Trigger) * MAX_TRIGGERS);
}

void addTriggerFromResource(char *key[], char *value[])
{
	int i, triggerName, count, targetType, targetName;

	triggerName = count = targetType = targetName = -1;

	for (i=0;i<MAX_PROPS_FILES;i++)
	{
		if (strcmpignorecase("TRIGGER_NAME", key[i]) == 0)
		{
			triggerName = i;
		}

		else if (strcmpignorecase("TRIGGER_COUNT", key[i]) == 0)
		{
			count = i;
		}

		else if (strcmpignorecase("TRIGGER_TYPE", key[i]) == 0)
		{
			targetType = i;
		}

		else if (strcmpignorecase("TRIGGER_TARGET", key[i]) == 0)
		{
			targetName = i;
		}
	}

	if (triggerName == -1 || count == -1 || targetType == -1 || targetName == -1)
	{
		printf("Trigger is missing resources\n");

		exit(1);
	}

	addTrigger(value[triggerName], atoi(value[count]), getTriggerTypeByName(value[targetType]), value[targetName]);
}

static void addTrigger(char *triggerName, int count, int targetType, char *targetName)
{
	int i;

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == FALSE)
		{
			trigger[i].inUse = TRUE;

			trigger[i].count = count;
			trigger[i].targetType = targetType;

			STRNCPY(trigger[i].triggerName, triggerName, sizeof(trigger[i].triggerName));
			STRNCPY(trigger[i].targetName, targetName, sizeof(trigger[i].targetName));

			printf("Added Trigger \"%s\" with count %d\n", trigger[i].triggerName, trigger[i].count);

			return;
		}
	}

	printf("No free slots to add Trigger \"%s\"\n", triggerName);

	exit(1);
}

void fireTrigger(char *name)
{
	int i;

	if (strlen(name) == 0)
	{
		return;
	}

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == TRUE && strcmpignorecase(trigger[i].triggerName, name) == 0)
		{
			trigger[i].count--;
			
			printf("Updating Trigger \"%s\", %d to go\n", trigger[i].triggerName, trigger[i].count);

			if (trigger[i].count <= 0)
			{
				printf("Firing Trigger \"%s\"\n", trigger[i].triggerName);

				switch (trigger[i].targetType)
				{
					case UPDATE_OBJECTIVE:
						updateObjective(trigger[i].targetName);
					break;

					case ACTIVATE_ENTITY:
						activateEntitiesWithRequiredName(trigger[i].targetName, TRUE);
					break;

					case RUN_SCRIPT:
						loadScript(trigger[i].targetName);

						readNextScriptLine();
					break;

					default:

					break;
				}

				trigger[i].inUse = FALSE;

				return;
			}
		}
	}
}

void writeTriggersToFile(FILE *fp)
{
	int i;

	for (i=0;i<MAX_TRIGGERS;i++)
	{
		if (trigger[i].inUse == TRUE)
		{
			fprintf(fp, "{\n");
			fprintf(fp, "TYPE TRIGGER\n");
			fprintf(fp, "TRIGGER_NAME %s\n", trigger[i].triggerName);
			fprintf(fp, "TRIGGER_COUNT %d\n", trigger[i].count);
			fprintf(fp, "TRIGGER_TYPE %s\n", getTriggerTypeByID(trigger[i].targetType));
			fprintf(fp, "TRIGGER_TARGET %s\n", trigger[i].targetName);
			fprintf(fp, "}\n\n");
		}
	}
}

int getTriggerTypeByName(char *name)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(name, type[i].name) == 0)
		{
			return type[i].id;
		}
	}

	printf("Unknown Trigger Type %s\n", name);

	exit(1);
}

char *getTriggerTypeByID(int id)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (id == type[i].id)
		{
			return type[i].name;
		}
	}

	printf("Unknown Trigger ID %d\n", id);

	exit(1);
}