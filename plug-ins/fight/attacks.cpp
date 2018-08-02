/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
 * ��� ����� �� ���� ��� 'Dream Land' ����������� Igor {Leo} � Olga {Varda}*
 * ��������� ������ � ��������� ����� ����, � ����� ������ ������ ��������:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    � ��� ���������, ��� ��������� � ����� � ���� MUD                    *
 ***************************************************************************/
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/
#include "attacks.h"
#include "damageflags.h"
#include "grammar_entities_impl.h"
#include "merc.h"
#include "def.h"

using namespace Grammar;

/* attack table */
struct attack_type	attack_table	[]		=
{
    { "none",		"����",			-1,		MultiGender::MASCULINE },  /*  0 */
    { "slice",		"����������� ����",	DAM_SLASH,	MultiGender::MASCULINE },	
    { "stab",		"�����",		DAM_PIERCE,	MultiGender::MASCULINE },
    { "slash",		"������� ����",	        DAM_SLASH,	MultiGender::MASCULINE },
    { "whip",		"�������� ����",	DAM_SLASH,	MultiGender::MASCULINE },
    { "claw",		"���� �������",		DAM_SLASH,	MultiGender::MASCULINE },  /*  5 */
    { "blast",		"�����",		DAM_BASH,	MultiGender::MASCULINE },
    { "pound",		"������� ����",		DAM_BASH,	MultiGender::MASCULINE },
    { "crush",		"�������� ����",	DAM_BASH,	MultiGender::MASCULINE },
    { "grep",		"����", 		DAM_SLASH,	MultiGender::MASCULINE },
    { "bite",		"����",			DAM_PIERCE,	MultiGender::MASCULINE },  /* 10 */
    { "pierce",		"�������� �����",	DAM_PIERCE,	MultiGender::MASCULINE },
    { "suction",	"������������ ����",    DAM_BASH,	MultiGender::MASCULINE },
    { "beating",	"������ ����",		DAM_BASH,	MultiGender::MASCULINE },
    { "digestion",	"�������������",        DAM_ACID,	MultiGender::MASCULINE },
    { "charge",		"�����",		DAM_BASH,	MultiGender::FEMININE},  /* 15 */
    { "slap",		"������",		DAM_BASH,	MultiGender::MASCULINE },
    { "punch",		"���� �������",		DAM_BASH,	MultiGender::MASCULINE },
    { "wrath",		"����",  		DAM_ENERGY,	MultiGender::MASCULINE },
    { "magic",		"���������� ����",	DAM_ENERGY,	MultiGender::MASCULINE },
    { "divine",		"������������ �������",	DAM_HOLY,	MultiGender::FEMININE},  /* 20 */
    { "cleave",		"������������� ����",	DAM_SLASH,	MultiGender::MASCULINE },
    { "scratch",	"���������� ����",	DAM_PIERCE,	MultiGender::MASCULINE },
    { "peck",		"���� ������",		DAM_PIERCE,	MultiGender::MASCULINE },
    { "peckb",		"���� ������",		DAM_BASH,	MultiGender::MASCULINE },
    { "chop",		"������� ����",		DAM_SLASH,	MultiGender::MASCULINE },  /* 25 */
    { "sting",		"������� ����",		DAM_PIERCE,	MultiGender::MASCULINE },
    { "smash",		"����������� ����",	DAM_BASH,	MultiGender::MASCULINE },
    { "shbite",		"���������� ����",      DAM_LIGHTNING,	MultiGender::MASCULINE },
    { "flbite",		"���������� ����",	DAM_FIRE,	MultiGender::MASCULINE },
    { "frbite",		"��������� ����",       DAM_COLD,	MultiGender::MASCULINE },  /* 30 */
    { "acbite",		"���������� ����",      DAM_ACID,	MultiGender::MASCULINE },
    { "chomp",		"������ ����",		DAM_PIERCE,	MultiGender::MASCULINE },
    { "drain",		"���������� ����� ����",DAM_NEGATIVE,	MultiGender::MASCULINE },
    { "thrust",		"�����",		DAM_PIERCE,	MultiGender::MASCULINE },
    { "slime",		"������ ������",	DAM_ACID,	MultiGender::MASCULINE },
    { "shock",		"������",		DAM_LIGHTNING,	MultiGender::MASCULINE },
    { "thwack",		"���� � �������",	DAM_BASH,	MultiGender::MASCULINE },
    { "flame",		"�����",		DAM_FIRE,	MultiGender::NEUTER},
    { "chill",		"�����",		DAM_COLD,	MultiGender::MASCULINE },
    { "cuff",		"������������",		DAM_BASH,	MultiGender::MASCULINE },
    { "hooves",		"���� ��������",	DAM_BASH,	MultiGender::MASCULINE },
    { "horns",		"���� ������",		DAM_BASH,	MultiGender::MASCULINE },
    { "spines",         "���� ��������",      	DAM_PIERCE, 	MultiGender::MASCULINE }, 
    { "cacophony",      "���������",      	DAM_SOUND,      MultiGender::FEMININE}, 
    { 0,		0,			0		}
};

