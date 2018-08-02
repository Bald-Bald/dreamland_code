/* $Id: scenario_urchin.cpp,v 1.1.2.16.6.3 2009/01/18 20:11:58 rufina Exp $
 *
 * ruffina, 2004
 */

#include "scenario_urchin.h"
#include "kidnapquest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "act.h"
#include "clanreference.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#define KS KidnapUrchinScenario

CLAN(battlerager);

bool KS::applicable( PCharacter *hero )
{
    return (hero->getClan( ) != clan_battlerager);
}

/*
 * hero messages
 */
void KS::msgRemoteReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) 
{
    act("$c1 ����� ������� �� $C4.", kid, 0, king, TO_ROOM);
    hero->printf( "%s � %s ��� �����������.\r\n", king->getNameP( '1' ).c_str( ), kid->getNameP( '1' ).c_str( ) );
    act("����� � $C3 �� ��������������!", hero, 0, king, TO_CHAR);
}
void KS::msgKingDeath( NPCharacter *king, Character *killer, PCharacter *hero ) 
{
    if(hero == killer) {
	act("{Y����$g�|�|���.... �� ���$g��|�|�� ����, ��� �������� � ����� ������.{x", killer, 0, 0, TO_CHAR);
	hero->send_to("{Y������� ����������.{x\r\n");
    } else {
	act("{Y$c1 ����� ���� ����, ��� �������� � ����� ������.{x", killer, 0, hero, TO_VICT);
	hero->send_to("{Y������� ����������.{x\r\n");
    }
}
void KS::msgKidDeath( NPCharacter *kid, Character *killer, PCharacter *hero ) 
{
    if(hero == killer) {
	act("{Y����$g�|�|���.... �� ���$g��|�|�� ����, ���� ����$g��|��|�� ��$g��|�|�� ������.{x", killer, 0, 0, TO_CHAR);
	hero->send_to("{Y������� ����������.{x\r\n");
    } else {
	act("{Y$c1 ����� ���$g��|�|�� ����, ���� ���� ���� �������� ������.{x", killer, 0, hero, TO_VICT);
	hero->send_to("{Y������� ����������.{x\r\n");
    }
}

/*
 * bandit actions
 */
void KS::actAttackHero( NPCharacter *bandit, PCharacter *hero ) 
{
    if (!hero->fighting) {
	act("$c1 ���������� '{g� ���� ���� �������!{x'.", bandit, 0, hero, TO_ROOM);
    }
}
void KS::actBeginKidnap( NPCharacter *bandit, NPCharacter *kid ) 
{
    act("$c1 ������� �� ����� �����, ����� ��� ����� ������.", bandit, 0, kid, TO_ROOM);
    act("$c1 ���������� '{g��� ������ ���� ����������{x'.", bandit, 0, kid, TO_ROOM);
    act("$c1 ������� $C4 �� ������� � ����� �� �����.", bandit, 0, kid, TO_ROOM);
}
void KS::actHuntStep( NPCharacter *bandit ) 
{
    if(number_percent() < 10)
	act("$c1 �� ���� � ������� ������ ����� ���.", bandit, 0, 0, TO_ROOM);
}
void KS::actKidnapStep( NPCharacter *bandit, NPCharacter *kid ) 
{
    if(number_percent() < 10)
	act("$c1 ����� �������� �� �������.", bandit, 0, 0, TO_ROOM);
}
void KS::actEmptyPath( NPCharacter *bandit, NPCharacter *kid ) 
{
    if(number_percent() < 10)
	act("$c1 ��������� ������� �� �����.", bandit, 0, 0, TO_ROOM);
}

/*
 * king actions
 */
