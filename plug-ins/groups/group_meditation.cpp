
/* $Id: group_meditation.cpp,v 1.1.2.7.6.1 2008/03/04 07:24:12 rufina Exp $
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

#include "affect.h"
#include "pcharacter.h"
#include "room.h"

#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

SPELL_DECL(Link);
VOID_SPELL(Link)::run( Character *ch, Character *victim, int sn, int level )
{
    int random = number_percent( );
    
    ch->endur /= 2;
    victim->mana = victim->mana + (ch->mana / 2 + random) / 2;
    ch->mana = 0;
}

SPELL_DECL(MagicConcentrate);
VOID_SPELL(MagicConcentrate)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;

  if (ch->isAffected(sn)) {
      act("�� ��� ���������� �������������$g���|��|���.", ch, 0, 0, TO_CHAR);
      return;
  }

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 7;
  af.location           = APPLY_NONE;
  af.modifier           = 0;
  af.bitvector          = 0;
  affect_to_char(ch,&af);

  ch->send_to("�� ����������, ��� ����������� ����������� � ���������� ��������� ��� ���� ����.\n\r");
}

