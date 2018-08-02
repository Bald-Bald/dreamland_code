
/* $Id: group_protective.cpp,v 1.1.2.19.6.15 2010-09-01 21:20:45 rufina Exp $
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
#include "profflags.h"

#include "affecthandler.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "clanreference.h"

#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "def.h"


CLAN(none);
GSN(stardust);
GSN(dispel_affects);

static inline bool has_sanctuary_msg( Character *ch, Character *victim )
{
    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
	if (victim == ch)
	    act("�� ��� ��� ������� ���������.", ch, 0, 0, TO_CHAR);
	else
	    act("$C1 ��� ��� ������� ���������.", ch, 0, victim, TO_CHAR);
	return true;
    }

    if (victim->isAffected(gsn_dark_shroud)) {
	if (victim == ch)
	    act("�� ��� ��� ������� ������ �����.", ch, 0, 0, TO_CHAR);
	else
	    act("$C1 ��� ��� ������� ������ �����.", ch, 0, victim, TO_CHAR);
	return true;
    }

    if (victim->isAffected(gsn_stardust)) {
	if (victim == ch)
	    act("�������� ���� ��� �������� ������ ����.", ch, 0, 0, TO_CHAR);
	else
	    act("�������� ���� ��� �������� ������ $C2.", ch, 0, victim, TO_CHAR);
	return true;
    }

    return false;
}

SPELL_DECL(Armor);
VOID_SPELL(Armor)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (victim->isAffected(sn)) {
	if (victim == ch)
	    act("�� ��� ������$g��|�|�� ����������� �����.", ch, 0, 0, TO_CHAR);
	else
	    act("$C1 ��� ������$G��|�|�� ����������� �����.", ch, 0, victim, TO_CHAR);
	return;
    }

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 7 + level / 6;
    af.modifier  = -1 * max(20,10 + level / 4); /* af.modifier  = -20;*/
    af.location  = APPLY_AC;
    affect_to_char( victim, &af );
    
    if (ch->getTrueProfession( )->getFlags( ch ).isSet(PROF_DIVINE)) {
	act("��������� ����� �������� ����.", victim, 0, 0, TO_CHAR);
	if (ch != victim)
	    act("��������� ����� �������� $C4.", ch, 0, victim, TO_CHAR);
    } else {
	act("��������� ����� �������� ����.", victim, 0, 0, TO_CHAR);
	if (ch != victim)
	    act("��������� ����� �������� $C4.", ch, 0, victim, TO_CHAR);
    }
}


SPELL_DECL(BarkSkin);
VOID_SPELL(BarkSkin)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( ch->isAffected(sn ) )
    {
	if (victim == ch)
	  ch->send_to("���� ���� �� ����� ����� ��� �������.\n\r");
	else
	  act_p("���� $C2 �� ����� ����� ��� �������.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -( int )(level * 1.5);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act_p( "���� $c2 ����������� �����.",
            victim, 0, 0, TO_ROOM,POS_RESTING);
    victim->println("������� �������� ���� ��������� ���� ����.");
    return;

}

enum {
    CANCEL_ALWAYS,
    CANCEL_DISPEL,
    CANCEL_NEVER
};
static int can_cancel( Character *ch, Character *victim )
{
    if (ch->is_npc( ) && victim->is_npc( )) {
	if (!is_same_group( ch, victim ))
	    return CANCEL_DISPEL;
	
	return CANCEL_ALWAYS;
    }

    if (!ch->is_npc( ) && !victim->is_npc( )) {
	if (ch == victim)
	    return CANCEL_ALWAYS;

	if (ch->is_immortal( ))
	    return CANCEL_ALWAYS;

	if (!IS_SET(victim->add_comm, PLR_NOCANCEL))
	    return CANCEL_ALWAYS;

	if (ch->getClan( ) != victim->getClan( ))
	    return CANCEL_DISPEL;

	if (ch->getClan( )->isDispersed( ))
	    return CANCEL_DISPEL;

	return CANCEL_ALWAYS;
    }

    if (ch->is_npc( ) && !victim->is_npc( )) {
	return CANCEL_NEVER;
    }

    if (victim->getNPC( )->behavior
	&& !victim->getNPC( )->behavior->canCancel( ch ))
	return CANCEL_NEVER;

    return CANCEL_ALWAYS;
}


