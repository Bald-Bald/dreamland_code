/* $Id: class_antipaladin.cpp,v 1.1.2.21.4.14 2009/09/11 11:24:54 rufina Exp $
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
#include "class_antipaladin.h"

#include "logstream.h"
#include "gsn_plugin.h"
#include "spelltemplate.h"

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "behavior_utils.h"

#include "act_move.h"
#include "mercdb.h"

#include "magic.h"
#include "fight.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "vnum.h"
#include "merc.h"
#include "act.h"
#include "interp.h"
#include "handler.h"
#include "def.h"

PROF(anti_paladin);
GSN(shadowblade);

/*----------------------------------------------------------------------------
 * Cleave 
 *---------------------------------------------------------------------------*/
class CleaveOneHit: public WeaponOneHit, public SkillDamage {
public:
    CleaveOneHit( Character *ch, Character *victim );
    
    virtual void calcTHAC0( );
    virtual void calcDamage( );
};

CleaveOneHit::CleaveOneHit( Character *ch, Character *victim )
	    : Damage( ch, victim, 0, 0 ), WeaponOneHit( ch, victim, false ), 
	      SkillDamage( ch, victim, gsn_cleave, 0, 0, DAMF_WEAPON )
{
}

void CleaveOneHit::calcDamage( )
{
    int chance;
    
    damBase( );
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
    damApplyPosition( );

    if (victim->is_immortal( ))
	chance = 0;
    else if (wield == 0)
	chance = 0;
    else {
	chance = 5 + (ch->getModifyLevel( ) - victim->getModifyLevel( ));
	chance = URANGE( 4, chance, 20 );
    }

    if (number_percent( ) < chance) {
	act_p("�� ���������� $C4 {R�������{x!",ch,0,victim,TO_CHAR,POS_RESTING);
	act_p("$c1 ��������� ���� {R�������{x!",ch,0,victim,TO_VICT,POS_RESTING);
	act_p("$c1 ��������� $C4 {R�������{x!",ch,0,victim,TO_NOTVICT,POS_RESTING);

	ch->setWait( 2 );

	handleDeath( );
	throw VictimDeathException( );
    }
    else {
	dam = ( dam * 2 + ch->getModifyLevel() );
    }

    damApplyDamroll( );
    
    damNormalize( );

    if (number_percent( ) < 50)
	protectSanctuary( );

    protectAlign( );
    protectImmune( );
    protectRazer( ); 
    protectMaterial( wield );
}

void CleaveOneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
    thacApplySkill( );
    thac0 -= 10 * (100 - gsn_cleave->getEffective( ch ));
}

SPELL_DECL(Deafen);
VOID_SPELL(Deafen)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  
  Affect af;

  if (ch == victim) {
    ch->send_to("�������� ����?\n\r");
    return;
  }

  if (victim->isAffected(sn)) {
    act_p("$C1 ��� ������ �� ������.",ch,0,victim,TO_CHAR,POS_RESTING);
    return;
  }

  if (saves_spell(level,victim, DAM_SOUND,ch, DAMF_SPELL)) {
	act("���� �� ������� �������� $C4.", ch, 0, victim, TO_CHAR);
	return;
  }

  af.where		= TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 10;
  af.modifier  = 0;
  af.location  = 0;
  af.bitvector = 0;
  affect_to_char(victim,&af);

  act_p("$C1 ������ ������ �� ������!",ch,0,victim,TO_CHAR,POS_RESTING);
  victim->send_to("������������� ���� �������� ����...�� ������ �� �������!\n\r");

}

/*
 * 'cleave' skill command
 */

