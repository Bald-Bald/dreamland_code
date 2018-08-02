
/* $Id: group_immortal.cpp,v 1.1.2.4 2008/03/04 07:24:12 rufina Exp $
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


SPELL_DECL(Amnesia);
VOID_SPELL(Amnesia)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	if (victim->is_npc())
		return;

	for (int i = 0; i < SkillManager::getThis( )->size(); i++) {
	    PCSkillData &data = victim->getPC( )->getSkillData( i );

	    if (data.learned.getValue( ) > 0) {
		data.learned /= 2;
		data.learned++;
	    }
	}

	act_p("�� ���������� ��� ������ ���������� �� ����.",
		victim,0,0,TO_CHAR,POS_RESTING);
	act_p("������ $c2 ���������� �������������.",
		victim,0,0,TO_ROOM,POS_RESTING);

}


