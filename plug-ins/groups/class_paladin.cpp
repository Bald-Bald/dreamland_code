/* $Id: class_paladin.cpp,v 1.1.2.13.6.10 2010-09-01 21:20:44 rufina Exp $
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

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "spelltemplate.h"

#include "fleemovement.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "gsn_plugin.h"
#include "occupations.h"
#include "act_move.h"
#include "mercdb.h"

#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "handler.h"
#include "vnum.h"
#include "merc.h"
#include "act.h"
#include "interp.h"
#include "def.h"

RACE(golem);
RACE(demon);

/*
 * 'layhands' skill command
 */

SKILL_RUNP( layhands )
{
    Character *victim;
    Affect af;

    if ( ch->is_npc() || !gsn_lay_hands->usable( ch ) )
    {
	ch->println("���� ��������� ���������� ������ ������ ���������� ���.");
	return;
    }

    if ( (victim = get_char_room(ch,argument)) == 0) {
	ch->send_to("�� �� ������ ����� ������ ���������.\n\r");
	return;
    }

    if ( ch->isAffected(gsn_lay_hands)) {
	ch->send_to("�� ���� �� ������ ���������������.\n\r");
	return;
    }

    ch->setWait( gsn_lay_hands->getBeats( ) );

    af.type = gsn_lay_hands;
    af.where = TO_AFFECTS;
    af.level = ch->getModifyLevel();
    af.duration = 2;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = 0;
    affect_to_char ( ch, &af );

    victim->hit = min( victim->hit + ch->getModifyLevel() * 5, (int)victim->max_hit );
    update_pos( victim );
    
    if (ch != victim) {
	act( "�� ���������� ���� �� $C4, � $M ���������� ������� �����.", ch, 0, victim, TO_CHAR);
	act( "$c1 ��������� �� ���� ����. ����� ��������� ���� ����.", ch, 0, victim, TO_VICT);
	act( "$c1 ��������� ���� �� $C4. $C1 �������� ������� �����.", ch, 0, victim, TO_NOTVICT);
    } else {
	act( "�� ���������� �� ���� ����: ���� ���������� ������� �����.", ch, 0, 0, TO_CHAR);
	act( "$c1 ��������� �� ���� ����. $c1 �������� ������� �����.", ch, 0, 0, TO_ROOM);
    }
    
    gsn_lay_hands->improve( ch, true, victim );
}

SPELL_DECL(Banishment);
VOID_SPELL(Banishment)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    if (!victim->is_npc( ) 
	|| (!IS_SET( victim->form, FORM_UNDEAD )
	   && !IS_SET( victim->act, ACT_UNDEAD )
	   && victim->getRace( ) != race_demon
	   && victim->getRace( ) != race_golem))
    {
	act_p("$C1 - ����� �� ������� � �� �����.", ch, 0, victim, TO_CHAR, POS_RESTING);
	return;
    }

    if (saves_spell(level, victim, DAM_HOLY, ch, DAMF_SPELL)) {
	act_p("� $C5, �������, ������ �� ����������.", ch, 0, victim, TO_CHAR, POS_RESTING);
	return;
    }
    
    act_p("��������� ������������� �������, ��� ��������� $c4 � $e ��������.",
	    victim, 0, 0, TO_ROOM, POS_RESTING);
	
    raw_kill( victim, -1, ch, FKILL_MOB_EXTRACT );
}


