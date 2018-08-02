/* $Id: group_benedictions.cpp,v 1.1.2.18.6.14 2009/09/11 11:24:54 rufina Exp $
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

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"


GSN(inspiration);
PROF(paladin);
PROF(anti_paladin);

SPELL_DECL(Benediction);
VOID_SPELL(Benediction)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;
    int strength = 0;

    if (victim->isAffected(sn)) {
	if (victim == ch)
	    act_p("�� ��� �����������$g��|�|��.", ch,0,0,TO_CHAR,POS_RESTING);
	else
	    act_p("$C1 ��� �����������$G��|�|��.", ch,0,victim,TO_CHAR,POS_RESTING);

	return;
    }

    if (IS_EVIL(victim))
	    strength = IS_EVIL(ch) ? 2 : (IS_GOOD(ch) ? 0 : 1);
    if (IS_GOOD(victim))
	    strength = IS_GOOD(ch) ? 2 : (IS_EVIL(ch) ? 0 : 1);
    if (IS_NEUTRAL(victim))
	    strength = IS_NEUTRAL(ch) ? 2 : 1;

    if (!strength) {
	act_p("������, ���� ���� �� ������������ � $C3.", ch, NULL, victim, TO_CHAR, POS_RESTING);
	return;
    }

    af.where        = TO_AFFECTS;
    af.type         = sn;
    af.level        = level;
    af.duration     = 5 + level/2;
    af.location     = APPLY_HITROLL;
    af.modifier     = (level / 8) * strength;
    af.bitvector    = 0;
    affect_to_char(victim, &af);

    af.location     = APPLY_SAVING_SPELL;
    af.modifier     = 0 - (level / 8) * strength;
    affect_to_char(victim, &af);

    if (victim != ch) {
        act_p("�� ������ $C3 ������������� ����� �����.", ch,0,victim,TO_CHAR,POS_RESTING);
        act_p("$c1 ����� ���� ������������� ����� �����.", ch,0,victim,TO_VICT,POS_RESTING);
    }
    else
	victim->println("�� ���������� ������������ ��������������.");

}

SPELL_DECL(Bless);
VOID_SPELL(Bless)::run( Character *ch, Object *obj, int sn, int level ) 
{
    Affect af;

    if (obj->behavior && obj->behavior->isLevelAdaptive( ))
    {
	act_p("$o1 ��������� ���� �������.",ch,obj,0,TO_CHAR,POS_RESTING);
	return;
    }
    if (obj->is_obj_stat(ITEM_BLESS))
    {
	act_p("$o1 ��� ����� ��������� ����.",ch,obj,0,TO_CHAR,POS_RESTING);
	return;
    }
    
    if (obj->is_obj_stat(ITEM_EVIL))
    {
	Affect *paf;

	paf = obj->affected->affect_find(gsn_curse);
	if (!savesDispel(level,paf != 0 ? paf->level : obj->level,0))
	{
	    if (paf != 0)
		affect_remove_obj( obj, paf);
	    act_p("��������� ���� �������� $o4.",ch,obj,0,TO_ALL,POS_RESTING);
	    REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
	    return;
	}
	else
	{
	    act_p("����������� ���� $o2 ����� �������������, ��� ���� �������������.",
		   ch,obj,0,TO_CHAR,POS_RESTING);
	    return;
	}
    }
    // let's check, may be a permanet affect
    if (number_percent() < level/4 )
    {  // permanet
      af.where	= TO_OBJECT;
      af.type		= sn;
      af.level	= level;
      af.duration	= -1;
      af.location	= APPLY_SAVES;
      af.modifier	= -1;
      af.bitvector	= ITEM_BLESS;
      affect_to_obj( obj, &af);
	    act_p("$o1 �������� ��������� ������ ������� ������.",
	    ch,obj,0,TO_ALL,POS_RESTING);
    }
    else // not a permanent effect
    {
      af.where	        = TO_OBJECT;
      af.type		= sn;
      af.level	        = level;
      af.duration	= (6 + level / 2);
      af.location	= APPLY_SAVES;
      af.modifier	= ch->isAffected( gsn_inspiration ) ? -3 : -1;
      af.bitvector	= ITEM_BLESS;
      affect_to_obj( obj, &af);
      act_p("��������� ���� �������� $o4.",ch,obj,0,TO_ALL,POS_RESTING);
    }
}

VOID_SPELL(Bless)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( victim->isAffected(sn ) ||
	 victim->isAffected(gsn_warcry ) )
    {
	if (victim == ch)
	  act("�� ��� �����������$g��|�|��.", ch,0, 0,TO_CHAR);
	else
	  act("$C1 ��� �����������$G��|�|��.", ch,0,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = (6 + level / 2);
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 8;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( victim, &af );
    victim->send_to("�� ���������� ������������ �������������.\n\r");
    if ( ch != victim )
	act("�� ������ $C3 ������������� ����� �����.", ch,0,victim,TO_CHAR);

}

SPELL_DECL(Calm);
VOID_SPELL(Calm)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *vch;
    int mlevel = 0;
    int count = 0;
    short high_level = 0;
    int chance;
    Affect af;

    /* get sum of all mobile levels in the room */
    for (vch = room->people; vch != 0; vch = vch->next_in_room)
    {
	if (vch->position == POS_FIGHTING)
	{
	    count++;
	    if (vch->is_npc())
	      mlevel += vch->getModifyLevel();
	    else
	      mlevel += vch->getModifyLevel() / 2;
	    high_level = max(high_level, vch->getModifyLevel() );
	}
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (ch->is_immortal()) /* always works */
      mlevel = 0;

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
	for (vch = room->people; vch != 0; vch = vch->next_in_room)
   	{
	    if (vch->is_npc() && (IS_SET(vch->imm_flags,IMM_SPELL) ||
				IS_SET(vch->act,ACT_UNDEAD)))
	      continue;

	    if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
	    ||  vch->isAffected(gsn_frenzy))
	      continue;

	    vch->send_to("����� ����������� ��������� ����.\n\r");
	    act( "����� ����������� ��������� $c4.", vch, 0, 0, TO_ROOM );

	    if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,false);


	    af.where = TO_AFFECTS;
	    af.type = sn;
	    af.level = level;
	    af.duration = level/4;
	    af.location = APPLY_HITROLL;
	    if (!vch->is_npc())
	      af.modifier = -5;
	    else
	      af.modifier = -2;
	    af.bitvector = AFF_CALM;
	    affect_to_char(vch,&af);

	    af.location = APPLY_DAMROLL;
	    affect_to_char(vch,&af);
	}
    }

}
SPELL_DECL(Frenzy);
VOID_SPELL(Frenzy)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (victim->isAffected(sn) || IS_AFFECTED(victim,AFF_BERSERK))
    {
	if (victim == ch)
	  ch->send_to("�� ��� � ������!\n\r");
	else
	  act_p("$C1 ��� � ������!",ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if (victim->isAffected(gsn_calm))
    {
	if (victim == ch)
	  ch->send_to("������ ���� ����� �� ����� ���������.\n\r");
	else
	  act_p("������ ����� �� ����� ��������� $C4.",
	         ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
	(IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
	(IS_EVIL(ch) && !IS_EVIL(victim))
       )
    {
	act_p("���� ���� �� ������������ � $C3.",ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level;
    af.duration	 = level / 3;
    af.modifier  = level / 6;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 12);
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);

    victim->send_to("����� ������ ��������� ����!\n\r");
    act_p("� ������ $c2 ���������� ����� ������!",
           victim,0,0,TO_ROOM,POS_RESTING);

}


SPELL_DECL(GroupDefense);
VOID_SPELL(GroupDefense)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *gch;
    Affect af;

    for( gch=room->people; gch != 0; gch=gch->next_in_room)
    {
	if( !is_same_group( gch, ch))
	    continue;

	if (spellbane( ch, gch ))
	    continue;

	if( gch->isAffected(gsn_armor ) ) {
	    if( gch == ch)
		act("�� ��� ������$g��|�|�� ����������� �����.", ch, 0, 0, TO_CHAR);
	    else
		act("$C1 ��� ������$G��|�|�� ����������� �����.", ch, 0, gch, TO_CHAR);
	    continue;
	}

	af.where     = TO_AFFECTS;
	af.type      = gsn_armor;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier  = -20;
	affect_to_char( gch, &af );

	act("��������� ����� �������� ����.", gch, 0, 0, TO_CHAR);
	if( ch != gch )
	    act("��������� ����� �������� $C4.", ch, 0, gch, TO_CHAR);
	
	if( gch->isAffected(gsn_shield ) )
	{
	  if (gch == ch)
	      act("�� ��� ������$g��|�|�� ����������� ����.", ch, 0, 0, TO_CHAR);
	  else
	      act("$C1 ��� ������$G��|�|�� ����������� ����.", ch, 0, gch, TO_CHAR);
	  continue;
	}

	af.where     = TO_AFFECTS;
	af.type      = gsn_shield;
	af.level     = level;
	af.duration  = level;
	af.location  = APPLY_AC;
	af.modifier   = -20;
	affect_to_char( gch, &af );

	act("������������ ������� �������� ���� �����.", gch, 0, 0, TO_CHAR);
	if( ch != gch )
	    act("������������ ������� �������� $C4 �����.", ch, 0, gch, TO_CHAR);
    }
}


SPELL_DECL(HealingLight);
VOID_SPELL(HealingLight)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af,af2;

    if ( room->isAffected( sn ))
    {
	ch->send_to("��� ������� ��� �������� ������������ ������.\n\r");
	return;
    }

    af.where     = TO_ROOM_CONST;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 25;
    af.location  = APPLY_ROOM_HEAL;
    af.modifier  = level;
    af.bitvector = 0;
    room->affectTo( &af );

    af2.where     = TO_AFFECTS;
    af2.type      = sn;
    af2.level	 = level;
    af2.duration  = level / 10;
    af2.modifier  = 0;
    af2.location  = APPLY_NONE;
    af2.bitvector = 0;
    affect_to_char( ch, &af2 );
    ch->send_to("������� ���������� ������������ ������.\n\r");
    act_p("$c1 �������� ������� ������������ ������.",
           ch,0,0,TO_ROOM,POS_RESTING);
    return;

}

AFFECT_DECL(HealingLight);
VOID_AFFECT(HealingLight)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "��� ������ �������� ������������ ������, ������� ������� "
                   "�������������� �������� �� {W%2$d{x � ������� {W%1$d{x ��%1$I��|���|���.",
		   paf->duration, paf->modifier )
	<< endl;
}

