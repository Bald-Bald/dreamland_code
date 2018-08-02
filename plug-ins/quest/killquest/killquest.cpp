/* $Id: killquest.cpp,v 1.1.2.26.6.10 2010-08-24 20:25:55 rufina Exp $
 *
 * ruffina, 2003
 */
#include "killquest.h"
#include "victimbehavior.h"
#include "questexceptions.h"
#include "profflags.h"

#include "selfrate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "handler.h"
#include "mercdb.h"
#include "merc.h"
#include "act.h"
#include "save.h"
#include "def.h"

KillQuest::KillQuest( )
{
}

void KillQuest::create( PCharacter *pch, NPCharacter *questman ) 
{
    NPCharacter *victim;
    Room *pRoom;
    int time;
    
    charName.setValue( pch->getName( ) );

    switch (number_range( 0, 5 )) {
    case 0: mode = 0; break;
    case 1:
    case 2: mode = 1; break;
    case 3:
    case 4: mode = 2; break;
    case 5: mode = 3; break;
    }
    
    if (rated_as_guru( pch )) 
	mode = 3;
    else if (rated_as_newbie( pch ) && mode > 1)
	mode = 0;

    victim = getRandomVictim( pch );
    
    if (victim->getProfession( )->getFlags( victim ).isSet(PROF_CASTER))
	mode++;
    
    if (!isMobileVisible( victim, pch ))
	mode++;

    assign<VictimBehavior>( victim );
    save_mobs( victim->in_room );
    
    time = number_range( 15, 25 ); 
    setTime( pch, time );
    
    pRoom = victim->in_room;
    roomName.setValue( pRoom->name );
    areaName.setValue( pRoom->area->name );
    mobName.setValue( victim->getShortDescr( ) );

    wiznet( "", "%s Lev %d, Qmode %d",
		 victim->getNameP('1').c_str( ),
		 victim->getRealLevel( ), 
		 mode.getValue( ) );

    tell_raw( pch, questman, "� ���� ���� ��� ���� ������� ���������!" );

    if (IS_GOOD( pch ))	{	
	tell_raw( pch, questman, "����������� ������ ���� ���� ��������!" );
	tell_fmt( "� ������� ���� �������� {W%3$#C4{G, ���������%3$G��|���|�� ��������� ���������.", pch, questman, victim );
    }
    else if (IS_EVIL( pch )) {
	tell_fmt( "������ ���� ������ ���� ��������� ���������� {W%3$#C2{G.", pch, questman,victim );
	tell_fmt( "���� ���������� ����� %3$P2!",  pch, questman, victim );
    } else {
	tell_raw( pch, questman, "���������� ������ ���� ���� ��������!");
	tell_fmt( "� ���� �����%3$G��|��|�� {W%3$#C1{G, � ������� ���� �������� %3$P2.",  pch, questman, victim );
    }

    tell_fmt( "�����, ��� %3$P2 ������ � ��������� ��� - {W%4$s{G!",  pch, questman, victim, pRoom->name );
    tell_fmt( "��� ��������� � ������ ��� ��������� {W%3$s{G.",  pch, questman, pRoom->area->name );
    tell_fmt( "� ���� ���� {Y%3$d{G ����%3$I��|��|� �� ���������� �������.",  pch, questman, time );
}

void KillQuest::destroy( ) 
{
    clearMobile<VictimBehavior>( );
}

bool KillQuest::isComplete( ) 
{
    return (state == QSTAT_FINISHED);
}

Quest::Reward::Pointer KillQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    Reward::Pointer r( NEW );
    
    if (hint.getValue( ) > 0) {
	r->gold = number_range( 1, 2 );
	r->points = number_range( 1, 4 );
	r->prac = 0;
	
    } else {
	switch (mode.getValue( )) {
	case 0:
		r->gold = number_range( 5, 8 );
		r->points = number_range( 5, 8 );
		r->wordChance = 10;
		r->scrollChance = 7;
		break;
	case 1: 
		r->gold = number_range( 8, 12 );
		r->points = number_range( 8, 12 );
		r->wordChance = 15;
		r->scrollChance = 10;
		if ( chance(5) )
		    r->prac = number_range(1, 2);
		break;
	case 2:
		r->gold = number_range( 12, 16 );
		r->points = number_range( 12, 16 );
		r->wordChance = 25;
		r->scrollChance = 12;
		if ( chance(7) )
		    r->prac = number_range(1, 3);
		break;
	default:
		r->gold = number_range( 16, 24 );
		r->points = number_range( 16, 24 );
		r->wordChance = 30;
		r->scrollChance = 15;
		if ( chance(10) )
		    r->prac = number_range(1, 4);
		break;
	};
    }

    if (ch->getClan( )->isDispersed( )) 
	r->points *= 2;
    else
	r->clanpoints = r->points;

    r->exp = (r->points + r->clanpoints) * 10;
    return Reward::Pointer( r );
}

void KillQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    if (isComplete( ))
	buf << "���� ������� {Y���������{x!" << endl
	    << "������� �� ���������������, �� ���� ��� ������ �����!" << endl;
    else 
	buf << "� ���� ������� - ���������� " << russian_case( mobName, '4' ) << "!" << endl
	    << "�����, ��� ������ ������ � ��������� ��� - " << roomName << endl
	    << "��� ��������� � ������ ��� ��������� " << areaName << "." << endl;
}

void KillQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    if (isComplete( ))
	buf << "��������� � �������� �� ��������.";
    else 
        buf << "���������� " << russian_case( mobName, '4' ) << " �� "
            << roomName << " (" << areaName << ").";
}

Room * KillQuest::helpLocation( )
{
    return findMobileRoom<VictimBehavior>( );
}

bool KillQuest::checkMobileVictim( PCharacter *pch, NPCharacter *mob )
{
    int level_diff; 
    
    if (!VictimQuestModel::checkMobileVictim( pch, mob ))
	return false;

    level_diff = mob->getRealLevel( ) - pch->getModifyLevel( );
     
    if (( mode == 0 && ( level_diff < -3 || level_diff > 0  ) )
	|| ( mode == 1 && ( level_diff <  0 || level_diff > 5  ) )
	|| ( mode == 2 && ( level_diff <  5 || level_diff > 10 ) )
	|| ( mode == 3 && ( ( level_diff < 10 || level_diff > 15 ) 
			     || mob->getProfession( )->getFlags( mob ).isSet(PROF_CASTER)) ))
	return false;

    return true;
}

