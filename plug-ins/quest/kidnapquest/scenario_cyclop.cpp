/* $Id: scenario_cyclop.cpp,v 1.1.2.5.6.2 2007/09/29 19:34:03 rufina Exp $
 *
 * ruffina, 2004
 */
#include "scenario_cyclop.h"
#include "kidnapquest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "act.h"
#include "interp.h"
#include "mercdb.h"
#include "handler.h"
#include "merc.h"
#include "def.h"

#define KS KidnapCyclopScenario

/*
 * Scenario by Ragnar
 */

/*------------------------------------------------------------------------------
 * Cyclop scenario, for evil cruel bastards 
 *----------------------------------------------------------------------------*/
bool KS::applicable( PCharacter *hero )
{
    return IS_EVIL(hero);
}

void KS::onQuestStart( PCharacter *hero, NPCharacter *questman, NPCharacter *king )
{
    tell_raw( hero, questman, 
	      "{W%s{G �����-�� ������������ ���� ������.",
                   king->getNameP( '3' ).c_str() );
    tell_raw( hero, questman, 
	     "��� %s � ��������� ��� ��������� {W%s{G ({W%s{G).",
                   GET_SEX(king, "���", "���", "��"), king->in_room->name, king->in_room->area->name );
}

/*
 * hero messages
 */
void KS::msgRemoteReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) 
{
    interpret_raw( king, "grin" );
    act("$c1 ����� �� $C4 ����������: '{g�, ��� � ���� ��������{x'.", king, 0, kid, TO_ROOM);
    act("$c1 ������� ���� �� ����.", king, 0, 0, TO_ROOM);
    hero->printf( "%s � %s ��� �����������.\r\n", king->getNameP( '1' ).c_str( ), kid->getNameP( '1' ).c_str( ) );
    act("�����, �������� $C4.", hero, 0, king, TO_CHAR);
}
void KS::msgKingDeath( NPCharacter *king, Character *killer, PCharacter *hero ) 
{
    if(hero == killer) {
	act("{Y�������, ���� ������ �� �������$g��|�|�� ��������� ��������, �� ���� ��������... �� ��������.{x", killer, 0, 0, TO_CHAR);
	act("{Y������� ����������.{x", killer, 0, 0, TO_CHAR);
    } else {
	act("{Y$c1 ������� ���� ������������ ������, ����� � ����� $c3.{x", killer, 0, hero, TO_VICT);
	act("{Y��� ���� �� ��� ��������, ���: ������� ����������.{x", killer, 0, hero, TO_VICT);
    }
}
void KS::msgKidDeath( NPCharacter *kid, Character *killer, PCharacter *hero ) 
{
    if(hero == killer) 
	act("{Y��� ������� - ��� �����.{x\r\n{Y������� ����������.{x", killer, 0, 0, TO_CHAR);
    else 
	act("{Y$c1 � ������ �������� ��������$g��|�|�� ���� ��� �����.{x\r\n{Y������� ����������.{x", killer, 0, hero, TO_VICT);
}

/*
 * bandit actions
 */
void KS::actAttackHero( NPCharacter *bandit, PCharacter *hero ) 
{
    if (hero->fighting) 
	return;

    if (chance( 10 )) {
	act("$c1 ������� ��-�� ����� �������� ��������� ������.", bandit, 0, 0, TO_ROOM);
	act("������������� �������� ������� $c2 {R<*) (*>= ! ���������� � �������� ������ ! =<*) (*>{x ���� ����", bandit, 0, hero, TO_VICT);
	act("�� � {R������� ���������.{x", bandit, 0, hero, TO_VICT);
	act("������������� �������� ������� $c2 {R<*) (*>= ! ���������� � �������� ������ ! =<*) (*>{x ���� $C2", bandit, 0, hero, TO_NOTVICT);
	act("$C1 � {R������� ���������.{x", bandit, 0, hero, TO_NOTVICT);
    }
    else {
	act("$c1 ���������� '{g���������� ���$G��|��|��, ���� �� ��� �� ��������$G���|��|��� � ����� ��������?{x'.", bandit, 0, hero, TO_ROOM);
	act("$c1 ���������� '{g������, ��� ���������� ���, ��� � ��$G��|�|�� ���������?{x'.", bandit, 0, hero, TO_ROOM);
	act("$c1 ���������� '{g������ ����� ������-������ �������� ������$G��|�|��? �� ��� �� ������...{x'.", bandit, 0, hero, TO_ROOM);
	act("$c1 ������ �������.", bandit, 0, hero, TO_ROOM);
    }
}
void KS::actBeginKidnap( NPCharacter *bandit, NPCharacter *kid ) 
{
    act("$c1 ����� �� ���� $C4 � ������ $S �����.", bandit, 0, kid, TO_ROOM);
    act("$c1 ������� $C3: '{g������, ����������� ���, � ������ ����� �� �������.{x", bandit, 0, kid, TO_ROOM);
}
void KS::actHuntStep( NPCharacter *bandit ) 
{
    if(number_percent() < 10)
	act("$c1 ���������� ������������� �� ��������.", bandit, 0, 0, TO_ROOM);
}
void KS::actKidnapStep( NPCharacter *bandit, NPCharacter *kid ) 
{
    if(number_percent() < 10)
	act("$C1, ����� $c4 �� �����, ����� ����-������� �� �����.", bandit, 0, kid, TO_ROOM);
}
void KS::actEmptyPath( NPCharacter *bandit, NPCharacter *kid ) 
{
    if(number_percent() < 10) {
	act("$c1 ������ ��������.", bandit, 0, 0, TO_ROOM);
	act("$c1 ����������: '{g��� � ���, ������,... �� ����{x'", bandit, 0, 0, TO_ROOM);
	
	if (kid->in_room == bandit->in_room)
	    act("$C1 ����� ������������.", bandit, 0, kid, TO_ROOM);
    }
}

