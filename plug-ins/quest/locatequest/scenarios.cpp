/* $Id: scenarios.cpp,v 1.1.2.8.6.1 2007/09/29 19:34:06 rufina Exp $
 *
 * ruffina, 2004
 */
#include "scenarios.h"
#include "locatequest.h"
#include "questexceptions.h"

#include "selfrate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "interp.h"
#include "mercdb.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*-----------------------------------------------------------------------------
 * LocateScenario, default implementation
 *----------------------------------------------------------------------------*/
bool LocateScenario::applicable( PCharacter * )
{
    return true;
}

int LocateScenario::getCount( PCharacter *pch )
{
    if (rated_as_guru( pch ))
	return number_range( 6, 10 );
    else if (rated_as_newbie( pch ))
	return number_range( 2, 4 );
    else
	return number_range( 2, 10 );
}

void LocateScenario::actWrongItem( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest, Object *obj )
{
    act( "$c1 ���������� '{g�������, �������, �� � �� �� ���� �����$g��|�|�� ����.'{x'", ch, 0, 0, TO_ROOM );
    act( "$c1 ���������� ���� $o4.", ch, obj, hero, TO_VICT );
    act( "$c1 ���������� $C3 $o4.", ch, obj, hero, TO_NOTVICT );
}

void LocateScenario::actLastItem( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest )
{
    act( "$c1 ���������� '{g��� �������, $C1. ������ ��� ������� � � ���� ����� ��������.{x'", 
	ch, 0, hero, TO_ROOM );
    act( "$c1 ���������� '{g� �������������� � ��� ������$g��|�|�� ������ ��������. ����� � ������ ���.{x'",
	ch, 0, hero, TO_ROOM );
}

void LocateScenario::actAnotherItem( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest )
{
    if (chance(1) && quest->delivered == 1) {
	act( "$c1 ���������� '{g��-��, ��� ���������, ��� 65535 ����� - � ������� ������ � ��� � �������.{x'", ch, 0, hero, TO_ROOM );
	interpret_raw( ch, "grin" );
	return;
    } 

    switch (number_range( 1, 3 )) {
    case 1:
	if (quest->delivered > 1) {
	    act( "$c1 ���������� '{g�, �� ���$G��|��|�� ��� $t!{x'", 
		    ch, russian_case( quest->itemName.getValue( ), '4' ).c_str( ), hero, TO_ROOM );
	    break;
	}
	/* FALLTHROUGH */
    case 2:
        act( "$c1 ���������� '{g������ �� ��� $t, �������� ������ �������.{x'", 
		ch, DLString(quest->delivered).c_str( ), 0, TO_ROOM );
	break;
    case 3:
	interpret_fmt( ch, "nod %s", hero->getNameP( ) );
	break;
    }
}

