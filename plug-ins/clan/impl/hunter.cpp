/* $Id: hunter.cpp,v 1.1.6.11.6.22 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2005
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

#include "hunter.h"
#include "cclantalk.h"

#include "commonattributes.h"
#include "char.h"

#include "commandtemplate.h"
#include "summoncreaturespell.h"
#include "spelltemplate.h"                                                 
#include "skillcommandtemplate.h"
#include "skill.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "fight.h"
#include "magic.h"
#include "save.h"
#include "material.h"
#include "damage.h"

#include "merc.h"
#include "gsn_plugin.h"
#include "comm.h"
#include "mercdb.h"
#include "handler.h"
#include "interp.h"
#include "vnum.h"
#include "wiznet.h"
#include "act.h"
#include "act_move.h"
#include "roomtraverse.h"
#include "def.h"

#define OBJ_VNUM_HUNTER_PIT           110

GSN(hunter_pit);
GSN(hunter_beacon);
GSN(hunter_snare);
GSN(prevent);
GSN(detect_trap);
GSN(dispel_affects);

CLAN(none);
CLAN(hunter);
CLAN(lion);
CLAN(flowers);

/*--------------------------------------------------------------------------
 * Hunter's Cleric 
 *-------------------------------------------------------------------------*/
void ClanHealerHunter::speech( Character *ach, const char *speech )
{
    PCharacter *wch;
    Character *carrier;
    Object *obj;
    int vnum = 0;
    ClanAreaHunter::Weapons::iterator i;
    ClanAreaHunter::Pointer clanArea;
    DLString msg( speech ), trouble( "trouble" );
    
    if (!( wch = ach->getPC( ) ))
	return;

    if (!trouble.strPrefix( msg ))
	return;
    
    msg.erase( 0, trouble.length( ) );

    if (!getClanArea( ))
	return;
    if (!( clanArea = getClanArea( ).getDynamicPointer<ClanAreaHunter>( ) ))
	return;
    if (!( vnum = clanArea->vnumByString( msg ) ))
	return;
   
    if (wch->getClan( ) != clanArea->getClan( )) {
	do_say(ch, "���� �������� ������ �����������!");
	return;
    }

    if (!wch->getAttributes( ).isAvailable( "hunterarmor" )) {
	do_say(ch, "��� �� ������ � ����?");
	return;
    }
    
    obj = get_obj_world_unique( vnum, wch );
    
    if (!obj) {
	do_say( ch, "�� ��� �� ������� ����� ���� ������!" );
    
	if (vnum == clanArea->armorVnum) 
	    obj = clanArea->createArmor( wch );
	else
	    obj = clanArea->createWeapon( wch, vnum );
	
	interpret_fmt( ch, "emote ������� %s.", obj->getShortDescr( '4' ).c_str( ) );
	interpret_fmt( ch, "say � ��� ���� ������ %s.", obj->getShortDescr( '4' ).c_str( ) );
	act( "$C1 ���� $o4 $c3.", wch, obj, ch, TO_ROOM );
	act( "$C1 ���� ���� $o4.", wch, obj, ch, TO_CHAR );
	obj_to_char( obj, wch );
	do_say( ch, "���� ������������! �� ������� �����!" );
	return;
    }

    if (( carrier = obj->getCarrier( ) )) {
	if (carrier == wch) {
	    do_say( ch, "��� ����� �����? ����� ��������� ������!" );
	    interpret_raw( ch, "smite", wch->getNameP( ));
	}
	else {
	    interpret_raw( ch, "say", "%s ��������� � %s!",
			    obj->getShortDescr( '1' ).c_str( ),
			    wch->sees( carrier, '4' ).c_str( ) );
	    interpret_raw( ch, "say", "%s ��������� � ���� %s ����� %s!",
			    wch->sees( carrier, '1' ).c_str( ),
			    carrier->in_room->area->name,
			    carrier->in_room->name );
	}
    }
    else {
	interpret_raw( ch, "say", "%s ��������� � ���� %s ����� %s!",
			obj->getShortDescr( '1' ).c_str( ),
			obj->getRoom( )->area->name, 
			obj->getRoom( )->name );
    }
}

/*--------------------------------------------------------------------------
 * Hunter's Clan Guard 
 *-------------------------------------------------------------------------*/
void ClanGuardHunter::actPush( PCharacter *wch )
{
    act( "$C1 ���������� ����� ������������ ������� � ������ ������� ����.\n\r...�� � ����� ������ ������������� � ��������� �� ���� ������ ����� �����.", wch, 0, ch, TO_CHAR );
    act( "$C1 ���������� ����� ������������ ������� � ������ ������� $c4\n\r... $c1 � ����� ������ �������� �� ���� ������ ����� �����.", wch, 0, ch, TO_ROOM );
}

void ClanGuardHunter::actGreet( PCharacter *wch )
{
    do_say( ch, "����� ����������, ���������� �������." );
    createEquipment( wch );
}

int ClanGuardHunter::getCast( Character *victim )
{
    int sn = -1;

    switch ( dice(1,16) )
    {
    case  0: 
    case  1:
	    if (!victim->isAffected( gsn_spellbane ))
		sn = gsn_dispel_affects;
	    break;
    case  2:
    case  3:
	    sn = gsn_acid_arrow;
	    break;
    case  4: 
    case  5:
	    sn = gsn_caustic_font;
	    break; 
    case  6:
    case  7:
    case  8:
    case  9:
	    sn = gsn_acid_blast;
	    break;
    default:
	    sn = -1;
	    break;
    }

    return sn;
}

