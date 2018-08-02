/* $Id$
 *
 * ruffina, 2004
 */
#include "misc_wearlocs.h"
#include "wearloc_utils.h"

#include "skillreference.h"
#include "room.h"
#include "affect.h"
#include "character.h"
#include "object.h"

#include "stats_apply.h"
#include "weapons.h"
#include "save.h"
#include "loadsave.h"
#include "act.h"
#include "merc.h"
#include "def.h"

GSN(second_weapon);
GSN(hand_to_hand);

/*
 * stuck-in
 */
bool StuckInWearloc::equip( Character *ch, Object *obj )
{
    obj->wear_loc.assign( this );
    saveDrops( ch );
    return true;
}

void StuckInWearloc::unequip( Character *ch, Object *obj )
{
    obj->wear_loc.assign( wear_none );
    saveDrops( ch );
}

/*
 * hair 
 */

bool HairWearloc::matches( Character *ch )
{
    return true;
}

bool HairWearloc::equip( Character *ch, Object *obj )
{
    obj->wear_loc.assign( this );
    saveDrops( ch );
    return true;
}

void HairWearloc::unequip( Character *ch, Object *obj )
{
    obj->wear_loc.assign( wear_none );
    saveDrops( ch );
}
void HairWearloc::affectsOnEquip( Character *ch, Object *obj ) { 
}

void HairWearloc::affectsOnUnequip( Character *ch, Object *obj ) { }
int HairWearloc::canWear( Character *ch, Object *obj, int flags ) {
    if (find( ch ) != NULL) {
	if (IS_SET(flags, F_WEAR_VERBOSE))
            ch->println("� ����� ������� ��� ���������� ���-�� ������.");
        return RC_WEAR_CONFLICT;
    }
    return DefaultWearlocation::canWear( ch, obj, flags );
}

/*
 * shield 
 */
int ShieldWearloc::canWear( Character *ch, Object *obj, int flags )
{
    Object *weapon;
    int rc;
    
    if (( rc = DefaultWearlocation::canWear( ch, obj, flags ) ) != RC_WEAR_OK)
	return rc;

    weapon = wear_wield->find( ch );
    
    if (weapon 
	&& ch->size < SIZE_LARGE 
	&& IS_WEAPON_STAT(weapon, WEAPON_TWO_HANDS))
    {
	if (IS_SET(flags, F_WEAR_VERBOSE))
	    ch->println("���� ���� ������ �������!");
	return RC_WEAR_CONFLICT;
    }

    return RC_WEAR_OK;
}


/*
 * wield 
 */
bool WieldWearloc::remove( Object *obj, int flags )
{
    Object *dual;

    if (!DefaultWearlocation::remove( obj, flags ))
	return false;

    if (IS_SET(flags, F_WEAR_REPLACE))
	return true;

    if (( dual = wear_second_wield->find( obj->carried_by ) )) 
	dual->wear_loc.assign( this );
    
    return true;
}

int WieldWearloc::canWear( Character *ch, Object *obj, int flags )
{
    int rc;
    
    if (( rc = DefaultWearlocation::canWear( ch, obj, flags ) ) != RC_WEAR_OK)
	return rc;
	
    if (!ch->is_npc( ) && obj->getWeight( ) > (get_str_app(ch).wield * 10)) {
	if (IS_SET(flags, F_WEAR_VERBOSE))
	    ch->println("�� �� ������ ���� �����������. ��� ������� ������ ��� ����.");
	return RC_WEAR_HEAVY;
    }

    if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS)
	&& !ch->is_npc( )
	&& ((ch->size < SIZE_LARGE && wear_shield->find( ch ))
	    || (ch->getRace( )->getSize( ) < SIZE_HUGE && wear_second_wield->find( ch ))))
    {
	if (IS_SET(flags, F_WEAR_VERBOSE))
	    ch->println("����� ����������� ����, � ���� ������ ���� ��� ��������� ����.");
	return RC_WEAR_LARGE;
    }

    return RC_WEAR_OK;
}

