
/* $Id: group_maladictions.cpp,v 1.1.2.21.6.14 2010-08-24 20:31:55 rufina Exp $
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
#include "affecthandlertemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "objectbehavior.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"
#include "drink_utils.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"



SPELL_DECL(Anathema);
VOID_SPELL(Anathema)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;
    int strength = 0;

    if (victim->isAffected(sn)) {
	act_p("$C1 ��� ������$G��|�|��.", ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if (IS_GOOD(victim))
	strength = IS_EVIL(ch) ? 2 : (IS_GOOD(ch) ? 0 : 1);
    else if (IS_EVIL(victim))
	strength = IS_GOOD(ch) ? 2 : (IS_EVIL(ch) ? 0 : 1);
    else
	strength = (IS_GOOD(ch) || IS_EVIL(ch)) ? 1:0;

    if (!strength) {
	act_p("�, ���.. �������, $C1 �������� ����� �����..", ch, NULL, victim, TO_CHAR, POS_RESTING);
	return;
    }
    
    level += (strength - 1) * 3;

    if (saves_spell( level, victim , DAM_HOLY, ch, DAMF_SPELL )) {
	ch->send_to("���� ������� ����������� ��������.\n\r");
	return;
    }

    af.where        = TO_AFFECTS;
    af.type         = sn;
    af.level        = level;
    af.duration     = 8 + level/10;
    af.location     = APPLY_HITROLL;
    af.modifier     = - (level / 5) * strength;
    affect_to_char(victim, &af);

    af.location     = APPLY_DAMROLL;
    af.modifier     = - (level / 5) * strength;
    affect_to_char(victim, &af);

    af.location     = APPLY_SAVING_SPELL;
    af.modifier     = (level / 5) * strength;
    affect_to_char(victim, &af);

    af.location     = APPLY_LEVEL;
    af.modifier     = -strength * 3;
    af.bitvector    = AFF_CURSE;
    affect_to_char(victim, &af);
    
    act_p("���� $c2 ���������� ����!\r\n�� ���������� ���� �����������.", 
	    ch, 0, victim, TO_VICT, POS_RESTING);
    act_p("���� ���� ���������� $C4!", ch, 0, victim, TO_CHAR, POS_RESTING);
    act_p("���� $c2 ���������� $C4!", ch, 0, victim, TO_NOTVICT, POS_RESTING);

}

SPELL_DECL(BlackDeath);
VOID_SPELL(BlackDeath)::run( Character *ch, Room *room, int sn, int level ) 
{ 
  Affect af;

  if (IS_SET(room->room_flags, ROOM_LAW))
    {
      ch->send_to("������������ ���� ����������������� ����� �����.\n\r");
      return;
    }
    if ( room->isAffected( sn ))
    {
     ch->send_to("��� ����� ��� �������� �����.\n\r");
     return;
    }

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 15;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_PLAGUE;
    room->affectTo( &af );

    ch->send_to("���� �������� ��� ������.\n\r");
    act_p("���� �������� ��� ������.\n\r",ch,0,0,TO_ROOM,POS_RESTING);
}

AFFECT_DECL(BlackDeath);
VOID_AFFECT(BlackDeath)::update( Room *room, Affect *paf )
{
    Affect plague;
    Character *vch;

    plague.where		= TO_AFFECTS;
    plague.type 		= gsn_plague;
    plague.level 		= paf->level - 1;
    plague.duration 		= number_range(1,((plague.level/2)+1));
    plague.location		= APPLY_NONE;
    plague.modifier 		= -5;
    plague.bitvector 		= AFF_PLAGUE;

    for (vch = room->people; vch != 0; vch = vch->next_in_room) {
	if ( !saves_spell(plague.level, vch, DAM_DISEASE, 0, DAMF_SPELL)
		&& !is_safe_rspell(paf->level,vch)
		&& !IS_AFFECTED(vch,AFF_PLAGUE)
		&& number_bits(3) == 0)
	{
	    act("�� ���������� ��� � ������ �����.", vch, 0, 0, TO_CHAR);
	    act("$c1 ������ � �������� ����� �����$g��|��|��.", vch, 0, 0,TO_ROOM);
	    affect_join(vch,&plague);
	}
    }
}



SPELL_DECL(Blindness);
VOID_SPELL(Blindness)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	
	Affect af;

	if ( IS_AFFECTED(victim, AFF_BLIND) )
	{
    act_p("$c1 � ��� ������ �� �����.",victim,0,0,TO_ROOM,POS_RESTING);
		return;
	}

	if ( saves_spell(level,victim,DAM_OTHER,ch, DAMF_SPELL) )
	{
		ch->send_to("�� ����������.\n\r");
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.location  = APPLY_HITROLL;
	af.modifier  = -4;
	af.duration  = 3+level / 15;
	af.bitvector = AFF_BLIND;
	affect_to_char( victim, &af );
	victim->send_to("���� ��������!\n\r");
	act_p("$c1 ������ ������ �� �����.",victim,0,0,TO_ROOM,POS_RESTING);
	return;

}


SPELL_DECL(Curse);
VOID_SPELL(Curse)::run( Character *ch, Object *obj, int sn, int level ) 
{
    Affect af;

    if (obj->behavior && obj->behavior->isLevelAdaptive( ))
    {
	act_p("$o1 ��������� ���� �������.",ch,obj,0,TO_CHAR,POS_RESTING);
	return;
    }
    if (IS_OBJ_STAT(obj,ITEM_EVIL))
    {
	act_p("$o1 ��� ����� ����������� ����.",ch,obj,0,TO_CHAR,POS_RESTING);
	return;
    }

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
    {
	Affect *paf;

	paf = obj->affected->affect_find(gsn_bless);
	if (!savesDispel(level,paf != 0 ? paf->level : obj->level,0))
	{
	    if (paf != 0)
		affect_remove_obj( obj, paf);
	    act_p("���� ���� �������� $o4.",ch,obj,0,TO_ALL,POS_RESTING);
	    REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
	    return;
	}
	else
	{
	    act_p("��������� ���� $o2 ������� ������������� ��� ����.",
		   ch,obj,0,TO_CHAR,POS_RESTING);
	    return;
	}
    }

    af.where        = TO_OBJECT;
    af.type         = sn;
    af.level        = level;
    af.duration     = (8 + level / 5);
    af.location     = APPLY_SAVES;
    af.modifier     = +1;
    af.bitvector    = ITEM_EVIL;
    affect_to_obj( obj, &af);

    act_p("�������� ���� �������� $o4.",ch,obj,0,TO_ALL,POS_RESTING);
}

VOID_SPELL(Curse)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (IS_AFFECTED(victim,AFF_CURSE)) {
	if (ch == victim)
	    act("�� ��� ������$g��|�|��.", ch, 0, 0, TO_CHAR);
	else
	    act("$C1 ��� ������$G��|�|��.", ch, 0, victim, TO_CHAR);
	return;
    }
    
    if (saves_spell(level,victim,DAM_NEGATIVE,ch, DAMF_SPELL)) {
      ch->send_to("�� ����������...\n\r");	
      return;
    }
    
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (8 + level / 10);
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

	if (victim == ch)
          act("�� ���������� ���� �������$g��|��|��.", victim, 0, 0, TO_CHAR);
	else
	  act( "$C1 �������� �������$G��|��|��.", ch,0,victim,TO_CHAR);
}

SPELL_DECL(CursedLands);
VOID_SPELL(CursedLands)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af;

  if (IS_SET(room->room_flags, ROOM_LAW))
    {
      ch->send_to("������������ ���� ����������������� ����� �����.\n\r");
      return;
    }
    if ( room->isAffected( sn ))
    {
     ch->send_to("��� ����� ��� ��������!\n\r");
     return;
    }

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 15;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_CURSE;
    room->affectTo( &af );

    ch->send_to("������������ ������������� �������� ��� �����.\n\r");
    act_p("������������ ������������� �������� ��� �����.\n\r",
           ch,0,0,TO_ROOM,POS_RESTING);


}


SPELL_DECL(DeadlyVenom);
VOID_SPELL(DeadlyVenom)::run( Character *ch, Room *room, int sn, int level ) 
{ 
	Affect af;

	if (IS_SET(room->room_flags, ROOM_LAW))
	{
		ch->send_to("������������ ���� ����������������� ����� �����.\n\r");
		return;
	}
	if ( room->isAffected( sn ))
	{
		ch->send_to("��� ������� ��� ��������� ����������� ����.\n\r");
		return;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sn;
	af.level     = ch->getModifyLevel();
	af.duration  = level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_ROOM_POISON;
	room->affectTo( &af );

	ch->send_to("������� ����������� ��������� �����������.\n\r");
	act_p("������� ����������� ��������� �����������.\n\r",
	ch,0,0,TO_ROOM,POS_RESTING);

}

AFFECT_DECL(DeadlyVenom);
VOID_AFFECT(DeadlyVenom)::update( Room *room, Affect *paf )
{
    Affect af;
    Character *vch;

    af.where	= TO_AFFECTS;
    af.type 	= gsn_poison;
    af.level 	= paf->level - 1;
    af.duration	= number_range(1,((af.level/5)+1));
    af.location	= APPLY_NONE;
    af.modifier	= -5;
    af.bitvector= AFF_POISON;

    for ( vch = room->people; vch != 0; vch = vch->next_in_room )
    {
	if ( !saves_spell(af.level ,vch,DAM_POISON, 0, DAMF_SPELL)
		&& !is_safe_rspell(paf->level,vch)
		&& !IS_AFFECTED(vch,AFF_POISON) && number_bits(3) == 0)
	{
	    vch->send_to("���� ������������.\n\r");
	    act_p("$c1 {g��������{x �����.",vch,0,0,TO_ROOM,POS_RESTING);
	    affect_join(vch,&af);
	}
    }
}


SPELL_DECL(EnergyDrain);
VOID_SPELL(EnergyDrain)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    if ( saves_spell( level, victim,DAM_NEGATIVE,ch, DAMF_SPELL) )
    {
	ch->send_to("�� ����������...\n\r");
        victim->send_to("���� ���� ��������.\n\r");
	return;
    }

    if( victim->getModifyLevel() <= 2 )
    {
	dam		 = ch->hit + 1;
    }
    else
    {
//	gain_exp( victim, 0 - number_range( level/5, 3 * level / 5 ) );
	victim->mana	/= 2;
	victim->move	/= 2;
	dam		 = dice(1, level);
	ch->hit		+= dam;
    }

    victim->send_to("�� ����������, ��� ���� ����� ������!\n\r");
    ch->send_to("�� ����������� ��������� �����!\n\r");
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,true, DAMF_SPELL);

    return;

}


SPELL_DECL(LethargicMist);
VOID_SPELL(LethargicMist)::run( Character *ch, Room *room, int sn, int level ) 
{ 
   Affect af;

  if (IS_SET(room->room_flags, ROOM_LAW))
    {
      ch->send_to("������������ ���� ����������������� ����� �����.\n\r");
      return;
    }
    if ( room->isAffected( sn ))
    {
     ch->send_to("������������� ����� ��� ������ ��� �����.\n\r");
     return;
    }

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 15;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_SLOW;
    room->affectTo( &af );

    ch->send_to("���������� ������������� ����� ��������� ��� �����.\n\r");
    act_p("���������� ������������� ����� ��������� ��� �����.",
           ch,0,0,TO_ROOM,POS_RESTING);


}

AFFECT_DECL(LethargicMist);
VOID_AFFECT(LethargicMist)::entry( Room *room, Character *ch, Affect *paf )
{
     act_p("{y� ������� �������� �����-�� �����.{x",ch, 0, 0, TO_CHAR, POS_SLEEPING);
}

VOID_AFFECT(LethargicMist)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "������������� �����, ���������� � �������, ��������� ����� {W%1$d{x ��%1$I�|��|���.",
		   paf->duration )
	<< endl;
}

VOID_AFFECT(LethargicMist)::update( Room *room, Affect *paf )
{
    Affect af;
    Character *vch;

    af.where	= TO_AFFECTS;
    af.type 	= gsn_slow;
    af.level 	= paf->level - 1;
    af.duration	= number_range(1,((af.level/5)+1));
    af.location	= APPLY_NONE;
    af.modifier	= -5;
    af.bitvector= AFF_SLOW;

    for (vch = room->people; vch != 0; vch = vch->next_in_room) {
	if ( !saves_spell(af.level ,vch,DAM_OTHER, 0, DAMF_SPELL)
		&& !is_safe_rspell(paf->level,vch)
		&& !IS_AFFECTED(vch,AFF_SLOW) && number_bits(3) == 0 )
	{
	    vch->send_to("���� �������� �����������.\n\r");
	    act_p("�������� $c2 �����������.",vch,0,0,TO_ROOM,POS_RESTING);
	    affect_join(vch,&af);
	}
    }
}


SPELL_DECL(Plague);
VOID_SPELL(Plague)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (saves_spell(level,victim,DAM_DISEASE,ch, DAMF_SPELL) ||
	(victim->is_npc() && IS_SET(victim->act,ACT_UNDEAD)))
    {
	if (ch == victim)
	  ch->send_to("�� ���������� ������ ��������, �� ��� ��������.\n\r");
	else
	  act_p("$C1 �� ����������$G��|�|�� � �������.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level * 3/4;
    af.duration  = (10 + level / 10);
    af.location  = APPLY_STR;
    af.modifier  = -1 * max(1,3 + level / 15);
    af.bitvector = AFF_PLAGUE;
    affect_join(victim,&af);

    victim->send_to("�� ������� �� ����, ����� ���� ����������� ������� ������.\n\r");
    act_p("$c1 ������ �� ����, ����� ���� ����������� ������� ������.",
	   victim,0,0,TO_ROOM,POS_RESTING);

}

AFFECT_DECL(Plague);
VOID_AFFECT(Plague)::update( Character *ch, Affect *paf ) 
{
    Affect plague;
    Character *vch;
    int dam;
        
    DefaultAffectHandler::update( ch, paf );

    act_p("$c1 ������ � ������, ����� ���� ���������� ��� $s ����.",
	  ch,0,0,TO_ROOM,POS_RESTING);

    ch->send_to("�� ������� � ������ �� ����.\n\r");
    
    if (paf->level <= 1) 
       return; 

    plague.where	= TO_AFFECTS;
    plague.type 	= gsn_plague;
    plague.level 	= paf->level - 1;
    plague.duration 	= number_range(1,2 * plague.level);
    plague.location	= APPLY_STR;
    plague.modifier 	= -5;
    plague.bitvector 	= AFF_PLAGUE;

    for ( vch = ch->in_room->people; vch != 0; vch = vch->next_in_room) {
	if (!saves_spell(plague.level + 2,vch,DAM_DISEASE, 0, DAMF_SPELL)
		&& !is_safe_rspell( vch )
		&& !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(2) == 0)
	{
	    vch->send_to("�� ���������� ��� � ������ ��������.\n\r");
	    act_p("$c1 ������ � �������� ����� �����$g��|��|��.",
		  vch,0,0,TO_ROOM,POS_RESTING);
	    affect_join(vch,&plague);
	}
    }

    dam = min( ch->getModifyLevel(), static_cast<short>( paf->level/5+1 ) );
    ch->mana -= dam;
    ch->move -= dam;
    damage_nocatch( ch, ch, dam, gsn_plague,DAM_DISEASE,false, DAMF_SPELL);

    if (number_range(1, 100) < 70 )
	damage_nocatch( ch, ch, max(ch->max_hit/20, 50), gsn_plague,DAM_DISEASE,true, DAMF_SPELL);
}
    
VOID_AFFECT(Plague)::entry( Character *ch, Affect *paf ) 
{
    Affect plague;
    Character *vch;

    DefaultAffectHandler::entry( ch, paf );

    if (paf->level <= 1)
	return;

    plague.where		= TO_AFFECTS;
    plague.type 		= gsn_plague;
    plague.level 		= paf->level - 1;
    plague.duration = number_range(1,2 * plague.level);
    plague.location	= APPLY_STR;
    plague.modifier = -5;
    plague.bitvector = AFF_PLAGUE;

    for (vch = ch->in_room->people; vch != 0; vch = vch->next_in_room)
	if ( !saves_spell(plague.level - 2,vch,DAM_DISEASE, 0, DAMF_SPELL)
		&& !vch->is_immortal()
		&& !IS_AFFECTED(vch,AFF_PLAGUE) 
		&& number_bits(6) == 0)
	{
	    vch->println( "�� ���������� ��� � ���������." );
	    act_p("$c1 ������ � �������� ����������.",vch,0,0,TO_ROOM,POS_RESTING);
	    affect_join(vch,&plague);
	}
}

SPELL_DECL(Poison);
VOID_SPELL(Poison)::run( Character *ch, Object *obj, int sn, int level ) 
{
	Affect af;

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
		if (drink_is_closed( obj, ch ))
		    return;

		if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
		{
			act_p("�� �� ������ �������� $o1.",ch,obj,0,TO_CHAR,POS_RESTING);
			return;
		}
		
		SET_BIT(obj->value[3], DRINK_POISONED);
		act_p("���� ��� ��������� � $o4.",ch,obj,0,TO_ALL,POS_RESTING);
		return;
	}

	if (obj->item_type == ITEM_WEAPON)
	{
		if ( IS_WEAPON_STAT(obj,WEAPON_FLAMING)
			|| IS_WEAPON_STAT(obj,WEAPON_FROST)
//				|| IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
//				|| IS_WEAPON_STAT(obj,WEAPON_SHARP)
			|| IS_WEAPON_STAT(obj,WEAPON_VORPAL)
			|| IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
			|| IS_WEAPON_STAT(obj,WEAPON_FADING)
			|| IS_WEAPON_STAT(obj,WEAPON_HOLY)
			|| IS_OBJ_STAT(obj,ITEM_BLESS)
			|| IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
		{
			act_p("�� �� ������ �������� $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
			return;
		}

		if (IS_WEAPON_STAT(obj,WEAPON_POISON))
		{
			act_p("������������� $o2 ��� �������.",ch,obj,0,TO_CHAR,POS_RESTING);
			return;
		}

		af.where	 = TO_WEAPON;
		af.type	 = sn;
		af.level	 = level / 2;
		af.duration	 = level/8;
		af.location	 = 0;
		af.modifier	 = 0;
		af.bitvector = WEAPON_POISON;
		affect_to_obj( obj, &af);

		act_p("������������� $o2 ���������� ��������.",ch,obj,0,TO_ALL,POS_RESTING);
		return;
	}

	act_p("�� �� ������ �������� $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
}

VOID_SPELL(Poison)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	Affect af;

	if ( saves_spell( level, victim,DAM_POISON,ch, DAMF_SPELL) )
	{
		act_p("���� $c2 ����������� ����������� �������, �� ��� ����� ��������.",
			victim,0,0,TO_ROOM,POS_RESTING);
		victim->send_to("�� ���������� ������ ����������, �� ��� ����� ��������.\n\r");
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (10 + level / 10);
	af.location  = APPLY_STR;
	af.modifier  = -2;
	af.bitvector = AFF_POISON;
	affect_join( victim, &af );
	victim->send_to("�� ���������� ���� ����� ����������.\n\r");
	act_p("$c1 �������� ����� ����������.",victim,0,0,TO_ROOM,POS_RESTING);

}

AFFECT_DECL(Poison);
VOID_AFFECT(Poison)::update( Character *ch, Affect *paf ) 
{ 
    int poison_damage;

    DefaultAffectHandler::update( ch, paf );

    if (!IS_AFFECTED(ch, AFF_POISON) || IS_AFFECTED(ch, AFF_SLOW))
	return;

    act_p("$c1 ������ � ���������� ����.", ch, 0, 0, TO_ROOM, POS_RESTING);
    ch->send_to("�� ������� � ����������� ����.\n\r");
    
    poison_damage = paf->level * number_range(1,5);
    
    if (ch->getRealLevel( ) < 20)
	poison_damage = paf->level * number_range(1,2);
    else if (ch->getRealLevel( ) < 40)
	poison_damage = paf->level * number_range(1,4);
    
    poison_damage = max( 1, poison_damage );
    damage_nocatch(ch, ch, poison_damage, gsn_poison, DAM_POISON, true, DAMF_SPELL);
}

SPELL_DECL(Slow);
VOID_SPELL(Slow)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( victim->isAffected(sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
	if (victim == ch)
	  ch->send_to("�� �� ������ ��������� ���������, ��� ������!\n\r");
	else
	  act_p("$C1 �� ����� ��������� ���������, ��� ������.",
	         ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if (saves_spell(level,victim,DAM_OTHER,ch, DAMF_SPELL)
    ||  IS_SET(victim->imm_flags,IMM_SPELL))
    {
	if (victim != ch)
	    ch->send_to("������ �� ���������.\n\r");
	victim->send_to("�� ���������� ���� ������� �����, �� ��� ����� ��������.\n\r");
	return;
    }

    if (IS_AFFECTED(victim,AFF_HASTE))
    {
	if (checkDispel(level,victim,gsn_haste))
	    return;
	
	if (victim != ch)
	    ch->send_to("���� ������� ����������� ��������.\n\r");

	victim->send_to("���� �������� �����������, �� ���� �� ���������.\n\r");
	return;
    }


    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (4 + level / 12);
    af.location  = APPLY_DEX;
    af.modifier  = - max(2,level / 12);
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    victim->send_to("���� �������� �����������...\n\r");
    act_p("�������� $c2 �����������.",victim,0,0,TO_ROOM,POS_RESTING);
    return;

}

SPELL_DECL(Weaken);
VOID_SPELL(Weaken)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (saves_spell( level, victim,DAM_OTHER,ch, DAMF_SPELL) ) {
      ch->send_to("�� ����������...\n\r");	
      return;
    }

    if (victim->isAffected(sn )) {
	if (victim == ch)
	    act("�� ��� �������$g��|�|��.", ch, 0, 0, TO_CHAR);
	else
	    act("$C1 ��� �������$G��|�|��.", ch, 0, victim, TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (4 + level / 12);
    af.location  = APPLY_STR;
    af.modifier  = -1 * (2 + level / 12);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    victim->send_to("�� ����������, ��� ���� �������� ����.\n\r");
    act_p("$c1 ������� �� ������.",victim,0,0,TO_ROOM,POS_RESTING);
    return;

}

SPELL_DECL(UnholyWord);
VOID_SPELL(UnholyWord)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *vch;
    Character *vch_next;
    int dam;
    
    if (!IS_EVIL(ch)) {
	ch->send_to( "��� ���� ���������� ����.\r\n" );
	return;
    }
    
    act_p("$c1 ���������� �������� �����!", ch,0,0,TO_ROOM,POS_RESTING);
    ch->send_to("�� ����������� �������� �����!\n\r");

    for (vch = room->people; vch != 0; vch = vch_next) {
	vch_next = vch->next_in_room;
	
	if (is_safe_spell(ch, vch, true ))
	    continue;

	if (vch->is_mirror( ) && (number_percent( ) < 50)) 
	    continue;
	
	if (is_safe( ch, vch ))
	    continue;
	
	if (IS_EVIL(vch))
	    continue;
	else if (IS_GOOD(vch))
	    dam = dice( level, 20 );
	else 
	    dam = dice( level, 15 );
    
	if (saves_spell( level, vch, DAM_NEGATIVE,ch, DAMF_SPELL )) {
	    dam /= 2;
	}
	else if (!IS_AFFECTED( vch, AFF_CURSE )) {
	    Affect af;
	    
	    af.where = TO_AFFECTS;
	    af.type  = sn;
	    af.level = level;
	    af.duration = 2 * level;
	    af.location = APPLY_HITROLL;
	    af.modifier = -1 * (level / 5);
	    af.bitvector = AFF_CURSE;
	    affect_to_char( vch, &af );
	    
	    af.location  = APPLY_SAVING_SPELL;
	    af.modifier  = level / 8;
	    affect_to_char( vch, &af );

	    vch->send_to("�� ���������� ���� �������������.\n\r");
	    
	    if (ch != vch)
		act("$C1 �������� �������������.",ch,0,vch,TO_CHAR);
	}

	vch->send_to("����������� ���� ��������� ����!\n\r");
	damage( ch, vch, dam, sn, DAM_NEGATIVE, true, DAMF_SPELL );
    }
}

SPELL_DECL(BlackFeeble);
VOID_SPELL(BlackFeeble)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (ch->isAffected(sn )) {
	ch->send_to("� ���� ��������.\n\r");
	return;
    }

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 30 ) + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( ch, &af );
    
    act_p( "��������� ���� ����� ��������� �������� $c2, ������� �������� ����.", ch, 0, 0, TO_ROOM,POS_RESTING);
    ch->send_to("����� ��������� �������� ����, ������� �������� ����.\n\r");
}

SPELL_DECL(Corruption);
VOID_SPELL(Corruption)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (IS_AFFECTED(victim,AFF_CORRUPTION))
	{
	 act_p("$C1 ��� ����� ������.",ch,0,victim,TO_CHAR,POS_RESTING);
	 return;
	}

    if (saves_spell(level, victim, DAM_NEGATIVE, ch, DAMF_SPELL) ||
	(victim->is_npc() && IS_SET(victim->act,ACT_UNDEAD)))
    {
	if (ch == victim)
	    act("�� �� ��������� �����������$g��|�|�� ���� ����������.", ch, 0, 0, TO_CHAR);
	else
	  act("$C1 ������� �� ���������$G��|��|�� ����������.",ch,0,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level * 3/4;
    af.duration  = (10 + level / 5);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_CORRUPTION;
    affect_join(victim,&af);
    
    act("�� ������������ � �����, ������� ����� ������.", victim, 0, 0, TO_CHAR);
    act("$c1 ����������� � �����, ������� ����� ������.", victim, 0, 0, TO_ROOM);
}
