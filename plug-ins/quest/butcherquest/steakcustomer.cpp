/* $Id: steakcustomer.cpp,v 1.1.2.12.6.4 2008/03/06 17:48:29 rufina Exp $
 *
 * ruffina, 2003
 */

#include "steakcustomer.h"
#include "butcherquest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "vnum.h"
#include "handler.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

void SteakCustomer::greet( Character *victim ) 
{
    if (ourHero( victim ))
	act( "$c1 ��������� ������� �� ����.", ch, 0, victim, TO_VICT );
}

bool SteakCustomer::givenCheck( PCharacter *hero, Object *obj )
{
    MOB_INDEX_DATA *orig;
    
    if (!getQuest( ))
	return false;

    if (obj->pIndexData->vnum != OBJ_VNUM_STEAK) {
	tell_fmt( "��� �� ��� ��������%1$G��|�|��! ��� ���� �� ����!", hero, ch );
	return false;
    }

    if (!( orig = get_mob_index( obj->value[2] ) )) {
	tell_fmt( "����, � ���� �� ��� �����%1$G��|�|��?!", hero, ch );
	return false;
    }

    if (quest->raceName != orig->race) {
	tell_fmt( "������� �����, �� � ���������%2$G��|�|�� ���� %3$s.",
		  hero, ch, quest->raceRusName.c_str( ) );
	return false;	
	
    } 
    
    if (quest->areaName != orig->area->name) {
	tell_raw( hero, ch, 
		"��� ����� ������� � %s, � �� � %s.",
		orig->area->name, quest->areaName.c_str( ) );
	return false;
    }

    return true;
}

void SteakCustomer::givenBad( PCharacter *hero, Object *obj )
{
    act("$c1 ���������� ���� $o4.", ch, obj, hero, TO_VICT);
    act("$c1 ���������� $C5 $o4.", ch, obj, hero, TO_NOTVICT);
}

void SteakCustomer::givenGood( PCharacter *hero, Object *obj )
{
    quest->delivered++;
    
    if (quest->delivered == quest->ordered) 
	tell_raw(hero, ch, "������� �� ������! ������� � �������� �� ��������.");
    else if (quest->delivered > quest->ordered) 
	tell_fmt("������, ����%1$G��|��|��.", hero, ch);
    else 
	tell_raw(hero, ch, "�������� �����...");
    
    act("$c1 ����-�� ������ $o4.", ch, obj, 0, TO_ROOM);
    extract_obj( obj );
}

void SteakCustomer::deadAction( Quest::Pointer quest, PCMemoryInterface *pcm, Character *killer )
{
    if (pcm->isOnline( )) 
	pcm->getPlayer( )->println( "{Y����� ������� � ����� � �������� ���������.{x" );

    ProtectedClient::deadAction( quest, pcm, killer );
}

