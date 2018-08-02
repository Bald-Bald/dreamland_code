
/* $Id: group_movement.cpp,v 1.1.2.11 2009/03/16 20:24:06 rufina Exp $
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

#include "spelltemplate.h"
#include "skillcommandtemplate.h"
#include "recallmovement.h"
#include "fleemovement.h"

#include "selfrate.h"

#include "so.h"

#include "pcharacter.h"
#include "mobilebehavior.h"
#include "room.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "playerattributes.h"
#include "object.h"
#include "affect.h"

#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "act_move.h"
#include "interp.h"
#include "clanreference.h"
#include "gsn_plugin.h"

#include "stats_apply.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "effects.h"
#include "act.h"
#include "vnum.h"
#include "def.h"

CLAN(battlerager);




SPELL_DECL(Knock);
VOID_SPELL(Knock)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
	char arg[MAX_INPUT_LENGTH];
	int chance=0;
	int door;

	target_name = one_argument( target_name, arg );

	if (arg[0] == '\0')
	{
		ch->send_to("��������� � ����� ����� ��� � ����� �����������.\n\r");
		return;
	}

	if (ch->fighting)
	{	
		ch->send_to("������� ���� ���������� ��������.\n\r");
		return;
	}
	
	if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY) ) >= 0) 
	{
		Room *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev = 0;

		pexit = ch->in_room->exit[door];
		if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
		{
			ch->send_to("����� ��� �������.\n\r");
			return;
		}
		if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
		{
			ch->send_to("�������� ������ �������...\n\r");
			return;
		}
		if ( IS_SET(pexit->exit_info, EX_NOPASS) )
		{
			ch->send_to("������������ ���� ��������� ������.\n\r");
			return;
		}
		chance = ch->getModifyLevel() / 5 + ch->getCurrStat(STAT_INT) + ch->getSkill( sn ) / 5;

		act_p("������ ���������� ���� �� ��������� ������� $d!",
			ch,0,pexit->keyword,TO_CHAR,POS_RESTING);

		act_p("������ ���������� ���� $c1 �������� ������� $d!",
			ch,0,pexit->keyword,TO_ROOM,POS_RESTING);

		if (ch->in_room->isDark())
			chance /= 2;

		// now the attack
		if (number_percent() < chance )
		{
			REMOVE_BIT(pexit->exit_info, EX_LOCKED);
			REMOVE_BIT(pexit->exit_info, EX_CLOSED);
			act_p( "$c1 � �������� ����������� $d!",
				ch, 0,pexit->keyword, TO_ROOM,POS_RESTING);
			act_p( "$d � �������� �������������.",
				ch, 0, pexit->keyword, TO_CHAR,POS_RESTING);

			// open the other side
			if ( ( to_room   = pexit->u1.to_room            ) != 0
				&& ( pexit_rev = to_room->exit[dirs[door].rev] ) != 0
				&& pexit_rev->u1.to_room == ch->in_room )
			{
				Character *rch;

				REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
				REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
				for ( rch = to_room->people; rch != 0; rch = rch->next_in_room )
					act_p( "�������� � �������� ������������� $d.",
						rch, 0, pexit_rev->keyword, TO_CHAR,POS_RESTING);
			}
		}
		else
		{
			act_p("���� ���� ��������� ��� ������, �� $d �������� ��������.",
				ch,0,pexit->keyword,TO_CHAR,POS_RESTING);
			act_p("���� $c2 ��������� ��� ������, �� $d �������� ��������.",
				ch,0,pexit->keyword,TO_ROOM,POS_RESTING);
		}
		return;
	}

	ch->send_to("��� ��� ����� �����.\n\r");
	return;

}


/*
 * 'sneak' skill command
 */