SKILL_RUNP( cleave )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    Object *obj;

    if ( MOUNTED(ch) ) {
	ch->send_to("�������� � �����, ������ ��� �������!\n\r");
	return;
    }

    one_argument( argument, arg );

    if (ch->master != 0 && ch->is_npc())
	return;

    if (!ch->is_npc() && !gsn_cleave->usable( ch )) {
	ch->send_to("�� �� ������ ��������� �������.\n\r");
	return;
    }

    if (arg[0] == '\0') {
	ch->send_to("��������� ���?\n\r");
	return;
    }

    if (( victim = get_char_room( ch, arg ) ) == 0) {
	ch->send_to("����� ��� �����.\n\r");
	return;
    }

    if (victim == ch) {
	ch->send_to("����???\n\r");
	return;
    }

    if (is_safe( ch, victim ))
	return;

    if ( ( obj = get_eq_char( ch, wear_wield ) ) == 0) {
	ch->send_to("��������� ��� ������.\n\r");
	return;
    }

    if (attack_table[obj->value[3]].damage != DAM_SLASH) {
	ch->send_to("����� ������� ����-��, ����� ���������� ������� �������.\n\r");
	return;
    }

    if (victim->fighting != 0) {
	ch->send_to("�������, ���� ���������� ��������.\n\r");
	return;
    }

    if (victim->hit < 0.9 * victim->max_hit && IS_AWAKE(victim) )
    {
	act_p( "$C1 ����$G��|�|�� � ������������ ������������... �� �� ������� ����������� ���������.",
		ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }

    ch->setWait( gsn_cleave->getBeats( )  );
    
    CleaveOneHit cleave_hit( ch, victim );
    
    try {
	if (!IS_AWAKE(victim) || ch->is_npc() || number_percent( ) < gsn_cleave->getEffective( ch ))
	{
	    gsn_cleave->improve( ch, true, victim );
	    cleave_hit.hit( );
	}
	else
	{
	    gsn_cleave->improve( ch, false, victim );
	    cleave_hit.miss( );
	}

	yell_panic( ch, victim,
	            "��������! ���-�� ����� �� ����!",
		    "��������! %1$^C1 ����� ���� ������� �������!" );
    }
    catch (const VictimDeathException& e) {                                     
    }
}


/*
 * shadow blade
 */

/*
 *  'shadow blade' behavior
 */
ShadowBlade::ShadowBlade( ) 
{ 
}

struct blade_param {
    int min_level;
    int value1, value2;
    int min_hr, min_dr;
    int max_hr, max_dr;
};

static const struct blade_param  blade_params [] = 
{
//    lvl   v1   v2     min hr/dr    max hr/dr
    { 30,   6,   6,     4,  4,       10, 10,   }, 
    { 40,   7,   7,     5,  5,       11, 11,   },
    { 50,   8,   8,     6,  6,       12, 12,   },
    { 60,   9,   10,    7,  7,       13, 13,   },
    { 70,   10,  10,    8,  8,       16, 16,   },
    { 75,   10,  11,    8,  8,       17, 17,   },
    { 80,   10,  11,    9,  9,       18, 18,   },
    { 85,   11,  12,    9,  9,       19, 19,   },
    { 90,   12,  11,    10, 10,      21, 21,   },
    { 95,   12,  11,    11, 11,      23, 23,   },
    { 0 },
};

static const struct blade_param * find_blade_param( int lvl )
{
    int i;
    for (i = 0; blade_params[i+1].min_level; i++) 
	if (blade_params[i+1].min_level >= lvl)
	    break;
    
    return &blade_params[i];
}

