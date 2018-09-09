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
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "commandtemplate.h"
#include "behavior_utils.h"
#include "skill.h"
#include "clanreference.h"
#include "object.h"
#include "objectbehavior.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "affect.h"
#include "race.h"
#include "npcharacter.h"
#include "room.h"
#include "desire.h"

#include "dreamland.h"
#include "save.h"
#include "merc.h"
#include "descriptor.h"
#include "wiznet.h"
#include "mercdb.h"
#include "act.h"
#include "interp.h"
#include "gsn_plugin.h"
#include "stats_apply.h"
#include "material.h"
#include "act_move.h"
#include "handler.h"
#include "act.h"
#include "def.h"
#include "vnum.h"

DESIRE(full);
DESIRE(hunger);
DESIRE(thirst);
DESIRE(bloodlust);
GSN(smell);


bool obj_has_name( Object *obj, const DLString &arg, Character *ch );
long long get_arg_id( const DLString &cArgument );


/*
 * find ':pocket' part of container name
 */
DLString get_pocket_argument( char *arg )
{
    DLString p;
    
    while (*arg++) {
	if (*arg == ':') {
	    p = arg + 1;
	    *arg = '\0';
	    p.colourstrip( );
	    return p;
	}
    }

    return "";
}

DLString get_pocket_argument( DLString &arg )
{
    DLString::size_type pos = arg.find(':');
    if (pos != DLString::npos) {
        DLString pocket = arg.size() > pos+1 ? arg.substr(pos+1) : "";
        arg = arg.substr(0, pos);
        return pocket;
    }

    return "";
}


bool parse_money_arguments( Character *ch, const char *arg, int amount, int &gold, int &silver )
{
    if ((!arg_is_silver( arg ) && !arg_is_gold( arg ) )) {
        if (!str_prefix( arg, "������" ) || !str_prefix( arg, "silver" )) {
            ch->println( "����� �������� ������ ���������: ������� ��� silver." );
            return false;
        }
        if (!str_prefix( arg, "�����" ) || !str_prefix( arg, "gold" )) {
            ch->println( "����� �������� ������ ���������: ������ ��� gold." );
            return false;
        }
        ch->println( "�� ������ ������� ���������� ����� � ������� (silver) ��� ������ (gold)." );
        return false;
    }
    if (amount < 0) {
        ch->println( "������������� ���������� �����?" );
        return false;
    }
    if (amount == 0) {
        ch->println( "���� �����, ��� ���?" );
        return false;
    }


    if (arg_is_silver( arg )) {
	    if (ch->silver < amount)
	    {
		    ch->send_to("� ���� ��� ������� �������.\n\r");
		    return false;
	    }

	    silver = amount;
    }
    else
    {
	    if (ch->gold < amount)
	    {
		    ch->send_to("� ���� ��� ������� ������.\n\r");
		    return false;
	    }

	    gold = amount;
    }

    return true;
}
		
/*
 *   GET OBJECT [[FROM] CONTAINER[:POCKET]]
 *   GET MOBILE [BY] OBJECT
 */

static void get_obj_on_victim( Character *ch, Character *victim, const char *arg )
{
    Object *obj;

    if (( obj = get_obj_wear_victim( victim, arg, ch ) ) == 0) {
	act("� $C2 ��� ������ �������� �� $t.", ch, is_number(arg) ? "���" : arg, victim, TO_CHAR);
	return;
    }
    
    act("�� ������ $C4 �� $o4.", ch, obj, victim, TO_CHAR);
    act("$c1 ����� ���� �� $o4.", ch, obj, victim, TO_VICT);
    act("$c1 ����� $C4 �� $o4.", ch, obj, victim, TO_NOTVICT);
    
    FENIA_VOID_CALL( obj, "Seize", "CC", ch, victim );
    FENIA_VOID_CALL( ch, "Seize", "CCO", ch, victim, obj );
    FENIA_VOID_CALL( victim, "Seize", "CCO", ch, victim, obj );

    FENIA_NDX_VOID_CALL( obj, "Seize", "OCC", obj, ch, victim );
    FENIA_NDX_VOID_CALL( ch->getNPC( ), "Seize", "CCCO", ch, ch, victim, obj );
    FENIA_NDX_VOID_CALL( victim->getNPC( ), "Seize", "CCCO", victim, ch, victim, obj );
}

/* RT part of the corpse looting code */
static bool oprog_get_money( Character *ch, Object *obj )
{
    ch->silver += obj->value[0];
    ch->gold += obj->value[1];

    if (IS_SET(ch->act,PLR_AUTOSPLIT))
	if (obj->value[0] > 1 || obj->value[1])
	    if (party_members_room( ch ).size( ) > 1)
		interpret_raw( ch, "split", "%d %d", obj->value[0], obj->value[1] );
    
    extract_obj( obj );
    return true;
}

bool oprog_get( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Get", "C", ch );
    FENIA_NDX_CALL( obj, "Get", "OC", obj, ch );
    BEHAVIOR_VOID_CALL( obj, get, ch );

    switch (obj->item_type) {
    case ITEM_MONEY:
	return oprog_get_money( ch, obj );
    }

    return false;
}

static bool oprog_fetch_corpse_pc( Character *ch, Object *obj, Object *container )
{
    if (!ch->is_immortal( ) && !container->hasOwner( ch ))
    {
	container->count--;
    }

    return false;
}

static bool oprog_fetch( Character *ch, Object *obj, Object *container )
{
    FENIA_CALL( container, "Fetch", "CO", ch, obj );
    FENIA_NDX_CALL( container, "Fetch", "OCO", container, ch, obj );
    BEHAVIOR_CALL( container, fetch, ch, obj );
    SKILLEVENT_CALL( ch, fetchItem, ch, obj, container );

    switch (container->item_type) {
    case ITEM_CORPSE_PC:
	return oprog_fetch_corpse_pc( ch, obj, container );
    }

    return false;
}

static bool oprog_can_get_corpse_pc( Character *ch, Object *obj )
{
    if (!ch->is_immortal( ) && !obj->hasOwner( ch ))
    {
	act("������, $o4 �� ����� �� ��������.",ch,obj,0,TO_CHAR);
	return false;
    }
    
    return true;
}

static bool oprog_can_get_furniture( Character *ch, Object *obj )
{
    if (count_users( obj ) > 0) {
	act("���-�� ���������� $o4.",ch,obj,0,TO_CHAR);
	return false;
    }

    return true;
}

static bool oprog_cant_get( Character *ch, Object *obj )
{
    FENIA_CALL( obj, "CantGet", "C", ch );
    FENIA_NDX_CALL( obj, "CantGet", "OC", obj, ch );
    return false;
}

