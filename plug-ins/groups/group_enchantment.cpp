
/* $Id: group_enchantment.cpp,v 1.1.2.28.6.14 2010-09-01 21:20:45 rufina Exp $
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

#include "spelltemplate.h"
#include "group_enchantment.h"
#include "xmlattributerestring.h"
#include "profflags.h"

#include "so.h"

#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "damage_impl.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"

GSN(inspiration);

SPELL_DECL(BlessWeapon);
VOID_SPELL(BlessWeapon)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect af;

    if (obj->item_type != ITEM_WEAPON) {
	ch->send_to("��� �� ������.\n\r");
	return;
    }

    if ( IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
	||  IS_OBJ_STAT(obj,ITEM_DARK)
	||  IS_OBJ_STAT(obj,ITEM_EVIL) )
    {
	ch->pecho("����������� �������� %1$O2 ��������� ���� ��������������.", obj);
	return;
    }
    
    if (IS_WEAPON_STAT(obj,WEAPON_HOLY)) {
	ch->pecho("%1$^O1 ��� �����������%1$G��|�|�� ��� ��������� �����.", obj);
	return;
    }
	
    af.type	 = sn;
    af.level	 = level / 2;
    af.duration	 = level / 8;

    af.where	 = TO_WEAPON;
    af.location	 = 0;
    af.modifier	 = 0;
    af.bitvector = WEAPON_HOLY;
    affect_to_obj( obj, &af);

    af.where     = TO_OBJECT;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL;
    affect_to_obj( obj, &af );

    ch->pecho("�� �������������� %1$O4 ��� ��������� �����.", obj);

}

SPELL_DECL(EnchantArmor);
VOID_SPELL(EnchantArmor)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect *paf;
    Affect af;
    int result, fail;
    int ac_bonus, add_ac;
    bool inspire;

    if (obj->item_type != ITEM_ARMOR)
    {
	ch->send_to("��� �� �������.\n\r");
	return;
    }

    if (obj->wear_loc != wear_none)
    {
	ch->send_to("���� ������ ���������� � ������ ���������.\n\r");
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_NOENCHANT)
	|| (obj->behavior && obj->behavior->isLevelAdaptive( )))
    {
	ch->send_to( "��� ���� �� �������� ���������.\r\n" );
	return;
    }

    inspire = ch->isAffected( gsn_inspiration );
    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != 0; paf = paf->next )
	{
	    if ( paf->location == APPLY_AC )
	    {
	    	ac_bonus = paf->modifier;
	    	fail += 5 * (ac_bonus * ac_bonus);
 	    }

	    else  /* things get a little harder */
	    	fail += 20;
    	}

    for ( paf = obj->affected; paf != 0; paf = paf->next )
    {
	if ( paf->location == APPLY_AC )
  	{
	    ac_bonus = paf->modifier;
	    fail += 5 * (ac_bonus * ac_bonus);
	}

	else /* things get a little harder */
	    fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,85);
    result = number_percent();
    
    if (inspire) 
	result *= 2;

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$o1 ���� ����������... � ����������!", ch,obj,0,TO_ALL);
	extract_obj(obj);
	return;
    }

    if (result < (fail / 3)) /* item disenchanted */
    {
	Affect *paf_next;

	act("$o1 �� ��� ���� ����������... �� ����� �������.", ch,obj,0,TO_ALL);
	obj->enchanted = true;

	/* remove all affects */
	for (paf = obj->affected; paf != 0; paf = paf_next)
	{
	    paf_next = paf->next;
	    ddeallocate( paf );
	}

	obj->affected = 0;

	/* clear all flags */
	obj->extra_flags = obj->pIndexData->extra_flags;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	ch->send_to("������ �� ���������.\n\r");
	return;
    }
    
    if (inspire) {
	act( "$o1 �� ��������� �������� ���� ������� �����..", ch, obj, 0, TO_ALL );
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	add_ac = - number_range( 3, 5 );
    }
    else if (result <= (90 - level/5))  /* success! */
    {
	act("������� ���� �������� $o4.",ch,obj,0,TO_ALL);
	add_ac = -1;
    }
    else  /* exceptional enchant */
    {
	act("$o1 ���������� ������������-������� ������!", ch,obj,0,TO_ALL);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	add_ac = -2;
    }

    if (ch->getTrueProfession( )->getFlags( ch ).isSet(PROF_MAGIC))
	SET_BIT(obj->extra_flags,ITEM_MAGIC);
		
    /* now add the enchantments */

    if (obj->level < LEVEL_HERO)
	obj->level = min(LEVEL_HERO - 1,obj->level + 1);
    
    affect_enchant( obj );

    af.where     = TO_OBJECT;
    af.bitvector = 0;
    af.type      = sn;
    af.level     = level;

    af.duration  = -1;
    af.location  = APPLY_AC;
    af.modifier  = add_ac;
    affect_enhance( obj, &af );
    
    if (inspire) {
	int add_hr, add_dr;
	
	add_hr = number_range( 0, obj->level / 50 );
	add_dr = number_range( 0, obj->level / 50 );

	if (add_hr > 0) {
	    af.duration  = 200;
	    af.location  = APPLY_HITROLL;
	    af.modifier  = add_hr;

	    affect_enhance( obj, &af );
	}

	if (add_dr > 0) {
	    af.duration  = 200;
	    af.location  = APPLY_DAMROLL;
	    af.modifier  = add_dr;

	    affect_enhance( obj, &af );
	}
    }
}