SPELL_DECL(Cancellation);
VOID_SPELL(Cancellation)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    AffectHandler::Pointer affect;
    bool found = false;

    switch (can_cancel( ch, victim )) {
    case CANCEL_ALWAYS:
	    break;
    case CANCEL_NEVER:
	    ch->println("���� ������� ����������� ��������, �������� dispel affects.");
	    return;
    case CANCEL_DISPEL:
	    if (!is_safe_spell( ch, victim, false ))	
		spell(gsn_dispel_affects, level, ch, victim);
	    return;
    }

    level += 2;
    
    /* unlike dispel affects, the victim gets NO save */

    for (int sn = 0; sn < skillManager->size( ); sn++) {
	affect = skillManager->find( sn )->getAffect( );

	if (affect && affect->isCancelled( ))
	    if (checkDispel( level, victim, sn )) 
		found = true;
    }

    if (found)
	ch->send_to("Ok.\n\r");
    else
	ch->send_to("���� ������� ����������� ��������.\n\r");
}


SPELL_DECL(DarkShroud);
VOID_SPELL(DarkShroud)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;
    
    if (has_sanctuary_msg( ch, victim ))
	return;

    if (IS_GOOD(victim)) // Not for good !!!
    {
       if (victim == ch)
	  act("������ ���� �� ����� �������� ����!!!", ch, 0, 0, TO_CHAR);
       else
       	  act("������ ���� �� ����� �������� $C4!!!", ch, 0, victim, TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    affect_to_char( victim, &af );
    act("{D������ ����{x �������� $c4.", victim, 0, 0, TO_ROOM);
    act("{D������ ����{x �������� ����.", victim, 0, 0, TO_CHAR);
}


SPELL_DECL(DispelAffects);
VOID_SPELL(DispelAffects)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    AffectHandler::Pointer affect;
    bool found = false;
    
    if (IS_AFFECTED(ch, AFF_CHARM))
	return;

    if (saves_spell(level, victim,DAM_OTHER, ch, DAMF_SPELL)) {
	victim->send_to("�� ���������� ������ ���� � ����.\n\r");
	ch->send_to("���� ������� ����������� ��������.\n\r");
	return;
    }

    for (int sn = 0; sn < skillManager->size( ); sn++) {
	affect = skillManager->find( sn )->getAffect( );

	if (affect && affect->isDispelled( ))
	    if (checkDispel( level, victim, sn )) 
		found = true;
    }

    if (found)
	ch->send_to("Ok.\n\r");
    else
	ch->send_to("���� ������� ����������� ��������.\n\r");
}


SPELL_DECL(DragonSkin);
VOID_SPELL(DragonSkin)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  
  Affect af;

  if ( victim->isAffected(sn ) )
    {
      if (victim == ch)
       	ch->send_to("���� ���� ��� ������, ��� ��������.\n\r");
      else
       	act_p("���� $C2 ��� ������, ��� ��������.",
               ch,0,victim,TO_CHAR,POS_RESTING);
      return;
    }
  af.where	= TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_AC;
  af.modifier  = - (2 * level);
  af.bitvector = 0;
  affect_to_char( victim, &af );
  act_p( "���� $c2 ���������� ������ ���������.",
          victim,0,0,TO_ROOM,POS_RESTING );
  victim->send_to("���� ���� ���������� ������ ���������.\n\r");
  return;

}

SPELL_DECL(EnhancedArmor);
VOID_SPELL(EnhancedArmor)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( victim->isAffected(sn ) )
    {
	if (victim == ch)
	  ch->send_to("������� ���� ��� �������� ����.\n\r");
	else
	  act_p("������� ���� ��� �������� $C4.",ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.modifier  = -60;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    victim->send_to("������� ������ �������� ����.\n\r");
    if ( ch != victim )
	act_p("������� ������ �������� $C4.",ch,0,victim,TO_CHAR,POS_RESTING);
    return;

}

SPELL_DECL(Fortitude);
VOID_SPELL(Fortitude)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (ch->isAffected(sn )) {
	act_p("�� ��� ����$g��|�|�� � ������� �� ����.", ch, 0, 0, TO_CHAR, POS_RESTING);
	return;
    }

    af.where = TO_RESIST;
    af.type = sn;
    af.duration = level / 10;
    af.level = ch->getModifyLevel();
    af.bitvector = RES_COLD|RES_NEGATIVE;
    af.location = 0;
    af.modifier = 0;
    affect_to_char(ch, &af);
    
    act_p("���� ������ ���� ���������������� � ���.", ch, 0, 0, TO_CHAR, POS_RESTING);

}



SPELL_DECL(SpellResistance);
VOID_SPELL(SpellResistance)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	Affect af;

	if (!ch->isAffected(sn))
	{
		ch->send_to("������ ���������� ��������� ���� ������� ����.\n\r");

		af.where = TO_RESIST;
		af.type = sn;
		af.duration = level / 10;
		af.level = ch->getModifyLevel();
		af.bitvector = RES_SPELL;
		af.location = 0;
		af.modifier = 0;
		affect_to_char(ch, &af);
	}
	else
		ch->send_to("�� ��� ������ ��� ������.\n\r");
	return;

}