static bool oprog_can_get( Character *ch, Object *obj )
{
    if (oprog_cant_get( ch, obj ))
        return false;

    switch (obj->item_type) {
    case ITEM_CORPSE_PC:
	return oprog_can_get_corpse_pc( ch, obj );
    case ITEM_FURNITURE:
	return oprog_can_get_furniture( ch, obj );
    }

    return true;
}

static bool oprog_can_fetch_corpse_pc( Character *ch, Object *container )
{
    if (ch->is_npc( )) {
	ch->send_to( "�� �� ������ ���������� ����� �����.\r\n" );
	return false;
    }
    
    if (ch->is_immortal( ))
	return true;
	
    if (container->hasOwner( ch ))
	return true;
	
    if (!container->killer)
	return true;

    if (str_cmp( ch->getNameP( ), container->killer ) 
	&& str_cmp( "!anybody!", container->killer )) 
    {
	ch->send_to( "��� �� ���� ������.\r\n" );
	return false;
    }
    
    if (container->count == 0) {
	ch->send_to("����� ���� ������ ������ �����.\n\r");
	return false;
    }

    return true;
}

static bool oprog_cant_fetch( Object *container, Character *ch, Object *obj, const DLString &pocket )
{
    FENIA_CALL( container, "CantFetch", "COs", ch, obj, pocket.c_str( ) );
    FENIA_NDX_CALL( container, "CantFetch", "OCOs", container, ch, obj, pocket.c_str( ) );
    return false;
}

static bool oprog_can_fetch( Character *ch, Object *container, Object *obj, const DLString &pocket )
{
    if (oprog_cant_fetch( container, ch, obj, pocket ))
	return false;

    switch (container->item_type) {
    case ITEM_CORPSE_PC:
	return oprog_can_fetch_corpse_pc( ch, container );
	
    case ITEM_CONTAINER:
	if (!pocket.empty( ) && !IS_SET(container->value[1], CONT_WITH_POCKETS)) {
	    act("���� �� ������� �������� �� ������ ������� � $o2.",ch,container,0,TO_CHAR);
	    return false;
	}
	
	if (IS_SET( container->value[1], CONT_CLOSED )) {
            ch->pecho("%1$^O4 ����� ������ �������.", container );
	    return false;
	}

	return true;

    case ITEM_KEYRING:
    case ITEM_CORPSE_NPC:
	return true;

    default:
        ch->pecho("%1$^O1 �� ���������, �� �� ������ ������ ������ �����.", container );
	return false;
    }
}

static bool can_get_obj( Character *ch, Object *obj )
{
    if (!oprog_can_get( ch, obj ))
	return false;

    if (!obj->can_wear( ITEM_TAKE ))
    {
        ch->pecho("�� �� ������ ����� %1$O4.", obj );
	return false;
    }

    if (obj->pIndexData->limit != -1)
    {
	if (obj->isAntiAligned( ch )) {
	    ch->pecho("%2$^s �� ��������� ���� ������� %1$O5, � �� ������� %1$P2.",
	              obj,
		      IS_NEUTRAL(ch) ? "���� ����������" : IS_GOOD(ch) ? "��������� ����" : "���� ������");
	    
	    ch->recho("%1$^C1 ���������� � ������ %2$O4.", ch, obj );
	    return false;
	}
    }

    if (ch->carry_number + obj->getNumber( ) > ch->canCarryNumber( ))
    {
	act_p( "$d: �� �� ������ ����� ������ �����.",ch,NULL,obj->getName( ),TO_CHAR,POS_RESTING );
	return false;
    }

    if (ch->getCarryWeight( ) + obj->getWeight( ) > ch->canCarryWeight( ))
    {
	act_p( "$d: �� �� ������ ������� ����� �������.",ch,NULL,obj->getName( ),TO_CHAR ,POS_RESTING);
	return false;
    }

    return true;
}

static bool get_obj( Character *ch, Object *obj )
{
    act_p( "�� ������ $o4.", ch, obj, 0, TO_CHAR,POS_RESTING);

    if (!IS_AFFECTED(ch,AFF_SNEAK))
	act_p( "$c1 ����� $o4.", ch, obj, 0, TO_ROOM,POS_RESTING);
	    
    obj_from_room( obj );
    obj_to_char( obj, ch );

    if (oprog_get( obj, ch ))
	return true;

    return false;
}

static bool get_obj_container( Character *ch, Object *obj, Object *container )
{
    ostringstream toChar, toRoom;
    DLString prep;
    
    switch (container->item_type) {
    case ITEM_KEYRING:
	toChar << "�� �������� $o4 � $O2.";
	toRoom << "$c1 ������� $o4 � $O2.";
	break;

    case ITEM_CONTAINER:
	if (IS_SET(container->value[1], CONT_PUT_ON)) 
	    prep = "��";
	else if (IS_SET(container->value[1], CONT_PUT_ON2)) 
	    prep = "�";
	else
	    prep = "��";
	
	toChar << "�� ������ $o4 " << prep << " $O2.";
	toRoom << "$c1 ����� $o4 " << prep << " $O2.";
	break;

    default:
	toChar << "�� ������ $o4 �� $O2.";
	toRoom << "$c1 ����� $o4 �� $O2.";
	break;
    }

    act( toChar.str( ).c_str( ), ch, obj, container, TO_CHAR );

    if (!IS_AFFECTED(ch, AFF_SNEAK))
	act( toRoom.str( ).c_str( ), ch, obj, container, TO_ROOM );

    obj_from_obj( obj );
    obj_to_char( obj, ch );

    if (oprog_get( obj, ch ))
	return true;
    
    if (oprog_fetch( ch, obj, container ))
	return true;
    
    return false;
}

void do_get_raw( Character *ch, Object *obj )
{
    if (can_get_obj( ch, obj ))
	get_obj( ch, obj );
}

bool do_get_raw( Character *ch, Object *obj, Object *container )
{
    DLString pocket;

    if (!oprog_can_fetch( ch, container, obj, pocket ))
	return true;
    
    if (can_get_obj( ch, obj ))
	get_obj_container( ch, obj, container );

    return false;
}

void do_get_all_raw( Character *ch, Object *container )
{
    Object *obj, *obj_next;
    DLString pocket;

    if (!oprog_can_fetch( ch, container, NULL, pocket ))
	return;

    for (obj = container->contains; obj; obj = obj_next) {
	obj_next = obj->next_content;

	if (!ch->can_see( obj ))
	    continue;
	
	if (do_get_raw( ch, obj, container ))
	    return;
    }
}