int WieldWearloc::wear(  Object *obj, int flags )
{
    int rc;
    
    if (( rc = DefaultWearlocation::wear( obj, flags ) ) != RC_WEAR_OK)
	return rc;

    if (obj->wear_loc != this)
	return rc;
    
    if (IS_SET(flags, F_WEAR_VERBOSE))
	reportWeaponSkill( obj->carried_by, obj );

    return RC_WEAR_OK;
}

void WieldWearloc::reportWeaponSkill( Character *ch, Object *obj )
{
    int skill, sn;
    const char *msg;
    
    sn = get_weapon_sn( obj );

    if (sn == gsn_hand_to_hand)
	return;

    skill = ch->getSkill( sn );

    if (skill >= 100)
	msg = "$o1 ���������� ������ ����!";
    else if (skill > 85)
	msg = "�� ���������� ���� � $o5 ��������� ��������.";
    else if (skill > 70)
	msg = "�� ������ �������� $o5.";
    else if (skill > 50)
	msg = "�� �������� ������������� �������� $o5.";
    else if (skill > 25)
	msg = "�� ���������� ���� ��������, ������������ $o5.";
    else if (skill > 1)
	msg = "�� ������� ������� � ����� $o4, ����� �������.";
    else
	msg = "�� �� ������, ��� ����������� $o5.";
    
    act( msg, ch, obj, 0, TO_CHAR );
}

/*
 * second wield
 */
bool SecondWieldWearloc::remove( Object *obj, int flags )
{
    return DefaultWearlocation::remove( obj, flags );
}

bool SecondWieldWearloc::matches( Character *ch )
{
    if (!DefaultWearlocation::matches( ch ))
	return false;

    if (!gsn_second_weapon->usable( ch )) 
	return false;

    return true;
}

int SecondWieldWearloc::canWear( Character *ch, Object *obj, int flags )
{
    int rc;
    Object *wield;
    
    if (( rc = DefaultWearlocation::canWear( ch, obj, flags ) ) != RC_WEAR_OK)
	return rc;
    
    if (wear_shield->find( ch ) || wear_hold->find( ch )) {
	if (IS_SET(flags, F_WEAR_VERBOSE))
	    ch->println( "���� ���� ��� ������ ����� ��� ���-���� ������." );
	return RC_WEAR_CONFLICT;
    }

    if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS) && ch->getRace( )->getSize( ) < SIZE_HUGE) {
	if (IS_SET(flags, F_WEAR_VERBOSE))
	    ch->println( "������ �� ��������� �������!" );
	return RC_WEAR_LARGE;
    }

    if (!( wield = wear_wield->find( ch ) )) {
	if (IS_SET(flags, F_WEAR_VERBOSE))
	    ch->println( "� ���� ��� ���� ���������� ������!" );
	return RC_WEAR_CONFLICT;
    }

    if (IS_WEAPON_STAT(wield, WEAPON_TWO_HANDS) && ch->getRace( )->getSize( ) < SIZE_HUGE) {
	if (IS_SET(flags, F_WEAR_VERBOSE))
	    ch->println( "��� ���� ������ ��������� �������!" );
	return RC_WEAR_LARGE;
    }

    if (obj->getWeight( ) > (get_str_app(ch).wield * 5)) {
	if (IS_SET(flags, F_WEAR_VERBOSE))
	    ch->println( "��� ������ ������� ������ ��� ����, ���� ������������ ��� ��� ���������." );
	return RC_WEAR_HEAVY;
    }

    return RC_WEAR_OK;
}

/*
 * tattoo
 */
int TattooWearloc::canWear( Character *ch, Object *obj, int flags )
{
    if (!ch->is_immortal( )) 
	return RC_WEAR_NOMATCH;
    else
	return DefaultWearlocation::canWear( ch, obj, flags );
}

bool TattooWearloc::canRemove( Character *ch, Object *obj, int flags )
{
    if (IS_SET(flags, F_WEAR_VERBOSE))
	act("���� ������������ ���� ����� �������� ���� �� $o2.", ch, obj, 0, TO_CHAR);

    return false;
}

