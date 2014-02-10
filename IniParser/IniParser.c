/**
 *  @author	 :   Moreau Cyril - Spl3en
 *  @file	   :   IniParser.c
 *
 *  More information in IniParser.h
 */

#include "IniParser.h"
#include <assert.h>
#include <string.h>

static IniParser *this = NULL;

/*
*   Private Functions Declarations
*/

static void
_ini_parser_register_value (IniParser *ip, char *buffer, int index);

static int
_ini_parser_get_index (IniParser *ip, char *msg);

/*
*   Public Functions
*/

/** * * * * * * * *
*   @Constructors  *
 * * * * * * * * * */

IniParser *
ini_parser_new (char *filename)
/**
*   Instancie un IniParser
*   @param  filename  : Name of the configuration file to parse
*   @return IniParser
*/
{
	IniParser *p = NULL;

	p = (IniParser *) malloc (sizeof (IniParser));
	assert (p != NULL);

	p->vList = bb_queue_new();
	p->sList = bb_queue_new();
	p->filename = filename;

	return p;
}

void
ini_parser_new_internal (char *filename)
{
	this = ini_parser_new(filename);
}

/** * * * * * * * *
*	 @Functions   *
 * * * * * * * * * */

int
ini_parser_register_key (IniParser *ip, char *field)
/**
*   Saves a key in IniParser
*   @param  field : the key
*   @return int   : The index of the saved key
*/
{
	assert(ip != NULL);
	assert(field != NULL);

	char *toList = NULL;

	str_cpy(&toList, field);

	bb_queue_add(ip->sList, toList);
	bb_queue_add(ip->vList, (void*) INI_PARSER_INDEX_ERROR);

	return bb_queue_get_length(ip->sList) - 1;
}

int
ini_parser_register_key_internal (char *field)
{
	return ini_parser_register_key(this, field);
}

void
ini_parser_reg_and_read (IniParser *ip)
/**
*   Automatically saves every key and reads the file
*/
{
	assert(ip != NULL);
	assert(ip->filename != NULL);

	FILE *fHandler = NULL;

	char c = 0,
		*buffer = NULL,
		*tmpstr = NULL;

	int pos = 0;
	int index = INI_PARSER_INDEX_ERROR;
	int endOfLoop = 0;
	int len = 0;
	int maxLen = 0;
	char tmp[1024 * 2];
	char section[1024 * 2] = {'\0'};
	int tmp_pos = 0;

	fHandler = fopen(ip->filename, "r");

	if (!fHandler)
	{
		printf("%s: Cannot read the %s file!\n", __FUNCTION__, ip->filename);
		exit(0);
	}

	c = fgetc(fHandler);

	while (c != EOF)
	{
		len++;

		if (c == '\n')
		{
			if (len > maxLen)
				maxLen = len;
			len = 0;
		}

		c = fgetc(fHandler);
	}

	buffer = malloc(maxLen + 1);
	memset(buffer, '\0', maxLen + 1);

	rewind(fHandler);
	c = fgetc(fHandler);

	while (!endOfLoop)
	{
		if (c == '#')
		{
			while ((c = fgetc(fHandler)) != EOF && c != '\n')
			{
				tmp[tmp_pos++] = c;
			}
			tmp[tmp_pos] = '\0';

			tmp_pos = 0;

			if (strcmp(tmp, "end") == 0)
			{
				// End of section
				section[0] = '\0';
			}

			else
			{
				strncpy(section, tmp, sizeof(section));
			}
		}

		if (c == '[' || c == ';')
		{
			while (c != EOF && c != '\n')
			{
				c = fgetc(fHandler);
			}
		}

		if (c == '\n')
		{
			buffer[pos] = '\0';
			pos = 0;
			tmpstr = str_trim(buffer);

			_ini_parser_register_value(ip, tmpstr, index);
			index = INI_PARSER_INDEX_ERROR;
		}

		else if (c == '=')
		{
			buffer[pos] = '\0';
			tmpstr = str_trim(buffer);
			pos = 0;

			if (section[0] != '\0')
				sprintf(tmp, "[%s]%s", section, tmpstr);
			else
				strcpy(tmp, tmpstr);

			index = ini_parser_register_key(ip, tmp);
		}

		else if (c == EOF)
		{
			if (index != INI_PARSER_INDEX_ERROR)
			{
				buffer[pos] = '\0';
				pos = 0;
				tmpstr = str_trim(buffer);
				_ini_parser_register_value(ip, tmpstr, index);
				index = INI_PARSER_INDEX_ERROR;
				break;
			}

			endOfLoop = 1;
		}

		else
			buffer[pos++] = c;

		c = fgetc(fHandler);
	}

	fclose(fHandler);
	free(buffer);
}

void
ini_parser_reg_and_read_internal ()
{
	ini_parser_reg_and_read(this);
}


