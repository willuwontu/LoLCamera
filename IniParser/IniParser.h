/**
 *  @author	 :   Moreau Cyril <spl3en.contact@gmail.com>
 *  @file	 :   IniParser.c
 *  @version :   1.4
 *
 *  IniParser is a set of functions facilitating the reading of an .ini file
 *  In order to do this, several conditions have to be respected:
 *  - A single value per line
 *  - The values are defined as follows: key = value
 *  - Numerical and alphabetical values are accepted, but numerical valued have to be handled with atoi() in order to be read.
 *  - Lines beginning with a [ will be considered as a comment, and everything which follow the character ; will be considered as one as well: anything can then written afterwards.
 *
 *  To use it, the set of keys of the .ini will have to be provided to the parser beforehand, 
 *  using the function ini_parser_register_key.
 *  Nevertheless, if all the keys of the file are needed, ini_parser_reg_and_read can be used to
 *  save and read the file at your place.
 *
 *  To retrieve a value, ini_parser_get_value(parser, key) will suffice. It will return a "char *" containing your value. (in ASCII format)
 *  The atoi() function should not be forgotten for numerical values.
 *
 *  In comparison, the method of retrival of the values of the IniParser can be assimilated to an associative array.
 *
 *  Example :
 *	  IniParser *parser = ini_parser_new("config.ini");
 *	  ini_parser_reg_and_read(parser);
 *	  int value = atoi(ini_parser_get_value(parser, "INI_VALUE"));
 *
 *	  * or with internal *
 *
 *	  ini_parser_new_internal("config.ini");
 *	  ini_parser_reg_and_read_internal();
 *	  int value = atoi(ini_parser_get_value_internal("INI_VALUE"));
 *
 *	Changelog :
 *	  [+] v1.1 :
 *		  - [ADD]	 Alphabetical variables are now accepted
 *
 *	  [+] v1.2 :
 *		  - [ADD]	 Comments are also allowed thanks to the ';' character
 *		  - [BUGFIX] ini_parser_unref : freed memory fixed
 *		  - [ADD]	 Usage of the BbQueues instead of GQueues
 *
 *	  [+] v1.3 :
 *		  - [ADD]	 ini_parser_reg_and_read : Saves and reads files in an automated fashion
 *
 *	  [+] v1.4 :
 *		  - [ADD]	 Every functions have their "singleton" version:
 *					  This allows us to get rid of the IniParser pointer
 */

#ifndef INIPARSER_H_INCLUDED
#define INIPARSER_H_INCLUDED

/* Librairies */
#include "../BbQueue/BbQueue.h"
#include "../Ztring/Ztring.h"

#define INI_PARSER_INDEX_ERROR  -1


typedef
struct _IniParser
{
	BbQueue *vList, // Value List
			*sList; // String List

	char *filename;

}	IniParser;


typedef struct
{
	char *key;
	void *res;

} 	KeyVal;

/** * * * * * * * *
*   @Constructors  *
 * * * * * * * * * */

IniParser *
ini_parser_new					(char *filename);
void
ini_parser_new_internal			(char *filename);

/** * * * * * * * *
*	 @Functions	 *
 * * * * * * * * * */

int
ini_parser_register_key	 (IniParser *ip, char *field);
int
ini_parser_replace_field	(IniParser *ip, char *field, char *replace);
void *
ini_parser_get_value		(IniParser *ip, char *field);
char
ini_parser_get_char 	(IniParser *ip, char *field);
void
ini_parser_read			 (IniParser *ip);
void
ini_parser_debug			(IniParser *ip);
void
ini_parser_reg_and_read	 (IniParser *ip);
BbQueue *
ini_parser_get_section (IniParser *ip, char *section);
int
ini_parser_register_key_internal	(char *field);
void *
ini_parser_get_value_internal	   (char *field);
int
ini_parser_replace_field_internal   (char *field, char *replace);
void
ini_parser_read_internal			();
void
ini_parser_debug_internal		   ();
void
ini_parser_reg_and_read_internal	();


/** * * * * * * * *
*   @Destructors   *
 * * * * * * * * * */

void
ini_parser_free		  (IniParser *parser);
void
ini_parser_free_internal ();




#endif // INIPARSER_H_INCLUDED
