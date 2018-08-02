/* $Id: fight_cmds.cpp,v 1.1.2.7 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * ��� ����� �� ���� ��� 'Dream Land' ����������� Igor {Leo} � Olga {Varda}*
 * ��������� ������ � ��������� ����� ����, � ����� ������ ������ ��������:*
 *    Igor S. Petrenko	    {NoFate, Demogorgon}                           *
 *    Koval Nazar	    {Nazar, Redrum}                 		   *
 *    Doropey Vladimir	    {Reorx}		                           *
 *    Kulgeyko Denis	    {Burzum}		                           *
 *    Andreyanov Aleksandr  {Manwe}		                           *
 *    � ��� ���������, ��� ��������� � ����� � ���� MUD	                   *
 ***************************************************************************/

#include "fleemovement.h"
#include "commandtemplate.h"
#include "skillcommand.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "act_move.h"
#include "interp.h"
#include "gsn_plugin.h"
#include "merc.h"
#include "stats_apply.h"
#include "mercdb.h"
#include "handler.h"
#include "fight.h"
#include "act.h"
#include "def.h"


PROF(samurai);


CMDRUN( kill )
{
    Character *victim;
    DLString arg, args = constArguments;

    arg = args.getOneArgument( );

    if (arg.empty( ))
    {
	ch->send_to("����� ����?\n\r");
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
	ch->send_to("����� ��� �����.\n\r");
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	ch->send_to("�� ������� ������ �� ����, ��� ������!\n\r");
	return;
    }

    if ( !victim->is_npc() )
    {
	ch->send_to("������� ������� � ������� MURDER.\n\r");
	return;
    }

    if ( victim == ch )
    {
	ch->send_to("{R�� ����� ����!{x ���...\n\r");
	multi_hit( ch, ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act_p( "�� $C1 ���� ������� ������!", ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }

     ch->setWaitViolence( 1 );

    
    if (gsn_mortal_strike->getCommand( )->run( ch, victim ))
	return;

    multi_hit( ch, victim );
}

CMDRUN( murder )
{
    Character *victim;
    DLString arg, args = constArguments;

    arg = args.getOneArgument( );

    if (arg.empty( ))
    {
	ch->send_to("�������� ����?\n\r");
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
	ch->send_to("����� ��� �����.\n\r");
	return;
    }

    if ( victim == ch )
    {
	ch->send_to("������������ - ��� ����������� ����.\n\r");
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act_p( "�� $C1 ���� ������� ������.", ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	ch->send_to("�� ������� ������ �� ����, ��� ������!\n\r");
	return;
    }

     ch->setWaitViolence( 1 );

//    if ( !victim->is_npc()
//	|| ( ch->is_npc() && victim->is_npc() ) )
    yell_panic( ch, victim,
		"��������! �� ���� ���-�� �����!",
		"��������! �� ���� ����%1$G��|�|�� %1$C1!",
		FYP_VICT_ANY );
    
    if (gsn_mortal_strike->getCommand( )->run( ch, victim ))
	return;

    multi_hit( ch, victim );
}



CMDRUN( flee )
{
    if (ch->fighting == 0) {
	if ( ch->position == POS_FIGHTING )
	    ch->position = POS_STANDING;

	ch->send_to("�� �� � ��� �� ����������.\n\r");
	return;
    }

    if (ch->getProfession( ) == prof_samurai
	&& ch->getRealLevel( ) > 10
	&& number_percent( ) < min( ch->getRealLevel( ) - 10, 90 ))
    {
	ch->send_to("��� ����� ������� ������� ������� ��� ����!\n\r");
	return;
    }

    FleeMovement( ch ).move( );
}

CMDRUN( slay )
{
    Character *victim;
    DLString arg, args = constArguments;

    arg = args.getOneArgument( );

    if (arg.empty( ))
    {
	ch->send_to("��������� ����?\n\r");
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
	ch->send_to("����� ��� �����.\n\r");
	return;
    }

    if ( ch == victim )
    {
	ch->send_to("������������ - ��� ����������� ����.\n\r");
	return;
    }

    if ( ( !ch->is_npc()
	    && !victim->is_npc()
	    && victim->getRealLevel( ) >= ch->get_trust( ) )
	|| ( ch->is_npc()
	    && !victim->is_npc()
	    && !victim->is_immortal( )
	    && victim->get_trust( ) >= ch->getRealLevel( ) ) )
    {
	ch->send_to("���� ������� ����������.\n\r");
	return;
    }

    act_p( "�� ������������ ����������� $C4!", ch, 0, victim, TO_CHAR,POS_RESTING);
    act_p( "$c1 ������������ ���������� ����!", ch, 0, victim, TO_VICT,POS_RESTING);
    act_p( "$c1 ������������ ���������� $C4!", ch, 0, victim, TO_NOTVICT,POS_RESTING);
    raw_kill( victim, -1, 0, FKILL_CRY|FKILL_GHOST|FKILL_CORPSE );
    if( !ch->is_npc() && !victim->is_npc() && ch != victim )
    {
	set_slain( victim );
    }
}




