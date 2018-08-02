/* $Id$
 *
 * ruffina, 2009
 */
#include "khuzdul_effects.h"
#include "language.h"
#include "languagemanager.h"

#include "skillreference.h"

#include "pcharacter.h"
#include "object.h"
#include "affect.h"

#include "act.h"
#include "loadsave.h"
#include "wearloc_utils.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

GSN(ancient_rage);
GSN(enchant_weapon);
GSN(fireproof);

bool FireproofWE::run( PCharacter *ch, Character *victim ) const
{
    Object *obj;
    Affect af;

    af.where     = TO_OBJECT;
    af.type      = gsn_fireproof;
    af.level     = ch->getModifyLevel( );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_BURN_PROOF;

    for (obj = victim->carrying; obj; obj = obj->next_content) {
	if (obj->wear_loc == wear_none)
	    continue;
	    
	if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF))
	    continue;

	af.duration  = number_range( af.level, 200 );
	affect_to_obj( obj, &af);
    }

    act( "{C�������������� �� $c6 ���������� ������������� �������.{x", victim, 0, 0, TO_ROOM );
    act( "{C���� �������������� ���������� ������������� �������.{x", victim, 0, 0, TO_CHAR );
    return true;
}

bool EnchantWeaponWE::run( PCharacter *ch, Character *victim ) const
{
    Object *obj;
    Affect af;

    obj = get_eq_char( victim, wear_wield );

    if (!obj) {
	if (ch != victim) {
	    victim->println( "�� ���������� ������ ����������� ������." );
	    act( "����� �� �������� ���� - $C1 ����� �� ������$G���|��|���.", ch, 0, victim, TO_CHAR );
	}
	else {
	    act( "����� �� �������� ���� - �� ����� �� ������$g���|��|���.", ch, 0, 0, TO_CHAR );
	}

	return false;
    }
    
    affect_enchant( obj );

    af.where     = TO_OBJECT;
    af.bitvector = 0;
    af.type      = gsn_enchant_weapon;
    af.level     = ch->getModifyLevel( );
    af.duration  = number_range( af.level, 200 );
    af.modifier  = number_range( 1 + af.level / 10, 
                                 1 + af.level / 7 );

    af.location  = APPLY_DAMROLL;
    affect_enhance( obj, &af );

    af.location  = APPLY_HITROLL;
    affect_enhance( obj, &af );

    victim->hitroll += af.modifier;
    victim->damroll += af.modifier;

    act( "{C������� �������� ��������� ����������� $o4!{x", ch, obj, 0, TO_ALL );
    return true;
}

bool BerserkWE::run( PCharacter *ch, Character *victim ) const
{
    Affect af;
    
    af.where	 = TO_AFFECTS;
    af.type	 = gsn_ancient_rage;
    af.level	 = ch->getModifyLevel( );
    af.duration	 = number_fuzzy( af.level / 8 );
    af.location	 = (number_bits( 1 ) ? APPLY_HITROLL : APPLY_DAMROLL);

    if (victim->isAffected( gsn_ancient_rage )) {
	act( "{C����� ������� ������ ����������� � ���� � ����� �����!{x", victim, 0, 0, TO_CHAR );
	af.modifier = 0;
    }
    else {
	act( "{C����� ������� ������ ���������� � ����!{x", victim, 0, 0, TO_CHAR );
	af.modifier = max( 1, number_range( af.level / 6, af.level / 5 ) );
    }

    act( "{C����� ������� ������ ���������� � $c6!{x", victim, 0, 0, TO_ROOM );
    affect_join( victim, &af );
    return true;
}

bool MendingWE::run( PCharacter *ch, Character *victim ) const
{
    Object *obj;

    for (obj = victim->carrying; obj; obj = obj->next_content) {
	if (obj->wear_loc == wear_none)
	    continue;

	obj->condition += number_range( 30, 50 );
	obj->condition = min( 100, obj->condition );
    }

    act( "{C������� ������� �������� �������� ����� ������ ��������������.{x", victim, 0, 0, TO_CHAR );
    act( "{C������� ������� �������� �������� ����� �������������� $c2.{x", victim, 0, 0, TO_ROOM );
    return true;
}


