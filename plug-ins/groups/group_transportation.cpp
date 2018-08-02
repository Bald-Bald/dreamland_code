
/* $Id: group_transportation.cpp,v 1.1.2.17.6.14 2010-08-24 20:23:09 rufina Exp $
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
#include "transportspell.h"
#include "recallmovement.h"
#include "profflags.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "clanreference.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "vnum.h"
#include "def.h"

CLAN(none);
PROF(samurai);

#define OBJ_VNUM_PORTAL		     25
#define OBJ_VNUM_HOLY_PORTAL	     34 

/*
 * 'mental block' spell
 */
SPELL_DECL(MentalBlock);
VOID_SPELL(MentalBlock)::run( Character *ch, Character *victim, int sn, int level ) 
{
    Affect af;

    if (victim->isAffected( sn )) {
	victim->println( "�� � ��� ���������� ��� ������� ����������� ��������." );
	
	if (ch != victim)
	    act( "$C1 � ��� ��������� ��� ������� ����������� ��������.", ch, 0, victim, TO_CHAR );

	return;
    }
    
    af.type  = sn;
    af.where = TO_AFFECTS;
    af.level = level;
    af.duration = level / 2;
    affect_to_char( victim, &af ); 
    
    victim->println( "������ �� ������ ����������� ��� ������� ����������� �������� � �����." );

    if (ch != victim)
	act( "$C1 ����� ����������� ��� ������� ����������� ��������.", ch, 0, victim, TO_CHAR );
}

/*
 * 'fly' spell
 */
SPELL_DECL(Fly);
VOID_SPELL(Fly)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	Affect af;

	if (is_flying( victim ))
	{
		if (victim == ch)
			ch->send_to("�� ��� ���������� � �������.\n\r");
		else
			act_p("$C1 ��� ��������� � �������.",ch,0,victim,TO_CHAR,POS_RESTING);
		return;
	}

	if (can_fly( victim ))
	{
		if (victim == ch)
			ch->send_to("�� � ��� ������ ��������� � ������.\n\r");
		else
			act_p("$C1 ����� ��������� � ������ � ��� ����� ������.",ch,0,victim,TO_CHAR,POS_RESTING);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = level + 3;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = AFF_FLYING;
	affect_to_char( victim, &af );

	victim->send_to("���� ���� ���������� �� �����.\n\r");

	act_p( "$c1 ����������� � ������.", victim, 0, 0, TO_ROOM,POS_RESTING);

	return;

}

/*
 * 'pass door' spell
 */
SPELL_DECL(PassDoor);
VOID_SPELL(PassDoor)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	if (victim == ch)
	  ch->send_to("�� ��� ������ ��������� ������ ��������.\n\r");
	else
	  act_p("$C1 ��� ����� ��������� ������ ��������.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );

    act("$c1 ���������� ������������$g��|��|��.", victim, 0, 0, TO_ROOM);
    act("�� ����������� ������������$g��|��|��.", victim, 0, 0, TO_CHAR);
}

/*
 * 'nexus' spell
 */
SPELL_DECL(Nexus);
VOID_SPELL(Nexus)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Object *portal, *stone;
    Room *to_room = 0, *from_room;
    int vnum;

    if (ch->getTrueProfession( )->getFlags( ch ).isSet(PROF_DIVINE)) 
	vnum = OBJ_VNUM_HOLY_PORTAL;
    else
	vnum = OBJ_VNUM_PORTAL;

    from_room = ch->in_room;

    if (victim->getModifyLevel() >= level + 3
      || !GateMovement( ch, victim ).canMove( ch )
      || saves_spell(ch->getModifyLevel(), victim, DAM_OTHER, ch, DAMF_SPELL)
      || IS_BLOODY( ch ))
    {
        ch->send_to("���� ������� ����������� ��������.\n\r");
        return;
    }

    to_room = victim->in_room;
    stone = get_eq_char(ch,wear_hold);
    if (!ch->is_immortal() &&  (stone == 0 || stone->item_type != ITEM_WARP_STONE))
    {
        ch->send_to("��� ����� ���������� ��������� ������, ���������� ������������.\n\r");
        return;
    }

    if (stone != 0 && stone->item_type == ITEM_WARP_STONE)
    {
        ch->pecho( "���� ����������� �������, ����������� � %1$O6.\r\n"
                   "%1$^O1 ���� ���������� � ��������!", stone );
        extract_obj(stone);
    }

    /* portal one */
    portal = create_object(get_obj_index(vnum),0);
    portal->timer = 1 + level / 5;
    portal->value[3] = to_room->vnum;

    obj_to_room(portal,from_room);

    act_p("��� ������ ���������� $o1.",ch,portal,0,TO_ROOM,POS_RESTING);
    act_p("����� ����� ���������� $o1.",ch,portal,0,TO_CHAR,POS_RESTING);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
	return;

    /* portal two */
    portal = create_object(get_obj_index(vnum),0);
    portal->timer = 1 + level / 5;
    portal->value[3] = from_room->vnum;

    obj_to_room(portal,to_room);

    if (to_room->people != 0)
    {
	act_p("��� ������ ���������� $o1.",
               to_room->people,portal,0,TO_ROOM,POS_RESTING);
	act_p("��� ������ ���������� $o1.",
               to_room->people,portal,0,TO_CHAR,POS_RESTING);
    }

}

