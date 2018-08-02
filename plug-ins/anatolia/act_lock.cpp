/* $Id$
 *
 * ruffina, 2004
 */
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "commandtemplate.h"
#include "objectbehavior.h"
#include "object.h"
#include "affect.h"
#include "room.h"
#include "pcharacter.h"
#include "character.h"

#include "save.h"
#include "act.h"
#include "act_move.h"
#include "act_lock.h"
#include "gsn_plugin.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

#define OBJ_VNUM_CORK 19 

GSN(golden_eye);
Object * get_obj_list_vnum( Character *ch, int vnum, Object *list );

static Object * get_key_carry( Character *ch, int vnum )
{
    Object *key, *ring;
    
    if (( key = get_obj_carry_vnum( ch, vnum ) ))
	return key;

    for (ring = get_obj_carry_type( ch, ITEM_KEYRING );
	 ring;
	 ring = get_obj_list_type( ch, ITEM_KEYRING, ring->next_content ))
    {
	if (( key = get_obj_list_vnum( ch, vnum, ring->contains ) ))
	    return key;
    }

    return NULL;
}

/*--------------------------------------------------------------------
 *    open 
 *-------------------------------------------------------------------*/
static bool oprog_cant_open( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "CantOpen", "C", ch );
    FENIA_NDX_CALL( obj, "CantOpen", "OC", obj, ch );
    return false;
}

static bool oprog_open(Object *obj, Character *ch)
{
    FENIA_CALL( obj, "Open", "C", ch );
    FENIA_NDX_CALL( obj, "Open", "OC", obj, ch );
    return false;
}

static bool oprog_open_msg(Object *obj, Character *ch)
{
    FENIA_CALL( obj, "OpenMsg", "C", ch );
    FENIA_NDX_CALL( obj, "OpenMsg", "OC", obj, ch );
    return false;
}

void open_door_extra ( Character *ch, int door, void *pexit )
{
    Room *to_room;
    EXIT_DATA *pexit_rev = 0;
    int exit_info;
    bool eexit = door == DIR_SOMEWHERE;

    if ( !pexit )
	    return;

    exit_info = eexit?
		    ((EXTRA_EXIT_DATA *) pexit)->exit_info
	    : ((EXIT_DATA *) pexit)->exit_info;

    if ( !IS_SET(exit_info, EX_CLOSED) )
    {
	    ch->println( "����� ��� �������." );
	    return;
    }

    if ( IS_SET(exit_info, EX_LOCKED) )
    {
	    ch->println( "����� �������." );
	    return;
    }

    REMOVE_BIT( eexit?
		    ((EXTRA_EXIT_DATA *) pexit)->exit_info
	    : ((EXIT_DATA *) pexit)->exit_info, EX_CLOSED);

    if ( eexit ) {
	act( "$c1 ��������� $n4.", ch, ((EXTRA_EXIT_DATA *) pexit)->short_desc_from, 0, TO_ROOM );
	act( "�� ���������� $n4.", ch, ((EXTRA_EXIT_DATA *) pexit)->short_desc_from, 0, TO_CHAR );
    }
    else {
	act( "$c1 ��������� $d.", ch, 0, ((EXIT_DATA *) pexit)->keyword, TO_ROOM );
	act( "�� ���������� $d.", ch, 0, ((EXIT_DATA *) pexit)->keyword, TO_CHAR );
    }


    /* open the other side */
    if ( !eexit
	    && ( to_room   = ((EXIT_DATA *) pexit)->u1.to_room) != 0
	    && ( pexit_rev = to_room->exit[dirs[door].rev] ) != 0
	    && pexit_rev->u1.to_room == ch->in_room )
    {
	    Character *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != 0; rch = rch->next_in_room )
		    act_p( "$d �����������.", rch, 0, pexit_rev->keyword, TO_CHAR,POS_RESTING );
    }
}

void open_door ( Character *ch, int door )
{
    if ( door < 0 || door > 5 )
	    return;

    open_door_extra( ch, door, ch->in_room->exit[door] );
}