SPELL_DECL(Prayer);
VOID_SPELL(Prayer)::run( Character *ch, char *, int sn, int level ) 
{ 
    Affect af;
    int lvl = max(ch->getRealLevel( ) + 10, 110);

    level = number_range(102, 110);

    if (ch->hit < ch->getRealLevel( ) || ch->mana < ch->getRealLevel( ) || ch->move < ch->getRealLevel( )) {
	act_p("�� ������� ������$g��|�|�� ��� �������.", ch, 0, 0, TO_CHAR, POS_RESTING);
	return;
    }

    ch->mana -= ch->getModifyLevel( );
    ch->move -= ch->getModifyLevel( );
    ch->hit -= ch->getModifyLevel( );
    update_pos(ch);
    ch->setWaitViolence( 1 );

    if (ch->isAffected(sn) 
	|| (number_percent() < number_fuzzy(1) + 50 - ch->getSkill( sn ) / 2))
    {
	// bad 
	act_p("�� ��������$g��|�|�� ����� ������ ���������!", ch, 0, 0, TO_CHAR, POS_RESTING);

	if (!ch->isAffected(gsn_weaken)) {
	    af.where = TO_AFFECTS;
	    af.type = gsn_weaken;
	    af.level = lvl;
	    af.duration = lvl / 15;
	    af.location = APPLY_STR;
	    af.modifier = -1 * (lvl / 4);
	    af.bitvector = 0;
	    affect_to_char(ch, &af);
	    ch->send_to("�� ����������, ��� ���� ������ �� ����.\n\r");
	    act_p("$c1 �������� ������ � ��������.", ch, NULL, NULL, TO_ROOM, POS_RESTING);
	}
	else if (!IS_AFFECTED(ch, AFF_CURSE) && !IS_SET(ch->imm_flags, IMM_NEGATIVE)) {
	    af.where = TO_AFFECTS;
	    af.type = gsn_curse;
	    af.level = lvl;
	    af.duration = lvl / 10;
	    af.location = APPLY_HITROLL;
	    af.modifier = -1 * (lvl / 7);
	    af.bitvector = AFF_CURSE;
	    affect_to_char(ch, &af);
	    af.location = APPLY_SAVING_SPELL;
	    af.modifier = lvl / 7;
	    affect_to_char(ch, &af);
	    act_p("�� ���������� ���� �������$g��|��|��.", ch, 0, 0, TO_CHAR, POS_RESTING);
	}
	else {
	    if (ch->position == POS_FIGHTING) {
		ch->send_to("���� ������� ��������� ���� ������������...\n\r");
		ch->setDazeViolence( 3 );
		ch->setWaitViolence( 1 );
	    }
	    else {
		af.where = TO_AFFECTS;
		af.type = gsn_sleep;
		af.level = lvl;
		af.duration = 3;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = AFF_SLEEP;
		affect_join(ch, &af);

		if (IS_AWAKE(ch)) {
		    ch->send_to("�� ���������....\n\r");
		    act_p("$c1 ��������.", ch, NULL, NULL, TO_ROOM, POS_RESTING);
		    ch->position = POS_SLEEPING;
		}
	    }
	}
	return;
    }

    if (number_percent() > ch->getRealLevel( ) * 3 * ch->getSkill( sn ) / 100) {
	// nothing 
	ch->send_to("���� ������� ������, ����� �������� �� ����� ������...\n\r");
	return;
    }

    // you did it! 

    ch->send_to("������������� ����� �������� �� ����!\n\r");

    af.where = TO_AFFECTS;
    af.type = sn;
    af.level = ch->getRealLevel( );
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = 0;
    af.duration = number_fuzzy(1 + ch->getRealLevel( ) / 8);
    affect_to_char(ch, &af);

    // random effects 
    sn = -1;

    if (ch->position == POS_FIGHTING && ch->fighting != NULL) {
	switch (number_range(0, 7)) {
	case 0:
	    if (IS_GOOD(ch) && IS_EVIL(ch->fighting))
		sn = gsn_ray_of_truth;
	    else if (IS_EVIL(ch) && IS_GOOD(ch->fighting))
		sn = gsn_demonfire;
	    else
		sn = gsn_flamestrike;
	    break;
	case 2:
	    sn = gsn_curse;
	    break;
	case 5:
	    sn = gsn_blindness;
	    break;
	case 7:
	    sn = gsn_cause_critical;
	    break;
	default:
	    break;
	}

	if (sn != -1)
	    spell( sn, level, ch, ch->fighting );

	ch->setWaitViolence( 1 );
    }
    else {
	switch (number_range(0, 31)) {
	case 0:	    sn = gsn_sanctuary;		    break;
	case 1:	    sn = gsn_bless;		    break;
	case 2:
	case 3:	    sn = gsn_heal;		    break;
	case 4:	    sn = gsn_haste;		    break;
	case 5:
	case 6:	    sn = gsn_refresh;               break;
	case 7:	    sn = gsn_benediction;	    break;
	case 8:	    sn = gsn_shield;		    break;
	case 9:		
	case 10:    sn = gsn_stone_skin;            break;
	case 11:
	case 12:    sn = gsn_armor;		    break;
	case 13:	
	    if (IS_EVIL( ch ))
		sn = gsn_protection_good;
	    break;
	case 14:
	    if (IS_GOOD( ch ))
		sn = gsn_protection_evil;
	    break;
	case 15:
	case 16:    sn = gsn_giant_strength;	    break;
	case 17:    sn = gsn_protective_shield;	    break;
	case 18:    sn = gsn_frenzy;                break;
	case 19:    sn = gsn_enhanced_armor;	    break;
	default:
	    break;
	}
	if (sn != -1)
	    spell( sn, level, ch, ch );
    }
    

}


