/**
 *  @author	 :   Moreau Cyril - Spl3en
 *  @file	   :   IniParser.c
 *
 *  Furthermore informations in IniParser.h
 */

#include "IniParser.h"

IniParser *this = NULL;

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
*   @param  filename  : Nom du fichier de configuration � parser
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
*	 @Methods	 *
 * * * * * * * * * */

int
ini_parser_register_key (IniParser *ip, char *field)
/**
*   Enregistre une clef dans l'iniParser
*   @param  field : la clef
*   @return int   : L'index de la cl� enregistr�e
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
*   Enregistre automatiquement toutes les cl�s et lis le fichier
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

	fHandler = fopen(ip->filename, "r");

	if (!fHandler)
	{
		printf("%s : Cannot read the %s file !\n", __FUNCTION__, ip->filename);
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

			index = ini_parser_register_key(ip, tmpstr);
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
*   Parse un fichier .ini.
*   @param ip : Le pointeur sur IniParser
*
*   /!\ Attention !
*
*   A partir de ce point, les clef doivent d�ja �tre enregistr�es manuellement via ini_parser_register_key
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

	fHandler = fopen(ip->filename, "r");

	if (!fHandler)
	{
		printf("%s : Cannot read the ini file !\n", __FUNCTION__);
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
*   Affiche l'ensemble des clef de l'IniParser.
*   Utile pour le debuggage.
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
*   Retourne le character de la clef associ�e
*   @param  char *field	 : clef dont on veut la valeur
*   @return char		  : character associ� ou 0 en cas d'absence de la clef
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
		return res[0];
}

void *
ini_parser_get_value (IniParser *ip, char *field)
/**
*   Retourne la valeur de la clef associ�e
*   @param  char *field	 : clef dont on veut la valeur
*   @return void *		  : pointeur g�n�rique sur la valeur retourn�e
*/
{
	int index;

	index = _ini_parser_get_index(ip, field);

	if (index == INI_PARSER_INDEX_ERROR)
		return NULL;

	return bb_queue_pick_nth (ip->vList, index);
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
*   Lib�re en m�moire un IniParser
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
