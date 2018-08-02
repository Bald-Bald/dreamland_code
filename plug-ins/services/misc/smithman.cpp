/* $Id$
 *
 * ruffina, 2004
 */
#include <iomanip>

#include "smithman.h"
#include "behavior_utils.h"
#include "attract.h"
#include "occupations.h"
#include "commandtemplate.h"

#include "affect.h"
#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"

#include "merc.h"
#include "mercdb.h"
#include "interp.h"
#include "handler.h"
#include "act.h"
#include "def.h"

#define OBJ_VNUM_HORSESHOE 107

WEARLOC(hooves);

/*-------------------------------------------------------------------------
 *  Smithman
 *------------------------------------------------------------------------*/
int Smithman::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) | (1 << OCC_SMITHMAN);
}

bool Smithman::specIdle( )
{
    if (chance(90))
	return false;

    interpret_raw(ch, "say", "��, ������ ���!!!");
    return true;
}

bool Smithman::canServeClient( Character *client )
{
    if (client->is_npc( ))
	return false;

    if (getKeeper( )->fighting) {
	say_act( client, getKeeper( ), "������, $c1, ��� ������ �� �� ����." );
	return false;
    }
    
    if (IS_GHOST( client )) {
	say_act( client, getKeeper( ), "�, $c1, ��� �� ������.. ��� ���� - ��� �������." );
	return false;
    }

    if (IS_AFFECTED( client, AFF_CHARM )) {
	say_act( client, getKeeper( ), "�� ���� � ���� �����������, $c1, ������� �� ������� �� ���� ������." );
	return false;
    }

    return true;
}

void Smithman::msgListRequest( Character *client ) 
{
    act( "�� ������� � $C4 ������ �����.", client, 0, getKeeper( ), TO_CHAR );
    act( "$c1 ������ $C4 ����������, ��� $E ����� ������.", client, 0, getKeeper( ), TO_ROOM );
}

void Smithman::msgListBefore( Character *client ) 
{
    tell_dim( client, getKeeper( ), "��� ������ ����, ��� � ����: " );
}

void Smithman::msgListAfter( Character *client )
{
    tell_dim( client, getKeeper( ), "����, �������, ���." );
}

void Smithman::msgListEmpty( Character *client )
{
    say_act( client, getKeeper( ), "������, $c1, �� ��� ���� � ���� ������� ��������." );
}

void Smithman::msgArticleNotFound( Character *client ) 
{
    interpret_raw( getKeeper( ), "eyebrow", client->getNameP( ) );
}

void Smithman::msgArticleTooFew( Character *client, Article::Pointer )
{
    say_act( client, getKeeper( ), "�� ��������." );
}

void Smithman::msgBuyRequest( Character *client ) 
{
    act( "�� ������� $C4 ��������� ����.", client, 0, getKeeper( ), TO_CHAR );
    act( "$c1 ������ $C4 ��������� $s.", client, 0, getKeeper( ), TO_ROOM );
}


/*-------------------------------------------------------------------------
 * SmithService 
 *------------------------------------------------------------------------*/
void SmithService::printLine( Character *client, 
                              Price::Pointer price,
			      const DLString &name,
			      const DLString &descr,
			      ostringstream &buf )
{
    ostringstream mbuf;
    
    price->toStream( client, mbuf );

    buf << dlprintf( "%9s.....{c%-9s{x - %s\r\n",
                     mbuf.str( ).c_str( ),
		     name.c_str( ),
		     descr.c_str( ) );
}

void SmithService::toStream( Character *client, ostringstream &buf ) const
{
    printLine( client, price, name.getValue( ), descr.getValue( ), buf );
}

bool SmithService::matches( const DLString &argument ) const
{
    return !argument.empty( ) && argument.strPrefix( name.getValue( ) );
}

int SmithService::getQuantity( ) const
{
    return 1;
}

/*-------------------------------------------------------------------------
 * HorseshoeSmithService 
 *------------------------------------------------------------------------*/
bool HorseshoeSmithService::visible( Character *client ) const
{
    return client->getWearloc( ).isSet( wear_hooves );
}

bool HorseshoeSmithService::available( Character *client, NPCharacter *smithman ) const
{
    if (visible( client )) 
	return true;

    say_act( client, smithman, "��, $c1, � ���� �� ���� �������� ���������?" );
    return false;
}