SPELL_DECL(EnchantWeapon);
VOID_SPELL(EnchantWeapon)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect *paf;
    Affect af;
    int result, fail;
    int hit_bonus, dam_bonus, added;

    if (obj->item_type != ITEM_WEAPON)
    {
	ch->send_to("��� �� ������.\n\r");
	return;
    }

    if (obj->wear_loc != wear_none)
    {
	ch->send_to("���� ������ ���������� � ������ ���������.\n\r");
	return;
    }

    if (IS_OBJ_STAT(obj, ITEM_NOENCHANT)
	|| (obj->behavior && obj->behavior->isLevelAdaptive( )))
    {
	ch->send_to( "��� ������ �� �������� ���������.\r\n" );
	return;
    }
    
    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;	/* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
    	for ( paf = obj->pIndexData->affected; paf != 0; paf = paf->next )
    	{
            if ( paf->location == APPLY_HITROLL )
            {
	    	hit_bonus = paf->modifier;
	    	fail += 2 * (hit_bonus * hit_bonus);
 	    }

	    else if (paf->location == APPLY_DAMROLL )
	    {
	    	dam_bonus = paf->modifier;
	    	fail += 2 * (dam_bonus * dam_bonus);
	    }

	    else  /* things get a little harder */
	    	fail += 25;
    	}

    for ( paf = obj->affected; paf != 0; paf = paf->next )
    {
	if ( paf->location == APPLY_HITROLL )
  	{
	    hit_bonus = paf->modifier;
	    fail += 2 * (hit_bonus * hit_bonus);
	}

	else if (paf->location == APPLY_DAMROLL )
  	{
	    dam_bonus = paf->modifier;
	    fail += 2 * (dam_bonus * dam_bonus);
	}

	else /* things get a little harder */
	    fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	fail -= 5;

    fail = URANGE(5,fail,95);
    result = number_percent();

    if (ch->isAffected( gsn_inspiration )) 
	result += result / 4;

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
	act("$o1 ������ �����������... � ����������!", ch,obj,0,TO_ALL);
	extract_obj(obj);
	return;
    }


   if (result < (fail / 2)) /* item disenchanted */
    {
	Affect *paf_next;

	act("$o1 �� ��� ���� ����������... �� ����� �������.", ch,obj,0,TO_ALL);
	obj->enchanted = true;

	/* remove all affects */
	for (paf = obj->affected; paf != 0; paf = paf_next)
	{
	    paf_next = paf->next;
	    ddeallocate( paf );
	}
	obj->affected = 0;

	/* clear all flags */
	obj->extra_flags = obj->pIndexData->extra_flags;
	return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
	ch->send_to("������ �� ���������.\n\r");
	return;
    }
    
    if (ch->isAffected( gsn_inspiration )) {
	act( "$o1 �� ��������� �������� ���� ������� �����..", ch, obj, 0, TO_ALL );
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = number_range( 1, 3 );
    }
    else if (result <= (100 - level/5))  /* success! */
    {
	act("������� ���� �������� $o4.",ch,obj,0,TO_ALL);
	added = 1;
    }
    else  /* exceptional enchant */
    {
	act("$o1 ���������� ������������-������� ������!", ch,obj,0,TO_ALL);
	SET_BIT(obj->extra_flags,ITEM_GLOW);
	added = 2;
    }
		
    if (ch->getTrueProfession( )->getFlags( ch ).isSet(PROF_MAGIC))
	SET_BIT(obj->extra_flags,ITEM_MAGIC);

    /* now add the enchantments */

    if (obj->level < LEVEL_HERO - 1)
	obj->level = min(LEVEL_HERO - 1,obj->level + 1);
    
    affect_enchant( obj );

    af.where     = TO_OBJECT;
    af.bitvector = 0;
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;
    af.modifier  = added;

    af.location  = APPLY_DAMROLL;
    affect_enhance( obj, &af );

    af.location  = APPLY_HITROLL;
    affect_enhance( obj, &af );
}