/*
 *
 *  get all
 *  get <name>
 *  get all.<name>
 *  get all.'<names list>'
 *
 *  get <name> [from] <container>[:<pocket>]
 *  get all [from] <container>[:<pocket>]
 *  get all.<name> [from] <container>[:<pocket>]
 *  get all.'<names list>' [from] <container>[:<pocket>]
 *
 *  get <victim> by <name>
 *
 */
CMDRUNP( get )
{
    Object *obj;
    Object *obj_next;
    Object *container;
    bool found;
    DLString origArguments = argument;
    DLString arguments = argument;
    DLString argAllObj, argTarget, argContainer;
    bool all, allDot;

    argAllObj = arguments.getOneArgument();
    if (arg_is_all( argAllObj )) {
        argTarget = "";
        all = true;
        allDot = false;
    } else if (arg_is_alldot( argAllObj )) {
        arguments = origArguments.substr(4);
        argTarget = arguments.getOneArgument();
        all = false;
        allDot = true;
    } else {
        argTarget = argAllObj;
        all = false;
        allDot = false;
    }

    argContainer = arguments.getOneArgument();
    if (arg_is_from( argContainer ) || arg_oneof_strict( argContainer, "��", "by" ))
        argContainer = arguments.getOneArgument( );


    if (argAllObj.empty( ))
    {
	ch->send_to("����� ���?\n\r");
	return;
    }

    if(argContainer.empty( ))
    {
        DLString that = is_number(argTarget.c_str( )) ? "�����" : argTarget;

	if (!all && !allDot)
	{
            /*  get <name> */
	    obj = get_obj_list( ch, argTarget.c_str( ), ch->in_room->contents );
	    
	    if (!obj)
		act_p( "�� �� ������ ����� $T.", ch, 0, that.c_str( ), TO_CHAR,POS_RESTING);
	    else
		do_get_raw( ch, obj );
	}
	else
	{
            /*
             *  get all
             *  get all.<name>
             *  get all.'<names list>'
             */
	    found = false;

	    dreamland->removeOption( DL_SAVE_OBJS );

	    for ( obj = ch->in_room->contents; obj; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( (all || obj_has_name( obj, argTarget, ch ))
			&& ch->can_see( obj ) )
		{
		    found = true;
		    do_get_raw( ch, obj );
		}
	    }

	    dreamland->resetOption( DL_SAVE_OBJS );

	    if ( !found )
	    {
		if (all)
		    ch->println("�� ������ �� ������ �����.");
                else if (allDot)
		    ch->println("�� �� ������ ������ ��������� �����.");
		else
		    act_p( "�� �� ������ ����� $T.", ch, 0, that.c_str( ), TO_CHAR,POS_RESTING);
	    }
	    else
		save_items( ch->in_room );
	}
    }
    else
    {
	DLString pocket;
        DLString that = is_number(argContainer.c_str( )) ? "�����" : argContainer;

        /*
         *  get <name> [from] <container>[:<pocket>]
         *  get all [from] <container>[:<pocket>]
         *  get all.<name> [from] <container>[:<pocket>]
         *  get all.'<names list>' [from] <container>[:<pocket>]
         *  or if container not found:
         *  get <victim> by <name>
         */

        // Disallow 'get <name> all.<container>' syntax.
	if (arg_is_alldot( argContainer ))
	{
	    ch->send_to("�� �� ������ ������� �����.\n\r");
	    return;
	}

        // Split out potential pocket argument from <container>:<pocket>.
	pocket = get_pocket_argument( argContainer );

        // Container not found, assume 'get <victim> by <name>' syntax.
	if ( !( container = get_obj_here( ch, argContainer ) ) )
	{
	    Character *victim = get_char_room( ch, argTarget );
	    
	    if (victim)
		get_obj_on_victim( ch, victim, argContainer.c_str( ) );
	    else
		act_p( "�� �� ������ ����� $T.", ch, 0, that.c_str( ), TO_CHAR,POS_RESTING);
	    return;
	}

	if (!oprog_can_fetch( ch, container, NULL, pocket ))
	    return;

	if (!all && !allDot)
	{
            /*  get <name> [from] <container>[:<pocket>] */
	    obj = get_obj_list( ch, argTarget.c_str( ), container->contains, pocket );

	    if(!obj) {
		act_p( "�� �� ������ ������ ��������� � $o6.", ch, container, 0, TO_CHAR,POS_RESTING);
		return;
	    }
	    
	    if (can_get_obj( ch, obj ))
		get_obj_container( ch, obj, container );
	}
	else {
            /*
             *  get all [from] <container>[:<pocket>]
             *  get all.<name> [from] <container>[:<pocket>]
             *  get all.'<names list>' [from] <container>[:<pocket>]
             */

	    if (IS_PIT(container) && !ch->is_immortal() )
	    {
		ch->send_to("�� ���� ����� �����!\n\r");
		return;
	    }
		
	    found = false;

	    for ( obj = container->contains; obj; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if (!all && !obj_has_name( obj, argTarget, ch ))
		    continue;

		if (!ch->can_see( obj ))
		    continue;

		if (!pocket.empty( ) && obj->pocket != pocket)
		    continue;

		if (pocket.empty( ) && !obj->pocket.empty( ))
		    continue;
		    
		found = true;

		if (!oprog_can_fetch( ch, container, obj, pocket ))
		    return;
		
		if (can_get_obj( ch, obj ))
		    get_obj_container( ch, obj, container );
	    }

	    if (!found) {
		if (!all)
		    act_p( "�� �� ������ ������ � $o6.", ch, container, 0, TO_CHAR,POS_RESTING);
		else
		    act_p( "�� �� ������ ������ ��������� � $o6.", ch, container, 0, TO_CHAR,POS_RESTING);
	    }
	}
    }
}


/*
 *   PUT OBJECT [IN|ON] CONTAINER[:POCKET]
 */

#define PUT_OBJ_STOP	-1
#define PUT_OBJ_ERR	0
#define PUT_OBJ_OK	1

static bool oprog_cant_put( Character *ch, Object *obj, Object *container,
	                    const char *pocket, bool verbose )
{
    FENIA_CALL( container, "CantPut", "COsi", ch, obj, pocket, verbose );
    FENIA_NDX_CALL( container, "CantPut", "OCOsi", container, ch, obj, pocket, verbose );
    return false;
}

static bool can_put_into( Character *ch, Object *container, const DLString &pocket )
{
    switch (container->item_type) {
    case ITEM_CONTAINER:
	if (IS_SET(container->value[1], CONT_CLOSED)) {
	    ch->println( "��� �������." );
	    return false;
	}

	if (!pocket.empty( ) && !IS_SET(container->value[1], CONT_WITH_POCKETS)) {
	    ch->println( "���� �� ������� �������� �� ������ �������." );
	    return false;
	}

	return true;

    case ITEM_KEYRING:
	return true;

    default:
	ch->println("��� �� ���������.");
	return false;
    }


}