bool open_portal( Character *ch, Object *obj )
{
    if ( !IS_SET(obj->value[1], EX_ISDOOR) )
    {
	    ch->println( "�� �� ������ ������� �����." );
	    return false;
    }

    if ( !IS_SET(obj->value[1], EX_CLOSED) )
    {
	    ch->println( "��� ��� �������." );
	    return false;
    }

    if ( IS_SET(obj->value[1], EX_LOCKED) )
    {
	    ch->println( "����� �������." );
	    return false;
    }

    REMOVE_BIT(obj->value[1], EX_CLOSED);
    act_p("�� ���������� $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
    act_p("$c1 ��������� $o4.",ch,obj,0,TO_ROOM,POS_RESTING);

    return true;
}

bool open_drink_container( Character *ch, Object *obj )
{
    if (!IS_SET(obj->value[3], DRINK_CLOSED)) {
	ch->pecho( "%1$^O1 � ��� �� �����%1$G��|�|��.", obj );
	return false;
    }
    
    if (IS_SET(obj->value[3], DRINK_LOCKED)) {
	if (IS_SET(obj->value[3], DRINK_CLOSE_CORK))
	    ch->pecho( "%1$^O1 ������ ��������%1$G��|�|�� �������, ����� ������.", obj );
	else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL))
	    ch->pecho( "%1$^O1 �����%1$G��|�|�� ������� � ��������%1$G�|��|��.", obj );
	else if (IS_SET(obj->value[3], DRINK_CLOSE_KEY))
	    ch->pecho( "%1$^O1 ������ �����%1$G��|�|��.", obj );
	else
	    ch->pecho( "%1$^O1 �����%1$G��|�|��.", obj );

	return false;
    }
    
    REMOVE_BIT(obj->value[3], DRINK_CLOSED);

    if (IS_SET(obj->value[3], DRINK_CLOSE_CORK)) {
	Object *cork;

	cork = create_object( get_obj_index( OBJ_VNUM_CORK ), 0 );
	obj_to_char( cork, ch );

	act( "�� ��������� ������ �� $O2.", ch, 0, obj, TO_CHAR );
	act( "$c1 �������� ������ �� $O2.", ch, 0, obj, TO_ROOM );
    }
    else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL)) {
	act( "�� ���������� ������ $O2.", ch, 0, obj, TO_CHAR );
	act( "$c1 ��������� ������ $O2.", ch, 0, obj, TO_ROOM );
    }
    else {
	act( "�� ���������� $O4.", ch, 0, obj, TO_CHAR );
	act( "$c1 ��������� $O4.", ch, 0, obj, TO_ROOM );
    }

    return true;
}

bool open_container( Character *ch, Object *obj )
{
    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
    {
	    ch->println( "��� ��� �������." );
	    return false;
    }

    if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
    {
	    ch->println( "�� �� ������ ������� �����." );
	    return false;
    }

    if ( IS_SET(obj->value[1], CONT_LOCKED) )
    {
	    ch->println( "����� �������." );
	    return false;
    }
    
    if (oprog_cant_open( obj, ch ))
	return false;

    REMOVE_BIT(obj->value[1], CONT_CLOSED);

    if (!oprog_open_msg( obj, ch )) {
	act_p("�� ���������� $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
	act_p("$c1 ��������� $o4.", ch, obj, 0, TO_ROOM,POS_RESTING );
    }

    oprog_open( obj, ch );
    return true;
}



CMDRUNP( open )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    EXTRA_EXIT_DATA *peexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	ch->println( "������� ���?" );
	return;
    }

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
	open_door( ch, door );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
	bool changed = false;
	
	switch (obj->item_type) {
	case ITEM_PORTAL:
	    changed = open_portal( ch, obj );
	    break;
	    
	case ITEM_DRINK_CON:
	    changed = open_drink_container( ch, obj );
	    break;
	    
	case ITEM_CONTAINER:
	    changed = open_container( ch, obj );
	    break;

	default:
	    ch->pecho( "%^O4 ���������� �������.", obj );
	    return;
	}
	
	if ( obj->in_room != 0 && changed )
	    save_items( obj->in_room );
	    
	return;
    }

    if ( ( ( peexit = get_extra_exit( arg, ch->in_room->extra_exit ) ) != 0 )
	    && ch->can_see( peexit ) )
    {
	open_door_extra( ch, DIR_SOMEWHERE, (void *)peexit );
	return;
    }	

    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}


/*--------------------------------------------------------------------
 *    close 
 *-------------------------------------------------------------------*/
static bool oprog_close( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Close", "C", ch );
    FENIA_NDX_CALL( obj, "Close", "OC", obj, ch );
    return false;
}

static void close_door( Character *ch, int door )
{
    // 'close door'
    Room *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = 0;

    pexit	= ch->in_room->exit[door];
    if ( IS_SET(pexit->exit_info, EX_CLOSED) )
    {
	    ch->println( "����� ��� �������." );
	    return;
    }

    SET_BIT(pexit->exit_info, EX_CLOSED);
    act_p( "$c1 ��������� $d.", ch, 0, pexit->keyword, TO_ROOM,POS_RESTING );
    ch->println( "Ok." );

    // close the other side
    if ( ( to_room   = pexit->u1.to_room            ) != 0
	    && ( pexit_rev = to_room->exit[dirs[door].rev] ) != 0
	    && pexit_rev->u1.to_room == ch->in_room )
    {
	    Character *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != 0; rch = rch->next_in_room )
		    act_p( "$d �����������.", rch, 0, pexit_rev->keyword, TO_CHAR,POS_RESTING );
    }
}