SPELL_DECL(TurnUndead);
VOID_SPELL(TurnUndead)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *vch, *vch_next;
    int dam;

    act_p( "$c1 ������ � ������� ��������� ������.", ch, 0, 0, TO_ROOM, POS_RESTING);
    act_p( "�� ������� � ������� ��������� ������.", ch, 0, 0, TO_CHAR, POS_RESTING);

    for (vch = room->people; vch != NULL; vch = vch_next) {
	vch_next = vch->next_in_room;

	if (is_safe_spell( ch, vch, true ))
	    continue;
	
	if (!IS_SET( vch->form, FORM_UNDEAD ))
	    continue;
	
	if (!IS_SET( vch->act, ACT_UNDEAD ))
	    continue;
	
	try {
	    if (saves_spell( level, vch, DAM_HOLY, ch, DAMF_SPELL )) {
		act("$C1 ���������� ���� ������ ������� �������� � ��������� � �����!", ch, 0, vch, TO_CHAR);
		act_p("�� ����������� ������ ������� ��������.", ch, 0, vch, TO_VICT, POS_RESTING);
		damage( ch, vch, 0, sn, DAM_HOLY, true, DAMF_SPELL );
	    }
	    else {
		act_p( "$c5 ���������� ��������� ����, ��������� � ����� ���������� � �������.", vch, 0, ch, TO_ROOM, POS_RESTING);
		act_p( "��������� ���� ���������� �����, ��������� � ����� ���������� � �������.", ch, 0, vch, TO_VICT, POS_RESTING);

		dam = dice( level, 12 ); 
		damage_nocatch( ch, vch, dam, sn, DAM_HOLY, true, DAMF_SPELL );
		FleeMovement( vch ).move( );
	    }
	} catch (const VictimDeathException &) {
	}
    }

}

SPELL_DECL(Turn);
VOID_SPELL(Turn)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *victim, *victim_next;

    Affect af;

    if ( ch->isAffected(sn ) )
    {
	ch->send_to("��� ���������� �������������� ������ �������.");
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 5;
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = 0;
    affect_to_char( ch, &af );

    for (victim = room->people; victim != 0; victim = victim_next)
    {
	int dam, align, level;

	victim_next = victim->next_in_room;
	level = ch->getModifyLevel( );

	if (is_safe_spell(ch,victim,true))
	    continue;
	if (is_safe(ch, victim))
          continue;

	if (IS_EVIL(ch) ) {
	    victim = ch;
	    ch->send_to("������� ���������� ������ ����!\n\r");
	}

	if (victim != ch) {
	    act_p("$c1 ������ �������� ����, ������� ������������� ��� �����!",
		   ch,0,0,TO_ROOM,POS_RESTING);
	    ch->send_to("�� ������ ��������� ����, ������� ������������� ��� �����!\n\r");
	}

	if (IS_GOOD(victim) || IS_NEUTRAL(victim)) {
	    act_p("���� �� ����� ��������� ����� $c3.",
		   victim,0,victim,TO_ROOM,POS_RESTING);
	    victim->send_to("���� �� ����� ��������� ���� �����.\n\r");
	    continue;
	}
	
	if (victim->is_npc( )
	    && victim->getNPC( )->behavior
	    && IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_CLANGUARD)))
	{
	    act_p("$c1 �� ����� �������� ���� ����.", victim, 0, 0, TO_ROOM, POS_RESTING);
	    continue;
	}
			     
	dam = dice( level, 10 );
	if ( saves_spell( level, victim,DAM_HOLY, ch, DAMF_SPELL ) )
	    dam /= 2;

	align = victim->alignment;
	align -= 350;

	if (align < -1000)
	    align = -1000 + (align + 1000) / 3;

	dam = (dam * align * align) / 1000000;

	damage( ch, victim, dam, sn, DAM_HOLY, true, DAMF_SPELL);

	if (victim->in_room == 0)
	    continue;
	
	if (FleeMovement( victim ).move( ))
	    break;
    }
}
