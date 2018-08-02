/* $Id: class_warrior.cpp,v 1.1.2.14.6.6 2008/05/27 21:30:03 rufina Exp $
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

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"
#include "handler.h"

#include "magic.h"
#include "fight.h"
#include "vnum.h"
#include "merc.h"
#include "act.h"
#include "interp.h"
#include "def.h"

GSN(inspiration);

/*
 * 'smithing' skill command
 */

SKILL_RUNP( smithing )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    Object *hammer;

    if ( ch->is_npc() || !gsn_smithing->usable( ch ))
    {
	ch->send_to("����?\n\r");
	return;
    }


    if ( ch->fighting )
    {
        ch->send_to( "������� ���� �������� ����������.\n\r");
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	ch->send_to("����� ���� �� ������ ������������?\n\r");
	return;
    }

    if (( obj = get_obj_carry(ch, arg)) == 0)
    {
	ch->send_to("� ���� ��� �����.\n\r");
	return;
    }

   if (obj->condition >= 100)
    {
	ch->send_to("�� ��� �� ����������.\n\r");
	return;
    }

    if (( hammer = get_eq_char(ch, wear_hold)) == 0)
    {
	ch->send_to("�� � ���� ��� ������� ������.\n\r");
	return;
    }

    if ( hammer->pIndexData->vnum != OBJ_VNUM_HAMMER )
    {
	ch->send_to("�� ��� �� �����.\n\r");
	return;
    }

    ch->setWaitViolence( 2 );

    if (ch->isAffected( gsn_inspiration )) {
	ch->pecho( "%1$^O1 ��%1$n��|�� ��� ������ ������, ������� ������������ ���.", obj );
	ch->recho( "%1$^O1 ��%1$n��|�� ��� ������ %2$C2, ������� ������������ ���.", obj, ch );
	obj->condition = 100;
    }
    else if ( number_percent() > gsn_smithing->getEffective( ch ) ) {
	gsn_smithing->improve( ch, false );
	act_p("$c1 ������� ������������ $o4, �� � ���� �� ����������.",ch,obj,0,TO_ROOM,POS_RESTING);
	act_p("� ���� �� ���������� ������������ $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
	hammer->condition -= 25;
    }
    else {
	gsn_smithing->improve( ch, true );
	act_p("$c1 ��������������� $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
	act_p("�� ���������������� $o4.\n\r",ch,obj,0,TO_CHAR,POS_RESTING);

	obj->condition += gsn_smithing->getEffective( ch ) / 2;
	obj->condition = max( 100, obj->condition );
	hammer->condition -= 25;
    }
    
    if (hammer->condition < 1)  
	extract_obj( hammer );
}

