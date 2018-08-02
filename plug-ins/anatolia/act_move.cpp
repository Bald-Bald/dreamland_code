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
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *	
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include "logstream.h"
#include "act_move.h"
#include "portalmovement.h"
#include "commandtemplate.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skill.h"
#include "affect.h"
#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "gsn_plugin.h"
#include "act.h"
#include "handler.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

/* command procedures needed */
void do_stand( Character *, const char * );
void do_visible( Character * );

/*-----------------------------------------------------------------------------
 * direction commands 
 *----------------------------------------------------------------------------*/
CMDRUNP( north )
{
    move_char( ch, DIR_NORTH, argument );
}

CMDRUNP( east )
{
    move_char( ch, DIR_EAST, argument );
}

CMDRUNP( south )
{
    move_char( ch, DIR_SOUTH, argument );
}

CMDRUNP( west )
{
    move_char( ch, DIR_WEST, argument );
}

CMDRUNP( up )
{
    move_char( ch, DIR_UP, argument );
}

CMDRUNP( down )
{
    move_char( ch, DIR_DOWN, argument );
}

/*--------------------------------------------------------------------
 *    scan 
 *-------------------------------------------------------------------*/
#define MILD(ch)     (IS_SET((ch)->comm, COMM_MILDCOLOR))

#define CLR_SCAN_MOB(ch)  (MILD(ch) ? "w" : "W")
#define CLR_SCAN_DIR(ch)  (MILD(ch) ? "c" : "C")
#define CLR_SCAN_DOOR(ch) (MILD(ch) ? "w" : "W")

static void scan_people( Room *room, Character *ch, int depth, int door, 
                         bool fShowDir, ostringstream &buf )
{
    Character *rch, *orig;
    bool found;
    bool fRus = ch->getConfig( )->ruexits;

    found = false;
    
    for (rch = room->people; rch != 0; rch = rch->next_in_room) {
	if (rch == ch) 
	    continue;
	if (rch->invis_level > ch->get_trust()) 
	    continue;
	if (!ch->can_see( rch ))
	    continue;

	if (!found) {
	    buf << "{" << CLR_SCAN_DIR(ch);

	    if (door != -1) {
		if (fShowDir)
		    buf << (fRus ? dirs[door].where : dirs[door].name);
		else
		    buf << "��������� " << depth;
	    }
	    else
		buf << "�����";
	    
	    buf << ":{x" << endl;
	    found = true;
	}

	orig = rch->getDoppel( ch );
	
	buf << "    {" << CLR_SCAN_MOB(ch) << ch->sees( orig, '1' ) << ".{x";

	if (IS_SET( orig->comm, COMM_AFK ))
	    buf << " {w[{CAFK{w]{x";

	buf << endl; 
    }
}

static Room * scan_room( Room *start_room, Character *ch, int depth, int door,
                       bool fShowDir, ostringstream &buf )
{
    EXIT_DATA *pExit;
    Room *room;
    bool fRus = ch->getConfig( )->ruexits;

    pExit = start_room->exit[door];
    
    if (!pExit || !ch->can_see( pExit ))
	return NULL;
    
    room = pExit->u1.to_room;

    if (IS_SET(pExit->exit_info, EX_CLOSED)) {
	buf << "{" << CLR_SCAN_DIR(ch);

	if (fShowDir)
	    buf << (fRus ? dirs[door].where : dirs[door].name);
	else
	    buf << "��������� " << depth;
	
	buf << ":{x" << endl
	    << "    {" << CLR_SCAN_DOOR(ch) << "�������� �����.{x" << endl;

	return NULL;
    }

    if (IS_SET(pExit->exit_info, EX_NOSCAN)) {
	buf << "{" << CLR_SCAN_DIR(ch);

	if (fShowDir)
	    buf << (fRus ? dirs[door].where : dirs[door].name);
	else
	    buf << "��������� " << depth;
	
	buf << ":{x" << endl << "    ���������� ���-���� ����������." << endl;
	return NULL;
    }
    
    scan_people( room, ch, depth, door, fShowDir, buf );
    return room;
}