void ClanGuardHunter::createEquipment( PCharacter *wch )
{
    Object *armor;
    ClanAreaHunter::Pointer clanArea;

    if (wch->getAttributes( ).isAvailable( "hunterarmor" ))
	return;
    if (!getClanArea( ))
	return;
    if (!( clanArea = getClanArea( ).getDynamicPointer<ClanAreaHunter>( ) ))
	return;
    
    wch->getAttributes( ).getAttr<XMLEmptyAttribute>( "hunterarmor" );
    armor = clanArea->createEquipment( wch ); 
    
    do_say( ch, "� ���� ���� ������� ������ ��������." );
    interpret( ch, "emote ������� �������� ������ ���������." );

    act("�� ��������� $o4 $C3.", ch, armor, wch, TO_CHAR);
    act("$c1 �������� ���� $o4.", ch, armor, wch, TO_VICT);
    act("$c1 �������� $o4 $C3.", ch, armor, wch, TO_NOTVICT);
    obj_to_char( armor, wch );

    do_say( ch, "�����! ���� ������ ����� �������, �� ����� ��� ������� �������� ������!" );
    do_say( ch, "������ ����� ��� 'trouble' � ��� ����... ��������, 'troublearmor'." );
}

/*--------------------------------------------------------------------------
 * Hunter's Equipment (base) 
 *-------------------------------------------------------------------------*/
HunterEquip::HunterEquip( )
{
    clan.assign( clan_hunter );
}

void HunterEquip::config( PCharacter *wch )
{
    obj->fmtShortDescr( obj->getShortDescr( ), wch->getNameP( ) );
    obj->setOwner( wch->getNameP( ) );
    obj->from = str_dup( wch->getNameP( ) );
    obj->level = wch->getRealLevel( );
    obj->cost = 0;
    
    if (obj->pIndexData->extra_descr) {
	char buf[MAX_STRING_LENGTH];

	sprintf( buf, obj->pIndexData->extra_descr->description, wch->getNameP( ) );
	obj->addExtraDescr( obj->pIndexData->extra_descr->keyword, buf );
    }
}   

void HunterEquip::get( Character *ch )
{
    canEquip( ch );
}

bool HunterEquip::canEquip( Character *ch )
{
    if (ch->is_immortal( ))
	return true;
    
    if (obj->hasOwner( ch ) && ch->getClan( ) == clan)
    {
	ch->pecho( "{C%1$^O1 ������%1$n��|�� ���������.{x", obj );
	return true;
    }
    else {
	ch->pecho( "�� �� ������ ������� %1$O5 � �������� %1$P2.", obj );
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	return false;
    }
}

/*---------------------------------------------------------------------------
 * Hunter's Armor 
 *-------------------------------------------------------------------------*/
void HunterArmor::wear( Character *ch )
{
    obj->level = ch->getRealLevel( );

    if (obj->affected) {
	Affect *paf;

	for (paf = obj->affected; paf; paf = paf->next)
	    addAffect( ch, paf );
    }
    else {
	Affect af;

	af.where = TO_OBJECT;
	af.type  = -1;
	af.duration = -1;
	af.bitvector = 0;

	af.location = APPLY_AC;
	addAffect( ch, &af );
	affect_to_obj( obj, &af );

	af.location = APPLY_DAMROLL;
	addAffect( ch, &af );
	affect_to_obj( obj, &af );
    }
}

void HunterArmor::addAffect( Character *ch, Affect *paf ) 
{
    int level = ch->getModifyLevel( );

    switch (paf->location) {
    case APPLY_DAMROLL:
	paf->level = level;
	paf->modifier = level / 7;
	return;
    case APPLY_AC:
	paf->level = level;
	paf->modifier = -level;
	return;
    }
}

bool HunterArmor::canLock( Character *ch ) 
{ 
    return obj->hasOwner( ch );
}

void HunterArmor::delete_( Character *ch ) 
{
    if (obj->hasOwner( ch ))
	extract_obj( obj );
}

bool HunterArmor::mayFloat( ) 
{
    return true;
}

/*---------------------------------------------------------------------------
 * Hunter's Weapon 
 *-------------------------------------------------------------------------*/
void HunterWeapon::wear( Character *ch )
{
    int i;
    
    struct weapon_param {
	int level;
	int value1, value2;
    };
    static const struct weapon_param params [] = {
	{ 10,  8,  3 },
	{ 15,  9,  3 },
	{ 20, 10,  4 },
	{ 30, 10,  5 },
	{ 40, 10,  6 },
	{ 50, 10,  7 },
	{ 55, 10,  8 },
	{ 60, 12,  7 },
	{ 70, 12,  8 },
	{ 80, 12,  9 },
	{ 90, 12, 10 },
	{200, 12, 11 },
    };
    static const int size = sizeof(params) / sizeof(*params); 

    obj->level = ch->getRealLevel( );

    for (i = 0; i < size; i++) 
	if (obj->level <= params[i].level) {
	    obj->value[1] = params[i].value1;
	    obj->value[2] = params[i].value2;
	    break;
	}

    if (obj->affected) {
	Affect *paf;

	for (paf = obj->affected; paf; paf = paf->next)
	    addAffect( ch, paf );
    }
    else {
	Affect af;

	af.where = TO_OBJECT;
	af.type  = -1;
	af.duration = -1;
	af.bitvector = 0;

	af.location = APPLY_HITROLL;
	addAffect( ch, &af );	
	affect_to_obj( obj, &af );

	af.location = APPLY_DAMROLL;
	addAffect( ch, &af );	
	affect_to_obj( obj, &af );
    }
}

void HunterWeapon::addAffect( Character *ch, Affect *paf ) 
{  
    int level = ch->getModifyLevel( );

    switch( paf->location ) {
    case APPLY_DAMROLL:
	paf->level = level;
	paf->modifier = level / 7;
	return;
    case APPLY_HITROLL:
	paf->level = level;
	paf->modifier = level / 5;
	return;
    }
}

void HunterWeapon::fight( Character *ch )
{
    if (obj->wear_loc != wear_wield)
	return;
    
    if (number_percent( ) >= 25)
	return;

    switch (obj->value[0]) {
    case WEAPON_SWORD:	fight_sword( ch );  return;
    case WEAPON_MACE:	fight_mace( ch );   return;
    case WEAPON_AXE:	fight_axe( ch );    return;
    }
}