void KS::actLegend( NPCharacter *king, PCharacter *hero, KidnapQuest::Pointer quest ) 
{
    act("$c1 ������� ���� '{G��� �������� ��� ������ �� ����, �������� �� ������� � ���� ������.{x'", king, 0, hero, TO_VICT);
    act("$c1 ������� ���� '{G�� ���� ������, ���� � ���� ��� ����� ���� ������.{x'", king, 0, hero, TO_VICT);
    act("$c1 ������� ���� '{G����� ���, ���� �� �� ����� � ����.{x'", king, 0, hero, TO_VICT);
    act("$c1 ������� ���� '{G������ ����� �� ���������� ���-�� � ������ $t{x'", king, quest->princeArea.getValue( ).c_str( ), hero, TO_VICT);
}
void KS::actGiveMark( NPCharacter *king, PCharacter *hero, Object * mark, int time ) 
{
    char buf[MAX_STRING_LENGTH];
    

    act("$c1 ������� ���� '{G� ������� ���, ��� � ���� ����, ����� ������ ���� ����� �����...{x'", king, 0, hero, TO_VICT);
    act("$c1 ������� ���� $o4.", king, mark, hero, TO_VICT);
    act("$c1 ������� $C3 $o4.", king, mark, hero, TO_NOTVICT);
    act("$c1 ������� ���� '{G������� ��� ��� � ��������!{x'", king, 0, hero, TO_VICT);
    sprintf( buf, "$c1 ������� ���� '{G����������� ������ ������������ ���, "
		  "���, ���� �� �� ��������� ��� �� ��� ����� {Y%d{G �����%s, "
		  "� ��� �������� ���-�� ������������.{x'",
	     time, GET_COUNT(time, "�", "�", "") );

    act(buf, king, 0, hero, TO_VICT);
}
void KS::actMarkLost( NPCharacter *king, PCharacter *hero, Object * mark ) 
{
    act("$c1 ������� ���� '{G��� �� ������$G��|�|��?!{x'", king, 0, hero, TO_VICT);
    act("$c1 ������� ���� '{G� �������, ������ ����� ��� �� � ���� �����.{x'", king, 0, hero, TO_VICT);
    act("$c1 ���� ���� ����� $o4.", king, mark, hero, TO_VICT);
    act("$c1 ���� $C3 ����� $o4.", king, mark, hero, TO_NOTVICT);
}
void KS::actAckWaitComplete( NPCharacter *king, PCharacter *hero ) 
{
    act("$c1 ������� ���� � ��� ����.", king, 0, hero, TO_VICT);
    act("$c1 ������� $C4 � ��� ����.", king, 0, hero, TO_NOTVICT);
    act("$c1 ������� ����: '{G��� ������ �� �������� � ����, ��� ��� ���� �������!{x'.", king, 0, hero, TO_VICT);
}

/*
 * kid actions
 */
void KS::actHeroWait( NPCharacter *kid ) 
{
    if(number_percent( ) < 10)
	act("$c1 ����� ������������ �������� � ���� ����� ��������.", kid, 0, 0, TO_ROOM);
}
void KS::actNoHero( NPCharacter *kid, PCharacter *hero ) 
{
    if (number_percent( ) < 10 && hero && hero->in_room != kid->in_room)
	act("$c1 ��������� � ������� $C2.", kid, 0, hero, TO_ROOM);
}
void KS::actHeroDetach( NPCharacter *kid, PCharacter *hero ) 
{
    if (hero)
	interpret_fmt( kid, "yell ��, %s, �� ���?!!!", hero->getNameP( ) );
}
void KS::actWrongGiver( NPCharacter *kid, Character *victim, Object *obj ) 
{
    act("$c1 ���� ������� �������� ������� �� $o4.", kid, obj, 0, TO_ROOM);
}
void KS::actWrongMark( NPCharacter *kid, Character *victim, Object *obj ) 
{
    act("$c1 ���� ������� �������� ������� �� $o4.", kid, obj, 0, TO_ROOM);
}
void KS::actGoodMark( NPCharacter *kid, Character *victim, Object *obj ) 
{
    interpret(kid, "grin");
    act("$c1 ���������� '{g������ ��� �� ���� �� ���������!{x'", kid, 0, 0, TO_ROOM);
    act("$c1 ���������� '{g����� �����?{x'", kid, 0, 0, TO_ROOM);
}
void KS::actReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) 
{
    act("$c1 ����� ������� �� $C4.", kid, 0, king, TO_ROOM);
    actAckWaitComplete(king, hero);
}
void KS::actBanditsUnleash( NPCharacter *kid, PCharacter *hero, NPCharacter *bandit ) 
{
    act("{Y������ ����� � �������� ������ ���������� � ���� ����������� ���� ����.{x", kid, 0, 0, TO_ROOM);
}