CMDRUNP( close )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    EXTRA_EXIT_DATA *peexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	ch->println( "������� ���?" );
	return;
    }

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
	close_door( ch, door );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
	if ( obj->item_type == ITEM_PORTAL )
	{
	    // portal stuff
	    if ( !IS_SET(obj->value[1],EX_ISDOOR)
		    || IS_SET(obj->value[1],EX_NOCLOSE) )
	    {
		ch->println( "�� �� ������ ������� �����." );
		return;
	    }

	    if ( IS_SET(obj->value[1],EX_CLOSED) )
	    {
		ch->println( "����� ��� �������." );
		return;
	    }

	    SET_BIT(obj->value[1],EX_CLOSED);
	    act_p("�� ���������� $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
	    act_p("$c1 ��������� $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
	}
	else if ( obj->item_type == ITEM_CONTAINER )
	{
	    // 'close object'
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		ch->println( "����� ��� �������." );
		return;
	    }

	    if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    {
		ch->println( "�� �� ������ ������� �����." );
		return;
	    }

	    SET_BIT(obj->value[1], CONT_CLOSED);
	    act_p("�� ���������� $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
	    act_p( "$c1 ��������� $o4.", ch, obj, 0, TO_ROOM,POS_RESTING );
	    oprog_close( obj, ch );
	}
	else if (obj->item_type == ITEM_DRINK_CON) {
	    // cork a bottle 
	    
	    if (!IS_SET(obj->value[3], DRINK_CLOSE_CORK|DRINK_CLOSE_NAIL|DRINK_CLOSE_KEY)) {
		act( "$O4 ���������� ������� ��� ����������.", ch, 0, obj, TO_CHAR );
		return;
	    }

	    if (IS_SET(obj->value[3], DRINK_CLOSED)) {
		act( "$O4 ��� �������.", ch, 0, obj, TO_CHAR );
		return;
	    }
	    
	    if (IS_SET(obj->value[3], DRINK_CLOSE_CORK)) {
		Object *cork = get_obj_carry_vnum( ch, OBJ_VNUM_CORK );

		if (!cork) {
		    act( "� ���� ��� ������ �� $O2.", ch, 0, obj, TO_CHAR );
		    act( "$c1 ����� �� �������� � ������� ������.", ch, 0, obj, TO_ROOM );
		    return;
		}

		extract_obj( cork );
		act( "�� ������������� $O4 �������.", ch, 0, obj, TO_CHAR );
		act( "$c1 ������������ $O4 �������.", ch, 0, obj, TO_ROOM );
	    }
	    else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL)) {
		act( "�� ���������� $O4 �������.", ch, 0, obj, TO_CHAR );
		act( "$c1 ��������� $O4 �������.", ch, 0, obj, TO_ROOM );
	    }
	    else {
		act( "�� ���������� $O4.", ch, 0, obj, TO_CHAR );
		act( "$c1 ��������� $O4.", ch, 0, obj, TO_ROOM );
	    }
	    
	    SET_BIT(obj->value[3], DRINK_CLOSED);
	}
	else {
	    ch->println( "��� �� ���������." );
	    return;
	}

	if ( obj->in_room != 0 )
		save_items( obj->in_room );

	return;
    }

    if ( ( ( peexit = get_extra_exit( arg, ch->in_room->extra_exit ) ) != 0 )
	    && ch->can_see( peexit ) )
    {
	if ( !IS_SET(peexit->exit_info, EX_ISDOOR) )
	{
		ch->println( "��� �� �����!" );
		return;
	}

	if ( IS_SET(peexit->exit_info, EX_CLOSED) )
	{
		ch->println( "����� ��� �������." );
		return;
	}

	SET_BIT(peexit->exit_info, EX_CLOSED);
	act_p( "$c1 ��������� $N4.", ch, 0, peexit->short_desc_from, TO_ROOM,POS_RESTING );
	ch->println( "Ok." );

	return;
    }	

    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}


/*--------------------------------------------------------------------
 *   lock 
 *-------------------------------------------------------------------*/
static void lock_door( Character *ch, int door )
{
    // 'lock door'
    Room *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = 0;
    Character *rch;

    pexit	= ch->in_room->exit[door];
    if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
    {
	    ch->println( "����� �� �������." );
	    return;
    }

    if ( IS_SET(pexit->exit_info, EX_NOLOCK) ) 
    {
	ch->println( "��� ���������� ��������." );
	return;
    }

    if ( pexit->key <= 0 )
    {
	    ch->println( "����� ��� �������� ��������." );
	    return;
    }

    if (!get_key_carry( ch, pexit->key))
    {
	    ch->println( "� ���� ��� �����." );
	    return;
    }

    if ( IS_SET(pexit->exit_info, EX_LOCKED) )
    {
	    ch->println( "����� ��� �������." );
	    return;
    }

    SET_BIT(pexit->exit_info, EX_LOCKED);
    ch->println( "*����*" );
    act_p( "$c1 �������� $d �� ����.", ch, 0, pexit->keyword, TO_ROOM,POS_RESTING );

    /* lock the other side */
    if ( ( to_room   = pexit->u1.to_room ) != 0
	    && ( pexit_rev = to_room->exit[dirs[door].rev] ) != 0
	    && pexit_rev->u1.to_room == ch->in_room )
    {
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	    for ( rch = to_room->people; rch != 0; rch = rch->next_in_room )
		    act_p( "$d �������������.", rch, 0, pexit_rev->keyword, TO_CHAR,POS_RESTING );
    }
}

