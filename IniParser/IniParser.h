/**
 *  @author	 :   Moreau Cyril <spl3en.contact@gmail.com>
 *  @file	   :   IniParser.c
 *  @version	:   1.4
 *
 *  IniParser est un ensemble de méthodes facilitant la lecture d'un fichier de type .ini.
 *  Pour se faire, il faut respecter un certain nombre de conditions :
 *  - Une seule valeur par ligne
 *  - Les valeurs sont définies ainsi : clef = valeur
 *  - Les valeurs numériques et alphabétiques sont acceptées, mais les valeurs numériques doivent être traitées avec atoi() pour être lues.
 *  - Les lignes commencant par un [ seront considérées comme commentaire, et tout ce qui suit le caractère ; sera considéré aussi comme tel : vous pouvez y écrire par la suite ce que vous souhaitez.
 *
 *  Pour l'utiliser, il faudra au préalable renseigner au parser l'ensemble des clef du .ini,
 *  via la fonction ini_parser_register_key.
 *  Néanmoins, si vous considérez avoir besoin de toutes les clées du fichier, vous pouvez utiliser ini_parser_reg_and_read qui se chargera
 *  d'enregistrer et de lire le fichier à votre place.
 *
 *  Pour récupérer une valeur, il suffira d'un ini_parser_get_value(parser, key) qui retournera un "char *" contenant votre valeur. (sous format ASCII)
 *  N'oubliez pas d'utiliser la fonction atoi() sur vos valeurs numériques.
 *
 *  En comparaison, la méthode de récupèration des valeurs de l'IniParser peut être assimilée à un tableau associatif.
 *
 *  Exemple :
 *	  IniParser *parser = ini_parser_new("config.ini");
 *	  ini_parser_reg_and_read(parser);
 *	  int value = atoi(ini_parser_get_value(parser, "INI_VALUE"));
 *
 *	  * ou avec internal *
 *
 *	  ini_parser_new_internal("config.ini");
 *	  ini_parser_reg_and_read_internal();
 *	  int value = atoi(ini_parser_get_value_internal("INI_VALUE"));
 *
 *	Changelog :
 *		[+] v1.1 :
 *			- [ADD]	 Les variables alphabétiques sont maintenant acceptées
 *
 *	  [+] v1.2 :
 *		  - [ADD]	 Les commentaires sont aussi permis grâce au caractère ';'
 *		  - [BUGFIX]  ini_parser_unref : mémoire libérée fixée
 *		  - [ADD]	 Utilisation des BbQueue au lieu des GQueue
 *
 *	  [+] v1.3 :
 *		  - [ADD]	 ini_parser_reg_and_read : Enregistre et lis le fichier de manière automatisée
 *
 *	  [+] v1.4 :
 *		  - [ADD]	 Toutes les fonctions ont maintenant leur version "singleton" :
 *					  On se débarrasse ainsi du pointeur sur IniParser
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