SPELL_DECL(HolyWord);
VOID_SPELL(HolyWord)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *vch;
    Character *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;

    bless_num = gsn_bless;
    curse_num = gsn_curse;
    frenzy_num = gsn_frenzy;

    act_p("$c1 ���������� ���������� {W������������ ����{x!",
           ch,0,0,TO_ROOM,POS_RESTING);
    ch->send_to("�� ����������� ���������� {W������������ ����{x!\n\r");

    for ( vch = room->people; vch != 0; vch = vch_next )
    {
	vch_next = vch->next_in_room;
	
	if (vch->is_mirror() && number_percent() < 50) 
	    continue;

	if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
	    (IS_EVIL(ch) && IS_EVIL(vch)) ||
	    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) )
	{
	    if (spellbane( ch, vch ))
		continue;

	  vch->send_to("�� ���������� ���� ����� �������������.\n\r");
	  spell(frenzy_num,level,ch, vch);
	  spell(bless_num,level,ch, vch);
	}

	else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
		 (IS_EVIL(ch) && IS_GOOD(vch)) )
	{
          if (!is_safe_spell(ch,vch,true))
          {
	    if (ch->fighting != vch && vch->fighting != ch)
		yell_panic( ch, vch );

            spell(curse_num,level,ch, vch);
            vch->send_to("������������ ���� ��������� ����!\n\r");
            dam = dice(level,6);
            damage(ch,vch,dam,sn,DAM_ENERGY, true, DAMF_SPELL);
          }
        }

	else if (IS_NEUTRAL(ch))
	{
          if (!is_safe_spell(ch,vch,true))
          {
	    if (ch->fighting != vch && vch->fighting != ch)
		yell_panic( ch, vch );

            spell(curse_num,level/2,ch, vch);
            vch->send_to("������������ ���� ��������� ����!\n\r");
            dam = dice(level,4);
            damage(ch,vch,dam,sn,DAM_ENERGY, true, DAMF_SPELL);
	  }
	}
    }

    ch->send_to("�� ���������� ���� �����������.\n\r");