SKILL_RUNP( sneak )
{
    Affect af;

    if (MOUNTED(ch))
    {
        ch->send_to("�� �� ������ ��������� ��������, ����� �� � �����.\n\r");
        return;
    }

//    ch->send_to("�� ��������� ��������� ����� ��������.\n\r");
    affect_strip( ch, gsn_sneak );

    if( IS_AFFECTED(ch,AFF_SNEAK)) {
      ch->send_to("�� � ��� ���������� ��������.\n\r");
      return;
    }

    if ( number_percent( ) < gsn_sneak->getEffective( ch ))
    {
	gsn_sneak->improve( ch, true );
	af.where     = TO_AFFECTS;
	af.type      = gsn_sneak;
	af.level     = ch->getModifyLevel();
	af.duration  = ch->getModifyLevel();
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
	ch->send_to("�� ��������� ������� �������������.\n\r");
    } else {
      gsn_sneak->improve( ch, false );
      ch->send_to("���� �� ������� ������� �������������.\n\r");
    }

    ch->setWait( gsn_sneak->getBeats( ) );
}

/*
 * 'hide' skill command
 */

SKILL_RUNP( hide )
{
	if ( MOUNTED(ch) )
	{
		ch->send_to("�� �� ������ ��������, ����� �� � �����.\n\r");
		return;
	}

	if ( RIDDEN(ch) )
	{
		ch->send_to("�� �� ������ ��������, ����� �� �������.\n\r");
		return;
	}

	if ( IS_AFFECTED( ch, AFF_FAERIE_FIRE ) )
	{
		ch->send_to("�� �� ������ ��������, ����� ���������.\n\r");
		return;
	}

	int forest = ch->in_room->sector_type == SECT_FOREST ? 60 : 0;
	forest += ch->in_room->sector_type == SECT_FIELD ? 60 : 0;

	ch->send_to("�� ��������� ��������.\n\r");

	int k = ch->getLastFightDelay( );

	if ( k >= 0 && k < FIGHT_DELAY_TIME )
		k = k * 100 /	FIGHT_DELAY_TIME;
	else
		k = 100;
		
	if ( number_percent( ) < (gsn_hide->getEffective( ch ) - forest) * k / 100 )
	{
		SET_BIT(ch->affected_by, AFF_HIDE);
		gsn_hide->improve( ch, true );
	}
	else
	{
		if ( IS_AFFECTED(ch, AFF_HIDE) )
			REMOVE_BIT(ch->affected_by, AFF_HIDE);
		gsn_hide->improve( ch, false );
	}

	ch->setWait( gsn_hide->getBeats( ) );
}


/*
 * TempleRecallMovement 
 */
class TempleRecallMovement : public RecallMovement {
public:
    TempleRecallMovement( Character *ch )
                   : RecallMovement( ch )
    {
    }
    TempleRecallMovement( Character *ch, Character *actor, Room *to_room )
                   : RecallMovement( ch, actor, to_room )
    {
    }

protected:
    virtual void msgOnMove( Character *wch, bool fLeaving )
    {
	if (fLeaving)
	    msgRoomNoParty( wch, 
		            "%1$^C1 ���������%1$G���|��|��� � �������.",
		            "%1$^C1 � %2$C1 ������������ � �������." );
	else
	    msgRoomNoParty( wch, 
	                    "%1$^C1 ������%1$G���|��|��� � �������.",
			    "%1$^C1 � %2$C1 ���������� � �������." );
    }
    virtual void msgOnStart( )
    {
	msgRoomNoParty( ch, 
	                "%1$^C1 ������ � �����������!",
			"%1$^C1 � %2$C1 ������ � �����������!" );
    }
    virtual void movePet( NPCharacter *pet )
    {
	TempleRecallMovement( pet, actor, to_room ).moveRecursive( );
    }
    virtual bool findTargetRoom( )
    {
	int point;
	
	if (to_room)
	    return true;

	if (!ch->getPC( )
	    && (!ch->leader || ch->leader->is_npc( ) || ch->leader->getPC( )->pet != ch))
	{
	    ch->pecho( "���� ������ ������������." );
	    return false;
	}

	if (ch->getPC( ))
	    point = ch->getPC( )->getHometown( )->getRecall( );
	else
	    point = ch->leader->getPC( )->getHometown( )->getRecall( );

	if (!( to_room = get_room_index( point ) )) {
	    ch->pecho( "�� ������������ ��������%1$G���|��|���.", ch );
	    return false;
	}
	
	return true;			     
    }
    bool checkSelfrate( )
    {
	if (ch->is_npc( ))
	    return true;

	if (!ch->desc)
	    return true;

	if (rated_as_expert( ch->getPC( ) ))
	    return checkPumped( );
	
	if (rated_as_guru( ch->getPC( ) )) {
	    ch->pecho( "�� ���� �� ����� ������ �����, �� ��� ��?" );
	    return false;
	}

	return true;
    }
    virtual bool canMove( Character *wch )
    {
	if (ch != actor)
	    return checkForsaken( wch );
	else
	    return checkMount( )
		   && checkShadow( )
		   && checkBloody( wch )
		   && checkSelfrate( )
		   && checkSameRoom( )
		   && checkForsaken( wch );
    }
    virtual bool tryMove( Character *wch )
    {
	if (ch != actor)
	    return applyInvis( wch );
	else
	    return applyInvis( wch )
		   && applyFightingSkill( wch, gsn_recall )
		   && applyMovepoints( );
    }
};

