/* $Id$
 *
 * ruffina, 2004
 */
#include "wearloc_utils.h"
#include "commandtemplate.h"

#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "loadsave.h"
#include "save.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

WEARLOC(hair);

/*
 * 'wear' command
 * 'wear <obj> [to <victim>]'
 * 'wear all'
 */
CMDRUNP( wear )
{
    Character *victim = ch;
    Object *obj;
    char cArg[MAX_INPUT_LENGTH];
    char argObj[MAX_INPUT_LENGTH], argTo[MAX_INPUT_LENGTH], argVict[MAX_INPUT_LENGTH];
    bool fHair = false;
    
    strcpy( cArg, argument );
    argument = one_argument( argument, argObj );
    argument = one_argument( argument, argTo );
    argument = one_argument( argument, argVict );

    if (!argObj[0]) {
	ch->println("������, ����������� ��� ����� ��� � ����?");
	return;
    }
    
    if (arg_is_to( argTo ) || arg_is_in( argTo )) {
        if (arg_oneof( argVict, "������", "hair" )) {
            fHair = true;
        }
        else if (( victim = get_char_room( ch, argVict  ) ) == 0) {
	    ch->println("�� ���� �� ������ ��� ������?");
	    return;
	} else if (victim != ch && !victim->is_npc( )) {
	    act("$C1 � ��������� ������� ��$G��|�|��!", ch, 0, victim, TO_CHAR);
	    return;
	}
    }
    else 
	one_argument( cArg, argObj );
    
    if (arg_is_all( argObj )) {
	Object *obj_next;
	
	if (victim != ch) {
	    ch->println("�� �� ������ ������� �����.");
	    return;
	}
	
	for (obj = ch->carrying; obj != 0; obj = obj_next) {
	    obj_next = obj->next_content;
	    
	    if (obj->wear_loc == wear_none && ch->can_see( obj ))
		wear_obj( ch, obj, F_WEAR_VERBOSE );
	}

	return;
    }
    
    if (( obj = get_obj_carry( ch, argObj ) ) == 0) {
	ch->println("� ���� ��� �����.");
	return;
    }

    if (ch == victim && fHair) {
        if (obj->getWeight( ) / 10 > 3) {
            ch->pecho( "%1$^O1 ������� �����%1$G��|��|��|��, ����� ���������� � ����� �������.", obj );
            return;
        }

        wear_hair->wear( obj, F_WEAR_VERBOSE );
        return;
    }
	
    if (ch == victim) {
	if (wear_obj( ch, obj, F_WEAR_VERBOSE | F_WEAR_REPLACE) == RC_WEAR_NOMATCH)
	    ch->println("�� �� ������ ������, ����������� ��� ������� ��� � �����.");
	return;
    }
    
    if (!obj->behavior || !obj->behavior->canDress( ch, victim )) {
	act("�� �� ������� ������ $o4 �� $C4.", ch, obj, victim, TO_CHAR);
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    
    if (wear_obj( victim, obj, 0 ) != RC_WEAR_OK) {
	if (obj->carried_by == victim) {
	    obj_from_char( obj );
	    obj_to_char( obj, ch );
	}
	act("�� ��������� ������ $o4 �� $C4, �� ����������.", ch, obj, victim, TO_CHAR);
	act("$c1 �������� ������ �� ���� $o4, �� �� �����.", ch, obj, victim, TO_VICT);
	act("$c1 �������� ������ �� $C4 $o4, �� �� �����.", ch, obj, victim, TO_NOTVICT);
	return;
    }

    act("�� ��������� $o4 �� $C4.", ch, obj, victim, TO_CHAR);
    act("$c1 �������� �� ���� $o4.", ch, obj, victim, TO_VICT);
    act("$c1 �������� �� $C4 $o4.", ch, obj, victim, TO_NOTVICT);
}



/*
 * 'remove' command
 * 'remove <obj> [from <victim>]'
 * 'remove all'
 */
CMDRUNP( remove )
{
    Character *victim = ch;
    Object *obj;
    char cArg[MAX_INPUT_LENGTH];
    char argObj[MAX_INPUT_LENGTH], argFrom[MAX_INPUT_LENGTH], argVict[MAX_INPUT_LENGTH];
    
    strcpy( cArg, argument );
    argument = one_argument( argument, argObj );
    argument = one_argument( argument, argFrom );
    argument = one_argument( argument, argVict );

    if (!argObj[0]) {
	ch->println("����� ���?");
	return;
    }

    if (arg_is_from( argFrom )) {
	if (( victim = get_char_room( ch, argVict ) ) == 0) {
	    ch->println("� ���� �� ������ ��� �����?");
	    return;
	}
	
	if (victim != ch && !victim->is_npc( )) {
	    act("$C1 � ��������� ��������� ��$G��|�|��!", ch, 0, victim, TO_CHAR);
	    return;
	}
    }
    else
	one_argument( cArg, argObj );
    
    if (arg_is_all( argObj )) {
        Object *obj_next;

	if (victim != ch) {
	    ch->println("�� �� ������ ������� �����.");
	    return;
	}

        for (obj = ch->carrying; obj != 0; obj = obj_next) {
            obj_next = obj->next_content;

            if (ch->can_see( obj ))
		obj->wear_loc->remove( obj, F_WEAR_VERBOSE );
        }

        return;
    }
    
    if (ch == victim) {
	if (( obj = get_obj_wear( ch, argObj ) ) == 0) {
	    ch->println("� ���� ��� �����.");
	    return;
	}

	obj->wear_loc->remove( obj, F_WEAR_VERBOSE );
	return;
    }
    
    if (( obj = get_obj_wear_victim( victim, argObj, ch ) ) == 0) {
	act("� $C2 ��� �����.", ch, 0, victim, TO_CHAR);
	return;
    }

    if (!obj->behavior || !obj->behavior->canDress( ch, victim )) {
	act("�� �� ������� ����� $o4 � $C2.", ch, obj, victim, TO_CHAR);
	return;
    }
    
    if (!obj->wear_loc->remove( obj, 0 )) {
	act("�� ��������� ����� $o4 � $C2, �� ����������.", ch, obj, victim, TO_CHAR);
	act("$c1 �������� ����� � ���� $o4, �� �� �����.", ch, obj, victim, TO_VICT);
	act("$c1 �������� ����� � $C2 $o4, �� �� �����.", ch, obj, victim, TO_NOTVICT);
	return;
    }
    
    act("�� �������� $o4 � $C2.", ch, obj, victim, TO_CHAR);
    act("$c1 ������� � ���� $o4.", ch, obj, victim, TO_VICT);
    act("$c1 ������� � $C2 $o4.", ch, obj, victim, TO_NOTVICT);
    
    if (obj->carried_by == victim) {
	obj_from_char( obj );
	obj_to_char( obj, ch );
    }
}