SPELL_DECL(Fireproof);
VOID_SPELL(Fireproof)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect af;
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
	ch->pecho("%1$^O1 ��� ������%1$G��|�|�� �� ����.", obj);
	return;
    }

    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy(level / 4);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_BURN_PROOF;
    affect_to_obj( obj, &af);

    act("�������� ���� �������� $o4.",ch,obj,0,TO_ALL);
}


SPELL_DECL(FlameOfGod);
VOID_SPELL(FlameOfGod)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect af;
    int chance;

    if (obj->item_type != ITEM_WEAPON) {
	ch->send_to( "��� �� ������.\r\n" );
	return;
    }
    
    if (obj->isAffected(sn )) {
	act_p("��������� ����� ��� ������ � $o6.", ch, obj, 0, TO_CHAR, POS_RESTING);
	return;
    }
    
    if ( IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
	||  IS_OBJ_STAT(obj,ITEM_DARK|ITEM_EVIL)
	||  IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) )
    {
	act_p("�� �� ������ ������ ��������� ����� � $o6.",ch,obj,0,TO_CHAR,POS_RESTING);
	return;
    }
    
    chance = level - obj->level + ch->getSkill( sn );
    chance -= number_percent( );

    if (chance < -10) {
	act_p("�� ��������$g��|�|�� ������ ���� � �� ��������� ������.", ch, 0, 0, TO_CHAR, POS_RESTING);
	extract_obj( obj );
	return;
    }

    if (chance < 0) {
	ch->send_to( "������ �� ���������.\r\n" );
	return;
    }

    af.level     = level;
    af.duration  = level / 4;

    af.where     = TO_WEAPON;
    af.type      = number_bits(2) ? gsn_flamestrike : gsn_fireball;
    af.location  = 0;
    af.modifier  = 10;
    af.bitvector = WEAPON_SPELL;
    affect_to_obj( obj, &af );
	
    af.where     = TO_OBJECT;
    af.type      = sn;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL;
    affect_to_obj( obj, &af );

    act_p("�� �������� � ����� � $o1 ���������� ��������� �����!", ch, obj, 0, TO_CHAR, POS_RESTING);
    act_p("$c1 ������� � ����� � $o1 ���������� ��������� �����!", ch, obj, 0, TO_ROOM, POS_RESTING);
}