CMDRUNP( lock )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    EXTRA_EXIT_DATA *peexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	    ch->println( "�������� ���?" );
	    return;
    }

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
	lock_door( ch, door );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
	// portal stuff
	if (obj->item_type == ITEM_PORTAL)
	{
		if ( !IS_SET(obj->value[1],EX_ISDOOR)
			|| IS_SET(obj->value[1],EX_NOCLOSE) )
		{
			ch->println( "�� �� ������ ������� �����." );
			return;
		}

		if (!IS_SET(obj->value[1],EX_CLOSED))
		{
			ch->println( "����� �� �������." );
			return;
		}

	    	if (IS_SET(obj->value[1],EX_NOLOCK))
		{
			ch->println( "��� ���������� ��������." );
			return;
		}

		if (obj->value[4] <= 0) 
		{
		    ch->println( "����� ��� �������� ��������." );
		    return;
		}

		if (!get_key_carry(ch,obj->value[4]))
		{
			ch->println( "� ���� ��� �����." );
			return;
		}

		if (IS_SET(obj->value[1],EX_LOCKED))
		{
			ch->println( "����� ��� �������." );
			return;
		}

		SET_BIT(obj->value[1],EX_LOCKED);
		act_p("�� ���������� $o4 �� ����.",ch,obj,0,TO_CHAR,POS_RESTING);
		act_p("$c1 ��������� $o4 �� ����.",ch,obj,0,TO_ROOM,POS_RESTING);
	}
	else if ( obj->item_type == ITEM_CONTAINER )
	{
	    // 'lock object'
	    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		    ch->println( "��� �� �������." );
		    return;
	    }

	    if ( obj->value[2] < 0 )
	    {
		    ch->println( "����� ��� �������� ��������." );
		    return;
	    }
	    
	    if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    {
		    ch->println( "��� ��� �������." );
		    return;
	    }

	    if ((obj->behavior && obj->behavior->canLock( ch ))
		|| get_key_carry( ch, obj->value[2])) 
	    {
		SET_BIT(obj->value[1], CONT_LOCKED);
		act("�� ���������� $o4 �� ����.",ch,obj,0,TO_CHAR);
		act("$c1 ��������� $o4 �� ����.", ch, obj, 0, TO_ROOM);
		
	    } else {
		ch->println( "� ���� ��� �����." );
		return;
	    }
	}
	else if (obj->item_type == ITEM_DRINK_CON) {
	    // lock drink containers

	    if (IS_SET(obj->value[3], DRINK_LOCKED)) {
		if (IS_SET(obj->value[3], DRINK_CLOSE_CORK))
		    ch->pecho( "%1$^O1 � ��� ������ ��������%1$G��|�|�� �������.", obj );
		else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL))
		    ch->pecho( "%1$^O1 � ��� �����%1$G��|�|�� ������� � ��������%1$G��|�|��.", obj );
		else
		    ch->pecho( "%1$^O1 � ��� ������ �����%1$G��|�|��.", obj );
	    }
	    else {
		ch->pecho("%1$^O1 ��� ���������� ���������� ��� ���������� ��������.", obj );
	    }
	    
	    return;
	}
	else {
	    ch->println( "��� �� ���������." );
	    return;
	}

	if ( obj->in_room != 0 )
		save_items( obj->in_room );

	return;
    }

    if ( ( ( peexit = get_extra_exit( arg, ch->in_room->extra_exit ) ) != 0 )
	    && ch->can_see( peexit ) )
    {
	if ( !IS_SET(peexit->exit_info, EX_ISDOOR) )
	{
		ch->println( "��� �� �����!" );
		return;
	}

	if ( !IS_SET(peexit->exit_info, EX_CLOSED) )
	{
		ch->println( "����� �� �������." );
		return;
	}

	if ( IS_SET(peexit->exit_info, EX_NOLOCK) ) 
	{
	    ch->println( "��� ���������� ��������." );
	    return;
	}

	if ( peexit->key <= 0 )
	{
		ch->println( "����� ��� �������� ��������." );
		return;
	}

	if (!get_key_carry( ch, peexit->key))
	{
		ch->println( "� ���� ��� �����." );
		return;
	}

	if ( IS_SET(peexit->exit_info, EX_LOCKED) )
	{
		ch->println( "����� ��� �������." );
		return;
	}

	SET_BIT(peexit->exit_info, EX_LOCKED);
	ch->println( "*����*" );
	act_p( "$c1 �������� $N4 �� ����.", ch, 0, peexit->short_desc_from, TO_ROOM,POS_RESTING );

	return;
    }


    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}



/*--------------------------------------------------------------------
 *    unlock 
 *-------------------------------------------------------------------*/
static void unlock_door( Character *ch, int door )
{
    // 'unlock door'
    Room *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = 0;
    Character *rch;

    pexit = ch->in_room->exit[door];
    if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
    {
	    ch->println( "����� �� �������." );
	    return;
    }

    if ( pexit->key <= 0 )
    {
	    ch->println( "����� ��� �������� ��������." );
	    return;
    }

    if (!get_key_carry( ch, pexit->key))
    {
	    ch->println( "� ���� ��� �����." );
	    return;
    }

    if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
    {
	    ch->println( "����� ��� �� �������." );
	    return;
    }

    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    ch->println( "*����*" );
    act_p( "$c1 ��������� ������ $d.", ch, 0, pexit->keyword, TO_ROOM,POS_RESTING );

    // unlock the other side
    if ( ( to_room = pexit->u1.to_room  ) != 0
	    && ( pexit_rev = to_room->exit[dirs[door].rev] ) != 0
	    && pexit_rev->u1.to_room == ch->in_room )
    {
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	    for ( rch = to_room->people; rch != 0; rch = rch->next_in_room )
		    act_p( "$d �������.", rch, 0, pexit_rev->keyword, TO_CHAR,POS_RESTING );
    }
}

