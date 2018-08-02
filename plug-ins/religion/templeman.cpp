/* $Id: templeman.cpp,v 1.1.2.4 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2005
 */

#include "templeman.h"

#include "pcharacter.h"
#include "npcharacter.h"

#include "religion.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "def.h"

RELIG(none);

/*----------------------------------------------------------------------
 * Templeman
 *---------------------------------------------------------------------*/
Templeman::Templeman( ) 
{
}

void Templeman::speech( Character *victim, const char *speech )
{
    Religion *chosen;
    PCharacter *pvict;
    
    if (!IS_AWAKE(ch))
	return;

    if (victim->is_npc()) {
	do_say(ch, "�����, ������ ��������.");
	return;
    }
    
    pvict = victim->getPC();

    if (!str_cmp( speech, "religion" )) {
	do_say(ch, "�� ������������� ������������� ��������?");
	do_say(ch, "���� ������ ������ ��������� 'help religion'.");
	do_say(ch, "�� ������, ��� ������� ������� ������� ������ ���� ���.");
	do_say(ch, "���� �� ���������, � �� ����� ��� ���������!");
	return;
    }

    chosen = religionManager->findExisting( DLString( speech ).toLower( ) );

    if (!chosen)
	return;
    
    if (pvict->getReligion( ) != god_none) {
	interpret_raw( ch, "say", "�� ��� ������ ���� ����! ���� ������� - %s",
		       pvict->getReligion( )->getShortDescr( ).c_str( ) );
	return;
    }
    
    if (!chosen->isAllowed( pvict )) {
	do_say(ch, "��� ������� �� ������������� ������ ��������� � �����.");
	return;
    }

    pvict->setReligion( chosen->getName( ) );
    interpret_raw( ch, "say", "� ���� ������ �� �������� ��������� ����� �������� %s",
		   pvict->getReligion( )->getShortDescr( ).c_str( ) );
}

void Templeman::greet( Character *victim )
{
    if (!IS_AWAKE(ch))
	return;

    if (!ch->can_see(victim) || victim->is_npc() || victim->is_immortal())
	return;

    interpret_fmt( ch, "smile %s", victim->getNameP( ) );
}