void ShadowBlade::fight( Character *ch )
{
    int level;
    Character *victim = ch->fighting;

    if (!victim || !ch || ch->getName( ) != owner.getValue( ))
	return;

    if (obj->wear_loc != wear_wield && obj->wear_loc != wear_second_wield) 
	return;
    
    level = ch->getModifyLevel( );

    if (victim->getModifyLevel( ) > level - 10 && IS_GOOD( victim )) {
	int coef = 100 - bonus;
	int vhp = (victim->hit * coef) / max( 1, (int)victim->max_hit );
	
	if (number_percent( ) > vhp && ++castCnt >= 100) {
	    const struct blade_param *p = find_blade_param( level );
	    Affect *paf;
	    int oldMod;
	    
	    castCnt = 0;
	    
	    if (++castChance > 80)
		castChance = 80;
	    
	    if (( paf = obj->affected->affect_find( gsn_shadowblade ) )) {
		oldMod = paf->modifier;
		paf->modifier = URANGE( p->min_hr, oldMod + 1, p->max_hr );
		ch->hitroll += paf->modifier - oldMod;

		if (paf->next != 0 && paf->next->type == gsn_shadowblade) {
		    oldMod = paf->next->modifier;
		    paf->next->modifier = URANGE( p->min_hr, oldMod + 1, p->max_hr );
		    ch->damroll += paf->next->modifier - oldMod;
		}
	    }
	    
	    obj->value[1] = std::max( obj->value[1], p->value1 );
	    obj->value[2] = std::max( obj->value[2], p->value2 );
	    obj->level = ch->getModifyLevel( );
	    act("{c������ {C������{c ��������� $o4.{x", ch, obj, 0, TO_CHAR);
	}
    }
    
    if (number_percent( ) > castChance)
	return;

    switch (number_range( 1, 4 )) {
    case 1:
	if (!IS_SET(victim->imm_flags, IMM_POISON)) {
	    act("����� {G���{x ������� �� ������ ������ $o2.", ch, obj, 0, TO_CHAR);
	    act("����� {G���{x ������� �� ������ $o2 � ����� $c2.", ch, obj, 0, TO_ROOM);
	    spell( gsn_poison, level + 1, ch, victim, FSPELL_BANE );
	}
	break;
    case 2:
	if (!IS_SET(victim->imm_flags, IMM_DISEASE)) {
	    ch->pecho("{c���������� ����{x �������� ���%1$G�|�|�|� %1$O4.", obj );
	    act("{c���������� ����{x �������� $o4 $c2.", ch, obj, 0, TO_ROOM);
	    spell( gsn_plague, level + 1, ch, victim, FSPELL_BANE );
	}
	break;
    case 3:
	if (!IS_AFFECTED( victim, AFF_CURSE )) {
	    ch->pecho("{D�������� ����{x ��������� ���%1$G�|�|�|� %1$O4.", obj );
	    ch->recho("%1$^O1 %2$C2 �������%1$n����|���� {D�������� �����{x.", obj, ch );
	    spell( gsn_curse, level + 1, ch, victim, FSPELL_BANE );
	}
	break;
    case 4:
	ch->pecho("{D���%1$G�|�|�|� %1$O1 ��������%1$n��|�� {x���������-�������{D ������.{x", obj );
	ch->recho("%1$^O1 %2$C2 ��������%1$n��|�� ���������-������� ������.", obj, ch );
	spell( gsn_energy_drain, level + 2, ch, victim, FSPELL_BANE );	
	break;
    }
}

bool ShadowBlade::area( )
{
    Character *ch;

    if (!( ch = obj->getCarrier( ) ))
	return false;

    if (ch->getTrueProfession( ) == prof_anti_paladin)
	return false;

    if (obj->wear_loc != wear_wield && obj->wear_loc != wear_second_wield)
	return false;
    
    if (ch->fighting == 0)
	return false;

    act( "$o1 �������� ��������� ���� � ������!", ch, obj, 0, TO_CHAR );
    act( "$o1 �������� ��������� $c4 � ������!", ch, obj, 0, TO_ROOM );
    unequip_char( ch, obj );
    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
    rawdamage( ch, ch, DAM_SLASH, std::min( ch->hit - 1, 1000 ), true );
    return false;
}

bool ShadowBlade::canEquip( Character *ch )
{
    if (ch->getTrueProfession( ) != prof_anti_paladin) {
	act( "$o1 ������������� �� ����� ���.", ch, obj, 0, TO_CHAR );
	unequip_char( ch, obj );
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	return false;
    }

    return true;
}

bool ShadowBlade::quit( Character *ch, bool count )
{
    if (ch->getName( ) != owner.getValue( )) {
	act( "$o1 �� ����������� ���� � �� ����� �������� ��� ������ � �����.", ch, obj, 0, TO_CHAR );

	if (obj->carried_by)
	    obj_from_char( obj );
	else if (obj->in_obj)
	    obj_from_obj( obj );
	else
	    obj_from_room( obj );

	obj_to_room( obj, ch->in_room );
    }

    return false;
}

