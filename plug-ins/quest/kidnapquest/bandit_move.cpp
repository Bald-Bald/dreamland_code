/* $Id: bandit_move.cpp,v 1.1.2.15.6.3 2008/03/21 22:41:35 rufina Exp $
 *
 * ruffina, 2004
 */

#include "pcharacter.h"
#include "npcharacter.h"
#include "fight.h"

#include "kidnapquest.h"
#include "scenario.h"
#include "bandit.h"


/*
 * ���������� �������
 */

bool KidnapBandit::canEnter( Room *const room )
{
    if (!Wanderer::canEnter( room ))
	return false;
    
    if (state == BSTAT_KIDNAP)
	if (getKingRoom( room ) || getAggrRoom( room ))
	    return false;
    
    return true;
}

void KidnapBandit::princeHunt( )
{
    Room *old_room = ch->in_room;
    
    if (!getPrince( )) 
	return;
    if (!getQuest( ))
	return;

    path.clear( );
    pathToTarget( old_room, prince->in_room, 10000 );
    
    if (!path.empty( )) {
	quest->getScenario( ).actHuntStep( ch );
	debug( "��� �����, � ����� ������ ��� ��� �������!" );
	makeOneStep( );
    }

    if (ch->in_room == old_room && path.empty( )) {
	quest->getScenario( ).actEmptyPath( ch, prince );
	debug( "�� ���� ��������� �� ����, ��-�� ��� �����?" );
	heroAttack( );
    }
}

void KidnapBandit::princeKidnap( )
{
    Room *old_room = ch->in_room;
    
    if (getHeroWorld( ) == NULL) 
	return;
    if (!getPrince( ))
	return;
    if (!getQuest( ))
	return;

    if (path.empty( ))
	pathWithDepth( old_room, 20, 5000 );

    if (!path.empty( )) {
	quest->getScenario( ).actKidnapStep( ch, prince );
	debug( "��� �����, � ����� ������ ��� ��� ����������!" );
	makeOneStep( );
    }

    if (ch->in_room == old_room && path.empty( )) {
	quest->getScenario( ).actEmptyPath( ch, prince );
	debug( "��� ������ ��� ������, ��-�� ��� �����?" );
	heroAttack( );
    }
}



