/* $Id: mobiles.cpp,v 1.1.2.12.6.6 2010/01/01 15:48:15 rufina Exp $
 *
 * ruffina, 2004
 */

#include "mobiles.h"
#include "objects.h"
#include "stealquest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "act.h"
#include "interp.h"
#include "handler.h"
#include "mercdb.h"
#include "def.h"

/* 
 * RobbedVictim 
 */
bool RobbedVictim::givenCheck( PCharacter *hero, Object *obj )
{
    return getQuest( ) && quest->check<RobbedItem>( obj );
}

void RobbedVictim::givenGood( PCharacter *hero, Object *obj )
{
    quest->state = QSTAT_FINISHED;

    act( "$c1 ���������� '{g������� ����, $C1!{x'", ch, 0, hero, TO_ROOM );
    say_act( hero, ch, "������� �� ��������������� � ����, ��� ��������� ���� � ���� ���������." );
    
    if (quest->itemWear != wear_none) {
	quest->itemWear->wear( obj, F_WEAR_VERBOSE );
	if (obj->wear_loc != wear_none)
	    interpret( ch, "smile" );
	else /* not enough experience to use the item */
	    interpret( ch, "emote �������� ���������." );
    }
}

void RobbedVictim::givenBad( PCharacter *hero, Object *obj )
{
    say_act( hero, ch, "�� � ����� ��� ���?" );
    act( "$c1 � ����������� ����� ����������� ���� $o4.", ch, obj, hero, TO_VICT );
    act( "$c1 � ����������� ����� ����������� $C3 $o4.", ch, obj, hero, TO_NOTVICT );
}

void RobbedVictim::deadFromIdiot( PCMemoryInterface *pcm )
{
    act("{Y����$g�|�|���! �� ���$g��|�|�� ����, ��� �������� � ����� ������.{x", pcm->getPlayer( ), 0, 0, TO_CHAR);
}

void RobbedVictim::deadFromSuicide( PCMemoryInterface *pcm )
{
    if (pcm->isOnline( )) 
	act_p("{Y$c1 �������� �������$g���|��|���. ������� ����������.{x", ch, 0, pcm->getPlayer( ), TO_VICT, POS_DEAD);
}

void RobbedVictim::deadFromKill( PCMemoryInterface *pcm, Character *killer )
{
    if (pcm->isOnline( )) 
	act_p("{Y$c1 ����� ���$g��|�|�� ����, ��� �������� � ����� ������.{x", killer, 0, pcm->getPlayer( ), TO_VICT, POS_DEAD);
}

void RobbedVictim::show( Character *victim, std::basic_ostringstream<char> &buf ) 
{
    if (ourHero( victim ) && getQuest( ) && !quest->isComplete( ))
	buf << "{x({Y�������{x) ";
}

void RobbedVictim::talkToHero( PCharacter *hero )
{
    if (!getQuest( ))
	return;
    
    if (ch->position == POS_SLEEPING)
	interpret_raw( ch, "wake" );

    switch (quest->state.getValue( )) {
    case QSTAT_INIT:
	    quest->state = QSTAT_HUNT_ROBBER;
	    quest->setTime( hero, number_range( 20, 30 ) );
	    quest->wiznet( "", "%s tells story", ch->getNameP( '1' ).c_str( ) );
	    
	    tell_fmt( "{1{W%3$N1{2, ����%4$g��|��|�� ���%4$g��|���|����, "
		      "����%4$g��|�|�� � ���� {1{W%5$N4{2.",
		      hero, ch,
		      quest->thiefName.c_str( ),
		      quest->thiefSex.getValue( ),
		      quest->itemName.c_str( ) );
	    tell_fmt( "�� ��� ������� ���-��� �������� � ��������%3$g�|�|�����.",
		      hero, ch,
		      quest->thiefSex.getValue( ) );
	    tell_fmt( "%3$^p1 ����� �� {1{W%4$s{2, � ���� ����� ��������� � ������ {1{W%5$s{2.",
		      hero, ch,
		      quest->thiefSex.getValue( ),
		      quest->thiefArea.c_str( ), 
		      quest->thiefRoom.c_str( ) );

	    if (!quest->chestRoom.empty( )) {
		tell_fmt( "� ������������ �����, �� ������, ������ ����� {1{W%3$s{2, ������ ������� �� ����..",
		          hero, ch,
			  quest->chestRoom.c_str( ) );
		tell_fmt( "������ �����, ������ ���� %3$p1 ��������%3$g��|�|�� ��� �������, � ������ �� ���������� � ������.",
		          hero, ch,
			  quest->thiefSex.getValue( ) );
	    }

	    tell_raw( hero, ch, "����� ��� ����������! ��� � �����������." );

	break;
    }
}


/* 
 * Robber 
 */
void Robber::show( Character *victim, std::basic_ostringstream<char> &buf ) 
{
    if (ourHero( victim )) 
	buf << "{R[���] {x";
}