/*    gain_exp( ch, -1 * number_range(1,10) * 5);           */
    ch->move /= (4/3);
    ch->hit /= (4/3);
}


SPELL_DECL(Inspire);
VOID_SPELL(Inspire)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *gch;
    Affect af;

    for( gch=room->people; gch != 0; gch=gch->next_in_room )
    {
	    if( !is_same_group( gch, ch) )
		    continue;

	    if (spellbane( ch, gch ))
		continue;

	    if ( gch->isAffected(sn ) )
	    {
	      if(gch == ch)
		  ch->pecho("�� ��� ����������%G��|�|��.", ch);
	      else
		  ch->pecho("%1$^C1 ��� ����������%1$G��|�|��.", gch);
	      continue;
	    }

	    af.where     = TO_AFFECTS;
	    af.type      = sn;
	    af.level     = level;
	    af.duration  = 6 + level;
	    af.location  = APPLY_HITROLL;
	    af.modifier  = level/12;
	    af.bitvector = 0;
	    affect_to_char( gch, &af );

	    af.location  = APPLY_SAVING_SPELL;
	    af.modifier  = 0 - level/12;
	    affect_to_char( gch, &af );

	    gch->send_to("�� ���������� �������������!\n\r");
	    if( ch != gch )
		act_p( "�� ������������� $C4 ����� ���������!", ch, 0, gch, TO_CHAR,POS_RESTING);

    }
}