static int can_put_obj_into( Character *ch, Object *obj, Object *container, const DLString &pocket, bool verbose )
{
    int pcount;

    if (obj == container) {
	if (verbose)
	    ch->send_to("�� �� ������ �������� ���-�� � ���� ��.\n\r");
	return PUT_OBJ_ERR;
    }
    
    if (oprog_cant_put( ch, obj, container, pocket.c_str( ), verbose )) 
	return PUT_OBJ_ERR;

    if (!can_drop_obj( ch, obj )) {
	act( "�� �� ������ ���������� �� $o2.", ch, obj, 0, TO_CHAR );
	return PUT_OBJ_ERR;
    }
    
    pcount = count_obj_in_obj( container );

    if (container->item_type == ITEM_KEYRING) {
	if (pcount >= container->value[0]) {
	    act( "�� $o6 �� �������� ���������� �����.", ch, container, 0, TO_CHAR );
	    return PUT_OBJ_STOP;
	}

	if (obj->item_type != ITEM_KEY && obj->item_type != ITEM_LOCKPICK) {
	    if (verbose)
		act( "�� $o4 �� ������ �������� ������ ����� ��� �������.", ch, container, 0, TO_CHAR );
	    return PUT_OBJ_ERR;
	}

	return PUT_OBJ_OK;
    }

    if (pcount > container->value[0]) {
	act_p("������ ���������� ������� ����� � $o4!", ch,container,0, TO_CHAR,POS_RESTING);
	return PUT_OBJ_STOP;
    }

    if (obj->getWeightMultiplier() != 100 && !IS_SET(container->value[1], CONT_NESTED)) {
	if (verbose)
	    ch->send_to("�������� ��� ���� ������ ����.\n\r");
	return PUT_OBJ_ERR;
    }

    if (obj->pIndexData->limit != -1) {
	act( "$o4 ������ ������� � ����� ���������.", ch,obj,0,TO_CHAR );
	return PUT_OBJ_ERR;
    }

    if (IS_SET(container->value[1],CONT_FOR_ARROW)
	    && (obj->item_type != ITEM_WEAPON
	    || obj->value[0]  != WEAPON_ARROW ))
    {
	if (verbose)
	    act_p("�� ������ �������� ������ ������ � $o4.",ch,container,0,TO_CHAR,POS_RESTING);
	return PUT_OBJ_ERR;
    }

    if (obj->getWeight( ) + container->getTrueWeight( ) > (container->value[0] * 10)
	||  obj->getWeight( ) > (container->value[3] * 10))
    {
	if (verbose)
	    ch->send_to("�� ������.\n\r");
	return PUT_OBJ_ERR;
    }

    if (obj->item_type == ITEM_POTION && IS_SET(container->wear_flags, ITEM_TAKE)) {
	pcount = count_obj_in_obj( container, ITEM_POTION );
		
       if (pcount > 15) {
	    act_p("����������� ����� ���������� �������� � $o4.",ch,container,0, TO_CHAR,POS_RESTING);
	    return PUT_OBJ_ERR;
       }
    }

    return PUT_OBJ_OK;
}

static bool oprog_put(Object *obj, Character *ch, Object *container)
{
    FENIA_CALL( ch, "Put", "COO", ch, obj, container );
    FENIA_CALL( obj, "Put", "COO", ch, obj, container );
    FENIA_CALL( container, "Put", "COO", ch, obj, container );

    FENIA_NDX_CALL( ch->getNPC( ), "Put", "CCOO", ch, ch, obj, container );
    FENIA_NDX_CALL( obj, "Put", "OCOO", obj, ch, obj, container );
    FENIA_NDX_CALL( container, "Put", "OCOO", container, ch, obj, container );
    
    SKILLEVENT_CALL( ch, putItem, ch, obj, container );

    return false;
}

static bool oprog_put_msg(Object *obj, Character *ch, Object *container)
{
    FENIA_CALL( container, "PutMsg", "COO", ch, obj, container );
    FENIA_NDX_CALL( container, "PutMsg", "OCOO", container, ch, obj, container );

    FENIA_CALL( obj, "PutMsg", "COO", ch, obj, container );
    FENIA_NDX_CALL( obj, "PutMsg", "OCOO", obj, ch, obj, container );

    return false;
}


static bool put_obj_container( Character *ch, Object *obj, Object *container, 
                               DLString &pocket )
{
    ostringstream toChar, toRoom;

    obj_from_char( obj );
    obj_to_obj( obj, container );
    
    if (container->item_type == ITEM_KEYRING) {
	toRoom << "$c1 ���������� $o4 �� $O4.";
	toChar << "�� ����������� $o4 �� $O4.";
    }
    else {

	if (!pocket.empty( )) 
	    obj->pocket = pocket;

	toRoom << "$c1 ������ $o4 "
	       << (IS_SET( container->value[1], CONT_PUT_ON|CONT_PUT_ON2 ) ?
			     "��" : "�")
	       << " $O4.";
	
	if (pocket.empty( ))
	    toChar << "�� ������� $o4 "
		   << (IS_SET( container->value[1], CONT_PUT_ON|CONT_PUT_ON2 ) ?
			     "��" : "�")
		   << " $O4.";
	else {
	    toChar << "�� ������� $o4 ";

	    if (IS_SET(container->value[1],CONT_PUT_ON|CONT_PUT_ON2)) {
		toChar << "�� $O4 � ��������� '" << pocket << "'.";
	    }
	    else if (!container->can_wear(ITEM_TAKE)) {
		toChar << "�� ����� $O2 � �������� '" << pocket << "'.";
	    }
	    else
		toChar << "� ������ $O2 � �������� '" << pocket << "'.";
	}
    }
    
    if (!oprog_put_msg( obj, ch, container )) {
	act( toRoom.str( ).c_str( ), ch, obj, container, TO_ROOM );
	act( toChar.str( ).c_str( ), ch, obj, container, TO_CHAR );
    }

    return oprog_put( obj, ch, container );
}