SPELL_DECL(MassSanctuary);
VOID_SPELL(MassSanctuary)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *gch;
    Affect af;

    for( gch=room->people; gch != 0; gch=gch->next_in_room)
    {
	if (!is_same_group( gch, ch ))
	    continue;

	if (spellbane( ch, gch ))
	    continue;
	
	if (has_sanctuary_msg( ch, gch ))
	    continue;

	af.where     = TO_AFFECTS;
	af.type      = gsn_sanctuary;
	af.level     = level;
	af.duration  = number_fuzzy( level/6 );
	af.bitvector = AFF_SANCTUARY;
	affect_to_char( gch, &af );

	act("{W����� ����{x �������� ����.", gch, 0, 0, TO_CHAR);
	if (ch != gch)
	    act("{W����� ����{x �������� $C4.", ch, 0, gch, TO_CHAR);
    }
}


SPELL_DECL(ProtectionCold);
VOID_SPELL(ProtectionCold)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( victim->isAffected(gsn_protection_cold) )
    {
	if (victim == ch)
          act("�� ��� ������$g��|�|�� �� ������.", ch,0, 0,TO_CHAR);
	else
          act("$C1 ��� ������$G��|�|�� �� ������.", ch,0,victim,TO_CHAR);
	return;
    }

    if ( victim->isAffected(gsn_protection_heat) )
    {
	if (victim == ch)
          act("�� ��� ������$g��|�|�� �� ����.", ch,0, 0,TO_CHAR);
	else
          act("$C1 ��� ������$G��|�|�� �� ����.", ch,0,victim,TO_CHAR);
	return;
    }

    if ( victim->isAffected(gsn_make_shield) )
    {
	if (victim == ch)
          act("�� ��� ������$g��|�|�� ������� �����.", ch,0,0,TO_CHAR);
	else
          act("$C1 ��� ������$G��|�|�� ������� �����.", ch,0,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = gsn_protection_cold;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    victim->send_to("���� ������������ �� ����������� ������ ���������� ����������.\n\r");
    if ( ch != victim )
	act_p("������������ $C2 �� ����������� ������ ���������� ����������.",
              ch,0,victim,TO_CHAR,POS_RESTING);
    return;

}


SPELL_DECL(ProtectionEvil);
VOID_SPELL(ProtectionEvil)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL)
    ||   IS_AFFECTED(victim, AFF_PROTECT_GOOD))
    {
	if (victim == ch)
          act("�� ��� ������$g��|�|��.", ch,0, 0,TO_CHAR);
	else
	  act("$C1 ��� ������$G��|�|��.", ch,0,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (10 + level / 5);
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char( victim, &af );
    victim->send_to("�� ���������� ������ ������� ���.\n\r");
    if ( ch != victim )
	act_p("$C1 �������� ������ ������� ���.",
               ch,0,victim,TO_CHAR,POS_RESTING);
    return;

}


SPELL_DECL(ProtectionGood);
VOID_SPELL(ProtectionGood)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD)
    ||   IS_AFFECTED(victim, AFF_PROTECT_EVIL))
    {
	if (victim == ch)
          act("�� ��� ������$g��|�|��.", ch,0, 0,TO_CHAR);
	else
	  act("$C1 ��� ������$G��|�|��.", ch,0,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (10 + level / 5);
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_GOOD;
    affect_to_char( victim, &af );
    victim->send_to("�� ���������� ������ ������ ���.\n\r");
    if ( ch != victim )
	act_p("$C1 �������� ������ ������ ���.",
               ch,0,victim,TO_CHAR,POS_RESTING);
    return;

}

SPELL_DECL(ProtectionHeat);
VOID_SPELL(ProtectionHeat)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( victim->isAffected(gsn_protection_heat) )
    {
	if (victim == ch)
          act("�� ��� ������$g��|�|�� �� ����.", ch,0, 0,TO_CHAR);
	else
          act("$C1 ��� ������$G��|�|�� �� ����.", ch,0,victim,TO_CHAR);
	return;
    }

    if ( victim->isAffected(gsn_protection_cold) )
    {
	if (victim == ch)
          act("�� ��� ������$g��|�|�� �� ������.", ch,0, 0,TO_CHAR);
	else
          act("$C1 ��� ������$G��|�|�� �� ������.", ch,0,victim,TO_CHAR);
	return;
    }

    if ( victim->isAffected(gsn_make_shield) )
    {
	if (victim == ch)
          act("�� ��� ������$g��|�|�� �������� �����.", ch,0,0,TO_CHAR);
	else
          act("$C1 ��� ������$G��|�|�� �������� �����.", ch,0,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = gsn_protection_heat;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    victim->send_to("���� ������������ �� ����������� ������� ���������� ����������.\n\r");
    if ( ch != victim )
	act_p("������������ $C2 �� ����������� ������� ���������� ����������.",
               ch,0,victim,TO_CHAR,POS_RESTING);
    return;

}

SPELL_DECL(ProtectionNegative);
VOID_SPELL(ProtectionNegative)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  Affect af;

    if (!ch->isAffected(sn))
    {
      ch->send_to("�� ������������ ��������� � ���������� ������.\n\r");

      af.where = TO_IMMUNE;
      af.type = sn;
      af.duration = level / 4;
      af.level = ch->getModifyLevel();
      af.bitvector = IMM_NEGATIVE;
      af.location = 0;
      af.modifier = 0;
      affect_to_char(ch, &af);
    }
  else
      ch->send_to("� ���� ��� ���� ��������� � ���������� ������.\n\r");
 return;

}


SPELL_DECL(ProtectiveShield);
VOID_SPELL(ProtectiveShield)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  
  Affect af;

  if (victim->isAffected(sn)) {
      if (victim == ch)
       	ch->send_to("�������� ��� ��� �������� ����.\n\r");
      else
       	act_p("�������� ��� ��� �������� $C4.",ch,0,victim,TO_CHAR,POS_RESTING);
      return;
  }

  af.where	= TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy( level / 30 ) + 3;
  af.location  = APPLY_AC;
  af.modifier  = -20;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  if (chance(1)) {
      act_p( "����������������� ��� �������� $c4.",victim,0,0,TO_ROOM,POS_RESTING);
      victim->send_to("����������������� ��� �������� ����.\n\r");
  } else {
      act_p( "�������� ��� �������� $c4.",victim,0,0,TO_ROOM,POS_RESTING);
      victim->send_to("�������� ��� �������� ����.\n\r");
  }
}


