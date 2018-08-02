/* $Id$
 *
 * ruffina, 2004
 */
#include "eating.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "clanreference.h"
#include "skillreference.h"
#include "desire.h"
#include "affect.h"

#include "raceflags.h"
#include "handler.h"
#include "magic.h"
#include "damage_impl.h"
#include "fight.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

CLAN(battlerager);
GSN(manacles);
GSN(poison);
DESIRE(full);
DESIRE(hunger);
RACE(cat);
RACE(felar);
RACE(fish);
RACE(mouse);
RACE(rat);

static bool oprog_eat( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Eat", "C", ch );
    FENIA_NDX_CALL( obj, "Eat", "OC", obj, ch );
    return false;
}

COMMAND(CEat, "eat")
{
    Object *obj;
    DLString args = constArguments, arg;

    arg = args.getOneArgument( );

    if (arg.empty( ))
    {
	    ch->send_to("������ ���?\n\r");
	    return;
    }
    
    if ( ( obj = get_obj_carry( ch, arg ) ) == 0 )
    {
	Character *mob;

	if (( mob = get_char_room( ch, arg ) ) && mob->is_npc( )) {
	    eatCarnivoro( ch, mob->getNPC( ) );
	    return;
	}
	
	ch->send_to("� ���� ��� �����.\n\r");
	return;
    }

    if ( !ch->is_immortal() )
    {
	    if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
	    {
		    ch->send_to("��� ����������.\n\r");
		    return;
	    }

	    if ( ch->isAffected(gsn_manacles)
		    && obj->item_type == ITEM_PILL )
	    {
		    ch->send_to("�� �� ������ ��������� �������� � ��������.\n\r");
		    return;
	    }

	    if(!ch->is_npc( ) 
		&& ch->getClan( ) == clan_battlerager 
		&& !ch->is_immortal( )
		&& obj->item_type == ITEM_PILL)
	    {
		ch->send_to("�� �� BattleRager, � �� ���������� ���!\n\r");
		return;
	    }


	    if (!ch->is_npc( ))
		for (int i = 0; i < desireManager->size( ); i++)
		    if (!desireManager->find( i )->canEat( ch->getPC( ) ))
			return;
    }

    if (obj->level > ch->getModifyLevel() && !ch->is_immortal() )
    {
	    ch->send_to("���� ���� ��������, ����� ��������� ���.\n\r");
	    return;
    }

    act_p( "$c1 ��� $o4.",  ch, obj, 0, TO_ROOM,POS_RESTING);
    act_p( "�� ��� $o4.", ch, obj, 0, TO_CHAR,POS_RESTING);
    if ( ch->fighting != 0 )
	     ch->setWaitViolence( 3 );

    switch ( obj->item_type )
    {
    case ITEM_FOOD:
	    eatFood( ch, obj->value[0]*2, obj->value[1]*2, obj->value[3] );
	    break;

    case ITEM_PILL:
	    spell_by_item( ch, obj );
	    break;
    }

    if ( ch->is_adrenalined() || ch->fighting )
    {
	     ch->setWaitViolence( 2 );
    }

    if (oprog_eat( obj, ch ))
	return;

    extract_obj( obj );
}

void CEat::eatFood( Character *ch, int cFull, int cHunger, int cPoison )
{
    if ( !ch->is_npc() )
    {
	PCharacter *pch = ch->getPC( );
	
	desire_hunger->eat( pch, cHunger );
	desire_full->eat( pch, cFull );
    }

    if (cPoison != 0)
    {
	    /* The food was poisoned! */
	    Affect af;

	    act_p( "$c1 ��������� �� ����� � ����������.", ch, 0, 0, TO_ROOM,POS_RESTING);
	    ch->send_to("�� ���������� �� ����� � �����������.\n\r");

	    af.where	 = TO_AFFECTS;
	    af.type      = gsn_poison;
	    af.level 	 = number_fuzzy( cFull / 2 );
	    af.duration  = cFull;
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
    }
}