CMDRUNP( put )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    DLString pocket;
    Object *container;
    Object *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (arg_is_in( arg2 ) || arg_is_on( arg2 ))
	argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	ch->send_to("�������� ��� � ����?\n\r");
	return;
    }

    if (arg_is_alldot( arg2 ))
    {
	ch->send_to("�� �� ������ ������� �����.\n\r");
	return;
    }
    
    pocket = get_pocket_argument( arg2 );

    if ( ( container = get_obj_here( ch, arg2 ) ) == 0 ) {
	act_p( "�� �� ������ ����� $T.", ch, 0, is_number(arg2) ? "�����" : arg2, TO_CHAR,POS_RESTING);
	return;
    }
    
    if (!can_put_into( ch, container, pocket ))
	return;

    if (!arg_is_alldot( arg1 ))
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == 0 )
	{
	    ch->send_to("� ���� ��� �����.\n\r");
	    return;
	}
	
	if (can_put_obj_into( ch, obj, container, pocket, true ) == PUT_OBJ_OK) 
	    put_obj_container( ch, obj, container, pocket );
    }
    else
    {
	Object *obj_next;
	bool found = false;

	/* 'put all container' or 'put all.obj container' */
	for (obj = ch->carrying; obj != 0; obj = obj_next) {
	    obj_next = obj->next_content;

	    if ((arg1[3] == '\0' || obj_has_name( obj, &arg1[4], ch ))
		&&   ch->can_see( obj )
		&&   obj->wear_loc == wear_none)
	    {

		switch (can_put_obj_into( ch, obj, container, pocket, false )) {
		case PUT_OBJ_STOP:
		    return;

		case PUT_OBJ_OK:
		    if (put_obj_container( ch, obj, container, pocket ))
			return;
		    found = true;
		    break;

		case PUT_OBJ_ERR:
		    break;
		}
	    }
	}
	
	if (!found) {
	    if (container->item_type == ITEM_KEYRING)
		act( "�� �� ���$g��|��|�� ������, ��� ����� �������� �� $o4.", ch, container, 0, TO_CHAR );
	    else
		act( "�� �� ���$g��|��|�� ������, ��� ����� �������� � $o4.", ch, container, 0, TO_CHAR );
        }
    }
}



/*
 * DROP OBJECT
 */

bool can_drop_obj( Character *ch, Object *obj, bool verbose )
{
    if (!IS_SET(obj->extra_flags, ITEM_NODROP))
	return true;

    if (!ch->is_npc() && ch->getRealLevel( ) >= LEVEL_IMMORTAL)
	return true;

    if (verbose)
	ch->println("�� �� ������ ���������� �� �����.");

    return false;
}

static bool oprog_drop( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Drop", "C", ch )
    FENIA_NDX_CALL( obj, "Drop", "OC", obj, ch )
    BEHAVIOR_CALL( obj, drop, ch )
    SKILLEVENT_CALL( ch, dropItem, ch, obj );

    return false;
}

#define DROP_OBJ_NORMAL  0
#define DROP_OBJ_EXTRACT 1

static int drop_obj( Character *ch, Object *obj )
{
    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );

    if (!IS_AFFECTED(ch, AFF_SNEAK))
	act( "$c1 ������� $o4.", ch, obj, 0, TO_ROOM );

    act( "�� �������� $o4.", ch, obj, 0, TO_CHAR );

    if (oprog_drop( obj, ch ))
	return DROP_OBJ_EXTRACT;
    
    if (IS_WATER( ch->in_room ) 
	&& !obj->may_float( ) 
	&& material_swims( obj ) == SWIM_NEVER)
    {
	if (!IS_AFFECTED(ch, AFF_SNEAK))
	    ch->recho( "%1$^O1 ���%1$n��|�� � %2$N6.", obj, ch->in_room->liquid->getShortDescr( ).c_str( ) );

	ch->pecho( "%1$^O1 ���%1$n��|�� � %2$N6.", obj, ch->in_room->liquid->getShortDescr( ).c_str( ) );
    }
    else if (IS_OBJ_STAT(obj, ITEM_MELT_DROP))
    {
	if (!IS_AFFECTED(ch, AFF_SNEAK))
	    ch->recho( "%1$^O1 ��������%1$n����|���� � ���.", obj );

	ch->pecho( "%1$^O1 ��������%1$n����|���� � ���.", obj );
    }
    else if (!IS_WATER( ch->in_room ) 
	     && ch->in_room->sector_type != SECT_AIR
	     && ch->in_room->sector_type != SECT_FOREST
	     && ch->in_room->sector_type != SECT_DESERT
	     && obj->pIndexData->vnum == OBJ_VNUM_POTION_VIAL
//	     && material_is_flagged( obj, MAT_FRAGILE )
	     && chance( 40 ))
    {
	if (!IS_AFFECTED(ch, AFF_SNEAK))
	    ch->recho( "%1$^O1 ������ � �������%1$n����|���� �� ������ �������.", obj );
	
	ch->pecho( "%1$^O1 ������ � �������%1$n����|���� �� ������ �������.", obj );
    }
    else
	return DROP_OBJ_NORMAL;

    extract_obj( obj );
    return DROP_OBJ_EXTRACT;
}

CMDRUNP( drop )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    DLString arguments = argument;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	ch->send_to("������� ���?\n\r");
	return;
    }

    if ( is_number( arg ) && get_arg_id( arg ) == 0)
    {
	/* 'drop NNNN coins' */
	int amount, gold = 0, silver = 0;

	amount   = atoi(arg);
	argument = one_argument( argument, arg );

	if (!parse_money_arguments( ch, arg, amount, gold, silver ))
	    return;

	ch->silver -= silver;
	ch->gold -= gold;
	get_money_here( ch->in_room->contents, gold, silver );
	obj = create_money( gold, silver );

	if ( IS_WATER( ch->in_room ) )
	{
	    extract_obj( obj );
	    if ( !IS_AFFECTED(ch, AFF_SNEAK) )
		act("������ ������ � ����� � $n6.", ch, ch->in_room->liquid->getShortDescr( ).c_str( ), 0, TO_ROOM);

	    act("������ ������ � ����� � $n6.", ch, ch->in_room->liquid->getShortDescr( ).c_str( ), 0, TO_CHAR);
	}
	else
	{
	    obj_to_room( obj, ch->in_room );

	    if ( !IS_AFFECTED(ch, AFF_SNEAK) )
		act_p( "$c1 ������� ��������� �����.", ch, 0, 0, TO_ROOM,POS_RESTING);

            ch->println( "�� �������� ��������� �����." );
	}

	return;
    }

    if (!arg_is_alldot( arg ))
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == 0 )
	{
	    ch->send_to("� ���� ��� �����.\n\r");
	    return;
	}

	if (can_drop_obj( ch, obj, true )) 
	    drop_obj( ch, obj );
    }
    else { /* 'drop all' or 'drop all.obj', drop all.'obj names' */
	bool found = false;
	Object *obj_next;

	dreamland->removeOption( DL_SAVE_OBJS );

        bool fAll = arg[3] == '\0';
        DLString objnames; 
        if (!fAll)
            objnames = DLString(arguments.substr(4)).getOneArgument( );
        
	for (obj = ch->carrying; obj != 0; obj = obj_next) {
	    obj_next = obj->next_content;

            if (!fAll && !obj_has_name( obj, objnames, ch ))
		continue;
	    if (!ch->can_see( obj ))
		continue;
	    if (obj->wear_loc != wear_none)
		continue;
	    if (!can_drop_obj( ch, obj, true ))
		continue;

	    found = true;
	    drop_obj( ch, obj );
	}

	dreamland->resetOption( DL_SAVE_OBJS );

	if (!found) {
	    if (arg[3] == '\0')
		act( "� ���� ������ ���.", ch, 0, arg, TO_CHAR );
	    else
		act( "� ���� ��� $T.", ch, 0, is_number(&arg[4]) ? "�����":&arg[4], TO_CHAR );
	}
	else {
	    save_items( ch->in_room );
	}
    }
}