void
ini_parser_read (IniParser *ip)
/**
*   Parses an .ini file
*   @param ip : The pointer to IniParser
*
*   /!\ Beware!
*
*   After this point, the keys must already be saved manually via ini_parser_register_key
*/
{
	assert(ip != NULL);
	assert(ip->filename != NULL);

	FILE *fHandler = NULL;

	char c = 0,
		*buffer = NULL,
		*tmpstr = NULL;

	int pos = 0;
	int index = INI_PARSER_INDEX_ERROR;
	int endOfLoop = 0;
	int len = 0;
	int maxLen = 0;
	char tmp[1024];
	char section[1024];
	int tmp_pos = 0;

	fHandler = fopen(ip->filename, "r");

	if (!fHandler)
	{
		printf("%s: Cannot read the ini file!\n", __FUNCTION__);
		exit(0);
	}

	c = fgetc(fHandler);

	while (c != EOF)
	{
		len++;

		if (c == '\n')
		{
			if (len > maxLen)
				maxLen = len;
			len = 0;
		}

		c = fgetc(fHandler);
	}

	buffer = malloc(maxLen + 1);

	rewind(fHandler);
	c = fgetc(fHandler);

	while (!endOfLoop)
	{
		if (c == '#')
		{
			while (c != EOF && c != '\n')
			{
				tmp[tmp_pos++] = fgetc(fHandler);
			}

			tmp_pos = 0;

			if (strcmp(tmp, "end") == 0)
			{
				// End of section
			}

			else
			{
				strncpy(section, tmp, sizeof(section));
			}
		}

		if (c == '[' || c == ';')
		{
			while (c != EOF && c != '\n')
			{
				c = fgetc(fHandler);
			}
		}

		else if (c == '\n')
		{
			buffer[pos] = '\0';
			pos = 0;
			tmpstr = str_trim(buffer);
			_ini_parser_register_value(ip, tmpstr, index);
			index = INI_PARSER_INDEX_ERROR;
		}

		else if (c == '=')
		{
			while (buffer[pos-1] == ' ')
			{
				pos--;
			}

			buffer[pos] = '\0';
			tmpstr = str_trim(buffer);
			pos = 0;

			index = _ini_parser_get_index(ip, tmpstr);
		}

		else if (c == EOF)
		{
			if (index != INI_PARSER_INDEX_ERROR)
			{
				buffer[pos] = '\0';
				pos = 0;
				tmpstr = str_trim(buffer);
				_ini_parser_register_value(ip, tmpstr, index);
				index = INI_PARSER_INDEX_ERROR;
				break;
			}

			endOfLoop = 1;
		}

		else
			buffer[pos++] = c;

		c = fgetc(fHandler);
	}

	fclose(fHandler);
	free(buffer);
}

void
ini_parser_read_internal ()
{
	ini_parser_read(this);
}

void
ini_parser_debug (IniParser *ip)
/**
*   Displays the set of keys of the IniParser.
*   Useful for debugging.
*/
{
	int i, len = bb_queue_get_length(ip->vList);

	char *value = NULL;
	char *str = NULL;

	for (i = 1 ; i < len + 1; i++)
	{
		value = (char *) bb_queue_pick_nth (ip->vList, i);
		str   = (char *) bb_queue_pick_nth (ip->sList, i);

		printf("'%s' = %s \n", str, value);
	}
}

void
ini_parser_debug_internal ()
{
	ini_parser_debug(this);
}

char
ini_parser_get_char (IniParser *ip, char *field)
/**
*   Returns the character of the associated key
*   @param  char *field	 : key we want the value of
*   @return char		 : associated character or 0 in case the key is missing
*/
{
	int index;

	index = _ini_parser_get_index(ip, field);

	if (index == INI_PARSER_INDEX_ERROR)
		return 0;

	char *res = bb_queue_pick_nth (ip->vList, index);

	if (res == NULL)
		return 0;

	else
	{
		char c = res[0];

		if (!is_letter(c))
			return 0;

		else return c;
	}
}

void *
ini_parser_get_value (IniParser *ip, char *field)
/**
*   Returns the value of the associated key
*   @param  char *field	 : key we want the value of
*   @return void *		 : generic pointer to the returned value
*/
{
	int index;

	index = _ini_parser_get_index(ip, field);

	if (index == INI_PARSER_INDEX_ERROR)
		return NULL;

	return bb_queue_pick_nth (ip->vList, index);
}

BbQueue *
ini_parser_get_section (IniParser *ip, char *section)
/**
*   Returns the keys/values of the associated section
*   @param  char *section : section we want the values of
*   @return BbQueue *     : Chained list of struct {char *key; void *res};
*/
{
	BbQueue *res = bb_queue_new();
	char tmp_section[1024];
	int idx = 1;

	foreach_bbqueue_item (ip->sList, char *key)
	{
		if (key[0] == '[')
		{
			str_bet_buffer(key, "[", "]", tmp_section);

			if (strcmp(section, tmp_section) == 0)
			{
				KeyVal *kv = malloc(sizeof(KeyVal));
				kv->key = str_bet(key, "]", (void*)-1);
				kv->res = bb_queue_pick_nth(ip->vList, idx);
				bb_queue_add(res, kv);
			}
		}

		idx++;
	}

	return res;
}

void *
ini_parser_get_value_internal (char *field)
{
	return ini_parser_get_value(this, field);
}

/*
*   Private Functions Definitions
*/
static void
_ini_parser_register_value (IniParser *ip, char *buffer, int index)
{
	if (index == INI_PARSER_INDEX_ERROR)
		return;

	char *value = strdup(buffer);

	bb_queue_remv_nth(ip->vList, index + 1);
	bb_queue_add_nth(ip->vList, value, index + 1);
}

static int
_ini_parser_get_index (IniParser *ip, char *buffer)
{
	int i;

	int fail = 1;
	char *field = NULL;
	int len =  bb_queue_get_length(ip->sList);

	for (i = 0; i < len; i++)
	{
		field = (char*) bb_queue_pick_nth(ip->sList, i + 1);

		if (strcmp(field, buffer) == 0)
		{
			fail = 0;
			break;
		}
	}

	if (!fail)
		return i + 1;

	else
		return INI_PARSER_INDEX_ERROR;
}

/** * * * * * * * *
*   @Destructors   *
 * * * * * * * * * */

void
ini_parser_free (IniParser *p)
/**
*   Frees IniParser from memory
*/
{
	if (p == NULL)
		return;

	bb_queue_free_all(p->sList, free);
	bb_queue_free_all(p->vList, free);

	free(p);
}


void
ini_parser_free_internal ()
{
	ini_parser_free(this);
}