/* shield cleave and may be destroy victim's equipment */
void HunterWeapon::fight_axe( Character *ch )
{
    Character *victim;
    int chance,ch_weapon,vict_shield;
    Object *shield;

    if ( ( victim = ch->fighting ) == 0 )
	return;
	
    chance=25;

    if ( ( shield = get_eq_char( victim, wear_shield )) == 0 )
	return;

    if (material_is_flagged( shield, MAT_INDESTR ) || shield->pIndexData->limit != -1)
	return;

    /* find weapon skills */
    ch_weapon = ch->getSkill(get_weapon_sn(ch, obj->wear_loc == wear_second_wield));
    vict_shield = std::max(1, gsn_shield_block->getEffective( ch ));
    /* modifiers */

    /* skill */
   chance = chance * ch_weapon / 200;
   chance = chance * 100 / vict_shield;

    /* dex vs. strength */
    chance += ch->getCurrStat(STAT_DEX);
    chance -= 2 * victim->getCurrStat(STAT_STR);

    /* level */
    chance += ch->getRealLevel( ) - victim->getRealLevel( );
    chance += obj->level - shield->level;

    /* and now the attack */

    if (number_percent() < chance){
    	ch->setWait( gsn_shield_cleave->getBeats( )  );
	act_p("$o1 ����������� ������� ��� $C2.",ch,obj,victim,TO_CHAR,POS_DEAD);
	act_p("$o1 ����������� ������� ���� ���.",ch,obj,victim,TO_VICT,POS_DEAD);
	act_p("$o1 ����������� ������� ��� $C2.",ch,obj,victim,TO_NOTVICT,POS_DEAD);
	extract_obj( get_eq_char(victim,wear_shield) );
    }else
    	ch->setWait( gsn_shield_cleave->getBeats( )  );
}

/* stun */
void HunterWeapon::fight_mace( Character *ch )
{
    Character *victim;
    int chance;

    if ( ( victim = ch->fighting ) == 0 )
	return;
	
    chance=25;

    if (number_percent() < chance){
	act_p("$o1 �������� $C4.",ch,obj,victim,TO_CHAR,POS_DEAD);
	act_p("$o1 �������� ����.",ch,obj,victim,TO_VICT,POS_DEAD);
	act_p("$o1 �������� $C4.",ch,obj,victim,TO_NOTVICT,POS_DEAD);
	SET_BIT(victim->affected_by,AFF_WEAK_STUN);
	ch->setWaitViolence( 2 );
    }
}

/* weapon destroy */
void HunterWeapon::fight_sword( Character *ch )
{
    Character *victim;
    Object *wield;
    int chance,ch_weapon,vict_weapon;

    if ( ( victim = ch->fighting ) == 0 )
	return;

    chance=25;

    if ( (wield = get_eq_char( victim, wear_wield )) == 0 )
	return;

    if (material_is_flagged( wield, MAT_INDESTR ) || wield->pIndexData->limit != -1 )
	return;

    /* find weapon skills */
    ch_weapon = ch->getSkill(get_weapon_sn(ch, obj->wear_loc == wear_second_wield));
    vict_weapon = std::max(1, victim->getSkill(get_weapon_sn(victim, false)));
    /* modifiers */

    /* skill */
    chance = chance * ch_weapon / 200;
    chance = chance * 100 / vict_weapon;

    /* dex vs. strength */
    chance += ch->getCurrStat(STAT_DEX) + ch->getCurrStat(STAT_STR);
    chance -= victim->getCurrStat(STAT_STR) +
			2 * victim->getCurrStat(STAT_DEX);

    chance += ch->getRealLevel( ) - victim->getRealLevel( );
    chance += obj->level - wield->level;

    if (number_percent() < chance){
    	ch->setWait( gsn_weapon_cleave->getBeats( )  );
	act_p("$o1 ���������� ������ $C2.",ch,obj,victim,TO_CHAR,POS_DEAD);
	act_p("$o1 ���������� ���� ������.",ch,obj,victim,TO_VICT,POS_DEAD);
	act_p("$o1 ����������  ������ $C2.",ch,obj,victim,TO_NOTVICT,POS_DEAD);
	extract_obj( get_eq_char(victim,wear_wield) );
    }else{
	act_p("$o1 �� ������ ����������� �� ������ $C2.",ch,obj,victim,TO_CHAR,POS_DEAD);
	act_p("$o1 �� ������ ����������� �� ������ ������.",ch,obj,victim,TO_VICT,POS_DEAD);
        act_p("$o1 �� ������ ����������� �� ������ $C2.",ch,obj,victim,TO_NOTVICT,POS_DEAD);
    	ch->setWait( gsn_weapon_cleave->getBeats( )  );
    }
}

/*--------------------------------------------------------------------------
 * Hunter's Clan Area 
 *-------------------------------------------------------------------------*/
Object * ClanAreaHunter::createEquipment( PCharacter *wch )
{
    Object *armor;
    Weapons::iterator i;

    armor = createArmor( wch );

    for (i = weapons.begin( ); i != weapons.end( ); i++) 
	obj_to_obj( createWeapon( wch, i->second ), armor );

    return armor;
}

Object * ClanAreaHunter::createArmor( PCharacter *wch )
{
    Object *armor;

    armor = create_object( get_obj_index( armorVnum ), 0 );
    armor->behavior.getDynamicPointer<HunterArmor>( )->config( wch );
    return armor;
}

Object * ClanAreaHunter::createWeapon( PCharacter *wch, int vnum )
{
    Object *weapon;
    
    weapon = create_object( get_obj_index( vnum ), 0 );
    weapon->behavior.getDynamicPointer<HunterWeapon>( )->config( wch );
    return weapon;
}

int ClanAreaHunter::vnumByString( const DLString& msg )
{
    Weapons::iterator i;

    if (msg == "armor" || msg == "armour")
	return armorVnum;
    else
	for (i = weapons.begin( ); i != weapons.end( ); i++) 
	    if (msg == i->first) 
		return i->second;

    return 0;
}


/*
 * 'hunt' command
 */
