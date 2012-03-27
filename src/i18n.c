/*
Copyright (C) 2009-2012 Parallel Realities

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
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "headers.h"

static HashTable table;

static int hashCode(char *);
static void put(char *, char *);
static void initTable(void);

void setLanguage()
{
	char language[MAX_LINE_LENGTH], c[MAX_LINE_LENGTH]
	char *lang, **key, **value;
	int read;
	FILE *fp;
	MOHeader header;
	MOEntry *original, *translated;

	initTable();

	#ifdef _WIN32
		GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, c, MAX_LINE_LENGTH);

		if (c[0] != '\0')
		{
			STRNCPY(language, c, MAX_LINE_LENGTH);

			GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, c, MAX_LINE_LENGTH);

			if (c[0] != '\0')
			{
				strncat(language, "_", MAX_MESSAGE_LENGTH - strlen(language) - 1);

				strncat(language, c, MAX_MESSAGE_LENGTH - strlen(language) - 1);
			}
		}
	#else
		if ((lang = getenv("LC_ALL")) || (lang = getenv("LC_CTYPE")) || (lang = getenv("LANG")))
		{
			STRNCPY(language, lang, MAX_LINE_LENGTH);
		}
	#endif

	if (strstr(language, ".") != NULL)
	{
		lang = strtok(language, ".");

		STRNCPY(language, lang, MAX_LINE_LENGTH);
	}

	snprintf(c, MAX_LINE_LENGTH, "%s/%s/LC_MESSAGES/edgar.mo", LOCALE_DIR, language);

	#if DEV == 1
		printf("Opening %s\n", c);
	#endif

	fp = fopen(c, "rb");

	if (fp == NULL)
	{
		printf("Failed to open %s/%s/LC_MESSAGES/edgar.mo\n", LOCALE_DIR, language);

		return;
	}

	read = fread(&header, sizeof(header), 1, fp);

	original = malloc(sizeof(MOEntry) * header.stringCount);

	translated = malloc(sizeof(MOEntry) * header.stringCount);

	if (original == NULL || translated == NULL)
	{
		printf("Failed to allocate %d bytes for translation strings\n", sizeof(MOEntry) * header.stringCount);

		exit(1);
	}

	#if DEV == 1
		printf("MO file has %d entries\n", header.stringCount);
	#endif

	fseek(fp, header.originalOffset, SEEK_SET);

	key = malloc(sizeof(char *) * header.stringCount);

	value = malloc(sizeof(char *) * header.stringCount);

	if (key == NULL || value == NULL)
	{
		printf("Failed to allocate a whole %d bytes for translation strings\n", sizeof(char *) * header.stringCount);

		exit(1);
	}

	for (i=0;i<header.stringCount;i++)
	{
		fread(&original[i].offset, sizeof(int), 1, fp);
		fread(&original[i].length, sizeof(int), 1, fp);

		key[i] = malloc(original[i].length + 1);

		if (key[i] == NULL)
		{
			printf("Failed to allocate a whole %d bytes for translation string\n", original[i].length + 1);

			exit(1);
		}
	}

	fseek(fp, header.translationOffset, SEEK_SET);

	for (i=0;i<header.stringCount;i++)
	{
		fread(&translation[i].offset, sizeof(int), 1, fp);
		fread(&translation[i].length, sizeof(int), 1, fp);

		value[i] = malloc(translation[i].length + 1);

		if (value[i] == NULL)
		{
			printf("Failed to allocate a whole %d bytes for translation string\n", translation[i].length + 1);

			exit(1);
		}
	}

	fseek(fp, original[0].offset, SEEK_SET);

	for (i=0;i<header.stringCount;i++)
	{
		fread(key[i], original[i].length, 1, fp);

		#if DEV == 1
			printf("msgid %d: %s\n", i, key[i]);
		#endif
	}

	fseek(fp, translation[0].offset, SEEK_SET);

	for (i=0;i<header.stringCount;i++)
	{
		fread(value[i], translation[i].length, 1, fp);

		#if DEV == 1
			printf("msgstr %d: %s\n", i, value[i]);
		#endif
	}

	fclose(fp);

	for (i=0;i<header.stringCount;i++)
	{
		put(key[i], value[i]);

		free(key[i]);

		free(value[i]);
	}

	free(key);

	free(value);

	free(original);

	free(translated);
}

static int hashCode(char *data)
{
	int i, length;
	unsigned int hash;

	length = strlen(data);

	hash = 0;

	for (i=0;length;i++)
	{
		hash = data[i] + (hash << 5) - hash;
	}

	return hash % TABLE_SIZE;
}

static void initTable()
{
	int i;

	table.bucket = malloc(sizeof(Bucket *) * TABLE_SIZE);

	if (table.bucket == NULL)
	{
		printf("Failed to allocate %d bytes for a hashtable\n", sizeof(Bucket *) * TABLE_SIZE);

		exit(1);
	}

	for (i=0;i<TABLE_SIZE;i++)
	{
		table.bucket[i] = malloc(sizeof(Bucket));

		table.bucket[i]->next = NULL;
	}
}

static void put(char *key, char *value)
{
	Bucket *bucket, *newBucket;
	unsigned int hash = hashCode(key);

	#if DEV == 1
		printf("Got %d from %s\n", hash, key);
	#endif

	bucket = table.bucket[hash];

	while (bucket->next != NULL)
	{
		bucket = bucket->next;
	}

	newBucket = malloc(sizeof(Bucket));

	if (newBucket == NULL)
	{
		printf("Failed to allocate a whole %d bytes for a hashtable bucket\n", sizeof(Bucket));

		exit(1);
	}

	newBucket->key   = malloc(strlen(key) + 1);
	newBucket->value = malloc(strlen(value) + 1);

	if (newBucket->key != NULL || newBucket->value)
	{
		printf("Failed to allocate a whole %d bytes for a translation\n", newBucket->key);

		exit(1);
	}

	STRNCPY(newBucket->key, key, strlen(key) + 1);
	STRNCPY(newBucket->value, value, strlen(value) + 1);

	newBucket->next = NULL;

	bucket->next = newBucket;
}

char *getTranslatedString(char *key)
{
	Bucket *bucket;
	unsigned int hash = hashCode(key);

	bucket = table.bucket[hash]->next;

	for (;bucket!=NULL;bucket=bucket->next)
	{
		if (strcmpignorecase(key, bucket->key) == 0)
		{
			return strlen(bucket->value) == 0 ? key : bucket->value;
		}
	}

	return key;
}

void cleanupLanguage()
{
	int i;
	Bucket *bucket, *p, *q;

	for (i=0;i<TABLE_SIZE;i++)
	{
		bucket = table.bucket[i];

		for (p=bucket->next;p!=NULL;p=q)
		{
			free(p->key);
			free(p->value);

			q = p->next;

			free(p);
		}

		free(bucket);
	}

	table.bucket = NULL;
}