CMDRUNP( unlock )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    int door;
    EXTRA_EXIT_DATA *peexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	    ch->println( "�������� ���?" );
	    return;
    }

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
	unlock_door( ch, door );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
	// portal stuff
	if ( obj->item_type == ITEM_PORTAL )
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {
		    ch->println( "�� �� ������ ����� �������." );
		    return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		    ch->println( "����� �� �������." );
		    return;
	    }

	    if (obj->value[4] <= 0)
	    {
		ch->println( "����� ��� �������� ��������." );
		return;
	    }

	    if (!get_key_carry(ch,obj->value[4]))
	    {
		    ch->println( "� ���� ��� �����." );
		    return;
	    }

	    if (!IS_SET(obj->value[1],EX_LOCKED))
	    {
		    ch->println( "����� ��� �� �������." );
		    return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act_p("�� ���������� ������ $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
	    act_p("$c1 ��������� ������ $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
	}
	else if ( obj->item_type == ITEM_CONTAINER )
	{
	    // 'unlock object'

	    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		    ch->println( "����� �� �������." );
		    return;
	    }

	    if ( obj->value[2] < 0 )
	    {
		    ch->println( "����� ��� �������� ��������." );
		    return;
	    }

	    if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    {
		    ch->println( "����� ��� �� �������." );
		    return;
	    }

	    if ((obj->behavior && obj->behavior->canLock( ch ))
		|| get_key_carry( ch, obj->value[2])) 
	    {
		REMOVE_BIT(obj->value[1], CONT_LOCKED);
		act_p("�� ���������� ������ $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
		act_p("$c1 ��������� ������ $o4.", ch, obj, 0, TO_ROOM,POS_RESTING );
		
	    } else {
		ch->println( "� ���� ��� �����." );
		return;
	    }
	}
	else if ( obj->item_type == ITEM_DRINK_CON ) {
	    Object *key;
	    
	    // uncork a bottle
	    if (!IS_SET(obj->value[3], DRINK_LOCKED)) {
		ch->println( "��� �� ������� � �� ����������." );
		return;
	    }

	    key = get_key_carry( ch, obj->value[4] );

	    if (!key) {
		if (IS_SET(obj->value[3], DRINK_CLOSE_CORK)) 
		    ch->println( "� ���� ����� �������� ������." );
		else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL))
		    ch->println( "� ���� ����� �������� ������." );
		else
		    ch->println( "� ���� ����� ������� ��� �������." );
		
		return;
	    }

	    if (IS_SET(obj->value[3], DRINK_CLOSE_CORK)) {
		act( "�� ������������ ������ � $O6 � ������� $o4.", ch, key, obj, TO_CHAR );
		act( "$c1 ����������� ������ � $O6 � ������� $o4.", ch, key, obj, TO_ROOM );
	    }
	    else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL)) {
		act( "�� ������������ ������ �� ������ $O2 � ������� $o4.", ch, key, obj, TO_CHAR );
		act( "$c1 ����������� ������ �� ������ $O2 � ������� $o4.", ch, key, obj, TO_ROOM );
	    }
	    else {
		act( "�� ���������� $o5 $O2.", ch, key, obj, TO_CHAR );
		act( "$c1 ��������� $o5 $O2.", ch, key, obj, TO_ROOM );
	    }

	    REMOVE_BIT(obj->value[3], DRINK_LOCKED);
		
	}
	else
	{
	    ch->println( "��� �� ���������." );
	    return;
	}

	if ( obj->in_room != 0 )
		save_items( obj->in_room );

	return;
    }

    if ( ( ( peexit = get_extra_exit( arg, ch->in_room->extra_exit ) ) != 0 )
	    && ch->can_see( peexit ) )
    {
	if ( !IS_SET(peexit->exit_info, EX_ISDOOR) )
	{
		ch->println( "��� �� �����!" );
		return;
	}

	if ( !IS_SET(peexit->exit_info, EX_CLOSED) )
	{
		ch->println( "����� �� �������." );
		return;
	}

	if ( peexit->key <= 0 )
	{
		ch->println( "����� ��� �������� ��������." );
		return;
	}

	if (!get_key_carry( ch, peexit->key))
	{
		ch->println( "� ���� ��� �����." );
		return;
	}

	if ( !IS_SET(peexit->exit_info, EX_LOCKED) )
	{
		ch->println( "����� ��� �� �������." );
		return;
	}

	REMOVE_BIT(peexit->exit_info, EX_LOCKED);
	ch->println( "*����*" );
	act_p( "$c1 ��������� ������ $N4.", ch, 0, peexit->short_desc_from, TO_ROOM,POS_RESTING );

	return;
    }

    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}




/*------------------------------------------------------------------------
 * Keyhole base class
 *-----------------------------------------------------------------------*/
const int Keyhole::MAX_KEY_TYPES     = 8;
const int Keyhole::LOCK_VALUE_MULTI  = -1;
const int Keyhole::LOCK_VALUE_BLANK  = -2;
const int Keyhole::ERROR_KEY_TYPE    = -3;

Keyhole::Keyhole( )
          : ch( NULL ), lockpick( NULL ), keyring( NULL ), key( NULL )
{
}

Keyhole::Keyhole( Character *ach ) 
	     : ch( ach )
{
}

Keyhole::Keyhole( Character *ach, Object *akey ) 
	     : ch( ach ), key( akey )
{
}

Keyhole::~Keyhole( )
{
}