SPELL_DECL(RayOfTruth);
VOID_SPELL(RayOfTruth)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam, align;

    if (IS_EVIL(ch) )
    {
	victim = ch;
	ch->send_to("������� ���������� ������ ����!\n\r");
    }

    if (victim != ch)
    {
	act_p("$c1 ���������� ������, ������� ������������� ��� �����!",
	       ch,0,0,TO_ROOM,POS_RESTING);
	ch->send_to("�� ����������� ������, ������� ������������� ��� �����!\n\r");
    }

    if (IS_GOOD(victim))
    {
	act_p("������������� ��� ����� �� ����� ��������� $c3.",
               victim,0,victim,TO_ROOM,POS_RESTING);
	victim->send_to("������������� ��� ����� �� ����� ��������� ����.\n\r");
	return;
    }

    dam = dice( level, 10 );

	if( ch->getTrueProfession( ) == prof_paladin ||
        ch->getTrueProfession( ) == prof_anti_paladin )
		dam = dam + dam / 2;

    if ( saves_spell( level, victim,DAM_HOLY, ch, DAMF_SPELL) )
	dam /= 2;
    
    if (victim->is_npc( ))
	align = victim->alignment;
    else if (IS_NEUTRAL(victim))
	align = 0;
    else
	align = -1000;
	
    align -= 350;

    if (align < -1000)
	align = -1000 + (align + 1000) / 3;

    dam = (dam * align * align) / 750000;
    
    if (!IS_AFFECTED(victim, AFF_BLIND))
	spell(gsn_blindness, 3 * level / 4, ch, victim);

    damage( ch, victim, dam, sn, DAM_HOLY ,true, DAMF_SPELL);
}


SPELL_DECL(RestoringLight);
VOID_SPELL(RestoringLight)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int mana_add;

    if (IS_AFFECTED(victim,AFF_BLIND))
	{
	 spell(gsn_cure_blindness,level,ch,victim);
	}
    if (IS_AFFECTED(victim,AFF_CURSE))
	{
	 spell(gsn_remove_curse,level,ch,victim);
	}
    if (IS_AFFECTED(victim,AFF_POISON))
	{
	 spell(gsn_cure_poison,level,ch,victim);
	}
    if (IS_AFFECTED(victim,AFF_PLAGUE))
	{
	 spell(gsn_cure_disease,level,ch,victim);
	}

    if (victim->hit != victim->max_hit)
	{
    	 mana_add = min( (victim->max_hit - victim->hit), (int)ch->mana );
    	 victim->hit = min( victim->hit + mana_add, (int)victim->max_hit );
    	 ch->mana -= mana_add;
	}
    update_pos( victim );
    victim->send_to("����� ����� ��������� ���� ����.\n\r");
    if ( ch != victim )
	ch->send_to("Ok.\n\r");
    return;

}