/*
 * 'portal' spell
 */
SPELL_DECL(Portal);
VOID_SPELL(Portal)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Object *portal, *stone;
    int vnum;
    
    if (ch->getTrueProfession( )->getFlags( ch ).isSet(PROF_DIVINE))
	vnum = OBJ_VNUM_HOLY_PORTAL;
    else
	vnum = OBJ_VNUM_PORTAL;

    if ( victim->getModifyLevel() >= level + 3
      || !GateMovement( ch, victim ).canMove( ch )
      || saves_spell(ch->getModifyLevel(), victim, DAM_OTHER, ch, DAMF_SPELL)
      || IS_BLOODY( ch ))
    {
        ch->send_to("���� ������� ����������� ��������.\n\r");
        return;
    }


    stone = get_eq_char(ch,wear_hold);
    if (!ch->is_immortal()
    &&  (stone == 0 || stone->item_type != ITEM_WARP_STONE))
    {
        ch->send_to("��� ����� ���������� ��������� ������, ���������� ������������.\n\r");
	return;
    }

    if (stone != 0 && stone->item_type == ITEM_WARP_STONE)
    {
        ch->pecho( "���� ����������� �������, ����������� � %1$O6.\r\n"
                   "%1$^O1 ���� ���������� � ��������!", stone );
     	extract_obj(stone);
    }

    portal = create_object(get_obj_index(vnum),0);
    portal->timer = 2 + level / 8;
    portal->value[3] = victim->in_room->vnum;

    obj_to_room(portal,ch->in_room);

    act_p("��� ������ ���������� $o1.",ch,portal,0,TO_ROOM,POS_RESTING);
    act_p("����� ����� ���������� $o1.",ch,portal,0,TO_CHAR,POS_RESTING);

    if (victim->in_room->people != 0)
    {
	act_p("�������� ���� ����������� ������.",
               victim->in_room->people,0,0,TO_ROOM,POS_RESTING);
	act_p("�������� ���� ����������� ������.",
               victim->in_room->people,0,0,TO_CHAR,POS_RESTING);
    }


}

/*
 * 'solar flight' spell
 */
SPELL_DECL_T(SolarFlight, GateSpell);
VOID_SPELL(SolarFlight)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    if (weather_info.sunlight != SUN_LIGHT) {
	ch->println( "��� ����� ���� ����� ��������� ����." );
	return;
    }
		
    GateSpell::run( ch, victim, sn, level );
}



/*
 * 'gaseous form' spell
 */
