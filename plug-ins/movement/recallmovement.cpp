/* $Id$
 *
 * ruffina, 2004
 */
#include "recallmovement.h"
#include "move_utils.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "affectflags.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

RecallMovement::RecallMovement( Character *ch )
                 : JumpMovement( ch )
{
}

RecallMovement::RecallMovement( Character *ch, Character *actor, Room *to_room )
                 : JumpMovement( ch, actor, to_room )
{
}

bool RecallMovement::moveAtomic( )
{
    if (ch == actor)
	msgOnStart( );

    return JumpMovement::moveAtomic( );
}

void RecallMovement::msgOnStart( )
{
}

bool RecallMovement::applyFightingSkill( Character *wch, SkillReference &skill )
{
    int chance;
    
    if (!wch->fighting)
	return true;

    if (wch != ch) {
	msgSelf( ch, "�� %2$C1 ���������!" );
	return false;
    }

    wch->pecho( "�� ����%1$G��|��|�� ���������!", wch );
    chance = skill->getEffective( wch );
    chance = 80 * chance / 100;
    
    if (number_percent( ) < chance) { /* historical bug here */
	skill->improve( wch, false );
	wch->setWaitViolence( 1 );
	wch->pecho( "���� ������� ����������� ��������!" );
	return false;
    }

    skill->improve( wch, false );
    msgSelfParty( wch, "�� �������� � ���� �����!", "%2$^C1 ������� � ���� �����!" );
    return true;
}

bool RecallMovement::checkMount( )
{
    if (actor->is_npc( ) && actor == ch && actor->mount) {
	msgSelfMaster( actor, "�� �� ������� ����� �������.", "%3$^C1 �� ������ ����� �������." );
	return false;
    }

    if (!horse)
	return true;

    if (!canOrderHorse( )) {
	ch->pecho( "���� ���������� ������� ���������." );
	return false;
    }

    return true;
}

bool RecallMovement::checkShadow( )
{
    if (!ch->is_npc( ) && ch->getPC( )->shadow != -1) {
	ch->pecho( "���� ������� � ����������� ����� � ������� ����� {D����{x." );
	return false;
    }

    return true;
}

bool RecallMovement::checkBloody( Character *wch )
{
    if (IS_BLOODY(wch)) {
	msgSelfParty( wch, 
		      "����� ��� ���� �� ����, ������%1$G��|��|��.",
		      "����� ��� ���� �� %1$C2." );
	return false;
    }

    return true;
}

bool RecallMovement::checkForsaken( Character *wch )
{
    if (!checkNorecall( wch ) || !checkCurse( wch )) {
	msgSelfParty( wch, 
		      "���� �������� ����.",
		      "���� �������� %1$C4." );
	return false;
    }

    return true;
}

bool RecallMovement::checkNorecall( Character *wch )
{
    if (IS_SET(from_room->room_flags, ROOM_NO_RECALL))
	return false;

    if (wch->death_ground_delay > 0
	&& wch->trap.isSet( TF_NO_RECALL ))
	return false;

    return true;
}

bool RecallMovement::checkCurse( Character *wch )
{
    if (IS_AFFECTED(wch, AFF_CURSE))
	return false;

    if (IS_RAFFECTED(from_room, AFF_ROOM_CURSE))
	return false;

    return true;
}

bool RecallMovement::applyInvis( Character *wch )
{
    if (IS_AFFECTED( wch, AFF_HIDE|AFF_FADE ))
	REMOVE_BIT(wch->affected_by, AFF_HIDE|AFF_FADE);

    strip_camouflage( wch );
    return true;
}

bool RecallMovement::applyMovepoints( )
{
    ch->move /= 2;
    return true;
}

bool RecallMovement::checkSameRoom( )
{
    return from_room != to_room;
}

void RecallMovement::moveFollowers( Character *wch ) 
{
    NPCharacter *pet;
    
    if (!wch || wch->is_npc( ))
	return;
    
    if (!( pet = wch->getPC( )->pet ))
	return;

    if (pet->in_room == to_room)
	return;

    movePet( pet );	
}

bool RecallMovement::checkPumped( )
{
    if (ch->getLastFightDelay( ) < FIGHT_DELAY_TIME) {
	ch->pecho( "���� ����� ������ ������� �����, �� �� ������ ��������������� �� �������." );
	return false;
    }

    return true;
}

bool RecallMovement::applyWaitstate( )
{
    ch->setWaitViolence( 1 );
    return true;
}