/*
 * 'blade of darkness' spell
 */
SPELL_DECL(BladeOfDarkness);
VOID_SPELL(BladeOfDarkness)::run( Character *ch, Object *blade, int sn, int level ) 
{ 
    ShadowBlade::Pointer behavior;
    Affect af;
    
    if (!blade->behavior 
	|| !(behavior = blade->behavior.getDynamicPointer<ShadowBlade>( ))) 
    {
        ch->send_to( "�� ��� �� ���������� ������!\n\r" );
        return;
    }
    
    if (ch->getName( ) != behavior->owner.getValue( )) {
	ch->send_to( "�� ������ ��������� ��� ���������� ������ �� ���� ����������� ������.\n\r" );
	return;
    }
    
    if ( IS_WEAPON_STAT(blade, WEAPON_FLAMING)
    ||   IS_WEAPON_STAT(blade, WEAPON_VAMPIRIC)
    ||   IS_WEAPON_STAT(blade, WEAPON_VORPAL)
    ||   IS_WEAPON_STAT(blade, WEAPON_POISON)
    ||   IS_WEAPON_STAT(blade, WEAPON_SHOCKING) )
    {
        ch->send_to( "������ �� ���������..\r\n" );
        return;
    }

    if (IS_WEAPON_STAT(blade, WEAPON_FADING)) {
        act( "���� ��� ��������� $o4.", ch, blade, 0, TO_CHAR );
        return;
    }

    af.where            = TO_WEAPON;
    af.type             = sn;
    af.level            = level / 2;
    af.duration         = level / 5;
    af.location         = 0;
    af.modifier         = 0;
    af.bitvector        = WEAPON_FADING;
    affect_to_obj( blade, &af );

    act( "�� ���������� $o4 ��������.", ch, blade, 0, TO_CHAR );
    act( "$c1 ��������� $o4 ��������.", ch, blade, 0, TO_ROOM );
}

/*
 * 'recall shadowblade' spell
 */
SPELL_DECL(RecallShadowBlade);
VOID_SPELL(RecallShadowBlade)::run( Character *ch, char *, int sn, int level ) 
{
    Object *blade = NULL;

    for (Object *obj = object_list; obj; obj = obj->next)
	if (obj->behavior) {
	    ShadowBlade::Pointer bhv = obj->behavior.getDynamicPointer<ShadowBlade>( );
	    
	    if (bhv && ch->getName( ) == bhv->owner.getValue( )) {
		if (obj->getCarrier( ) != ch) {
		    blade = obj;
		    break;
		}
	    }
	}
	
    if (!blade) {
	ch->send_to( "������ �� ���������..\n\r" );
	return;
    }

    if (blade->carried_by) {
	act( "$o1 �������� ��������.", blade->carried_by, blade, NULL, TO_ALL );
	obj_from_char( blade );
    }
    else if (blade->in_room) {
	if (blade->in_room->people)
	    act( "$o1 �������� ��������.", blade->in_room->people, blade, NULL, TO_ALL );

	obj_from_room( blade );
    }
    else 
	obj_from_obj( blade );

    obj_to_char( blade, ch );
    act( "$o1 ����������, ������.", ch, blade, NULL, TO_ALL );
}


/*
 *  'shadowblade' spell (blade creation)
 */