SPELL_DECL(GaseousForm);
TYPE_SPELL(bool, GaseousForm)::checkPosition( Character *ch ) const
{
    if (!DefaultSpell::checkPosition( ch ))
	return false;

    if (!ch->fighting) {
	ch->println( "�� ������ ������������ � ����� ������ � ���!" );
	return false;
    }

    if (ch->mount) {
	ch->pecho( "�� �� ������ ������������ � �����, ���� �� ������ ��� ������%G��|�|��!", ch );
	return false;
    }

    return true;
}
VOID_SPELL(GaseousForm)::run( Character *ch, Character *, int sn, int level ) 
{ 
    Room *target;

    if (ch->isAffected(sn)) {
	ch->println("��� ���������� �������������� ������ �������.");
	return;
    }
    
    if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)) {
	ch->println("���� ������� ����������� ��������.");
	return;
    }

    target = get_random_room_vanish( ch );

    if (target && number_bits(1) == 1) {
	postaffect_to_char( ch, sn, 1 );
	
	transfer_char( ch, ch, target,
		    "%1$^C1 ������������ � ������ ������, �������� ������������ �����.\r\n%1$^C1 ��������!",
		    "�� ������������� � ������ ������, �������� ������������ �����.\r\n�� ���������!",
		    "���������� ����� � ����� �������� ��������� �����..\r\n�� �������� ������ ���������� �������������� ������ %1$C2.",
		    "���� ���� �����������, ����������� � ������� ���������." );
    } else {
	ch->send_to("���� �� ������� ������������ � �����.\r\n");
    }
}


/*
 * 'teleport' spell
 */
SPELL_DECL(Teleport);
VOID_SPELL(Teleport)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    if (!ch->is_npc()) 
	victim = ch;

    if (HAS_SHADOW(victim) 
	|| IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
	|| (victim != ch && IS_SET(victim->imm_flags, IMM_SUMMON))
	|| (!ch->is_npc() && victim->fighting)
	|| (victim != ch && saves_spell( level - 5, victim, DAM_OTHER, ch, DAMF_SPELL)) 
	|| IS_BLOODY(victim))
    {
	ch->send_to("���� ������� ����������� ��������.\n\r");
	return;
    }
    
    victim->dismount( );
    transfer_char( victim, ch, get_random_room(victim),
		"%1$^C1 ��������!",
		(victim != ch ? "���� ���������������!" : ""),
		"%1$^C1 ���������� �� �����." );
}



/*-----------------------------------------------------------------------
 * WordOfRecallMovement
 *-----------------------------------------------------------------------*/
class WordOfRecallMovement : public RecallMovement {
public:
    WordOfRecallMovement( Character *ch )
		     : RecallMovement( ch )
    {
    }
    WordOfRecallMovement( Character *ch, Character *actor, Room *to_room )
		     : RecallMovement( ch, actor, to_room )
    {
    }

protected:
    virtual bool findTargetRoom( )
    {
	int point;
	
	if (to_room)
	    return true;

	if (ch->is_npc( ))
	    return false;

	if (( point = ch->getClan( )->getRecallVnum( ) ) <= 0)
	    point = ch->getPC( )->getHometown( )->getRecall( );

	if (point <= 0 || !( to_room = get_room_index( point ) )) {
	    msgSelf( ch, "You are completely lost." );
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
		   && checkBloody( wch )
		   && checkForsaken( wch );
    }
    virtual bool tryMove( Character *wch )
    {
	if (ch != actor)
	    return applyInvis( wch );
	else
	    return applyInvis( wch )
		   && applyMovepoints( );
    }
    virtual void movePet( NPCharacter *pet )
    {
	WordOfRecallMovement( pet, actor, to_room ).moveRecursive( );
    }
    virtual bool checkBloody( Character *wch )
    {
	if (IS_BLOODY(wch)) {
	    msgSelfParty( wch, 
	                  "...� �������� �������.", 
			  "����� ��� ���� �� %1$C2." );
	    return false;
	}

	return true;
    }
    virtual void msgOnMove( Character *wch, bool fLeaving )
    {
	if (fLeaving) 
	     msgRoomNoParty( wch,
		             "%1$^C1 ��������.",
			     "%1$^C1 � %2$C1 ��������." );
	else
	    msgRoomNoParty( wch, 
	                    "%1$^C1 ���������� � �������.",
	                    "%1$^C1 � %2$C1 ���������� � �������." );
    }
};

/*
 * 'word of recall' spell
 */
SPELL_DECL(WordOfRecall);
VOID_SPELL(WordOfRecall)::run( Character *ch, Character *, int sn, int level ) 
{ 
    if (ch->getProfession( ) == prof_samurai && ch->fighting) {
	ch->send_to("���� ����� �� ��������� ���� ��������������� ������ ��������!\n\r");
	return;
    }

    WordOfRecallMovement( ch ).move( );
}