SPELL_DECL(SanctifyLands);
VOID_SPELL(SanctifyLands)::run( Character *ch, Room *room, int sn, int level ) 
{ 
  if (number_bits(1) == 0)
    {
      ch->send_to("���� ������� ����������� ��������.\n\r");
      return;
    }

  if (IS_RAFFECTED(room,AFF_ROOM_CURSE))
	{
	 room->affectStrip( gsn_cursed_lands);
	 ch->send_to("��� ����� ��������� �� ���������.\n\r");
	 act_p("��� ����� ��������� �� ���������.\n\r",
                ch,0,0,TO_ROOM,POS_RESTING);
	}
  if (IS_RAFFECTED(room,AFF_ROOM_POISON))
	{
	 room->affectStrip( gsn_deadly_venom);
	 ch->send_to("�������� ����, ���������� ��� �����, ������������.\n\r");
	 act_p("�������� ����, ���������� ��� �����, ������������.\n\r",
                ch,0,0,TO_ROOM,POS_RESTING);
	}
  if (IS_RAFFECTED(room,AFF_ROOM_SLEEP))
	{
	 ch->send_to("��� ����� ������������ �� ������������� ���.\n\r");
	 act_p("��� ����� ������������ �� ������������� ���.\n\r",
                ch,0,0,TO_ROOM,POS_RESTING);
	 room->affectStrip( gsn_mysterious_dream);
	}
  if (IS_RAFFECTED(room,AFF_ROOM_PLAGUE))
	{
	 ch->send_to("��� ����� ��������� �� ��������.\n\r");
	 act_p("��� ����� ��������� �� ��������.\n\r",
                ch,0,0,TO_ROOM,POS_RESTING);
	 room->affectStrip( gsn_black_death);
	}
  if (IS_RAFFECTED(room,AFF_ROOM_SLOW))
	{
	 ch->send_to("������������� �����, ���������� ��� �����, ������������.\n\r");
	 act_p("������������� �����, ���������� ��� �����, ������������.\n\r",
                ch,0,0,TO_ROOM,POS_RESTING);
	 room->affectStrip( gsn_lethargic_mist);
	}
    return;

}

SPELL_DECL(Wrath);
VOID_SPELL(Wrath)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;
    Affect af;

    if ( !ch->is_npc() && IS_EVIL(ch) )
	victim = ch;

    if ( IS_GOOD(victim) ) {
	act_p( "������������ ���� �������� $c4.", victim, 0, 0, TO_ROOM,POS_RESTING);
	act_p( "������������ ���� �������� ����.", victim, 0, 0, TO_CHAR,POS_RESTING);
	return;
    }

    if ( IS_NEUTRAL(victim) ) {
	act_p( "��� ���������� �� ��������� �� $C4.", ch, 0, victim, TO_CHAR,POS_RESTING );
	return;
    }

    dam = dice(level,14);

    if ( saves_spell( level, victim, DAM_HOLY,ch, DAMF_SPELL ) )
	dam /= 2;

    if (!IS_AFFECTED(victim, AFF_CURSE) && !saves_spell( level, victim, DAM_HOLY, ch, DAMF_SPELL ) )
    {
	af.where	 = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 2*level;
	af.location  = APPLY_HITROLL;
	af.modifier  = -1 * (level / 8);
	af.bitvector = AFF_CURSE;
	affect_to_char( victim, &af );

	af.location  = APPLY_SAVING_SPELL;
	af.modifier  = level / 8;
	affect_to_char( victim, &af );

	victim->send_to("�� ���������� ���� �������������.\n\r");

	if ( ch != victim )
	    act_p("$C1 �������� �������������.",ch,0,victim,TO_CHAR,POS_RESTING);
    }

    damage( ch, victim, dam, sn, DAM_HOLY, true, DAMF_SPELL );
}