Keyhole::Pointer Keyhole::locate( Character *ch, Object *key )
{
    Keyhole::Pointer null;
    int keyVnum = key->pIndexData->vnum;
    
    for (Room *room = room_list; room; room = room->rnext) {
	if (!ch->can_see( room ))
	    continue;

	for (int d = 0; d < DIR_SOMEWHERE; d++)
	    if (room->exit[d] && room->exit[d]->key == keyVnum)
		if (!room->exit[d]->u1.to_room || ch->can_see( room->exit[d] ))
		    return DoorKeyhole::Pointer( NEW, ch, room, d, key );

	for (EXTRA_EXIT_DATA *ex = room->extra_exit; ex; ex = ex->next)
	    if (ex->key == keyVnum)
		if (ch->can_see( ex ))
		    return ExtraExitKeyhole::Pointer( NEW, ch, room, ex, key );
    }

    for (Object *obj = object_list; obj; obj = obj->next) {
	if (!ch->can_see( obj ) 
	    || !ch->can_see( obj->getRoom( ) )
	    || (obj->getCarrier( ) && !ch->can_see( obj->getCarrier( ) )))
	    continue;

	if (obj->item_type == ITEM_PORTAL && obj->value[4] == keyVnum)
	    return PortalKeyhole::Pointer( NEW, ch, obj, key );

	if (obj->item_type == ITEM_CONTAINER && obj->value[2] == keyVnum)
	    return ContainerKeyhole::Pointer( NEW, ch, obj, key );
    }

    return null;
}

Keyhole::Pointer Keyhole::create( Character *ch, const DLString &arg )
{
    Object *obj;
    EXTRA_EXIT_DATA *peexit;
    int door;
    Keyhole::Pointer null;

    if (( obj = get_obj_here( ch, arg.c_str( ) ) )) {
	if (obj->item_type == ITEM_PORTAL)
	    return PortalKeyhole::Pointer( NEW, ch, obj );

	if (obj->item_type == ITEM_CONTAINER)
	    return ContainerKeyhole::Pointer( NEW, ch, obj );

	act( "� $o6 ��� �������� ��������.", ch, obj, 0, TO_CHAR );
	return null;
    }

    if (( peexit = get_extra_exit( arg.c_str( ), ch->in_room->extra_exit ) )
		&& ch->can_see( peexit ))
    {
	return ExtraExitKeyhole::Pointer( NEW, ch, ch->in_room, peexit );
    }

    if (( door = find_exit( ch, arg.c_str( ), 
                            FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY) ) >= 0)
    {
	return DoorKeyhole::Pointer( NEW, ch, ch->in_room, door );
    }

    return null;
}

void Keyhole::argsPickLock( const DLString &arg )
{
    char buf[MAX_INPUT_LENGTH];
    char *pbuf = buf;

    strcpy( buf, arg.c_str( ) );
    
    while (*pbuf++) {
	if (*pbuf == ':') {
	    argLockpick = pbuf + 1;
	    *pbuf = 0;
	    argKeyring = buf;
	    return;
	}
    }

    argLockpick = arg;
}

bool Keyhole::isPickProof( )
{
    return IS_SET(getLockFlags( ), bitPickProof( ));
}

bool Keyhole::isCloseable( )
{
    return IS_SET(getLockFlags( ), bitCloseable( ));
}

bool Keyhole::hasKey( )
{
    return getKey( ) > 0;
}

bool Keyhole::isLockable( )
{
    if (!hasKey( ))
	return false;
    
    if (bitUnlockable( ) == 0)
	return true;
	
    return !IS_SET(getLockFlags( ), bitUnlockable( ));
}

int Keyhole::getLockType( )
{
    return (hasKey( ) ? getKey( ) % MAX_KEY_TYPES : ERROR_KEY_TYPE);
}

bool Keyhole::doPick( const DLString &arg )
{
    bitstring_t flags = getLockFlags( );

    if (!isLockable( )) {
	ch->println( "����� ��� �������� ��������." );
	return false;
    }

    if (!IS_SET(flags, bitLocked( ))) {
	ch->println( "����� ��� �� �������." );
	return false;
    }
    
    if (!checkGuards( ))
	return false;
    
    if (isPickProof( )) {
	ch->println( "���� ����� ������� �� ������." );
	return false;
    }
    
    argsPickLock( arg );

    if (!findLockpick( ))
	return false;
    
    msgTryPickOther( );

    if (!checkLockPick( lockpick )) {
	act( "�� �� ���$g���|�|��� ���������� $o4 � ��� �������� ��������.", ch, lockpick, 0, TO_CHAR );
	ch->setWait( gsn_pick_lock->getBeats( ) / 2 );
	return false;
    }
    
    msgTryPickSelf( );

    if (number_percent( ) >= gsn_pick_lock->getEffective( ch )) {
	if (number_percent( ) >= gsn_pick_lock->getEffective( ch )
	    && number_percent( ) > lockpick->value[1]) 
	{
	    ch->pecho( "  ... �� ������� ����� �������, ������� %1$P2!", lockpick );
	    extract_obj( lockpick );
	}
	else
	    ch->println( "  ... �� ���� ����������� �� � ���� �� ��������." );

	gsn_pick_lock->improve( ch, false );
	ch->setWait( gsn_pick_lock->getBeats( ) );
	return false;
    }
    
    unlock( );

    gsn_pick_lock->improve( ch, true );
    ch->setWait( gsn_pick_lock->getBeats( ) / 2 );
    record( lockpick );
    return true;
}