void CEat::eatCarnivoro( Character *ch, NPCharacter *mob )
{
    bool isFelar, isMouse, isFish;
    bool wasPoisoned;
    int diff, dam, gain;
    
    if (ch->fighting) {
	ch->send_to( "������ �� ���������� - ���� �� �� �����!\r\n" );
	return;
    }
    
    isFelar = (ch->getRace( ) == race_felar || ch->getRace( ) == race_cat);
    isMouse = (mob->getRace( ) == race_mouse || mob->getRace( ) == race_rat);
    isFish = (mob->getRace( ) == race_fish);
    
    if (!isFelar) {
	if (!isMouse && !isFish) {
	    ch->println("��� �������� �� ������� ���� ������ �������!");
	}
	else {
	    act("��������� ���� �����, $c1 �������� �������� � ������� $C4, �� ����� ���� �� �������.", ch, 0, mob, TO_ROOM);
	    act("�� ��� ��������� ���� �����, �� ��������� �������� � ������� $C4.. �� ����� ���� �� �������.", ch, 0, mob, TO_CHAR);
	}

	return;
    }
    else {
	if (!isMouse && !isFish) {
	    act("$c1, ������, �����$g��|�|�� $C4 �� ����������� �����.", ch, 0, mob, TO_ROOM);
	    act("��� �� �����! ���� � �� ����� �� $Y ��������.", ch, 0, mob, TO_CHAR);
	    return;
	}
    }
    

    if (mob->master) {
	act("$c1 � ��������� ������� ������ ��� ���� $C2.", ch, 0, mob, TO_ROOM);
	act("�� � ��������� �������� ������ ��� ���� $C2.", ch, 0, mob, TO_CHAR);
	
	if (mob->master == ch) {
	    act("$C1 � ������ ������� �� $c4.", ch, 0, mob, TO_ROOM);
	    act("$C1 � ������ ������� �� ����.", ch, 0, mob, TO_CHAR);
	}
	else if (mob->master->in_room == mob->in_room) {
	    act("$C1 ������ �������� �� ����� �������!", mob->master, 0, mob, TO_ROOM);
	    act("$C1 ������ �������� �� ���� �����!", mob->master, 0, mob, TO_CHAR);  
	}
	else
	    act("$C1 ��������� � ���, ������ ����� ������.", ch, 0, mob, TO_ALL);
	
	return;
    }
    
    if (!ch->is_npc( ))
 	for (int i = 0; i < desireManager->size( ); i++)
	    if (!desireManager->find( i )->canEat( ch->getPC( ) ))
 		return;
   
    act("$c1 � ������� ��������� ���������� ������ � ������� � $C4!", ch, 0, mob, TO_ROOM);
    act("�� � ������� ��������� ����������� ������ � ������� � $C4!", ch, 0, mob, TO_CHAR);

    diff = max( 1, ch->getRealLevel( ) - mob->getRealLevel( ) );
    dam = diff * 10;
    gain = mob->getRealLevel( ); 
    wasPoisoned = (IS_AFFECTED(mob, AFF_POISON)); 

    if (dam >= mob->hit) {
	Object *obj, *obj_next;
	
	death_cry( mob, 99 );
	act("�� ��� $C4.", ch, 0, mob, TO_CHAR);
	act("$c1 ��� $C4.", ch, 0, mob, TO_ROOM);

	for (obj = mob->carrying; obj; obj = obj_next) {
	    obj_next = obj->next_content;
	    obj_from_char( obj );
	    obj_to_room( obj, ch->in_room );
	}
	
	extract_char( mob );
	eatFood( ch, gain, gain, wasPoisoned );
    }
    else {
	RawDamage( ch, mob, DAM_OTHER, dam ).hit( true );

	if (mob->position >= POS_FIGHTING)
	    multi_hit( mob, ch );
    }
}