SPELL_DECL(ShadowBlade);
VOID_SPELL(ShadowBlade)::run( Character *ch, char *, int sn, int level ) 
{
    Affect af;
    int cnt;
    Object *obj, *blade;
    OBJ_INDEX_DATA *pObjIndex;
    ShadowBlade::Pointer bhv;
    const struct blade_param *param;

    for (cnt = 0, obj = object_list; obj; obj = obj->next)
	if (obj->behavior) {
	    bhv = obj->behavior.getDynamicPointer<ShadowBlade>( );

	    if (bhv && ch->getName( ) == bhv->owner.getValue( )) {
		cnt++;

		if (cnt >= 2) {
		    ch->send_to( "�� � ���� ��� ���� ���������� ������.\r\n" );
		    return;
		}
	    }
	}
    
    pObjIndex = find_obj_unique_index<ShadowBlade>( );
    if (!pObjIndex) {
	ch->send_to( "� ���� ���-�� ����������.. �� �� ������ ������ ������� ���� ������.\r\n" );
	LogStream::sendError( ) << "ShadowBlade: NULL obj index" << endl;
	return;
    }

    blade = create_object( pObjIndex, 0 );
    blade->level = ch->getModifyLevel( );
    bhv = blade->behavior.getDynamicPointer<ShadowBlade>( );
    bhv->owner = ch->getName( );
    
    param = find_blade_param( ch->getModifyLevel( ) );
    blade->value[1] = param->value1;
    blade->value[2] = param->value2;

    af.where = TO_OBJECT;
    af.type = sn;
    af.level = level;
    af.duration = -1;
    af.bitvector = 0;

    af.location = APPLY_HITROLL;
    af.modifier = param->min_hr;
    affect_to_obj( blade, &af );

    af.location = APPLY_DAMROLL;
    af.modifier = param->min_dr;
    affect_to_obj( blade, &af );
    
    obj_to_char( blade, ch );

    act( "�� �������� $o4!", ch, blade, NULL, TO_CHAR );
    act( "$c1 ������� $o4!", ch, blade, NULL, TO_ROOM );
}

/*---------------------------------------------------------------------------
 * AntipaladinGuildmaster
 *--------------------------------------------------------------------------*/
void AntipaladinGuildmaster::give( Character *victim, Object *obj ) 
{
    ShadowBlade::Pointer behavior;
    int price = 300;

    if (!obj->behavior || !(behavior = obj->behavior.getDynamicPointer<ShadowBlade>( ))) {
	say_act( victim, ch, "��� �� ���������� ������, $c1." );
    }
    else if (victim->getName( ) != behavior->owner.getValue( )) {
	say_act( victim, ch, "���� ������ �� ����������� ����, $c1." );
    }
    else if (IS_SET(obj->extra_flags, ITEM_NOSAC)
	     || IS_SET(obj->wear_flags, ITEM_NO_SAC)) {
	say_act( victim, ch, "��� ���� ������� ��� ��������� �������� ������." );
    }
    else if (victim->is_npc( ) || victim->getPC( )->questpoints < price) {
	say_act( victim, ch, "� ���� �� ������ qp ��� ������ �������." );
    }
    else {
	victim->getPC( )->questpoints -= price;
	SET_BIT(obj->extra_flags, ITEM_NOSAC|ITEM_NOPURGE);
	SET_BIT(obj->wear_flags, ITEM_NO_SAC);
	act("$c1 ����������� � ������ ������ � ���������� �������� ����������.", ch, 0, 0, TO_ROOM);
    }

    act( "$c1 ���������� $o4 $C3.", ch, obj, victim, TO_NOTVICT );
    act( "$c1 ���������� ���� $o4.", ch, obj, victim, TO_VICT );

    obj_from_char( obj );
    obj_to_char( obj, victim );
}


SPELL_DECL(PowerWordStun);
VOID_SPELL(PowerWordStun)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	
	Affect af;

	if ( saves_spell( level, victim, DAM_OTHER, ch, DAMF_SPELL) )
	{
		ch->send_to("�� ����������..\n\r");
		return;
	}

	if (victim->isAffected(sn )) {
	    if (ch == victim)
		act("�� ��� ������$g��|�|��.", ch, 0, 0, TO_CHAR);
	    else
		act("$C1 ��� ������$g��|�|��.", ch, 0, victim, TO_CHAR);
	    return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 50;
	af.location  = APPLY_DEX;
	af.modifier  = -level / 25;
	af.bitvector = AFF_STUN;
	affect_to_char( victim, &af );

	act("{r$c1 ������$g��|�|��{x.",victim, 0, 0,TO_ROOM);
	act("{R�� ������$g��|�|��{x.",victim, 0, 0, TO_CHAR);
}