/*
 * 'give' command
 */
#define GIVE_MODE_USUAL   0
#define GIVE_MODE_PRESENT 1
bool omprog_give( Object *obj, Character *ch, Character *victim )
{
    FENIA_CALL( obj, "Give", "CC", ch, victim )
    FENIA_NDX_CALL( obj, "Give", "OCC", obj, ch, victim )
    BEHAVIOR_VOID_CALL( obj, give, ch, victim )

    if (obj->carried_by != victim)
	return true;
    
    FENIA_CALL( victim, "Give", "CO", ch, obj );
    FENIA_NDX_CALL( victim->getNPC( ), "Give", "CCO", victim, ch, obj );
    BEHAVIOR_VOID_CALL( victim->getNPC( ), give, ch, obj );
	
    if (obj->carried_by != victim)
	return true;
	
    return oprog_get( obj, victim );
}

static bool oprog_present( Object *obj, Character *ch, Character *victim )
{
    FENIA_CALL( obj, "Present", "CC", ch, victim );
    FENIA_NDX_CALL( obj, "Present", "OCC", obj, ch, victim );
    return false;
}

static void give_obj_char( Character *ch, Object *obj, Character *victim, int mode = GIVE_MODE_USUAL )
{
    if (ch == victim) {
	ch->printf("%s ����?\n\r", (mode ? "��������" : "����"));
	return;
    }

    if ( !victim->is_npc() && IS_GHOST( victim ) )
    {
	ch->printf("����� ����� ���-�� %s ��������?\n\r", (mode ? "��������" : "����"));
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	ch->send_to("�� �� ������ ���������� �� �����.\n\r");
	return;
    }

    if ( victim->carry_number + obj->getNumber( ) > victim->canCarryNumber( ) )
    {
	act_p( "$C1 �� ����� ����� ������� �����.", ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }

    if (victim->getCarryWeight( ) + obj->getWeight( ) > victim->canCarryWeight( ) )
    {
	act_p( "$C1 �� ����� ����� ����� �������.", ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }

    if ( !victim->can_see( obj ) )
    {
	act_p( "$C1 �� ����� �����.", ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }

    if (obj->pIndexData->limit != -1)
    {
	if (obj->isAntiAligned( victim )) {
	    ch->pecho("%1$^C1 �� ������ ������� ���� �����.", victim);
	    return;
	}
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );

    switch (mode) {
    case GIVE_MODE_USUAL:
    default:
	act( "$c1 ���� $o4 $C3.", ch, obj, victim, TO_NOTVICT );
	act( "$c1 ���� ���� $o4.", ch, obj, victim, TO_VICT );
	act( "�� ����� $o4 $C3.", ch, obj, victim, TO_CHAR );
	break;

    case GIVE_MODE_PRESENT:
	act( "$c1 ����� $o4 $C3.", ch, obj, victim, TO_NOTVICT );
	act( "$c1 ����� ���� $o4.", ch, obj, victim, TO_VICT );
	act( "�� ������ $o4 $C3.", ch, obj, victim, TO_CHAR );

	if (oprog_present( obj, ch, victim ))
	    return;

	break;
    }
    
    omprog_give( obj, ch, victim );
}

static bool mprog_bribe( Character *victim, Character *giver, int gold, int silver )
{
    FENIA_CALL( victim, "Bribe", "Cii", giver, gold, silver );
    FENIA_NDX_CALL( victim->getNPC( ), "Bribe", "CCii", victim, giver, gold, silver );
    BEHAVIOR_VOID_CALL( victim->getNPC( ), bribe, giver, gold, silver );
    return false;
}

/* 'give NNNN coins victim' */
static void give_money_char( Character *ch, int gold, int silver, Character *victim, int mode = GIVE_MODE_USUAL )
{
    if (ch == victim)
    {
	    ch->send_to("���� ����?\n\r");
	    return;
    }

    if ( !victim->is_npc() && IS_GHOST( victim ) )
    {
	    ch->send_to("����� ����� ���-�� ���� ��������?\n\r");
	    return;
    }

    if( ( victim->getCarryWeight( ) + gold + silver / 10 ) > victim->canCarryWeight( ) )
    {
	    act_p( "$c1 �� ����� ����� ����� ���.", victim, 0, ch, TO_VICT,POS_RESTING);
	    return;
    }

    ch->silver	-= silver;
    ch->gold	-= gold;
    victim->silver 	+= silver;
    victim->gold	+= gold;
    
    if (silver > 0) {
	DLString slv( silver );
	if (mode == GIVE_MODE_PRESENT) {
	    act( "$c1 ����� ���� $t �������.", ch, slv.c_str( ), victim, TO_VICT);
	    act("�� ������ $C3 $t �������.",ch, slv.c_str( ), victim, TO_CHAR);
	} else {
	    act( "$c1 ���� ���� $t �������.", ch, slv.c_str( ), victim, TO_VICT);
	    act("�� ����� $C3 $t �������.",ch, slv.c_str( ), victim, TO_CHAR);
	}
    }
    
    if (gold > 0) {
	DLString gld( gold );
	if (mode == GIVE_MODE_PRESENT) {
	    act( "$c1 ����� ���� $t ������.", ch, gld.c_str( ), victim, TO_VICT);
	    act("�� ������ $C3 $t ������.",ch, gld.c_str( ), victim, TO_CHAR);
	} else {
	    act( "$c1 ���� ���� $t ������.", ch, gld.c_str( ), victim, TO_VICT);
	    act("�� ����� $C3 $t ������.",ch, gld.c_str( ), victim, TO_CHAR);
	}
    }

    if (mode == GIVE_MODE_PRESENT) {
	act( "$c1 ����� $C3 ��������� �����.",  ch, 0, victim, TO_NOTVICT);
    } else {
	act( "$c1 ���� $C3 ��������� �����.",  ch, 0, victim, TO_NOTVICT);
    }
    
    mprog_bribe( victim, ch, gold, silver );
}

static void give_money( Character *ch, char *arg1, char *arg2, char *argument, int mode = GIVE_MODE_USUAL )
{
    Character *victim;
    int amount;
    int gold = 0, silver = 0;

    amount   = atoi(arg1);
    if (!parse_money_arguments( ch, arg2, amount, gold, silver ))
	return;

    argument = one_argument( argument, arg2 );
    if ( arg2[0] == '\0' )
    {
	    ch->send_to("���� ��� � ����?\n\r");
	    return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
    {
	    ch->send_to("��� ����� ���.\n\r");
	    return;
    }

    give_money_char( ch, gold, silver, victim, mode );
}

CMDRUNP( give )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    Character *victim;
    Object  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	    ch->send_to("���� ��� � ����?\n\r");
	    return;
    }

    if (is_number( arg1 ) && get_arg_id( arg1 ) == 0) {
	give_money( ch, arg1, arg2, argument );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == 0 )
    {
	ch->send_to("� ���� ��� �����.\n\r");
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
    {
	ch->send_to("����, ���� ��� �����?.\n\r");
	return;
    }
    
    give_obj_char( ch, obj, victim );
}

CMDRUNP( present )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    Character *victim;
    Object *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (arg1[0] == '\0' || arg2[0] == '\0') {
	ch->println( "��� � ���� �� ������ ��������?" );
	return;
    }

    if (is_number( arg1 ) && get_arg_id( arg1 ) == 0) {
	give_money( ch, arg1, arg2, argument, GIVE_MODE_PRESENT );
	return;
    }

    if (( obj = get_obj_carry( ch, arg1 ) ) == 0) {
	ch->println( "� ���� ��� �����." );
	return;
    }

    if (( victim = get_char_room( ch, arg2 ) ) == 0) {
	ch->println( "��� ����, �� ���������� ��������." );
	return;
    }
    
    give_obj_char( ch, obj, victim, GIVE_MODE_PRESENT );
}

/*
 * 'sacrifice' command
 */
static bool can_sacrifice( Character *ch, Object *obj, bool needSpam ) 
{
	if (!ch->can_see( obj ))
	    return false;

	if (obj->item_type == ITEM_CORPSE_PC)
	{
		if (needSpam)
			ch->send_to("����� ��� �� ����������.\n\r");
		return false;
	}

	if ( !obj->can_wear(ITEM_TAKE) || obj->can_wear(ITEM_NO_SAC) 
	     || IS_SET(obj->extra_flags, ITEM_NOSAC))
	{
		if (needSpam) 
		    ch->pecho( "%1$^O1 �� ������%1$n��|�� ����������������.", obj );
		return false;
	}

	if ( IS_SET(obj->item_type,ITEM_FURNITURE)
		&& ( count_users(obj) > 0 ) )
	{
		if (needSpam) 
		    ch->pecho( "%1$^O1 ��������%1$n����|����.", obj );
		return false;
	}

  return true;
}


static int rescue_nosac_items ( Object *container, Room *room )
{
    Object *item;
    Object *obj_next;
    int count = 0;
    
    for ( item = container->contains; item; item = obj_next ) {
	obj_next = item->next_content;
	
	if (item->contains) 
	    count += rescue_nosac_items(item, room);

	if (item->can_wear(ITEM_NO_SAC) || IS_SET(item->extra_flags, ITEM_NOSAC)) {
	    obj_from_obj(item);
	    obj_to_room(item, room);	
	    count++;
	}
    }
    
    return count;
}

static bool oprog_sac( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Sac", "C", ch )
    FENIA_NDX_CALL( obj, "Sac", "OC", obj, ch )
    BEHAVIOR_CALL( obj, sac, ch )
    return false;
}


int sacrifice_obj( Character *ch, Object *obj, bool needSpam )
{
	int silver = -1;

	if ( !can_sacrifice(ch, obj, needSpam) )
		return -1;

        silver = number_range(number_fuzzy(obj->level), obj->cost / 10);

	if (needSpam)
	    act_p( "$c1 �������� � ������ ����� $o4.", ch, obj, 0, TO_ROOM,POS_RESTING);

	if (oprog_sac( obj, ch ))
		return silver;
	
	if (needSpam)
	    wiznet( WIZ_SACCING, 0, 0, "%^C1 �������� �� ����������� %O4.", ch, obj );

	if (rescue_nosac_items(obj, ch->in_room)) 
	    if (needSpam)
		act( "��������� ����, ������� � $o6, �� ����� ���� ��������� � ������ � ������ $T.", 
		     ch, obj, terrains[ch->in_room->sector_type].fall, TO_ALL );

	extract_obj( obj );
	return silver;
}

CMDRUNP( sacrifice )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	Object *obj, *next_obj;
	int silver, mana_gain;

	mana_gain=-1;

	one_argument( argument, arg );

	if ( arg[0] == '\0' || is_name( arg, ch->getNameP( '7' ).c_str() ) )
	{
		act_p( "$c1 ���������� ���� � ������ �����, �� ��� ������� ������������.",
			ch, 0, 0, TO_ROOM,POS_RESTING);
		ch->send_to("���� ������� ���� ������ � �������� ������ �� �����.\n\r");
		return;
	}

	if (IS_SET( ch->in_room->room_flags, ROOM_NOSAC )) {
            ch->send_to("��� �� ����� ������� ���� ������.\r\n");
	    return;
	}

	if (arg_is_all( arg ))
	{
		int count = 0;
		obj = ch->in_room->contents;

		silver = 0;
		dreamland->removeOption( DL_SAVE_OBJS );

		while( obj!=0 )
		{
			if (can_sacrifice( ch, obj, false))
			{
				next_obj = obj->next_content;
				silver += sacrifice_obj( ch, obj, false);
				count++;
				obj = next_obj;
			}
			else
				obj = obj->next_content;
		}

		dreamland->resetOption( DL_SAVE_OBJS );

		save_items( ch->in_room );
		
		if (count == 0) {
		    act("�� �� ���$g��|��|�� ������ ����������� ��� ����������������.", ch, 0, 0, TO_CHAR);
		    return;
		}
		
		act( "$c1 �������� � ������ ����� ���, ��� ��������� $T.", ch, 0, terrains[ch->in_room->sector_type].where, TO_ROOM );
		wiznet( WIZ_SACCING, 0, 0, "%^C1 sends up all items in %s as a burnt offering.", ch, ch->in_room->name );

		if (silver==0) {
		    return;
		}
	}
	else
	{
		obj = get_obj_list( ch, arg, ch->in_room->contents );
		if ( obj == 0 )
		{
			ch->send_to("�� �� �������� ���.\n\r");
			return;
		}

		if ( (  obj->item_type == ITEM_CORPSE_NPC  )
			&& number_percent() < gsn_crusify->getEffective( ch ) )
		{
			mana_gain = ch->getModifyLevel();
			gsn_crusify->improve( ch, true );
		} 	

		if ( ( silver=sacrifice_obj(ch, obj, true) )<0 )
			return;
	}

	if (mana_gain != -1 )
	{
		ch->mana += mana_gain;
		sprintf(buf,"���� ���� ���� %d ������� �� ��������.\n\r", mana_gain);
		ch->send_to(buf);
	}

	sprintf(buf,"���� ���� ���� %d ��������%s �� ����������������.\n\r",
		silver,GET_COUNT(silver,"�� ������","�� ������","�� �����"));
	ch->send_to(buf);

	ch->silver += silver;

	if (IS_SET(ch->act,PLR_AUTOSPLIT))
	    if (silver > 1)
		if (party_members_room( ch ).size( ) > 1)
		    interpret_raw( ch, "split", "%d", silver );
}

// ��� ��������� ������� "������������� �������" ,)
static void mprog_vomit( Character *ch )
{
    FENIA_VOID_CALL( ch, "Vomit", "C", ch );
    FENIA_NDX_VOID_CALL( ch->getNPC( ), "Vomit", "CC", ch, ch );

    for (Object *obj = ch->carrying; obj; obj = obj->next_content) {
	FENIA_VOID_CALL( obj, "Vomit", "C", ch );
	FENIA_NDX_VOID_CALL( obj, "Vomit", "OC", obj, ch );
    }
}

CMDRUNP( vomit )
{
    if (!ch->is_npc( )) {
	if (desire_bloodlust->applicable( ch->getPC( ) )) {
	    ch->send_to("�� ���� ������, �� ��� ��?\n\r");
	    return;
	}
	
	desire_full->vomit( ch->getPC( ) );
	desire_hunger->vomit( ch->getPC( ) );
	desire_thirst->vomit( ch->getPC( ) );
    }

    act_p("$c1 ���������� ��� ������ � ��� � �������� �������.",ch,0,0,TO_ROOM,POS_RESTING);
    ch->send_to("�� ���������� ���� ������� ������������� �������.\n\r");

    mprog_vomit( ch );
}

/*
 * fenia-related commands: use 
 */
static bool oprog_use( Object *obj, Character *ch, const char *argument )
{
    FENIA_CALL( obj, "Use", "Cs", ch, argument );
    FENIA_NDX_CALL( obj, "Use", "OCs", obj, ch, argument );
    BEHAVIOR_CALL( obj, use, ch, argument );
    SKILLEVENT_CALL( ch, useItem, ch, obj, argument );

    switch(obj->item_type) {
        case ITEM_POTION:
            if (obj->carried_by != ch || obj->wear_loc != wear_none) 
                ch->pecho("%1$^O1 ����%1$G��|��|�� ���������� � ����� ���������.", obj);
            else {
                DLString idArg = DLString( obj->getID( ) ) + " " + argument;
                interpret_cmd( ch, "quaff", idArg.c_str( ) );
             }
            return true;
        case ITEM_SCROLL:
            if (obj->carried_by != ch || obj->wear_loc != wear_none) 
                ch->pecho("%1$^O1 ����%1$G��|��|�� ���������� � ����� ���������.", obj);
            else {
                DLString idArg = DLString( obj->getID( ) ) + " " + argument;
                interpret_cmd( ch, "recite", idArg.c_str( ) );
            }
            return true;
        case ITEM_WAND:
            if (obj->wear_loc != wear_hold) 
                ch->pecho("%1$^O4 ������ ���������� ������ � �����.", obj);
            else
                interpret_cmd( ch, "zap", argument );
            return true;
        case ITEM_STAFF:
            if (obj->wear_loc != wear_hold) 
                ch->pecho("%1$^O4 ������ ���������� ������ � �����.", obj);
            else
                interpret_cmd( ch, "brandish", argument );
            return true;
    }

    return false;
}

CMDRUNP( use )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;

    argument = one_argument( argument, arg );

    if (!arg[0]) {
	ch->send_to("��������������� ���?\n\r");
	return;
    }

    if ( !( obj = get_obj_here( ch, arg ) ))
    {
	act_p( "�� �� ������ ����� �����.", ch, 0, 0, TO_CHAR,POS_RESTING);
	return;
    }
    
    if (oprog_use( obj, ch, argument ))
	return;
    
    if (obj->carried_by == ch) {
	act("�� ������� � ����� $o4, �� ����, ��� � ���� ������.", ch, obj, 0, TO_CHAR);
	act("$c1 ������ � ����� $o4, ���� �� ����, ��� � ���� ������.", ch, obj, 0, TO_ROOM);
    } else {
	act("�� ���������� ���������� $o4, �� ����, ��� � ���� ������.", ch, obj, 0, TO_CHAR);
	act("$c1 ���������� ��������� $o4, ���� �� ����, ��� � ���� ������.", ch, obj, 0, TO_ROOM);
    }
}	

