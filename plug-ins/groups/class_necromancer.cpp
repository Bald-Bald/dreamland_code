/* $Id$
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

#include "playerattributes.h"

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"

#include "magic.h"
#include "fight.h"
#include "vnum.h"
#include "handler.h"
#include "effects.h"
#include "damage_impl.h"
#include "act.h"
#include "merc.h"
#include "interp.h"
#include "def.h"



SPELL_DECL(PowerWordKill);
VOID_SPELL(PowerWordKill)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	int dam;

	if ( victim->fighting )
	{
		ch->send_to("�� �� ������ ���������������.. ������ ������� ������ ��������.\n\r");
		return;
	}

	ch->setWait( skill->getBeats( ) );

	act_p("����� �����, ��������� �����, ��������� $C4.",
		ch, 0, victim, TO_CHAR, POS_RESTING);
	act_p("$c1 ������� ����� �����, �������� $C4.",
		ch, 0, victim, TO_NOTVICT, POS_RESTING);
	act_p("$C1 ������� ����� �����, �������� ����.",
		victim, 0, ch, TO_CHAR, POS_RESTING);

	if ( victim->is_immortal()
		|| saves_spell(level,victim,DAM_MENTAL, ch, DAMF_SPELL)
		|| number_percent () > 50 )
	{
		dam = dice( level , 24 ) ;
		damage(ch, victim , dam , sn, DAM_MENTAL, true, DAMF_SPELL);
		return;
	}

	victim->send_to("���� {R�����{x!\n\r");

	group_gain( ch, victim );
	raw_kill( victim, -1, ch, FKILL_CRY|FKILL_GHOST|FKILL_CORPSE );
	pk_gain( ch, victim );
}



SPELL_DECL(Insanity);
VOID_SPELL(Insanity)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	
	Affect af;

	if ( victim->is_npc() )
	{
		ch->send_to("��� ���������� ����� �������������� ������ ������ �������.\n\r");
		return;
	}

	if ( saves_spell( level, victim,DAM_OTHER, ch, DAMF_SPELL) )
	{
		ch->send_to("�� ����������...\n\r");	
		return;
	}

	if  (IS_AFFECTED(victim,AFF_BLOODTHIRST )) {
	    act("$C1 ��� ������ �����!", ch, 0, victim, TO_CHAR);
	    return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 10;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = AFF_BLOODTHIRST;
	affect_to_char( victim, &af );
	victim->send_to("������� ���������� ����!\n\r");
	act_p("����� $c2 ���������� ������.",victim,0,0,TO_ROOM,POS_RESTING);
	return;

}