SKILL_RUNP( hunt )
{
    char arg[MAX_STRING_LENGTH];
    Character *victim;
    Road road;
    bool fArea;
    
    if (!gsn_hunt->available( ch )) {
	ch->send_to("�� �� ������ ���������.\n\r");
	return;
    }
    if (!gsn_hunt->usable( ch ))
	return;

    one_argument( argument, arg );

    if( arg[0] == '\0' ) {
	ch->send_to( "���� �����������?\n\r");
	return;
    }

    fArea = !(ch->is_immortal());

    if (fArea && gsn_world_find->available( ch )) {
	if (number_percent() < gsn_world_find->getEffective( ch )) {
	    fArea = false;
	    gsn_world_find->improve( ch, true );
	}
	else {
	    gsn_world_find->improve( ch, false );
	    ch->send_to ("����� ������ ������������, ����� ������ �� ����� ����!\n\r");
	}
    }

    victim = get_char_area( ch, arg);

    if (!fArea && victim == 0)
	victim = get_char_world( ch, arg);

    if (victim == 0) {
	ch->send_to("��� ������ ����� � ����� ������.\n\r");
	return;
    }

    if (victim->in_room == 0) {
	ch->send_to("�� �� ������ ����� ����������, ��� ��������� ����.\n\r");
	return;
    }

    if( ch->in_room == victim->in_room ) {
	act_p( "$C1 ����� �����!", ch, 0, victim, TO_CHAR,POS_RESTING );
	return;
    }

    /*
     * Deduct some movement.
     */
    if (!ch->is_immortal()) {
	if (ch->endur > 2)
	    ch->endur -= 3;
	else {
	    ch->send_to( "���� ���� ���������� � �� �� ������ ���������!\n\r");
	    return;
	}
    }

    act( "$c1 �������������� ����������� ��������� � ����� �� �����.", ch, 0, 0, TO_ROOM );

    ch->setWait( gsn_hunt->getBeats( )  );
    
    road = room_first_step( 
		    ch,
		    ch->in_room, 
		    victim->in_room, 
		    true, false, false );
    
    if (road.type == Road::DOOR)
	act( "$C1 �� $t ������.", ch, dirs[road.value.door].name, victim, TO_CHAR );
    else
	act( "���� �� ������� ������, ��� ������ � $C3.", ch, 0, victim, TO_CHAR );
}

SPELL_DECL(FindObject);
VOID_SPELL(FindObject)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    char buf[MAX_INPUT_LENGTH];
    ostringstream buffer;
    Object *obj;
    Object *in_obj;
    bool found;
    int number = 0, max_found;
    
    found = false;
    number = 0;
    max_found = ch->is_immortal() ? 200 : 2 * level;

    for ( obj = object_list; obj != 0; obj = obj->next )
    {
	if ( !ch->can_see( obj ) || !is_name( target_name, obj->getName( ))
		|| number_percent() > 2 * level
		|| ch->getModifyLevel() < obj->level
		|| IS_OBJ_STAT(obj, ITEM_NOFIND) )
	    continue;

	found = true;
	number++;

	for ( in_obj = obj; in_obj->in_obj != 0; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != 0 && ch->can_see(in_obj->carried_by))
	{
	    sprintf( buf, "������� � %s\n\r",
		ch->sees(in_obj->carried_by,'2').c_str() );
	}
	else
	{
	    if (ch->is_immortal() && in_obj->in_room != 0)
		sprintf( buf, "��������� � %s [������� %d]\n\r",
		    in_obj->in_room->name, in_obj->in_room->vnum);
	    else
		sprintf( buf, "��������� � %s\n\r",
		    in_obj->in_room == 0
			? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = Char::upper(buf[0]);
	buffer << buf;

	if (number >= max_found)
	    break;
    }

    if ( !found )
	ch->send_to("� Dream Land ��� ������ �������� �� ���.\n\r");
    else
	page_to_char( buffer.str( ).c_str( ), ch );
}




SPELL_DECL(TakeRevenge);
VOID_SPELL(TakeRevenge)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    Object *obj;
    Room *room;

    if (!IS_DEATH_TIME( ch ))
    {
	ch->send_to("������� ������ ������ � ����� ���������.\n\r");
	return;
    }
    
    obj = get_obj_world_unique( OBJ_VNUM_CORPSE_PC, ch );
    room = (obj ? obj->getRoom( ) : 0);

    if (room == 0)
	ch->send_to("���, ������ ���� ���� ��������� �� ����.\n\r");
    else if ( IS_SET(room->affected_by,AFF_ROOM_PREVENT) )
	ch->send_to ("������, �� ���� �� ������� ��������� ����.\n\r");
    else
	transfer_char( ch, ch, room );
}