void HorseshoeSmithService::purchase( Character *client, NPCharacter *smithman, const DLString &, int )
{
    Affect af;
    Object *old_shoe, *shoe;
    int level, hr, dr;

    if (!price->canAfford( client )) {
	say_act( client, smithman, "� ���� �� ������� $n2, ����� �������� ��� ������.", price->toCurrency( ).c_str( ) );
	return;
    }

    level = client->getModifyLevel( );
    shoe = create_object( get_obj_index( OBJ_VNUM_HORSESHOE ), level );
    shoe->level = level;

    for (int j = 0; j < 4; j++)
	if (level < 25)		shoe->value[j] = min( level , 15 );
	else if (level < 60)	shoe->value[j] = max( 20, number_fuzzy( 20 ) );
	else if (level < 80)	shoe->value[j] = max( 23, number_fuzzy( 23 ) );
	else if (level < 90)	shoe->value[j] = max( 26, number_fuzzy( 26 ) );
	else			shoe->value[j] = max( 30, number_fuzzy( 30 ) );

    
    if (level < 10)		{ hr = 1; dr = 1; }
    else if (level < 20)	{ hr = 3; dr = 1; }
    else if (level < 30)	{ hr = 3; dr = 2; }
    else if (level < 50)	{ hr = 4; dr = 4; }
    else if (level < 75)	{ hr = 5; dr = 6; }
    else if (level < 90)	{ hr = 6; dr = 6; }
    else			{ hr = 10; dr = 10; }
    
    af.where = TO_OBJECT;
    af.bitvector = 0;
    af.duration = -1;
    af.type = 0;
    af.level = level;
    
    af.location = APPLY_HITROLL;
    af.modifier = hr;
    affect_to_obj( shoe, &af );

    af.location = APPLY_DAMROLL;
    af.modifier = dr;
    affect_to_obj( shoe, &af );
    
    price->deduct( client );
    act( "$C1 �������� � ���� $n4.", client, price->toString( client ).c_str( ), smithman, TO_CHAR );
    
//    act( "$c1 ������������� ��� ���� ������.", smithman, 0, 0, TO_ROOM );

    if ((old_shoe = get_eq_char( client, wear_hooves ))) {
	unequip_char( client, old_shoe );
	act( "$c1 ������� � ���� ������ �������.", smithman, 0, client, TO_VICT );
	act( "$c1 ������� � $C2 ������ �������.", smithman, 0, client, TO_NOTVICT );
    }

    obj_to_char( shoe, client );
    equip_char( client, shoe, wear_hooves);
    act( "$c1 ����������� ����� ������� �� ���� ������.", smithman, 0, client, TO_VICT );
    act( "$c1 ����������� ����� ������� �� ������ $C2.", smithman, 0, client, TO_NOTVICT );

    if (client->getSex( ) == SEX_FEMALE && chance( 50 )) {
	act( "$c1 ������� ���� �� �����, ������������ '{g������, ���������!{x'", smithman, 0, client, TO_VICT );
	act( "$c1 ������� $C4 �� �����, ������������ '{g������, ���������!{x'", smithman, 0, client, TO_NOTVICT );
    }
}


/*-------------------------------------------------------------------------
 * ItemSmithService 
 *------------------------------------------------------------------------*/
bool ItemSmithService::visible( Character * ) const
{
    return true;
}

bool ItemSmithService::available( Character *, NPCharacter * ) const
{
    return true;
}

void ItemSmithService::purchase( Character *client, NPCharacter *smithman, const DLString &constArguments, int )
{
    Object *obj;
    DLString argument( constArguments );
    DLString arg;
    
    arg = argument.getOneArgument( );

    if (arg.empty( )) {
	say_act( client, smithman, "�� ����, ����� ��� $t �����?", verb.getValue( ).c_str( ) );
	return;
    }

    obj = get_obj_carry( client, arg.c_str( ) );

    if (!obj) {
	say_act( client, smithman, "�����������? � ���� ��� �����." );
	return;
    }

    act( "�� ������������ $C3 $o4.", client, obj, smithman, TO_CHAR );
    act( "$c1 ����������� $C3 $o4.", client, obj, smithman, TO_NOTVICT );

    if (obj->pIndexData->limit != -1) {
	say_act( client, smithman, "��� ���� - ���������. � ������ �� ���� ��� ��������." );
	return;
    }
    
    if (!checkPrice( client, smithman, price ))
	return;

    smith( client, smithman, obj );
}   

bool ItemSmithService::checkPrice( Character *client, NPCharacter *smithman, Price::Pointer price ) const
{
    if (!price->canAfford( client )) {
	say_act( client, smithman, noMoney.getValue( ).c_str( ), price->toCurrency( ).c_str( ) );
	return false;
    }

    return true;
}

/*-------------------------------------------------------------------------
 * BurnproofSmithService 
 *------------------------------------------------------------------------*/