SPELL_DECL(HungerWeapon);
VOID_SPELL(HungerWeapon)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect af;
    int chance;

    if (obj->pIndexData->item_type != ITEM_WEAPON) {
	ch->send_to("��� �� ������.\r\n");
	return;
    } 

    if (IS_WEAPON_STAT(obj, WEAPON_HOLY)
	||  IS_OBJ_STAT(obj, ITEM_BLESS)
	||  IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)) 
    {
	    act_p("���� � �����!", ch, 0, 0, TO_ALL, POS_RESTING);
	    rawdamage(ch, ch, DAM_HOLY, 
		    (ch->hit - 1) > 1000 ? 1000 : (ch->hit - 1), true );
	    return;
    } 

    if (IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)) {
	act_p("$o1 ��� ������ ����� �����.", ch, obj, 0, TO_CHAR, POS_RESTING );
	return;
    }

    chance = ch->getSkill( sn );	

    if (IS_WEAPON_STAT(obj, WEAPON_FLAMING))	chance /= 2;
    if (IS_WEAPON_STAT(obj, WEAPON_FROST))	chance /= 2;
    if (IS_WEAPON_STAT(obj, WEAPON_SHARP))	chance /= 2;
    if (IS_WEAPON_STAT(obj, WEAPON_VORPAL))	chance /= 2;
    if (IS_WEAPON_STAT(obj, WEAPON_SHOCKING))	chance /= 2;
    if (IS_WEAPON_STAT(obj, WEAPON_FADING))	chance /= 2;
     
    if (number_percent() < chance) {    
	af.where	= TO_WEAPON;
	af.type 	= sn;
	af.level	= level / 2;
	af.duration	= level / 4;
	af.location	= 0;
	af.modifier	= 0;
	af.bitvector	= WEAPON_VAMPIRIC;
	affect_to_obj( obj, &af);
	
	af.where     = TO_OBJECT;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = ITEM_ANTI_GOOD|ITEM_ANTI_NEUTRAL;
	affect_to_obj( obj, &af );
	
	act_p("�� ��������� $o3 ���� ����� ����� �����...", ch, obj, 0, TO_CHAR, POS_RESTING);
	act_p("$c1 ����������� ������� �� $o4, �$g��|��|� ����� ���������� {r�������{x", ch, obj, 0, TO_ROOM, POS_RESTING);
    } 
    else 
	act_p("�������.", ch, obj, 0, TO_CHAR, POS_RESTING);

}


SPELL_DECL(Mend);
VOID_SPELL(Mend)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    int result,skill;

    if ( obj->condition > 99 )
    {
    ch->send_to("��� ���� �� ��������� � �������.\n\r");
    return;
    }

    if (obj->wear_loc != wear_none)
    {
	ch->send_to("��� ������� ���� ������ ���������� � ������ ���������.\n\r");
	return;
    }

    skill = gsn_mend->getEffective( ch ) / 2;
    result = number_percent ( ) + skill;

    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	  result -= 5;
    if (IS_OBJ_STAT(obj,ITEM_MAGIC))
	  result += 5;

    if (ch->isAffected( gsn_inspiration )) {
	ch->pecho( "%1$^O1 ��%1$n��|�� ��� ������ ������, ������� ������������ ���.", obj );
	ch->recho( "%1$^O1 ��%1$n��|�� ��� ������ %2$C2, ������� ������������ ���.", obj, ch );
	obj->condition = 100;
    }
    else if (result >= 50)
    {
	ch->pecho( "%1$^O1 ������%1$n����|���� ����� ������, ������� ������������ ���.", obj );
	ch->recho( "%1$^O1 ������%1$n����|���� ����� ������, ������� ������������ ���.", obj );
	obj->condition += result;
	obj->condition = min( obj->condition , 100 );
    }
    else if ( result >=10)
    {
	ch->send_to("������ �� ���������.\n\r");
    }
    else
    {
	ch->pecho( "%1$^O1 ���� ��������%1$n��|��... � ������%1$n����|����!", obj );
	ch->recho( "%1$^O1 ���� ��������%1$n��|��... � ������%1$n����|����!", obj );
	extract_obj(obj);
    }
}


SPELL_DECL(Recharge);
VOID_SPELL(Recharge)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    int chance, percent;

    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
	ch->println("�� ������ ������������ ���������� ������ � ��������� ������� � �������.");
	return;
    }

    if (obj->value[0] >= 3 * level / 2)
    {
	ch->send_to("���� �� ������� ���������� ��� �������������� ���� ����������.\n\r");
	return;
    }

    if (obj->value[1] == 0)
    {
	ch->send_to("��� ���������� ������ �� ����� ���� �������������.\n\r");
	return;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[0]; /* harder to do high-level spells */
    chance -= (obj->value[1] - obj->value[2]) *
	      (obj->value[1] - obj->value[2]);

    chance = max(level/2,chance);

    percent = number_percent();

    if (percent < chance / 2)
    {
	act_p("$o1 ����� ����������.",ch,obj,0,TO_CHAR,POS_RESTING);
	act_p("$o1 ����� ����������.",ch,obj,0,TO_ROOM,POS_RESTING);
	obj->value[2] = max(obj->value[1],obj->value[2]);
	obj->value[1] = 0;
	return;
    }

    else if (percent <= chance)
    {
	int chargeback,chargemax;

	act_p("$o1 ����� ����������.",ch,obj,0,TO_CHAR,POS_RESTING);
	act_p("$o1 ����� ����������.",ch,obj,0,TO_CHAR,POS_RESTING);

	chargemax = obj->value[1] - obj->value[2];

	if (chargemax > 0)
	    chargeback = max(1,chargemax * percent / 100);
	else
	    chargeback = 0;

	obj->value[2] += chargeback;
	obj->value[1] = 0;
	return;
    }

    else if (percent <= min(95, 3 * chance / 2))
    {
	ch->send_to("������ �� ���������.\n\r");
	if (obj->value[1] > 1)
	    obj->value[1]--;
	return;
    }

    else /* whoops! */
    {
	act_p("$o1 ���� ���������� � ����������!",ch,obj,0,TO_CHAR,POS_RESTING);
	act_p("$o1 ���� ���������� � ����������!",ch,obj,0,TO_ROOM,POS_RESTING);
	extract_obj(obj);
    }

}