SPELL_DECL(Resilience);
VOID_SPELL(Resilience)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (!ch->isAffected(sn)) {
      ch->println("�� ������������ ������������ � �������������� ������.");

      af.where = TO_RESIST;
      af.type = sn;
      af.duration = level / 10;
      af.level = ch->getModifyLevel();
      af.bitvector = RES_ENERGY;
      af.location = 0;
      af.modifier = 0;
      affect_to_char(ch, &af);
    }
  else
      ch->println("� ���� ��� ���� ������������ � �������������� ������.");

}

SPELL_DECL(Sanctuary);
VOID_SPELL(Sanctuary)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (has_sanctuary_msg( ch, victim ))
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    act("{W����� ����{x �������� $c4.", victim, 0, 0, TO_ROOM);
    act("{W����� ����{x �������� ����.", victim, 0, 0, TO_CHAR);
}

SPELL_DECL(Stardust);
VOID_SPELL(Stardust)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (has_sanctuary_msg( ch, victim ))
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    affect_to_char( victim, &af );
    act("��������� {W�{w��{W��{w��� {W�{w��� ����������� ������ $c2.", victim, 0, 0, TO_ROOM);
    act("��������� {W�{w��{W��{w��� {W�{w��� ����������� ������ ����.", victim, 0, 0, TO_CHAR);
}

SPELL_DECL(Shield);
VOID_SPELL(Shield)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( victim->isAffected(sn ) )
    {
	if (victim == ch)
	    act("�� ��� ������$g��|�|�� ����������� ����.", ch, 0, 0, TO_CHAR);
	else
	    act("$C1 ��� ������$G��|�|�� ����������� ����.", ch, 0, victim, TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (8 + level / 3);
    af.location  = APPLY_AC;
    af.modifier  = -1 * max(20,10 + level / 3); /* af.modifier  = -20;*/
    af.bitvector = 0;
    affect_to_char( victim, &af );

    if (ch->getTrueProfession( )->getFlags( ch ).isSet(PROF_DIVINE)) {
	act("������������ ������� �������� ���� �����.", victim, 0, 0, TO_CHAR);
	act("������������ ������� �������� $c4 �����.", victim, 0, 0, TO_ROOM);
    } else {
	act("��������� ��� �������� ����.", victim, 0, 0, TO_CHAR);
	act("��������� ��� �������� $c4.", victim, 0, 0, TO_ROOM);
    }
}


SPELL_DECL(StoneSkin);
VOID_SPELL(StoneSkin)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( ch->isAffected(sn ) )
    {
	if (victim == ch)
	  ch->send_to("���� ���� ��� ������ ��� ������.\n\r");
	else
	  act_p("���� $C2 ��� ������ ��� ������.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (10 + level / 5);
    af.location  = APPLY_AC;
    af.modifier  = -1 * max(40,20 + level / 2);  /*af.modifier=-40;*/
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act_p( "���� $c2 ���������� ������ �����.",
            victim, 0, 0, TO_ROOM,POS_RESTING);
    victim->send_to("���� ���� ���������� ������ �����.\n\r");
}