void BurnproofSmithService::smith( Character *client, NPCharacter *smithman, Object *obj )
{
    if (obj->item_type != ITEM_CONTAINER && obj->item_type != ITEM_DRINK_CON) {
	say_act( client, smithman, "��� �� ���������, � ���� � �� ����." );
	return;
    }
       
    if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
	say_act( client, smithman, "����. �� ��� ������ ������ ���-�� ������." );
	return;
    }
    
    price->deduct( client );
    obj->level += 1;

    SET_BIT(obj->extra_flags, ITEM_BURN_PROOF);

    act( "$c1 ������������ ���-�� $o4 � ���������� ����.", smithman, obj, client, TO_VICT );
    act( "$c1 ������������ ���-�� $o4 � ���������� $C3.", smithman, obj, client, TO_NOTVICT ); 
}

/*-------------------------------------------------------------------------
 * AlignSmithService 
 *------------------------------------------------------------------------*/
void AlignSmithService::smith( Character *client, NPCharacter *smithman, Object *obj )
{
    int boff, bon;

    if (!obj->isAntiAligned( client )) {
	say_act( client, smithman, "��� � ��� ��� ���� ��������, $c1." );
	return;
    }
    
    if (!obj->getRealShortDescr( )) {
	say_act( client, smithman, "$c1, ���� �������� ������ ��������� �������� ����� � �������� ������� ��� ����." );
	return;
    }
    
    price->deduct( client );
    obj->level += 1;
    
    if (IS_GOOD( client )) {
	boff = ITEM_ANTI_GOOD    | ITEM_EVIL;
	bon  = ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL;
    }
    else if (IS_NEUTRAL( client )) {
	boff = ITEM_ANTI_NEUTRAL | ITEM_EVIL;
	bon  = ITEM_ANTI_GOOD    | ITEM_ANTI_EVIL;
    } 
    else {
	boff = ITEM_ANTI_EVIL;
	bon  = ITEM_ANTI_GOOD    | ITEM_ANTI_NEUTRAL;
    }
    
    REMOVE_BIT( obj->extra_flags, boff );
    SET_BIT( obj->extra_flags, bon );

    act( "$C1 ���-�� ������ � $o5 � ���������� ����.", client, obj, smithman, TO_CHAR );
    act( "$C1 ���-�� ������ � $o5 � ���������� $c3.", client, obj, smithman, TO_ROOM );
}

/*-------------------------------------------------------------------------
 * SharpSmithService 
 *------------------------------------------------------------------------*/
void SharpSmithService::toStream( Character *client, ostringstream &buf ) const
{
    printLine( client, price, name.getValue( ), descr.getValue( ), buf );
    printLine( client, extraPrice, name.getValue( ), extraDescr.getValue( ), buf );
}

void SharpSmithService::smith( Character *client, NPCharacter *smithman, Object *obj )
{
    Price::Pointer myprice;

    if (obj->item_type != ITEM_WEAPON) {
	say_act( client, smithman, "� � ����� �������? $o1 �� ������.", obj );
	return;
    }

    if (IS_WEAPON_STAT(obj, WEAPON_SHARP|WEAPON_VORPAL)) {
	say_act( client, smithman, "$o1 � ��� ���� ������ ������.", obj );
	return;
    }

    if (IS_WEAPON_STAT( obj, WEAPON_HOLY )) {
	say_act( client, smithman, "$o1 ��������� ��������� �����, ������� ��� �� � ����.", obj );
	return;
    }

    if (IS_WEAPON_STAT( obj, WEAPON_FLAMING | WEAPON_FROST |
   			     WEAPON_VAMPIRIC | WEAPON_SHOCKING | WEAPON_POISON ))
    {
	myprice = extraPrice;
	say_act( client, smithman, "� $o2 � ��� ���� ����� ������, ������ �����, ������ � ������.", obj );
    }
    else
	myprice = price;

    if (!checkPrice( client, smithman, myprice ))
	return;

    myprice->deduct( client );
    obj->level += 1;

    SET_BIT( obj->value[4], WEAPON_SHARP );

    act( "$C1 ����� $o4 � ���������� ����.", client, obj, smithman, TO_CHAR );
    act( "$C1 ����� $o4 � ���������� $c3.", client, obj, smithman, TO_ROOM );
}


/*-------------------------------------------------------------------------
 * 'smith' command
 *------------------------------------------------------------------------*/
CMDRUN( smith )
{
    DLString arguments = constArguments;
    Smithman::Pointer smithman;
    
    smithman = find_attracted_mob_behavior<Smithman>( ch, OCC_SMITHMAN );

    if (!smithman) {
	ch->send_to( "����� ��� �������.\r\n" );
	return;
    }

    if (ch->is_npc( )) {
	ch->send_to( "���� ����������� �� �����, ������.\r\n" );
	return;
    }
    
    if (arguments.empty( ) || arguments == "list")
	smithman->doList( ch->getPC( ) );
    else
	smithman->doBuy( ch->getPC( ), arguments );
}