SPELL_DECL_T(Wolf, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, Wolf)::createMobile( Character *ch, int level ) const 
{
    return createMobileAux( ch, ch->getModifyLevel( ), 
	                 ch->hit, 
			 (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
			 number_range(level/15, level/10),
			 number_range(level/3, level/2),
			 number_range(level/8, level/6) );
}

/*--------------------------------------------------------------------------
 * trap skills
 *-------------------------------------------------------------------------*/
/*
 * base object for hunters trap
 */
HunterTrapObject::HunterTrapObject( ) 
	    : activated( false )
{
}

bool HunterTrapObject::checkPrevent( Character *victim )
{
    if (victim->can_see( obj ))
	return true;

    if (!victim->is_npc( ) 
	    && (victim->getClan( ) == clan_none 
		|| victim->getClan( ) == clan_flowers))
	return true;

    if (!victim->isAffected( gsn_prevent ))
	return false;

    if (!saves_spell( ownerLevel, victim, DAM_NONE ))
	return false;

    act("���� ������ ����� �������� ���� �� ������� ���������.", victim, 0, 0, TO_CHAR);
    act("���� ����� �������� $c4 �� ������� ���������.", victim, 0, 0, TO_ROOM);
    return true;
}

bool HunterTrapObject::checkRoom( Room *r )
{
    if (r->clan != clan_none && r->clan != clan_hunter)
	return false;
    
    if (IS_SET(r->room_flags, ROOM_SAFE | ROOM_LAW | ROOM_NO_DAMAGE))
	return false;
    
    return true;
}

bool HunterTrapObject::checkTrapConditions( Character *ch, Skill &skill )
{
    if (obj->carried_by != ch) {
	ch->println( "������� ��� � �����." );
	return false;
    }
    
    if (!skill.usable( ch ))
	return false;

    if (skill.getLearned( ch ) <= 1) {
	ch->println( "������������� �������." );
	return false;
    }
    
    if (ch->mana < skill.getMana( )) {
	ch->println( "� ���� ������������ ������� ��� �����." );
	return false;
    }

    if (ch->position != POS_STANDING) {
	ch->println( "��� ������� ������� ������ ����." );
	return false;
    }
    
    if (IS_SET(ch->in_room->affected_by, AFF_ROOM_PREVENT)) {
	ch->println( "���� �������� ��� ��������� �� ������� ���������." );
	return false;
    }

    return true;
} 

bool HunterTrapObject::visible( const Character *ch ) 
{
    if (ch->is_immortal( ))
	return true;

    if (!activated)
	return true;

    if (ch->is_npc( ))
	return false;
    
    if (ch->getName( ) == ownerName.getValue( ))
	return true;

    if (ch->isAffected(gsn_detect_trap))
	return true;

    return false;
}

void HunterTrapObject::log( Character *ch, const char *verb )
{
    wiznet( WIZ_FLAGS, 0, 110, 
	    "��������� �������: %^C1 %s %O4 � [%d] '%s'",
	    ch, verb, obj, ch->in_room->vnum, ch->in_room->name );
}

/*
 * beacon trap
 */
bool HunterBeaconTrap::hasTrigger( const DLString &t )
{
    return (t == "use");
}

bool HunterBeaconTrap::use( Character *ch, const char *cArgs ) 
{
    PCharacter *victim;
    DLString args = cArgs;
    
    if (!gsn_hunter_beacon->available( ch ))
	return false;

    if (!checkTrapConditions( ch, *gsn_hunter_beacon ))
	return true;
    
    if (!checkRoom( ch->in_room )) {
	ch->println( "����� ������ ������������� �����." );	
	return true;
    }
    
    if (ch->isAffected( gsn_hunter_beacon )) {
	ch->println( "� ������� ��������� ����������� ����� ������ ������� ���� �������." );
	return true;
    }
    
    args.colourstrip( );
    args.stripWhiteSpace( );
    if (args.empty( )) {
	ch->println( "�� ���� ������ ������ ����������� ����?" );
	return true;
    }

    victim = get_player_world( ch->getPC( ), args.c_str( ) );
    if (victim == NULL) {
	ch->println( "������ � ����� ������ �� �������." );
	return true;
    }

    if (is_safe_nomessage( ch, victim )) {
	ch->println( "������ �� ��������� � ����� ��." );
	return true;
    }
    
    if (!chance( gsn_hunter_beacon->getEffective( ch ) )) {
	act( "���� ������� ���������� $o4 ���������� ��������.", ch, obj, 0, TO_CHAR );
	ch->mana -= gsn_hunter_beacon->getMana( ) / 2;
	ch->setWait( gsn_hunter_beacon->getBeats( ) / 2 );
	gsn_hunter_beacon->improve( ch, false );

	if (!chance( ch->getPC( )->getClanLevel( ) * 10 )) {
	    act( "��-�� ��������� ��������� �� ����������� $o4.", ch, obj, 0, TO_CHAR );
	    act( "$c1 ����� �������� ���������� ���������� $o4.", ch, obj, 0, TO_ROOM );
	    extract_obj( obj );
	}
	
	return true;
    } 
    
    act( "�� �������������� $o4 � ������������ ������� �� ��������� $C2.", ch, obj, victim, TO_CHAR );
    act( "$c1 ������������� � ����������� $o4.", ch, obj, 0, TO_ROOM );
    
    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
    REMOVE_BIT( obj->wear_flags, ITEM_TAKE );
    obj->timer = ch->getPC( )->getClanLevel( ) * 5;
    obj->setDescription( activeDescription.getValue( ).c_str( ) );
    
    activated = true;
    victimName = victim->getName( );
    quality = gsn_hunter_beacon->getEffective( ch );
    ownerName = ch->getName( );
    ownerLevel = ch->getModifyLevel( );
    charges = number_range( 1, ch->getPC( )->getClanLevel( ) );
    
    postaffect_to_char( ch, gsn_hunter_beacon, number_range( 0, 1 ) );
    ch->setWait( gsn_hunter_beacon->getBeats( ) );
    ch->mana -= gsn_hunter_beacon->getMana( );
    gsn_hunter_beacon->improve( ch, true );
    
    log( ch, "�������������" );
    return true; 
}

void HunterBeaconTrap::greet( Character *victim )
{
    if (!activated || !obj->in_room)
	return;
    
    if (victim->is_npc( ) || victim->is_immortal( ))
	return;
    
    if (victimName.getValue( ) != victim->getName( ))
	return;
	
    if (checkPrevent( victim ))
	return;

    if (!chance( quality + 10 ))
	return;

//    act( "����� � ����� ��������� ������.", victim, 0, 0, TO_ALL );

    clantalk( *clan_hunter, 
	      "��������! �������� ����, ������������� � '%s' � ����������� �� ��������� %s.",
	      obj->in_room->name, victim->getNameP( '2' ).c_str( ) );
    
    log( victim, "������������" );

    if (( charges = charges - 1 ) <= 0)
	extract_obj( obj );
}

    
/*
 * damage from getting captured into the snare
 */
struct HunterSnareDamage : public Damage {
    HunterSnareDamage( Character *ch, HunterSnareTrap::Pointer snare, bool fMovement ) 
                      : Damage( ch, ch, DAM_SLASH, 0, DAMF_WEAPON ) 
    {
	this->snare = snare;
	this->fMovement = fMovement;
    }
    
    virtual ~HunterSnareDamage( ) {
    }
	
    virtual void message( ) {
	if (fMovement) {
	    msgChar( "%^O1\6���� ���� ��� ������", snare->getObj( ) );
	    msgRoom( "%^C1 �������� �� ����, �������� �� ������� � %O4 ����", ch, snare->getObj( ) );
	}
	else {
	    msgRoom( "%^O1\6%C4", snare->getObj( ), ch );
	    msgChar( "%^O1\6����", snare->getObj( ) );
	}
    }

    virtual void calcDamage( ) {
	int level = snare->getObj( )->level;

	if (fMovement) 
	    dam = number_range( level / 2, level * 2 );	
	else 
	    dam = number_range( level * 5, level * 8 );

	if (ch->getClan( ) == clan_lion)
	    dam += dam / 5;

	dam = dam * snare->getQuality( ) / 100;

	protectSanctuary( );
	protectImmune( );
	protectRazer( );
	protectMaterial( snare->getObj( ) );
    }

protected:
    HunterSnareTrap::Pointer snare;
    bool fMovement;
};

    

/*
 * snare trap
 */
bool HunterSnareTrap::hasTrigger( const DLString &t )
{
    return (t == "use");
}

bool HunterSnareTrap::use( Character *ch, const char *cArgs ) 
{
    if (!gsn_hunter_snare->available( ch ))
	return false;

    if (!checkTrapConditions( ch, *gsn_hunter_snare ))
	return true;

    if (!checkRoom( ch->in_room )) {
	ch->println( "����� ���������� ���������� � ������������� ������." );	
	return true;
    }
    
    if (ch->isAffected( gsn_hunter_snare )) {
	ch->println( "���������� ������ ��� ���������� ����� ������ �������." );
	return true;
    }

    if (obj->level > ch->getModifyLevel( )) {
	ch->println( "���������� ����� ������� ������� ������ ��� ������ ���������." );
	return true;
    }

    if (!ownerName.getValue( ).empty( )) {
	ch->println( "� ���� ������� ��� ���-�� �������." );
	return true;
    }
    
    if (!chance( gsn_hunter_snare->getEffective( ch ))) {
	if (!chance( ch->getPC( )->getClanLevel( ) * 10 )) {
	    ch->pecho( "�� ��������� �������� %1$O4, �� ��������� � %1$P4 ����������� ����. ��� ������!", obj );
	    ch->recho( "%2$^C1 �������� �������� %1$O4, �� �������� � %1$P4 ����������� ����.", obj, ch );
	    rawdamage( ch, ch, DAM_PIERCE, ch->hit / 10, true );
	}
	else {
	    ch->pecho( "�� ��������� ���������� %1$O4, �� ������ ������� %1$P2.", obj );
	    ch->recho( "%2$^C1 �������� ���������� %1$O4, �� ������ ������ %1$P2.", obj, ch );
	}

	ch->setWait( gsn_hunter_snare->getBeats( ) / 2 );
	gsn_hunter_snare->improve( ch, false );
	extract_obj( obj );
	return true;
    }
    
    act( "�� �������������� � ���������� $o4.", ch, obj, 0, TO_CHAR );
    act( "$c1 ������������� � ��������� $o4.", ch, obj, 0, TO_ROOM );

    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
    obj->timer = 24 * 60;
    REMOVE_BIT( obj->wear_flags, ITEM_TAKE );
    obj->setDescription( activeDescription.getValue( ).c_str( ) );
    
    activated = true;
    ownerName = ch->getName( );
    ownerLevel = ch->getModifyLevel( );
    quality = gsn_hunter_snare->getEffective( ch );

    postaffect_to_char( ch, gsn_hunter_snare, number_range( 1, 3 ) );
    ch->setWait( gsn_hunter_snare->getBeats( ) );
    ch->mana -= gsn_hunter_snare->getMana( );
    gsn_hunter_snare->improve( ch, true );

    log( ch, "���������" );
    return true;
}

void HunterSnareTrap::greet( Character *victim )
{
    if (!activated || !obj->in_room)
	return;

    if (is_safe_rspell_nom( ownerLevel, victim ))
	return;

    if (get_eq_char( victim, wear_hold_leg ))
	return;

    if (is_flying( victim ))
	return;

    if (!chance( quality ))
	return;

    if (checkPrevent( victim ))
	return;

    obj_from_room( obj );
    obj_to_char( obj, victim );
    equip_char( victim, obj, wear_hold_leg );
    SET_BIT(obj->wear_flags, ITEM_TAKE);
    obj->fmtDescription( "����������� %s ����� ���.", obj->getShortDescr( '1' ).c_str( ) );
    obj->timer = 24;
    activated = false;
    
    act( "���� ���� ������ � $o4!", victim, obj, 0, TO_CHAR );
    act( "$c1 �����$g��|�|�� � $o4!", victim, obj, 0, TO_ROOM );

    try {
	HunterSnareDamage( victim, this, false ).hit( true );
	victim->setWait( gsn_hunter_snare->getBeats( ) );
    } catch (const VictimDeathException &) {
    }

    log( victim, "�������� �" );
}


bool HunterSnareTrap::checkRoom( Room *r )
{
    if (!HunterTrapObject::checkRoom( r ))
	return false;

    switch (r->sector_type) {
    case SECT_FOREST:
    case SECT_HILLS:
    case SECT_FIELD:
    case SECT_MOUNTAIN:
	return true;
    default:
	return false;
    }
}

void HunterSnareTrap::fight( Character *ch )
{
    if (obj->wear_loc != wear_hold_leg)
	return;

    ch->move -= move_dec( ch );
}

void HunterSnareTrap::entry( )
{
    Character *ch = obj->carried_by;
    
    if (obj->wear_loc != wear_hold_leg)
	return;

    if (is_flying( ch ))
	return;
    
    try {
	HunterSnareDamage( ch, this, true ).hit( true );
    } catch (const VictimDeathException &) {
    }
}

int HunterSnareTrap::getQuality( ) const
{
    return quality.getValue( );
}

/*
 * shovel for pit trap
 */
bool HunterShovel::hasTrigger( const DLString &t )
{
    return (t == "use");
}

bool HunterShovel::use( Character *ch, const char *cArgs ) 
{
    Object *pit;
    HunterPitTrap::Pointer bhv;
    int moveCost, chance;
    
    if (!gsn_hunter_pit->available( ch ))
	return false;
    
    if (obj->wear_loc == wear_none) {
	act( "�� �� ������� $o4 � �����.", ch, obj, 0, TO_CHAR );
	return true;
    }
    
    if (!checkTrapConditions( ch, *gsn_hunter_pit ))
	return true;

    if (!checkRoom( ch->in_room )) {
	ch->println( "������� ����� ���������� ��� ������� ���." );	
	return true;
    }
    
    moveCost = ch->max_move / 4;

    if (ch->move < moveCost) {
	act( "�� ������� ����$g��|�|��.", ch, 0, 0, TO_CHAR );
	return true;
    }
    
    if (obj->condition < 10) {
	ch->pecho( "%1$^O1 ������� �������%1$G���|��|���|���.", obj );
	return true;
    }
    
    pit = get_obj_room_vnum( ch->in_room, OBJ_VNUM_HUNTER_PIT );

    if (!pit) {
	pit = create_object( get_obj_index( OBJ_VNUM_HUNTER_PIT ), 0 );
	obj_to_room( pit, ch->in_room );
    }
    
    if (!pit->behavior || !(bhv = pit->behavior.getDynamicPointer<HunterPitTrap>( ))) {
	ch->println( "���-�� �� ���.." );
	return true;
    }
    
    if (!bhv->isFresh( ) && !bhv->isOwner( ch )) {
	ch->println( "������ ������� ��� ����� ������ ����� ���, �� ����� ��� ������." );
	return true;
    }
    
    if (bhv->getSteaks( )) {
	ch->println( "��� ��� ��� ������������� � ���� ������." );
	return true;
    }

    bhv->setOwner( ch );
    chance = gsn_hunter_pit->getEffective( ch );

    if (bhv->getDepth( ) == 0) {
	act( "�� ��������� ������ $o4.", ch, pit, 0, TO_CHAR );
	act( "$c1 �������� ������ $o4.", ch, pit, 0, TO_ROOM );
	bhv->setDepth( 1 );
    }
    else {
	if (number_percent( ) < number_fuzzy( chance )) {
	    act( "�� �������� $O5, ��� ������ �������� $o4.", ch, pit, obj, TO_CHAR );
	    act( "$c1 ������� $O5, �������� $o4.", ch, pit, obj, TO_ROOM );
	    bhv->setDepth( bhv->getDepth( ) + 1 );
	    gsn_hunter_pit->improve( ch, true );
	}
	else {
	    act( "�� �������� $o4 � �����, �� ����������� �� ������.", ch, obj, 0, TO_CHAR );
	    act( "$c1 ������� $o4 � �����, �� ���������� �� ������.", ch, obj, 0, TO_ROOM );
	    gsn_hunter_pit->improve( ch, false );
	}
    }
    
    bhv->setDescription( );

    if (number_percent( ) < 10) {
	ch->pecho( "%1$^O1 ������ ���%1$n����|����.", obj );
	obj->condition = max( 1, obj->condition - 10 );
    }
    
    ch->setWait( gsn_hunter_pit->getBeats( ) );
    ch->move -= moveCost;
    ch->mana -= gsn_hunter_pit->getMana( );
    
    save_items( ch->in_room );
    return true;
}

bool HunterShovel::checkRoom( Room *r )
{
    if (!HunterTrapObject::checkRoom( r ))
	return false;

    switch (r->sector_type) {
    case SECT_FOREST:
    case SECT_HILLS:
    case SECT_FIELD:
	return true;
    default:
	return false;
    }
}
   
/*
 * steaks for pit trap
 */
bool HunterPitSteaks::hasTrigger( const DLString &t )
{
    return (t == "use");
}

bool HunterPitSteaks::use( Character *ch, const char * cArgs )
{
    DLString args = cArgs;
    HunterPitTrap::Pointer bhv;
    Object *pit;

    if (!gsn_hunter_pit->available( ch ))
	return false;

    if (!checkTrapConditions( ch, *gsn_hunter_pit ))
	return true;

    if (obj->level > ch->getModifyLevel( )) {
	act( "�� ������������ ����$g��|��|��, ����� ������������ $o4.", ch, obj, 0, TO_CHAR );
	return true;
    }

    pit = get_obj_room_vnum( ch->in_room, OBJ_VNUM_HUNTER_PIT );
    if (!pit) {
	act( "����� ������ �������� $o4.", ch, obj, 0, TO_CHAR );
	act( "$c1 ����� ������� $o5, ���, ���� �� ��� ��������.", ch, obj, 0, TO_ROOM );
	return true;
    }

    if (!pit->behavior || !(bhv = pit->behavior.getDynamicPointer<HunterPitTrap>( ))) {
	ch->println( "� ���� ���� ���-�� �� ���.." );
	return true;
    }
    
    if (bhv->getSteaks( )) {
	ch->println( "��� ��� ��� ������������� � ���� ������." );
	return true;
    }

    if (!bhv->isOwner( ch )) {
	ch->println( "��� ��� ������� ������ �������." );
	return true;
    }
    
    act("�� �������������� �� ��� $O2 $o4 � ��������� ���������� ���.", ch, obj, pit, TO_CHAR); 
    act("$c1 ������������� �� ��� $O2 $o4 � ��������� ��������� ���.", ch, obj, pit, TO_ROOM); 
    bhv->setReady( ch );
    obj_from_char( obj );
    obj_to_obj( obj, pit );
    ch->setWait( gsn_hunter_pit->getBeats( ) );

    log( ch, "�������������" );
    return true;
}


/*
 * damage from falling into the pit
 */
struct HunterPitDamage : public Damage {
    HunterPitDamage( Character *ch, HunterPitTrap::Pointer pit ) 
                     : Damage( ch, ch, DAM_BASH, 0, DAMF_WEAPON ) 
    {
	this->pit = pit;
    }

    virtual ~HunterPitDamage( ) {
    }
    
    virtual void message( ) {
	msgRoom( "%^O1 � %O6\6 %C4", pit->getSteaks( ), pit->getObj( ), ch );
	msgChar( "%^O1 � %O6\6 ����", pit->getSteaks( ), pit->getObj( ) );
    }
    
    virtual void calcDamage( ) {
	dam = pit->getSteaks( )->level * number_range( 30, 40 );

	if (ch->getClan( ) == clan_lion)
	    dam += dam / 5;

	dam += dam * 10 * max(0, pit->getSize( ) - victim->size) / 100;
	dam = dam * pit->getQuality( ) / 100;

	protectSanctuary( );
	protectImmune( );
	protectRazer( );
	protectMaterial( pit->getSteaks( ) );
    }

    virtual void postDamageEffects( ) {
	Object *obj = pit->getSteaks( );

	if (obj->item_type != ITEM_WEAPON)
	    return;

	if (IS_WEAPON_STAT(obj, WEAPON_POISON)) {
	    if (!saves_spell( obj->level, ch, DAM_POISON )) {   
		Affect af;

		act("�� ����������, ��� �� ���������������� �� ����� �����.", ch, 0, 0, TO_CHAR);
		act("$c1 �������$g��|�|�� ���� �� $o2.", ch, obj, 0, TO_ROOM);

		af.where     = TO_AFFECTS;
		af.type      = gsn_poison;
		af.level     = obj->level;
		af.duration  = obj->level / 4;
		af.location  = APPLY_STR;
		af.modifier  = max( 1, obj->level / 20 );
		af.bitvector = AFF_POISON;
		affect_join( ch, &af );
	    }
	}
    }

protected:
    HunterPitTrap::Pointer pit;
};


/*
 * pit trap 
 */
void HunterPitTrap::greet( Character *victim ) 
{
    if (!activated || !obj->in_room || !getSteaks( ))
	return;

    if (is_safe_rspell_nom( ownerLevel, victim )) 
	return;

    if (is_flying( victim )) 
	return;

    if (victim->size > getSize( )) 
	return;
    
    if (!chance( number_fuzzy( quality ) )) 
	return;
    
    if (checkPrevent( victim )) 
	return;

    activated = false;
    act("�� �������������� � $o4 � ������� ����� �� $O4!", victim, obj, getSteaks( ), TO_CHAR);
    act("$c1 ������������� � $o4 � ������ ����� �� $O4!", victim, obj, getSteaks( ), TO_ROOM);
    
    try { 
	HunterPitDamage( victim, this ).hit( true );

	act("�� ������� ��������.", victim, 0, 0, TO_CHAR);
	act("$c1 ������ ��������.", victim, 0, 0, TO_ROOM);
	victim->position = POS_STUNNED;
	victim->setWait( gsn_hunter_pit->getBeats( ) );
    } catch (const VictimDeathException &) {
    }

    log( victim, "������ �" );
    
    extract_obj( getSteaks( ) );
    unsetReady( );
}

bool HunterPitTrap::area( )
{
    if (getSteaks( ))
	return false;
    
    if (chance( 90 ))
	return false;

    if (getDepth( ) > 0)
	setDepth( getDepth( ) - 1 );

    if (getDepth( ) == 0 && chance( 10 )) {
	extract_obj( obj );
	return true;
    }

    return false;
}

void HunterPitTrap::setDepth( int depth )
{
    this->depth = depth;
}

int HunterPitTrap::getDepth( ) const
{
    return depth.getValue( );
}

int HunterPitTrap::getSize( ) const
{
    return getDepth( ) / 3;
}

void HunterPitTrap::unsetReady( )
{
    activated = false;
    setDescription( );
    obj->timer = 0;
}

void HunterPitTrap::setReady( Character *ch )
{
    activated = true;
    ownerName = ch->getName( );
    ownerLevel = ch->getModifyLevel( );
    quality = gsn_hunter_pit->getEffective( ch );
    obj->setDescription( activeDescription.getValue( ).c_str( ) );
    obj->timer = 60 * 24;
}

Object * HunterPitTrap::getSteaks( ) 
{
    return obj->contains;
}

bool HunterPitTrap::isOwner( Character *ch ) const
{
    return !ch->is_npc( ) && ownerName.getValue( ) == ch->getName( );
}

void HunterPitTrap::setOwner( Character *ch )
{
    ownerName = ch->getName( );
}

int HunterPitTrap::getQuality( ) const
{
    return quality.getValue( );
}

bool HunterPitTrap::isFresh( ) const
{
    return ownerName.getValue( ).empty( );
}

void HunterPitTrap::setDescription( )
{
    obj->fmtDescription( 
	    "� ����� ������ ��� %s �������.", 
	    size_table.message(URANGE( SIZE_TINY, getSize( ), SIZE_GARGANTUAN ), '2' ).c_str( ) );
}

/*
 * 'detect trap' spell
 */
SPELL_DECL(DetectTrap);
VOID_SPELL(DetectTrap)::run( Character *ch, Character *, int sn, int level ) 
{ 
    Affect af;

    if (ch->isAffected(sn)) {
	ch->println( "�� � ��� � ��������� �������� ������ �� �������.");
	return;
    }

    af.where		= TO_AFFECTS;
    af.type             = sn;
    af.level            = level;
    af.duration         = max( 6, ch->getPC( )->getClanLevel( ) * 2 );
    affect_to_char(ch,&af);

    act("������ �� ������ �������� ����� �������.", ch, 0, 0, TO_CHAR);
    act("������ $c2 ���������� ����� ������������.", ch, 0, 0, TO_ROOM);
}