SPELL_DECL(WeaponMorph);
VOID_SPELL(WeaponMorph)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    Object *obj;
    int result, fail;
    DLString args( target_name ), arg1, arg2;

    arg1 = args.getOneArgument( );
    arg2 = args.getOneArgument( );
    
    if (!( obj = get_obj_carry( ch, arg1.c_str( ) ) )) {
	ch->println( "� ���� ��� �����." );
	return;
    }

    if (obj->item_type != ITEM_WEAPON) {
	ch->println("�� ������ ������� ������ ������ �� ������.");
	return;
    }

    if (obj->value[0] == WEAPON_MACE
	    || obj->value[0] == WEAPON_ARROW
	    || IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
    {
	ch->println("�� �� ������ ������� ������ �� ����� ������.");
	return;
    }

    if (obj->pIndexData->limit != -1) {
	ch->println("��� ���� - ���������. �� ������ �� ������ �������.");
	return;
    }

    fail = 70;
    fail -=level / 3;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    fail -= 15;

    if (IS_OBJ_STAT(obj,ITEM_GLOW))
	    fail -= 5;

    fail = URANGE(5,fail,95);
    result = number_percent();

    if (result < (fail / 3))  /* item destroyed */
    {
	obj->getRoom( )->echo( POS_RESTING, 
	    "{W%1$^O1 ���� ��������%1$n��|��... � ������%1$n����|����!{x", obj );
	return;
    }

    obj->value[0] = WEAPON_MACE;
    obj->value[3] = DAMW_POUND;
    obj->getRoom( )->echo( POS_RESTING, 
	"{W%1$^O1 ������%1$n����|���� {R����-������� �����{W � ���������%1$n��|�� ����� �����.{x", obj );

    obj->setName( "mace ������" );
    obj->setShortDescr( fmt( 0, "�����|��|��|��|��|��|�� �����|�|�|�|�|��|� %C2", ch ).c_str( ) );
    obj->setDescription( fmt( 0, "������� ������ (mace) ������� %C5 � ������ �����.", ch ).c_str( ) );
    dress_created_item( sn, obj, ch, arg2 );
}


SPELL_DECL(WintersTouch);
VOID_SPELL(WintersTouch)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect af;

    if (obj->item_type != ITEM_WEAPON) {
	act_p("$o1 - �� ������.", ch, obj, 0, TO_CHAR, POS_RESTING);
	return;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING) || IS_WEAPON_STAT(obj,WEAPON_SHOCKING)) {
	act_p("� $o5, �������, ������ �� ����������.", ch, obj, 0, TO_CHAR, POS_RESTING);
	return;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_FROST)) {
	act_p("$o1 � ��� ���� ������������.", ch, obj, 0, TO_CHAR, POS_RESTING);
	return;
    }

    af.where        = TO_WEAPON;
    af.type         = sn;
    af.level        = level / 2;
    af.duration     = level / 4;
    af.location     = 0;
    af.modifier     = 0;
    af.bitvector    = WEAPON_FROST;
    affect_to_obj( obj, &af );
    
    act_p("�� ������� $o4 �� ������ ������.", ch, obj, 0, TO_CHAR, POS_RESTING);

}
