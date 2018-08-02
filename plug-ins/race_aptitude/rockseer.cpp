/* $Id: rockseer.cpp,v 1.1.2.2 2008/03/04 07:24:12 rufina Exp $
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
#include "skillmanager.h"
#include "spelltemplate.h"

#include "pcharacter.h"
#include "affect.h"
#include "dreamland.h"
#include "magic.h"
#include "handler.h"
#include "fight.h"
#include "merc.h"
#include "act.h"
#include "def.h"

SPELL_DECL(MeldIntoStone);
VOID_SPELL(MeldIntoStone)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  
  Affect af;

  if ( victim->isAffected(sn ) )
    {
      if (victim == ch)
       	ch->send_to("���� ���� ��� ������� �����.\n\r");
      else
       	act_p("���� $C2 ��� ������� �����.",ch,0,victim,TO_CHAR,POS_RESTING);
      return;
    }
  af.where	= TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level / 5;
  af.location  = APPLY_AC;
  af.modifier  = -100;
  af.bitvector = 0;
  affect_to_char( victim, &af );
  act_p( "���� $c2 ������������, ��������� �������� �����.",
          victim,0,0,TO_ROOM,POS_RESTING);
  victim->send_to("���� ���� ������������, ��������� �������� �����.\n\r");
  return;

}