/*
 * king actions
 */
void KS::actLegend( NPCharacter *king, PCharacter *hero, KidnapQuest::Pointer quest ) 
{
    act("$c1 ���������� ������� �� ����.", king, 0, hero, TO_VICT);
    act("$c1 ���������� ������� �� $C4.", king, 0, hero, TO_NOTVICT);
    interpret_raw(king, "sigh");
    act("$c1 ������� ���� '{G��������$g��|�|�� �, ��� � $t ���� ���� �������.{x", king, quest->princeArea.getValue( ).c_str( ), hero, TO_VICT);
    act("$c1 ������� ���� '{G������������� ������������...{x'", king, 0, hero, TO_VICT);
}
void KS::actGiveMark( NPCharacter *king, PCharacter *hero, Object * mark, int time ) 
{
    char buf[MAX_STRING_LENGTH];

    if(number_percent() < 50) {
	act("$c1 ������� ���� '{G� �� ��$g��|�|�� �������$g��|�|�� ���, �� ����� ������� ����.{x'", king, 0, hero, TO_VICT);
	act("$c1 ������� ���� '{G���, ������ ��� ���������, ����� ��� �������� ���������.{x'", king, 0, hero, TO_VICT);
	act("$c1 ���� ���� $o4.", king, mark, hero, TO_VICT);
	act("$c1 ������� $C3 $o4.", king, mark, hero, TO_NOTVICT);
	act("$c1 ������� ���� '{G������� ��� ����.{x'", king, 0, hero, TO_VICT);
	sprintf( buf, "$c1 ������� ���� '{G� �������! ��������, ��� ����� {Y%d{G �����%s �� ��� ��� �� �����������.{x",
		 time, GET_COUNT(time, "�", "�", "") );
    } else {
	act("$c1 ������� ���� '{G� �� � ��$g��|�|�� �������$g��|�|�� ���, �� ��� ������, ���� ������ �����������, � ������� ���.{x'", king, 0, hero, TO_VICT);
	act("$c1 ������� ���� '{G���, ������ ��� ���������, ����� ��� �������� ���������.{x'", king, 0, hero, TO_VICT);
	act("$c1 ���� ���� $o4.", king, mark, hero, TO_VICT);
	act("$c1 ������� $C3 $o4.", king, mark, hero, TO_NOTVICT);
	sprintf( buf, "$c1 ������� ���� '{G�������! ����� {Y%d{G �����%s �� ������ ���� �����!{x",
		 time, GET_COUNT(time, "�", "�", "") );
    }

    act(buf, king, 0, hero, TO_VICT);
}
void KS::actMarkLost( NPCharacter *king, PCharacter *hero, Object * mark ) 
{
    act("$c1 ������� ���� '{G������� ������������? �����, ������ �������� �� ����.{x'", king, 0, hero, TO_VICT);
    act("$c1 ���� ���� $o4.", king, mark, hero, TO_VICT);
    act("$c1 ���� $C3 $o4.", king, mark, hero, TO_NOTVICT);
}
void KS::actAckWaitComplete( NPCharacter *king, PCharacter *hero ) 
{
    act("$c1 ������� ���� '{G�� ���� ���� �����$G��|�|��, ������$G��|��|��.{x'.", king, 0, hero, TO_VICT);
    act("$c1 ��������� ������� �� ����.", king, 0, hero, TO_VICT);
    act("$c1 ��������� ������� �� $C4.", king, 0, hero, TO_NOTVICT);
}

/*
 * kid actions
 */
void KS::actHeroWait( NPCharacter *kid ) 
{
    if(number_percent( ) < 10)
	act("$c1 ������ �������, �� �������, ���� �� �� ���������.", kid, 0, 0, TO_ROOM);
}
void KS::actNoHero( NPCharacter *kid, PCharacter *hero ) 
{
    if (number_percent( ) < 10 && hero && hero->in_room != kid->in_room)
	act("$c1 ���-��� �����������, ������� $C4.", kid, 0, hero, TO_ROOM);
}
void KS::actHeroDetach( NPCharacter *kid, PCharacter *hero ) 
{
    interpret_raw( kid, "yell", "������������. � � ���� �������!!!!" );
}
void KS::actWrongGiver( NPCharacter *kid, Character *victim, Object * ) 
{
    act("$c1 ������� ��� �������������� �� ����", kid, 0, victim, TO_VICT);
    act("$c1 ������� ��� �������������� �� $C4", kid, 0, victim, TO_NOTVICT);
}
void KS::actWrongMark( NPCharacter *kid, Character *victim, Object * ) 
{
    act("$c1 ������� ���� '{G��� �� ��� �������.{x'", kid, 0, victim, TO_VICT);
}
void KS::actGoodMark( NPCharacter *kid, Character *victim, Object *obj ) 
{
    interpret_raw(kid, "flip");
    act("$c1, ����� �������, ��������� ������� ���� � �����.", kid, 0, victim, TO_VICT);
    act("$c1, ����� �������, ��������� ������� � ����� $C3.", kid, 0, victim, TO_NOTVICT);
}
void KS::actReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) 
{
    interpret(king, "grin");
    act("$c1 ����� �� $C4 ����������: '{g�, ��� � ���� ��������{x'.", king, 0, kid, TO_ROOM);
    act("$c1 ������� ���� �� ����.", king, 0, 0, TO_ROOM);
    actAckWaitComplete(king, hero);
}
void KS::actBanditsUnleash( NPCharacter *kid, PCharacter *hero, NPCharacter *bandit ) 
{
    act("{Y���� ������������� ��������� ����������� ��������.{x", hero, 0, 0, TO_CHAR);
}