CMDRUNP( scan )
{
    ostringstream buf;
    char arg1[MAX_INPUT_LENGTH];
    Room *room;
    int door, depth;
    int range;

    if (ch->desc == 0)
	return;

    if (ch->position < POS_SLEEPING) {
	ch->println( "�� ������ �� ������, ����� �����..." );
	return;
    }

    if (ch->position == POS_SLEEPING) {
	ch->println( "�� �����! � ������ ������ ������ ���!" );
	return;
    }

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0')
    {
	act( "$c1 ����������� ��� ������.", ch, 0, 0, TO_ROOM );
	buf << "������������, �� ������:" << endl;
	scan_people( ch->in_room, ch, 0, -1, true, buf );

	for (door = 0; door < DIR_SOMEWHERE; door++)
	    scan_room( ch->in_room, ch, 1, door, true, buf );

	ch->send_to( buf );
	return;
    }

    door = direction_lookup( arg1 );

    if (door < 0) {
	ch->println( "� ����� �������?" );
	return;
    }

    act( "�� ���������� �������� $T.", ch, 0, dirs[door].leave, TO_CHAR );
    act( "$c1 ���������� ������� $T.", ch, 0, dirs[door].leave, TO_ROOM );
    
    range = max( 1, ch->getModifyLevel() / 10 );
    room = ch->in_room;

    for (depth = 1; depth <= range; depth++) {
	room = scan_room( room, ch, depth, door, false, buf );

	if (!room)
	    break;
    }
    
    if (!buf.str( ).empty( )) {
	ch->println( "�� ������:" );
	ch->send_to( buf );
    }
}

/*--------------------------------------------------------------------
 *  stand / wake / sit / rest / sleep 
 *-------------------------------------------------------------------*/
DLString oprog_msg( Object *obj, const char *tag )
{
    DLString msg;
    Scripting::IdRef ID_MSG( tag );
    Scripting::Register regObj, regObjIndex, reg;
    
    if (!FeniaManager::wrapperManager)
	return msg;

    regObj = FeniaManager::wrapperManager->getWrapper( obj );
    regObjIndex = FeniaManager::wrapperManager->getWrapper( obj->pIndexData );

    try { 
	reg = *regObj[ID_MSG];
	if (reg.type != Scripting::Register::NONE)
	    msg = reg.toString( ); 
	
	if (msg.empty( )) {
	    reg = *regObjIndex[ID_MSG];
	    if (reg.type != Scripting::Register::NONE)
		msg = reg.toString( ); 
	}
    } 
    catch (const Exception &e) { 
	LogStream::sendWarning( ) << e.what( ) << endl;
    }
    
    return msg;
}

static bool oprog_msg_furniture( Object *obj, Character *ch, const char *tagRoom, const char *tagChar )
{
    DLString msgRoom, msgChar;
    
    msgRoom = oprog_msg( obj, tagRoom );
    msgChar = oprog_msg( obj, tagChar );
    
    if (msgChar.empty( ) && msgRoom.empty( ))
	return false;

    if (!msgChar.empty( ))
	ch->pecho( msgChar.c_str( ), obj, ch );

    if (!msgRoom.empty( ))
	ch->recho( POS_RESTING, msgRoom.c_str( ), obj, ch );

    return true;
}