/*
 * commands with separate sub-cmds: make, throw, search
 */
CMDRUNP( make )
{
    DLString args = argument, arg = args.getOneArgument( );

    if (!arg.empty( )) {
	if (arg.strPrefix( "arrow" ) || arg.strPrefix( "������" ) || arg.strPrefix( "������")) {
	    interpret_cmd( ch, "makearrow", args.c_str( ) );
	    return;
	}
		
	if (arg.strPrefix( "bow" ) || arg.strPrefix( "���" )) {
	    interpret_cmd( ch, "makebow", args.c_str( ) );
	    return;
	}
    }

    ch->println("�� ������ ���������� ������ ���(bow) ��� ������(arrow).");
}

CMDRUNP( search )
{
    DLString args = argument, arg = args.getOneArgument( );

    if (!arg.empty( )) {
	if (arg.strPrefix( "stones" ) || arg.strPrefix( "�����" )) {
	    interpret_cmd( ch, "searchstones", args.c_str( ) );
	    return;
	}
    }

    ch->println("�� ������ ������ ������ �����(stones).");
}

CMDRUNP( throw )
{
    DLString args = argument, arg = args.getOneArgument( );

    if (!arg.empty( )) {
	if (arg.strPrefix( "spear" ) || arg.strPrefix( "�����" )) {
	    interpret_cmd( ch, "throwspear", args.c_str( ) );
	    return;
	}

	if (arg.strPrefix( "stone" ) || arg.strPrefix( "������" )) {
	    interpret_cmd( ch, "throwstone", args.c_str( ) );
	    return;
	}
    }

    interpret_cmd( ch, "throwdown", argument );
}