/*
 * 'recall' skill command
 */

SKILL_RUNP( recall )
{
    TempleRecallMovement( ch ).move( );
}

/*
 * EscapeMovement
 */
class EscapeMovement : public FleeMovement {
public:
    EscapeMovement( Character *ch, const char *arg )
               : FleeMovement( ch )
    {
	this->arg = arg;
    }

protected:
    virtual bool findTargetRoom( )
    {
	peexit = get_extra_exit( arg, from_room->extra_exit );
	door = find_exit( ch, arg, FEX_NO_EMPTY|FEX_NO_INVIS );

	if ((!peexit || !ch->can_see( peexit )) && door < 0) {
	    ch->pecho( "� ���� ��� �� ����������?" );
	    return false;
	}

	if (peexit) {
	    door = DIR_SOMEWHERE;
	    exit_info = peexit->exit_info;
	    to_room = peexit->u1.to_room;
	}
	else {
	    pexit = from_room->exit[door];
	    exit_info = pexit->exit_info;
	    to_room = pexit->u1.to_room;
	}

	return true;
    }
    virtual bool canMove( Character *wch )
    {
	if (!checkMovepoints( wch ))
	    return false;

	if (!canFlee( wch )) {
	    ch->pecho( "���-�� �� ���� ���� ������� � ���� �����������." );
	    return false;
	}
	else
	    return true;
    }
    virtual bool tryMove( Character *wch )
    {
	if (!FleeMovement::tryMove( wch ))
	    return false;

	if (wch != ch)
	    return true;
	
	return applySkill( gsn_escape );
    }
    virtual int getMoveCost( Character *wch )
    {
	return 1;
    }
    virtual bool checkCyclicRooms( Character *wch ) 
    {
	if (from_room == to_room) {
	    ch->pecho( "�� �� ������ ������� ����, �������� ������ �����." );
	    return false;
	}

	return true;
    }
    virtual bool checkPositionHorse( )
    {
	ch->pecho( "������� �����, � ����� ��� ������." );
	return false;
    }
    virtual bool checkPositionRider( )
    {
	ch->pecho( "�� ���� ������ ���-�� ����� � �� ���� �������." );
	return false;
    }
    virtual bool checkPositionWalkman( )
    {
	if (ch->fighting == 0) {
	    if (ch->position == POS_FIGHTING)
		ch->position = POS_STANDING;

	    ch->pecho( "�� ������ �� � ��� �� ��������." );
	    return false;
	}

	return true;
    }

    const char *arg;
};

/*
 * 'escape' skill command
 */

SKILL_RUNP( escape )
{
    char arg[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if (!gsn_escape->usable( ch )) {
	ch->println( "�������� flee. �����, ��� ���� ������?" );
	return;
    }

    if (arg[0] == '\0') {
	ch->println( "����� �����������." );
	return;
    }

    EscapeMovement( ch, arg ).move( );
}