void Keyhole::unlock( )
{
    REMOVE_BIT(getLockFlags( ), bitLocked( ));
    ch->in_room->echo( POS_RESTING, "*����*" );
}

bool Keyhole::checkLockPick( Object *o )
{
    if (o->item_type != ITEM_LOCKPICK)
	return false;
	
    if (!ch->can_see( o ) && !ch->can_hear( o ))
	return false;
	
    if (o->value[0] == LOCK_VALUE_MULTI)
	return true;
	
    return o->value[0] == getLockType( );
}

bool Keyhole::checkGuards( )
{
    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
	if (rch->is_npc( )
		&& IS_AWAKE(rch)
		&& ch->getModifyLevel( ) + 5 < rch->getModifyLevel( ))
	{
	    act( "$C1 ������ ����� �����, ����������� ����������� �����.", ch, 0, rch, TO_CHAR );
	    return false;
	}

    return true;
}

bool Keyhole::findLockpick( )
{
    if (!argKeyring.empty( )) {
	if (!( keyring = get_obj_list_type( ch, argKeyring, ITEM_KEYRING, ch->carrying ) )) {
	    ch->println( "� ���� ��� ������ ������ ��� ������." );
	    return false;
	}

	if (!( lockpick = get_obj_list_type( ch, argLockpick, ITEM_LOCKPICK, keyring->contains ) )) {
	    act( "�� $o6 �� �������� ������ ��������.", ch, keyring, 0, TO_CHAR );
	    return false;
	}
    }
    else if (!( lockpick = get_obj_list_type( ch, argLockpick, ITEM_LOCKPICK, ch->carrying )) ) {
	ch->println( "� ���� ��� ����� �������." );
	return false;
    }

    return true;
}

void Keyhole::record( Object *obj )
{
    char *ed_text;
    DLString edText, edEntry;
    
    if (!obj->getOwner( ) || ch->getName( ) != obj->getOwner( ))
	return;

    if (!( ed_text = get_extra_descr( obj->getName( ), obj->extra_descr ) ))
	return;
    
    edText  = ed_text;
    edEntry = getDescription( ).ruscase( '2' );
   
    if (edText.find( edEntry ) != DLString::npos)
	return;

    obj->addExtraDescr( obj->getName( ), 
                        edText + "       " + edEntry + "\n" );
}

bool Keyhole::doLore( ostringstream &buf )
{
    if (number_percent( ) >= gsn_golden_eye->getEffective( ch ))
	return false;

    if (!isLockable( )) 
	buf << "��� ���� �� ����������� �����." << endl;
    else if (isPickProof( )) 
	buf << "��������� ���������� �� ������ ����� �� "
	    << getDescription( ).ruscase( '6' ) << "." << endl;
    else
	buf << "��������� ����� �� "
	    << getDescription( ).ruscase( '6' ) << "." << endl;
    
    if (key->value[0] == 0)
	buf << "�����������, ���� � �������." << endl;

    if (key->value[1] > 0)
	buf << "�����������, ���� �� �����." << endl;

    gsn_golden_eye->improve( ch, true );
    return true;
}

bool Keyhole::doExamine( )
{
    if (!isLockable( ))
	return false;

    if (number_percent( ) >= gsn_golden_eye->getEffective( ch ))
	return false;
	
    if (isPickProof( )) 
	act( "����� ������� �� ������.", ch, 0, 0, TO_CHAR );
    else {
	act( "����� �� ������ ����� ������� ����������.", ch, 0, 0, TO_CHAR );

	for (Object *o = ch->carrying; o; o = o->next_content) {
	    if (checkLockPick( o )) {
		ch->pecho( "%1$^O1 �������� �����%1$n��|��.", o );
		continue;
	    }

	    if (!ch->can_see( o ) && !ch->can_hear( o ))
		continue;

	    if (o->item_type == ITEM_KEYRING) 
		for (Object *l = o->contains; l; l = l->next_content)
		    if (checkLockPick( l )) 
			ch->pecho( "%1$^O1 �� %2$O6 �������� �����%1$n��|��.", o, l );
	}
    }
    
    gsn_golden_eye->improve( ch, true );
    return true;
}


/*------------------------------------------------------------------------
 * ItemKeyhole 
 *-----------------------------------------------------------------------*/
ItemKeyhole::ItemKeyhole( Character *ch, Object *obj )
{
    this->ch = ch;
    this->obj = obj;
}
ItemKeyhole::ItemKeyhole( Character *ch, Object *obj, Object *key )
{
    this->ch = ch;
    this->obj = obj;
    this->key = key;
}
int & ItemKeyhole::getLockFlags( )
{
    return obj->value[1];
}
bool ItemKeyhole::checkGuards( )
{
    return !obj->in_room || Keyhole::checkGuards( );
}
void ItemKeyhole::unlock( )
{
    Keyhole::unlock( );

    if (obj->in_room)
	save_items( obj->in_room );
}
void ItemKeyhole::msgTryPickSelf( )
{
    act( "�� ��������� ������������� $o4 � �������� �������� $O2.", ch, lockpick, obj, TO_CHAR );
}
void ItemKeyhole::msgTryPickOther( )
{
    act( "$c1 ���������� � ����� $O2.", ch, lockpick, obj, TO_ROOM );
}
DLString ItemKeyhole::getDescription( )
{
    DLString buf;

    buf << obj->getShortDescr( );
    if (obj->getCarrier( ) == 0)
	buf << " �� '" << obj->getRoom( )->name << "'";

    return buf;
}
/*------------------------------------------------------------------------
 * ContainerKeyhole 
 *-----------------------------------------------------------------------*/
