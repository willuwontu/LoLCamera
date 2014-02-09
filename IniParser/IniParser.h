/**
 *  @author	 :   Moreau Cyril <spl3en.contact@gmail.com>
 *  @file	   :   IniParser.c
 *  @version	:   1.4
 *
 *  IniParser est un ensemble de m�thodes facilitant la lecture d'un fichier de type .ini.
 *  Pour se faire, il faut respecter un certain nombre de conditions :
 *  - Une seule valeur par ligne
 *  - Les valeurs sont d�finies ainsi : clef = valeur
 *  - Les valeurs num�riques et alphab�tiques sont accept�es, mais les valeurs num�riques doivent �tre trait�es avec atoi() pour �tre lues.
 *  - Les lignes commencant par un [ seront consid�r�es comme commentaire, et tout ce qui suit le caract�re ; sera consid�r� aussi comme tel : vous pouvez y �crire par la suite ce que vous souhaitez.
 *
 *  Pour l'utiliser, il faudra au pr�alable renseigner au parser l'ensemble des clef du .ini,
 *  via la fonction ini_parser_register_key.
 *  N�anmoins, si vous consid�rez avoir besoin de toutes les cl�es du fichier, vous pouvez utiliser ini_parser_reg_and_read qui se chargera
 *  d'enregistrer et de lire le fichier � votre place.
 *
 *  Pour r�cup�rer une valeur, il suffira d'un ini_parser_get_value(parser, key) qui retournera un "char *" contenant votre valeur. (sous format ASCII)
 *  N'oubliez pas d'utiliser la fonction atoi() sur vos valeurs num�riques.
 *
 *  En comparaison, la m�thode de r�cup�ration des valeurs de l'IniParser peut �tre assimil�e � un tableau associatif.
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
 *			- [ADD]	 Les variables alphab�tiques sont maintenant accept�es
 *
 *	  [+] v1.2 :
 *		  - [ADD]	 Les commentaires sont aussi permis gr�ce au caract�re ';'
 *		  - [BUGFIX]  ini_parser_unref : m�moire lib�r�e fix�e
 *		  - [ADD]	 Utilisation des BbQueue au lieu des GQueue
 *
 *	  [+] v1.3 :
 *		  - [ADD]	 ini_parser_reg_and_read : Enregistre et lis le fichier de mani�re automatis�e
 *
 *	  [+] v1.4 :
 *		  - [ADD]	 Toutes les fonctions ont maintenant leur version "singleton" :
 *					  On se d�barrasse ainsi du pointeur sur IniParser
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
