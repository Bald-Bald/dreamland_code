
/* $Id: group_other.cpp,v 1.1.2.7 2009/08/16 02:50:31 rufina Exp $
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
#include "group_other.h"
#include "spelltemplate.h"
#include "skillcommandtemplate.h"

#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"

#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "gsn_plugin.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"

/*
 * ExoticSkill
 */
bool ExoticSkill::visible( Character * ) const
{
    return false;
}
bool ExoticSkill::available( Character * ) const
{
    return false;
}
bool ExoticSkill::usable( Character *, bool ) const
{
    return false;
}
int ExoticSkill::getLearned( Character *ch ) const
{
    return ch->getRealLevel( ) * 3;
}


/*
 * misc spells
 */


SPELL_DECL(GeneralPurpose);
VOID_SPELL(GeneralPurpose)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_PIERCE ,true);
    return;

}


SPELL_DECL(HighExplosive);
VOID_SPELL(HighExplosive)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    dam = number_range( 30, 120 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_PIERCE ,true);
    return;

}

SPELL_DECL(Sebat);
VOID_SPELL(Sebat)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  Affect af;

  if ( ch->isAffected(sn ) )
    {
      ch->send_to("��������� �������������� ������ �������.\n\r" );
      return;
    }
  af.where		= TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location  = APPLY_AC;
  af.modifier  = -30;
  af.bitvector = 0;
  affect_to_char( ch, &af );
  act_p( "������������ ��� �������� $c4.",ch, 0,0,TO_ROOM,POS_RESTING);
  ch->send_to("������������ ��� �������� ����.\n\r");
  return;

}

SPELL_DECL(Matandra);
VOID_SPELL(Matandra)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  
  int dam;

  if ( ch->isAffected(sn ) )
    {
      ch->send_to("��������� �������������� ��� ���� �� ���� ������ �������.\n\r" );
      return;
    }

  postaffect_to_char( ch, sn, 5 );

  dam = dice(level, 7);
  damage(ch,victim,dam,sn,DAM_HOLY, true);
}

SPELL_DECL(Kassandra);
VOID_SPELL(Kassandra)::run( Character *ch, Character *, int sn, int level ) 
{ 
    if ( ch->isAffected(sn ) )
      {
	ch->send_to("�� ������ ������� ����������� ���� �����������.\n\r");
	return;
      }

    postaffect_to_char( ch, sn, 5 );

    ch->hit = min( ch->hit + 150, (int)ch->max_hit );
    update_pos( ch );

    ch->send_to("����� ����� ��������� ���� ����.\n\r");
    act_p("$c1 �������� �����.", ch, 0, 0, TO_ROOM,POS_RESTING);
}

SPELL_DECL(DragonStrength);
VOID_SPELL(DragonStrength)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;

  if (ch->isAffected(sn))
    {
      ch->send_to("���� ������� ��� ����������� ����.\n\r");
      return;
    }

  af.where		= TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = ch->getModifyLevel() / 3;
  af.bitvector = 0;

  af.modifier = max( ch->getModifyLevel() / 10, 2 );
  af.location = APPLY_HITROLL;
  affect_to_char(ch, &af);

  af.modifier = max( ch->getModifyLevel() / 10, 2 );
  af.location = APPLY_DAMROLL;
  affect_to_char(ch, &af);

  af.modifier = -max( ch->getModifyLevel() / 10, 2 );
  af.location = APPLY_AC;
  affect_to_char(ch, &af);

  af.modifier = max( ch->getModifyLevel() / 30, 1 );
  af.location = APPLY_STR;
  affect_to_char(ch, &af);

  af.modifier = -max( ch->getModifyLevel() / 30, 1 );;
  af.location = APPLY_DEX;
  affect_to_char(ch, &af);

  ch->send_to("���� ������� ����������� ����.\n\r");
  act_p("$c1 ���������� �������.", ch, 0, 0, TO_ROOM,POS_RESTING);

}

