
/* $Id: group_healing.cpp,v 1.1.2.13.6.4 2008/05/21 08:15:31 rufina Exp $
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

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"



SPELL_DECL(Aid);
VOID_SPELL(Aid)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( ch->isAffected(sn ) )
      {
	ch->send_to("��� ���������� �������������� ������ �������.\n\r");
	return;
      }

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 50;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( ch, &af );

    victim->hit += level * 5;
    update_pos( victim );
    victim->send_to("����� ����� ��������� ���� ����.\n\r");
    act_p("$c1 �������� �����.", victim, 0, 0, TO_ROOM,POS_RESTING);
    if (ch != victim) ch->send_to("Ok.\n\r");
    return;

}


SPELL_DECL(Assist);
VOID_SPELL(Assist)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	
	Affect af;

	if ( ch->isAffected(sn ) )
	{
		ch->send_to("��� ���������� �������������� ������ �������.\n\r");
		return;
	}

	af.where	 = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 50;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char( ch, &af );

	victim->hit += 100 + level * 5;
	update_pos( victim );
	victim->send_to("����� ����� ��������� ���� ����.\n\r");
	act_p("$c1 �������� �����.", victim, 0, 0, TO_ROOM,POS_RESTING);
	if ( ch != victim )
		ch->send_to("Ok.\n\r");
	return;

}

SPELL_DECL(Refresh);
VOID_SPELL(Refresh)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    victim->move = min( victim->move + level, (int)victim->max_move );
    if (victim->max_move == victim->move)
    {
        act("�� ���$g��|��|�� ���!", victim, 0, 0, TO_CHAR);
    }
    else
	victim->send_to("��������� ��������.\n\r");

    if ( ch != victim )
	ch->send_to("Ok.\n\r");
}




SPELL_DECL(CureLight);
VOID_SPELL(CureLight)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int heal;

    heal = dice(1, 8) + level / 4 + 5;
    
    victim->hit = min( victim->hit + heal, (int)victim->max_hit );
    update_pos( victim );
    victim->send_to("�� ���������� ���� ������ �����!\n\r");

    if ( ch != victim )
	ch->send_to("Ok.\n\r");
}

SPELL_DECL(CureSerious);
VOID_SPELL(CureSerious)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int heal;

    heal = dice(2, 8) + level / 3 + 10;

    victim->hit = min( victim->hit + heal, (int)victim->max_hit );
    update_pos( victim );
    victim->send_to("�� ���������� ���� �����!\n\r");

    if ( ch != victim )
	ch->send_to("Ok.\n\r");
}

SPELL_DECL(CureCritical);
VOID_SPELL(CureCritical)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int heal;
    
    heal = dice(3, 8) + level / 2 + 20;
    
    victim->hit = min( victim->hit + heal, (int)victim->max_hit );
    update_pos( victim );
    victim->send_to("�� ���������� ���� ������� �����!\n\r");

    if ( ch != victim )
	ch->send_to("Ok.\n\r");
}


SPELL_DECL(Heal);
VOID_SPELL(Heal)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
//    victim->hit = min( victim->hit + 100 + level / 10, victim->max_hit );

    victim->hit += level * 2 + number_range( 40, 45 ); 
    victim->hit = min( victim->hit, victim->max_hit );
    
    update_pos( victim );
    victim->send_to("����� ����� ��������� ���� ����.\n\r");

    if ( ch != victim )
	ch->send_to("Ok.\n\r");
}

SPELL_DECL(SuperiorHeal);
VOID_SPELL(SuperiorHeal)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
//    victim->hit += 170 + level + dice(1,20);

    victim->hit += level * 3 + number_range( 100, 120 ); 
    victim->hit = min( victim->hit, victim->max_hit );

    update_pos( victim );
    victim->send_to("����� ����� ��������� ����.\n\r");

    if ( ch != victim )
	ch->send_to("Ok.\n\r");
}


SPELL_DECL(MasterHealing);
VOID_SPELL(MasterHealing)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
//  victim->hit += 300 + level + dice(1, 40);
  
    victim->hit += level * 5 + number_range( 60, 100 ); 
    victim->hit = min( victim->hit, victim->max_hit );

    update_pos( victim );
    victim->send_to("����� ����� ��������� ���� ����.\n\r");

    if ( ch != victim )
	ch->send_to("Ok.\n\r");
}


SPELL_DECL(MassHealing);
VOID_SPELL(MassHealing)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *gch;

    for ( gch = room->people; gch != 0; gch = gch->next_in_room )
    {
	if ((ch->is_npc() && gch->is_npc()) ||
	    (!ch->is_npc() && !gch->is_npc()))
	{
	    if (spellbane( ch, gch ))
		continue;

	    spell(gsn_heal,level,ch, gch);
	    spell(gsn_refresh,level,ch, gch);
	}
    }

}

SPELL_DECL(GroupHeal);
VOID_SPELL(GroupHeal)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *gch;
    int heal_sn;

    if (gsn_master_healing->usable( ch ))
	heal_sn = gsn_master_healing;
    else if (gsn_superior_heal->usable( ch ))
	heal_sn = gsn_superior_heal;
    else
	heal_sn = gsn_heal;

    for ( gch = room->people; gch != 0; gch = gch->next_in_room )
    {
	if( !is_same_group( gch, ch ) )
		continue;

	if (spellbane( ch, gch ))
	    continue;
	
	spell(heal_sn,level,ch, gch);
	spell(gsn_refresh,level,ch, gch);
    }

}

SPELL_DECL(EmpathicHealing);
VOID_SPELL(EmpathicHealing)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af, *paf;
    int hp;
    bool removed = false;

    if (ch == victim) {
	ch->send_to( "��� ���������� �� ������ ������������ ������ �� ������.\r\n" );
	return;
    }

    if (ch->isAffected(sn )) {
	ch->send_to( "��� ���������� �������������� ������ �������.\r\n" );
	return;
    }

    if (IS_AFFECTED(victim, AFF_PLAGUE)
	&& (paf = victim->affected->affect_find(gsn_plague)) != NULL)
    {
        affect_join( ch, paf );
	ch->send_to( "�� ���������� ��� � ���������.\r\n" );
	removed = true;
        affect_strip( victim, gsn_plague );
    }

    if ( IS_AFFECTED(victim, AFF_POISON)
	&& (paf = victim->affected->affect_find(gsn_poison)) != NULL)
    {
        affect_join( ch, paf );
	ch->send_to( "�� ���������� ���� ����� ����������.\r\n" );
	removed = true;
        affect_strip( victim, gsn_poison );
    }
   
    af.where            = TO_AFFECTS;
    af.type             = sn;
    af.level            = level;
    af.location         = APPLY_NONE;
    af.modifier         = 0;

    if (!removed && victim->max_hit == victim->hit) {
	act_p( "�������, $C1 ��������� �����$G��|�|��", ch, 0, victim, TO_CHAR, POS_RESTING);
	af.duration = 1;
    
    } else {
	act_p( "����������������, �� ���������� ���� $C2 �� ����������� ����.", ch, 0, victim, TO_CHAR, POS_RESTING);
	act_p( "����������������, $c1 ��������� ���� ���� �� ����������� ����.", ch, 0, victim, TO_VICT, POS_RESTING);
	act_p( "����������������, $c1 ��������� ���� $C2 �� ����������� ����.", ch, 0, victim, TO_NOTVICT, POS_RESTING);

	hp = victim->max_hit - victim->hit;
	hp = URANGE( 0, hp, ch->hit - 1 );

	victim->hit += hp;
	ch->hit     -= hp;
	update_pos( victim );

	af.duration         = hp / 100;
	af.bitvector        = AFF_REGENERATION;
    }

    affect_join( ch, &af );

}