ContainerKeyhole::ContainerKeyhole( Character *ch, Object *obj )
	  : ItemKeyhole( ch, obj )
{
}
ContainerKeyhole::ContainerKeyhole( Character *ch, Object *obj, Object *key )
	  : ItemKeyhole( ch, obj, key )
{
}
bitstring_t ContainerKeyhole::bitPickProof( )
{
    return CONT_PICKPROOF;
}
bitstring_t ContainerKeyhole::bitLocked( )
{
    return CONT_LOCKED;
}
bitstring_t ContainerKeyhole::bitCloseable( ) 
{
    return CONT_CLOSEABLE;
}
bitstring_t ContainerKeyhole::bitUnlockable( ) 
{
    return 0;
}
int ContainerKeyhole::getKey( )
{
    return obj->value[2];
}
/*------------------------------------------------------------------------
 * ExitKeyhole 
 *-----------------------------------------------------------------------*/
bitstring_t ExitKeyhole::bitPickProof( )
{
    return EX_PICKPROOF;
}
bitstring_t ExitKeyhole::bitLocked( )
{
    return EX_LOCKED;
}
bitstring_t ExitKeyhole::bitCloseable( ) 
{
    return EX_ISDOOR;
}
bitstring_t ExitKeyhole::bitUnlockable( ) 
{
    return EX_NOLOCK;
}
/*------------------------------------------------------------------------
 * PortalKeyhole 
 *-----------------------------------------------------------------------*/
PortalKeyhole::PortalKeyhole( Character *ch, Object *obj )
	  : ItemKeyhole( ch, obj )
{
}
PortalKeyhole::PortalKeyhole( Character *ch, Object *obj, Object *key )
	  : ItemKeyhole( ch, obj, key )
{
}
int PortalKeyhole::getKey( )
{
    return obj->value[4];
}
/*------------------------------------------------------------------------
 * DoorKeyhole 
 *-----------------------------------------------------------------------*/
DoorKeyhole::DoorKeyhole( Character *ch, Room *room, int door )
{
    this->ch = ch;
    this->room = room;
    this->door = door;
    pexit = room->exit[door];
    to_room = pexit->u1.to_room;
    pexit_rev = (to_room ? to_room->exit[dirs[door].rev] : 0);
}

DoorKeyhole::DoorKeyhole( Character *ch, Room *room, int door, Object *key )
{
    this->ch = ch;
    this->room = room;
    this->door = door;
    this->key = key;
    pexit = room->exit[door];
    to_room = pexit->u1.to_room;
    pexit_rev = (to_room ? to_room->exit[dirs[door].rev] : 0);
}

int & DoorKeyhole::getLockFlags( )
{
    return pexit->exit_info;
}
void DoorKeyhole::unlock( )
{
    ExitKeyhole::unlock( );
    
    if (pexit_rev && pexit_rev->u1.to_room == room) {
	REMOVE_BIT(pexit_rev->exit_info, bitLocked( ));
	to_room->echo( POS_RESTING, "������� ����� �������." );
    }
}
void DoorKeyhole::msgTryPickSelf( )
{
    act( "�� ��������� ������������� $o4 � �������� ��������.", ch, lockpick, 0, TO_CHAR );
}
void DoorKeyhole::msgTryPickOther( )
{
    act( "$c1 ���������� � ����� ����� $t ������.", ch, dirs[door].leave, 0, TO_ROOM );
}
DLString DoorKeyhole::getDescription( )
{
    DLString buf;
    
    buf << "����|�|�|�|�|��|� �� '" << room->name << "'";
    if (to_room)
	buf <<  " � '" << to_room->name << "'";

    return buf;
}
int DoorKeyhole::getKey( )
{
    return pexit->key;
}
/*------------------------------------------------------------------------
 * ExtraExitKeyhole 
 *-----------------------------------------------------------------------*/
ExtraExitKeyhole::ExtraExitKeyhole( Character *ch, Room *room, EXTRA_EXIT_DATA *peexit )
{
    this->ch = ch;
    this->room = room;
    this->peexit = peexit;
}

ExtraExitKeyhole::ExtraExitKeyhole( Character *ch, Room *room, EXTRA_EXIT_DATA *peexit, Object *key )
{
    this->ch = ch;
    this->room = room;
    this->peexit = peexit;
    this->key = key;
}

int & ExtraExitKeyhole::getLockFlags( )
{
    return peexit->exit_info;
}

void ExtraExitKeyhole::msgTryPickSelf( )
{
    act( "�� ��������� ������������� $o4 � �������� �������� $N2.", ch, lockpick, peexit->short_desc_from, TO_CHAR );
}
void ExtraExitKeyhole::msgTryPickOther( )
{
    act( "$c1 ���������� � ����� $N2.", ch, 0, peexit->short_desc_from, TO_ROOM );
}

DLString ExtraExitKeyhole::getDescription( )
{
    return peexit->short_desc_from;
}
int ExtraExitKeyhole::getKey( )
{
    return peexit->key;
}
