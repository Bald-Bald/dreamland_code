
/* $Id: group_enhancement.cpp,v 1.1.2.11.6.6 2010-09-01 21:20:45 rufina Exp $
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
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "handler.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"


SPELL_DECL(GiantStrength);
VOID_SPELL(GiantStrength)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( victim->isAffected(sn ) )
    {
	if (victim == ch)
	  ch->send_to("�� �� ������ ���� ��� �������!\n\r");
	else
	  act_p("$C1 �� ����� ���� ��� �������.",ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if (IS_AFFECTED(victim,AFF_WEAKEN))
    {
	if (checkDispel(level,victim, gsn_weaken))
	    return;
	
	if (victim != ch)
	    ch->send_to("���� ������� ����������� ��������.\n\r");

	victim->send_to("�������� ��������... �� ���� �� ���������.\n\r");
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = (10 + level / 3);
    af.location  = APPLY_STR;
    af.modifier  = max(2,level / 10);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    victim->send_to("�� ����������� ������� �������!\n\r");
    act_p("$c1 ���������� ������� �������.",
           victim,0,0,TO_ROOM,POS_RESTING);
    return;

}

SPELL_DECL(Haste);
VOID_SPELL(Haste)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (victim->isAffected(sn) || IS_QUICK(victim))
    {
	if (victim == ch)
	  ch->send_to("�� �� ������ ��������� �������, ��� ������!\n\r");
	else
	  act_p("$C1 �� ����� ��������� ��� �������.",
	         ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (checkDispel(level,victim, gsn_slow))
	    return;
	
	if (victim != ch)
	    ch->send_to("���� ������� ����������� ��������.\n\r");

	victim->send_to("���� �������� ���������� �������... �� ���� �� ���������.\n\r");
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/2;
    else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = max(2,level / 12 );
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    victim->send_to("���� �������� ���������� ������� �������.\n\r");
    act_p("�������� $c2 ���������� ������� �������.",
           victim,0,0,TO_ROOM,POS_RESTING);
    if ( ch != victim )
	ch->send_to("Ok.\n\r");
    return;

}




SPELL_DECL(Infravision);
VOID_SPELL(Infravision)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
	if (victim == ch)
	  ch->send_to("�� ��� ������ � �������.\n\r");
	else
	  act_p("$C1 ��� ����� � �������.\n\r",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }
    act_p( "����� $c2 ���������� ������� ������.\n\r",
            victim, 0, 0, TO_ROOM,POS_RESTING);

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    victim->send_to("���� ����� ���������� ������� ������.\n\r");
    return;

}

SPELL_DECL(Learning);
VOID_SPELL(Learning)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect	af;

    if ( victim->is_npc() )
    {
        ch->send_to("��� ��� �� �������.\n\r");
        return;
    }

    if( victim->isAffected(gsn_learning) ) 
    {
        if (victim == ch)
            ch->send_to("���� �� ������.\n\r");
        else
            act_p("$C1 ��� ������.\n\r", ch,0,victim,TO_CHAR,POS_RESTING);
        return;
  }

  af.where	= TO_AFFECTS;
  af.type	= sn;
  af.level	= level;
  af.duration	= level / 10 + 1;
  af.location	= APPLY_NONE;
  af.modifier	= 0;
  af.bitvector	= 0;
  affect_to_char( victim, &af );
    
  victim->send_to("�� ���������������� �� �����.\n\r");

  if (ch != victim)
      act_p("$C1 ����� ������� �����!\n\r", ch,0,victim,TO_CHAR,POS_RESTING);

}