CMDRUNP( stand )
{
	Object *obj = 0;
	
	if (argument[0] != '\0')
	{
		if (ch->position == POS_FIGHTING)
		{
			ch->println( "����� ������� ��������� ���������?" );
			return;
		}

		obj = get_obj_list(ch,argument,ch->in_room->contents);

		if (obj == 0)
		{
			ch->println( "�� �� ������ ����� �����." );
			return;
		}

		if ( obj->item_type != ITEM_FURNITURE
			|| ( !IS_SET(obj->value[2],STAND_AT)
				&& !IS_SET(obj->value[2],STAND_ON)
				&& !IS_SET(obj->value[2],STAND_IN) ) )
		{
			ch->println( "�� �� ������ ������ �� ����." );
			return;
		}

		if (ch->on != obj && count_users(obj) >= obj->value[0])
		{
			act_p("�� $o6 ��� ���������� �����.",
				ch,obj,0,TO_ROOM,POS_DEAD);
			return;
		}
	}

	switch ( ch->position.getValue( ) )
	{
	case POS_SLEEPING:
		if ( IS_AFFECTED(ch, AFF_SLEEP) )
		{
			ch->println( "�� �� ������ ����������!" );
			return;
		}

		if (obj == 0)
		{
			ch->println( "�� ������������ � �������." );
			act_p( "$c1 ����������� � ������.", ch, 0, 0, TO_ROOM,POS_RESTING );
			ch->on = 0;
		}
		else if (!oprog_msg_furniture( obj, ch, "msgWakeStandRoom", "msgWakeStandChar" )) {
		    if (IS_SET(obj->value[2],STAND_AT))
		    {
			    act_p("�� ������������ � ����������� ����� $o2.",ch,obj,0,TO_CHAR,POS_DEAD);
			    act_p("$c1 ����������� � ���������� ����� $o2.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else if (IS_SET(obj->value[2],STAND_ON))
		    {
			    act_p("�� ������������ � ����������� �� $o4.",ch,obj,0,TO_CHAR,POS_DEAD);
			    act_p("$c1 ����������� � ���������� �� $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else
		    {
			    act_p("�� ������������ � ����������� � $o4.",ch,obj,0,TO_CHAR,POS_DEAD);
			    act_p("$c1 ����������� � ���������� � $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		}

		if (IS_HARA_KIRI(ch))
		{
			ch->println( "�� ���������� ��� ����� ��������� ���� ����." );
			REMOVE_BIT(ch->act,PLR_HARA_KIRI);
		}

		ch->position = POS_STANDING;
		interpret_raw(ch, "look", "auto");
		break;

	case POS_RESTING:
	case POS_SITTING:
		if (obj == 0)
		{
			ch->println( "�� �������." );
			act_p( "$c1 ������.", ch, 0, 0, TO_ROOM,POS_RESTING );
			ch->on = 0;
		}
		else if (!oprog_msg_furniture( obj, ch, "msgStandRoom", "msgStandChar" )) {
		    if (IS_SET(obj->value[2],STAND_AT))
		    {
			    act_p("�� ����������� ����� $o2.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ���������� ����� $o2.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else if (IS_SET(obj->value[2],STAND_ON))
		    {
			    act_p("�� ����������� �� $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ���������� �� $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else
		    {
			    act_p("�� ����������� � $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ���������� � $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		}

		ch->position = POS_STANDING;
		break;

	case POS_STANDING:
		ch->println( "�� ��� ������." );
		break;

	case POS_FIGHTING:
		ch->println( "�� ��� ����������!" );
		break;
	}

	return;
}



CMDRUNP( rest )
{
	Object *obj = 0;

	if (ch->position == POS_FIGHTING)
	{
		ch->println( "�� �� �� ����������!" );
		return;
	}

	if (MOUNTED(ch))
	{
		ch->println( "�� �� ������ ��������, ����� �� � �����." );
		return;
	}

	if (RIDDEN(ch))
	{
		ch->println( "�� �� ������ ��������, ����� �� �������." );
		return;
	}

	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	{
		ch->println( "�� ����� � �� ������ ����������." );
		return;
	}

	if ( ch->death_ground_delay > 0
		&& ch->trap.isSet( TF_NO_MOVE ) )
	{
		ch->println( "���� ������� ��������." );
		return;
	}

	/* okay, now that we know we can rest, find an object to rest on */
	if (argument[0] != '\0')
	{
		obj = get_obj_list(ch,argument,ch->in_room->contents);

		if (obj == 0)
		{
			ch->println( "�� �� ������ ����� �����." );
			return;
		}
	}
	else
		obj = ch->on;

	if (obj != 0)
	{
		if ( ( obj->item_type != ITEM_FURNITURE )
			|| ( !IS_SET(obj->value[2],REST_ON)
				&& !IS_SET(obj->value[2],REST_IN)
				&& !IS_SET(obj->value[2],REST_AT) ) )
		{
			ch->println( "�� �� ������ �������� �� ����." );
			return;
		}

		if (obj != 0 && ch->on != obj && count_users(obj) >= obj->value[0])
		{
			act_p("�� $o6 ��� ���������� �����.",ch,obj,0,TO_CHAR,POS_DEAD);
			return;
		}

		ch->on = obj;
	}
	
	switch ( ch->position.getValue( ) )
	{
	case POS_SLEEPING:
		if (DIGGED(ch)) {
		    ch->println( "�� ������������." );
		} 
		else if (obj == 0)
		{
			ch->println( "�� ������������ � �������� ��������." );
			act_p("$c1 ����������� � ������� ��������.",ch,0,0,TO_ROOM,POS_RESTING);
		}
		else if (!oprog_msg_furniture( obj, ch, "msgWakeRestRoom", "msgWakeRestChar" )) {
		    if (IS_SET(obj->value[2],REST_AT))
		    {
			    act_p("�� ������������ � �������� �������� ����� $o2.",
				    ch,obj,0,TO_CHAR,POS_SLEEPING);
			    act_p("$c1 ����������� � ������� �������� ����� $o2.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else if (IS_SET(obj->value[2],REST_ON))
		    {
			    act_p("�� ������������ � �������� �������� �� $o4.",
				    ch,obj,0,TO_CHAR,POS_SLEEPING);
			    act_p("$c1 ����������� � ������� �������� �� $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else
		    {
			    act_p("�� ������������ � �������� �������� � $o4.",
				    ch,obj,0,TO_CHAR,POS_SLEEPING);
			    act_p("$c1 ����������� � ������� �������� � $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		}
		ch->position = POS_RESTING;
		break;

	case POS_RESTING:
		ch->println( "�� ��� ���������." );
		break;

	case POS_STANDING:
		if (obj == 0)
		{
			ch->println( "�� �������� ��������." );
			act_p( "$c1 ������� ��������.", ch, 0, 0, TO_ROOM,POS_RESTING );
		}
		else if (!oprog_msg_furniture( obj, ch, "msgSitRestRoom", "msgSitRestChar" )) {
		    if (IS_SET(obj->value[2],REST_AT))
		    {
			    act_p("�� �������� ����� $o2 � ���������.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ������� ����� $o2 � ��������.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else if (IS_SET(obj->value[2],REST_ON))
		    {
			    act_p("�� �������� �� $o4 � ���������..",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ������� �� $o4 � ��������.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else
		    {
			    act_p("�� �������� �������� � $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ������� �������� � $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		}
		ch->position = POS_RESTING;
		break;

	case POS_SITTING:
		if (obj == 0)
		{
			ch->println( "�� ���������." );
			act_p("$c1 ��������.",ch,0,0,TO_ROOM,POS_RESTING);
		}
		else if (!oprog_msg_furniture( obj, ch, "msgRestRoom", "msgRestChar" )) {
		    if (IS_SET(obj->value[2],REST_AT))
		    {
			    act_p("�� ��������� ����� $o2.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 �������� ����� $o2.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else if (IS_SET(obj->value[2],REST_ON))
		    {
			    act_p("�� ��������� �� $o6.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 �������� �� $o6.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else
		    {
			    act_p("�� ��������� � $o6.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 �������� � $o6.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		}
		ch->position = POS_RESTING;

		if (IS_HARA_KIRI(ch))
		{
			ch->println( "�� ����������, ��� ����� ��������� ���� ����." );
			REMOVE_BIT(ch->act,PLR_HARA_KIRI);
		}

		break;
	}

	return;
}


CMDRUNP( sit )
{
	Object *obj = 0;

	if (ch->position == POS_FIGHTING)
	{
		ch->println( "����� ������� ��������� ���������?" );
		return;
	}

	if (MOUNTED(ch))
	{
		ch->println( "�� �� ������ �����, ����� �� � �����." );
		return;
	}

	if (RIDDEN(ch))
	{
		ch->println( "�� �� ������ �����, ����� �� �������." );
		return;
	}

	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	{
		ch->println( "�� ����� � �� ������ ����������." );
		return;
	}

	if ( ch->death_ground_delay > 0
		&& ch->trap.isSet( TF_NO_MOVE ) )
	{
		ch->println( "���� �� �� ������!" );
		return;
	}

    /* okay, now that we know we can sit, find an object to sit on */
	if (argument[0] != '\0')
	{
		obj = get_obj_list(ch,argument,ch->in_room->contents);

		if (obj == 0)
		{
			if ( IS_AFFECTED(ch, AFF_SLEEP) )
			{
				ch->println( "�� ����� � �� ������ ����������." );
				return;
			}

			ch->println( "�� �� ������ ����� �����." );
			return;
		}
	}
	else
		obj = ch->on;

	if (obj != 0)
	{
		if ( ( obj->item_type != ITEM_FURNITURE )
			|| ( !IS_SET(obj->value[2],SIT_ON)
				&& !IS_SET(obj->value[2],SIT_IN)
				&& !IS_SET(obj->value[2],SIT_AT) ) )
		{
			ch->println( "�� �� ������ ����� �� ���." );
			return;
		}

		if (obj != 0 && ch->on != obj && count_users(obj) >= obj->value[0])
		{
			act_p("�� $o6 ��� ������ ���������� �����.",ch,obj,0,TO_CHAR,POS_DEAD);
			return;
		}

		ch->on = obj;
	}

	switch (ch->position.getValue( ))
	{
	case POS_SLEEPING:
		if (obj == 0)
		{
			ch->println( "�� ������������ � ��������." );
			act_p( "$c1 ����������� � �������.", ch, 0, 0, TO_ROOM,POS_RESTING );
		}
		else if (!oprog_msg_furniture( obj, ch, "msgWakeSitRoom", "msgWakeSitChar" )) {
		    if (IS_SET(obj->value[2],SIT_AT))
		    {
			    act_p("�� ������������ � �������� ����� $o2.",ch,obj,0,TO_CHAR,POS_DEAD);
			    act_p("$c1 ����������� � ������� ����� $o2.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else if (IS_SET(obj->value[2],SIT_ON))
		    {
			    act_p("�� ������������ � �������� �� $o4.",ch,obj,0,TO_CHAR,POS_DEAD);
			    act_p("$c1 ����������� � ������� �� $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else
		    {
			    act_p("�� ������������ � �������� � $o4.",ch,obj,0,TO_CHAR,POS_DEAD);
			    act_p("$c1 ����������� � ������� � $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		}

		ch->position = POS_SITTING;
		break;

	case POS_RESTING:
		if (obj == 0)
			ch->println( "�� ����������� �����." );
		else if (!oprog_msg_furniture( obj, ch, "msgSitRoom", "msgSitChar" )) {
		    if (IS_SET(obj->value[2],SIT_AT))
		    {
			    act_p("�� �������� ����� $o2.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ������� ����� $o2.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }

		    else if (IS_SET(obj->value[2],SIT_ON))
		    {
			    act_p("�� �������� �� $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ������� �� $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else
		    {
			    act_p("�� �������� � $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ������� � $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		}

		ch->position = POS_SITTING;
		break;

	case POS_SITTING:
		ch->println( "�� ��� ������." );
		break;

	case POS_STANDING:
		if (obj == 0)
		{
			ch->println( "�� ��������." );
			act_p("$c1 ������� �� �����.",ch,0,0,TO_ROOM,POS_RESTING);
		}
		else if (!oprog_msg_furniture( obj, ch, "msgSitRoom", "msgSitChar" )) {
		    if (IS_SET(obj->value[2],SIT_AT))
		    {
			    act_p("�� �������� ����� $o2.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ������� ����� $o2.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else if (IS_SET(obj->value[2],SIT_ON))
		    {
			    act_p("�� �������� �� $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ������� �� $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		    else
		    {
			    act_p("�� �������� � $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
			    act_p("$c1 ������� � $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
		    }
		}
		ch->position = POS_SITTING;
		break;
	}

	if (IS_HARA_KIRI(ch))
	{
		ch->println( "�� ����������, ��� ����� ��������� ���� ����." );
		REMOVE_BIT(ch->act,PLR_HARA_KIRI);
	}
	return;
}

GSN(curl);

CMDRUNP( sleep )
{
    Object *obj = 0;
    ostringstream toMe, toRoom;

    if (MOUNTED(ch))
    {
	    ch->println( "�� �� ������ �����, ����� �� � �����." );
	    return;
    }

    if (RIDDEN(ch))
    {
	    ch->println( "�� �� ������ �����, ����� �� �������." );
	    return;
    }

    if ( ch->death_ground_delay > 0
	    && ch->trap.isSet( TF_NO_MOVE ) )
    {
	    ch->println( "���� �� �� ���!" );
	    return;
    }

    switch ( ch->position.getValue( ) ) {
    case POS_SLEEPING:
	ch->println( "�� ��� �����." );
	return;

    case POS_FIGHTING:
	ch->println( "�� �� �� ����������!" );
	return;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
	if (argument[0] == '\0' && ch->on == 0)
	{
	    ch->position = POS_SLEEPING;

	    toMe << "�� ���������";
	    toRoom << "%1$^C1 ��������";
	    
	    if (gsn_curl->getEffective( ch ) > 1 ) {
		toMe << ", ����������� ���������";
		toRoom << ", ����������� ���������";
	    }
	}
	else  /* find an object and sleep on it */
	{
	    if (argument[0] == '\0')
		    obj = ch->on;
	    else
		    obj = get_obj_list( ch, argument,  ch->in_room->contents );

	    if (obj == 0)
	    {
		    ch->println( "�� �� ������ ����� �����." );
		    return;
	    }

	    if ( obj->item_type != ITEM_FURNITURE
		    || ( !IS_SET(obj->value[2],SLEEP_ON)
			    && !IS_SET(obj->value[2],SLEEP_IN)
			    && !IS_SET(obj->value[2],SLEEP_AT)))
	    {
		    ch->println( "�� �� ������ ����� �� ����!" );
		    return;
	    }

	    if (ch->on != obj && count_users(obj) >= obj->value[0])
	    {
		    act_p("�� $o6 �� �������� ���������� ����� ��� ����.",
			    ch,obj,0,TO_CHAR,POS_DEAD);
		    return;
	    }

	    ch->on = obj;
	    ch->position = POS_SLEEPING;

	    if (oprog_msg_furniture( obj, ch, "msgSleepRoom", "msgSleepChar" ))
		return;

	    toMe << "�� �������� ����� ";
	    toRoom << "%1$^C1 ������� ����� ";

	    if (IS_SET(obj->value[2],SLEEP_AT))
	    {
		toMe << "����� %2$O2";
		toRoom << "����� %2$O2";
	    }
	    else if (IS_SET(obj->value[2],SLEEP_ON))
	    {
		toMe << "�� %2$O4";
		toRoom << "�� %2$O4";
	    }
	    else
	    {
		toMe << "� %2$O4";
		toRoom << "� %2$O4";
	    }
	    
	    if (gsn_curl->getEffective( ch ) > 1 ) {
		toMe << ", ����������� ���������";
		toRoom << ", ����������� ���������";
	    }

	}
	break;
    }

    toMe << ".";
    toRoom << ".";
    ch->pecho( toMe.str( ).c_str( ), ch, obj );
    ch->recho( POS_RESTING, toRoom.str( ).c_str( ), ch, obj );
}

static bool mprog_wake( Character *ch, Character *waker )
{
    FENIA_CALL( ch, "Wake", "C", waker );
    FENIA_NDX_CALL( ch->getNPC( ), "Wake", "CC", ch, waker );
    return false;
}

/* COMPAT */ void do_stand( Character *ch, const char *argument )
{
    interpret_raw( ch, "stand", argument ); 
}

CMDRUNP( wake )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' ) { 
	if (DIGGED(ch) && ch->position <= POS_SLEEPING)
	    interpret_raw( ch, "rest", argument );
	else {
	    undig( ch );
	    do_stand( ch, argument );
	}

	return; 
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 ) { 
	ch->println( "����� ��� �����." ); 
	return; 
    }

    if (ch == victim) { 
	ch->println( "�� �� ������ ��������� ��� ����!" ); 
	return; 
    }

    if (IS_AWAKE(victim)) { 
	act_p( "$C1 ��� �� ����.", ch, 0, victim, TO_CHAR,POS_RESTING ); 
	return; 
    }

    if (IS_AFFECTED(victim, AFF_SLEEP)) { 
	act_p( "�� �� ������ ��������� $S!", ch, 0, victim, TO_CHAR,POS_RESTING );  
	return; 
    }

    act_p( "$c1 ����� ����.", ch, 0, victim, TO_VICT,POS_SLEEPING );
    do_stand(victim,"");
    mprog_wake( victim, ch );
}


/*
 * Contributed by Alander
 */
CMDRUNP( visible )
{
    do_visible( ch );
}


CMDRUNP( flyup )
{
    interpret_raw( ch, "fly", "up" );
}

CMDRUNP( flydown )
{
    interpret_raw( ch, "fly", "down" );
}

CMDRUNP( fly )
{
    char arg[MAX_INPUT_LENGTH];

    if (ch->is_npc())
	return;

    argument = one_argument(argument,arg);

    if (!str_cmp(arg,"up") || !str_cmp(arg,"�����"))
    {
	if (!can_fly( ch )) {
	    ch->println( "��� ����, ����� ������, ����� ������ ��� �����." );
	    return;
	}

	if (!ch->posFlags.isSet( POS_FLY_DOWN )) {
	    ch->println( "�� ��� �������." );
	    return;
	}

	ch->posFlags.removeBit( POS_FLY_DOWN );
	ch->println( "�� ��������� ������." );
	ch->recho( "%^C1 �������� ������.", ch );
    }
    else if (!str_cmp(arg,"down") || !str_cmp(arg,"����"))
    {
	if (!is_flying( ch )) {
	    ch->println( "���� ���� ��� �� �����." );
	    return;
	}

	ch->posFlags.setBit( POS_FLY_DOWN );
	ch->println( "���� ���� �������� ���������� �� �����." );
	ch->recho( "%^C1 �������� ���������� �� �����.", ch );
    }
    else
    {
	ch->println( "��������� {lEfly � 'up' ��� 'down'{lR'��������' ��� '��������'{lx." );
	return;
    }

    ch->setWait( gsn_fly->getBeats( ) );
}


/*
 * ���������� �����.
 */
CMDRUNP( walk )
{
    EXTRA_EXIT_DATA *peexit;

    // Must be entered extra exit name
    if (argument[0] == '\0') {
	ch->println( "� ���� �� ��������� ����, ��� ����?" );
	return;
    }

    peexit = get_extra_exit( argument, ch->in_room->extra_exit );

    if (peexit == 0) {
	ch->println( "�� �� �������� ����� �����." );
	return;
    }

    move_char( ch, peexit );
}


CMDRUNP( enter )
{
    Object *portal;
    
    if (!argument[0])
	portal = get_obj_room_type( ch, ITEM_PORTAL );
    else
	portal = get_obj_list( ch, argument, ch->in_room->contents );

    if (portal == 0) {
	ch->println( "�� �� ������ ����� ���." );
	return;
    }
    
    if (portal->item_type != ITEM_PORTAL) {
	ch->println( "�� �� �������� ���� ������." );
	return;
    }

    PortalMovement( ch, portal ).move( );
}


