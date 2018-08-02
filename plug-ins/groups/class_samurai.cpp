/* $Id: class_samurai.cpp,v 1.1.2.21.6.14 2009/08/16 02:50:30 rufina Exp $
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
#include "class_samurai.h"


#include "skill.h"
#include "spelltemplate.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "desire.h"

#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"
#include "magic.h"
#include "fight.h"
#include "vnum.h"
#include "merc.h"
#include "effects.h"
#include "handler.h"
#include "act.h"
#include "interp.h"
#include "def.h"

GSN(none);
PROF(samurai);
DESIRE(thirst);
DESIRE(hunger);
short get_wear_level( Character *ch, Object *obj );

/*
 * 'enchant sword' skill command
 */

SKILL_RUNP( enchant )
{
    Object *obj;
    int wear_level, mana;

    if ( !ch->is_npc() &&   !gsn_enchant_sword->usable( ch ) )
    {
	ch->send_to("����?\n\r");
	return;
    }

    if (argument[0] == '\0') /* empty */
    {
        ch->send_to("����� ������ �� ������ ��������?\n\r");
        return;
    }

    obj = get_obj_carry (ch, argument);

    if (obj == 0)
    {
        ch->send_to("� ���� ��� �����.\n\r");
        return;
    }

    wear_level = get_wear_level( ch, obj );

    if (wear_level > ch->getRealLevel( ))
    {
	ch->pecho("�� ����%G��|��|�� ������� %d ������, ����� �������� ���.", ch, wear_level );
        act( "$c1 �������� �������� $o1, �� ��� ������� ������.", ch, obj, 0, TO_ROOM);
        return;
    }
    
    mana = gsn_enchant_sword->getMana( );
    
   if (ch->mana < mana )
	{
	 ch->send_to("� ���� ������������ ������� ��� �����.\n\r");
	 return;
	}

   if ( number_percent() > gsn_enchant_sword->getEffective( ch ) )
	{
	 ch->send_to("�� �� ������ ������������������.\n\r");
        act_p( "$c1 ������� �������� $o1, �� �� ��������� ����� ��� ��� ��������.",
            ch, obj, 0, TO_ROOM,POS_RESTING );
	ch->setWait( gsn_enchant_sword->getBeats( ) );
	gsn_enchant_sword->improve( ch, false );
	ch->mana -= mana / 2;
	 return;
	}
    ch->mana -= mana;

    gsn_enchant_weapon->getSpell( )->run(  ch,  obj, gsn_enchant_weapon,  ch->getModifyLevel() );

    gsn_enchant_sword->improve( ch, true );
    ch->setWait( gsn_enchant_sword->getBeats( ) );
    return;
}

/*
 * 'explode' skill command
 */
static void yell_explode( Character *ch, Character *victim )
{
    yell_panic( ch, victim,
                "��������! ���-�� �������� ��������� ����!",
		"��������! %1$^C1 �������� ��������� ����!" );
}


SKILL_RUNP( explode )
{
    Character *vch, *vch_next, *victim;
    int dam, hp_dam, dice_dam, mana;
    int hpch, level;
    char arg[MAX_INPUT_LENGTH];

    victim = ch->fighting;
    dam = 0;
    level = ch->getModifyLevel( );

    if (ch->is_npc() || !gsn_explode->usable( ch ) ) {
	ch->send_to("�����? ��� ���?\n\r");
	return;
    }

    if (victim == 0) {
	one_argument(argument, arg);

	if ( arg == 0 ) {
	    ch->send_to("�� ������� �� ����������� ����������.\n\r");
	    return;
	}

	if ((victim = get_char_room(ch,arg)) == 0) {
	    ch->send_to("����� ��� �����.\n\r");
	    return;
	}
    }

    mana= gsn_explode->getMana( );

    if (ch->mana < mana ) {
	ch->send_to("� ���� �� ������� ������� ��� ����.\n\r");
	return;
    }

    ch->mana -= mana;

    act("$c1 ��������� ���-��.",ch,0,victim,TO_NOTVICT);
    act("$c1 ��������� ���-�� ���������� ��� �����!", ch,0,victim,TO_VICT);
    act("����� ��� ������!",ch,0,0,TO_CHAR);

    ch->setWait( gsn_explode->getBeats( ) );

    if (!ch->is_npc() && number_percent() >= gsn_explode->getEffective( ch )) {
	damage(ch,victim,0,gsn_explode,DAM_FIRE, true, DAMF_WEAPON);
	gsn_explode->improve( ch, false, victim );
	yell_explode( ch, victim );
	return;
    }

    gsn_explode->improve( ch, true, victim );

    hpch = max( 10, (int)ch->hit );
    hp_dam  = number_range( hpch/9+1, hpch/5 );
    dice_dam = dice(level,20);

    if (!(is_safe(ch,victim))) {
	dam = max(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
	fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);
    }

    for (vch = victim->in_room->people; vch != 0; vch = vch_next) {
	vch_next = vch->next_in_room;

	if ( vch->is_mirror() && ( number_percent() < 50 ) ) 
	    continue;

	if (is_safe_spell(ch,vch,true)
	    ||  (vch->is_npc() && ch->is_npc()
	    &&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if ( is_safe(ch, vch) )
	    continue;

	if (vch == victim) /* full damage */ {
	    fire_effect(vch,level,dam,TARGET_CHAR);
	    damage(ch,vch,dam,gsn_explode,DAM_FIRE,true, DAMF_WEAPON);
	}
	else /* partial damage */ {
	    fire_effect(vch,level/2,dam/4,TARGET_CHAR);
	    damage(ch,vch,dam/2,gsn_explode,DAM_FIRE,true, DAMF_WEAPON);
	}
	
	yell_explode( ch, vch);
    }

    if (!ch->is_npc() && number_percent() >= gsn_explode->getEffective( ch )) {	
	fire_effect(ch,level/4,dam/10,TARGET_CHAR);
	damage(ch,ch,(ch->hit / 10),gsn_explode,DAM_FIRE,true, DAMF_WEAPON);
    }
}



/*
 * 'target' skill command
 */

SKILL_RUNP( target )
{
    Character *victim;


    if ( !ch->is_npc() &&   !gsn_target->usable( ch ) )
    {
	ch->send_to("�� �� ������, ��� ����� �������� ����, �������� � ������.\n\r");
	return;
    }

    if (ch->fighting == 0)
    {
        ch->send_to("������ �� �� ����������.\n\r");
        return;
    }

    if (argument[0] == '\0')
    {
        ch->send_to("�������� ����? �� ���?\n\r");
        return;
    }

    if (( victim = get_char_room (ch, argument)) == 0 )
    {
        ch->send_to("�� �� ������ ����� �����.\n\r");
        return;
    }


    /* check victim is fighting with him */

    if ( victim->fighting != ch)
    {
	act("�� $E �� ��������� � �����.", ch, 0, victim, TO_CHAR);
        return;
    }


  ch->setWait( gsn_target->getBeats( ) );

    if (victim == ch->fighting) {
	act("�� � ��� � $Y ����������.", ch, 0, victim, TO_CHAR);
	return;
    }

  if (!ch->is_npc() && number_percent() <
	(gsn_target->getEffective( ch ) / 2) )
    {
      gsn_target->improve( ch, false, victim );

    ch->fighting = victim;

    act_p("$c1 ������ $s ���� �� $C4!",ch,0,victim,TO_NOTVICT,POS_RESTING);
    act_p("�� ������� ���� ���� �� $C4!",ch,0,victim,TO_CHAR,POS_RESTING);
    act_p("$c1 ������ ���� ���� �� ����!",ch,0,victim,TO_VICT,POS_RESTING);
      return;
    }

ch->send_to("�� ���������, �� �� ������. �������� ��� ���!.\n\r");
      gsn_target->improve( ch, false, victim );

    return;
}


/*
 * 'harakiri' skill command
 */

SKILL_RUNP( harakiri )
{
    int chance;
    Affect  af;

    if ( MOUNTED(ch) )
    {
        ch->send_to("�� �� ������ ������� ��������, ���� �� ������!\n\r");
        return;
    }

    if ( (chance = gsn_hara_kiri->getEffective( ch )) == 0)
    {
	ch->send_to("�� ��������� ����� ����, �� �� ������ ������� ����� ����.\n\r");
	return;
    }

    if (ch->isAffected(gsn_hara_kiri))
    {
	act("���� �� ����$g��|�|�� ��������� � ����� - �������� ����� �������.", ch, 0, 0, TO_CHAR);
	return;
    }

    /* fighting */
    if (ch->position == POS_FIGHTING || ch->fighting)
    {
	ch->send_to("��������� ���� ���� ��������� �� �����.\n\r");
	return;
    }

    if(SHADOW(ch)) {
      ch->send_to("�� ���������� ������ ���� ����.\n\r");
      act_p("$c1 �� ����� ���� ������� ���� ��������.\n...���� �� ������.",
             ch, 0, 0, TO_ROOM,POS_RESTING);
      return;
    }

    if (number_percent() < chance)
    {
	Affect af;

	ch->setWaitViolence( 1 );

        ch->hit = 1;
        ch->mana = 1;
	ch->move = 1;
	
	desire_hunger->reset( ch->getPC( ) );
	desire_thirst->reset( ch->getPC( ) );

	ch->send_to("�� ��������� ���� ����� � �����, ����� ������� ��� �����.\n\r");
	act_p("$c1 ��������� ���� ���� � ���� ������.", ch,0,0,TO_ROOM,POS_FIGHTING);
	gsn_hara_kiri->improve( ch, true );
	interpret_raw( ch, "sleep" );
	SET_BIT(ch->act,PLR_HARA_KIRI);

               af.where     = TO_AFFECTS;
               af.type      = gsn_hara_kiri;
               af.level     = ch->getModifyLevel();
               af.duration  = 10;
               af.location  = APPLY_NONE;
               af.modifier  = 0;
               af.bitvector = 0;
               affect_to_char( ch, &af );
    }

    else
    {
	ch->setWaitViolence( 2 );

               af.where     = TO_AFFECTS;
               af.type      = gsn_hara_kiri;
               af.level     = ch->getModifyLevel();
               af.duration  = 0;
               af.location  = APPLY_NONE;
               af.modifier  = 0;
               af.bitvector = 0;
               affect_to_char( ch, &af );

	ch->send_to("�� �� ������ �������� ���� �����. ���� ��� �� ��� �����!.\n\r");
	gsn_hara_kiri->improve( ch, false );
    }
}


/*
 * 'katana' skill command
 */

SKILL_RUNP( katana )
{
	Object *katana;
	Affect af;
	Object *part;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int mana;

	one_argument( argument, arg );

	if ( ch->is_npc() || !gsn_katana->usable( ch ) )
	{
		ch->send_to("���?\n\r");
		return;
	}

	if ( ch->isAffected(gsn_katana) )
	{
		ch->send_to("�� � ���� ��� ���� ������!\n\r");
		return;
	}
	
	mana = gsn_katana->getMana( );

	if ( ch->mana < mana )
	{
		ch->send_to("� ���� �� ������� ������� ��� ������.\n\r");
		return;
	}

	if ( arg[0] == '\0' )
	{
		ch->send_to("������� ������? �� ����?\n\r");
		return;
	}

	if ( ( part = get_obj_carry( ch, arg ) ) == 0 )
	{
		ch->send_to("� ���� ���� �� ����� ������.\n\r");
		return;
	}

	if ( part->pIndexData->vnum != OBJ_VNUM_CHUNK_IRON )
	{
		ch->send_to("� ���� ��� ������� ���������.\n\r");
		return;
	}

	if (SHADOW(ch))
	{
		ch->send_to("���� ���� ��� ����� �������� ����� �������.\n\r���������� ������� ������ � ����� ��������.\n\r");
		act_p("$c1 ������ �� ����� ����� �������� ������� ������.\n\r...������ �������.",
			ch, 0, 0, TO_ROOM,POS_RESTING);
		return;
	}

	if ( number_percent( ) > ( gsn_katana->getEffective( ch ) / 3 ) * 2 )
	{
		ch->send_to("������� �� �������, � �� ���������� ���.\n\r");
		extract_obj(part);
		return;
	}

	ch->setWait( gsn_katana->getBeats( ) );

	if ( !ch->is_npc() && number_percent() < gsn_katana->getEffective( ch ) )
	{
		af.where  = TO_AFFECTS;
		af.type	= gsn_katana;
		af.level	= ch->getModifyLevel();
		af.duration	= ch->getModifyLevel();
		af.modifier	= 0;
		af.bitvector 	= 0;
		af.location	= 0;
		affect_to_char(ch,&af);

		katana = create_object( get_obj_index( OBJ_VNUM_KATANA_SWORD), ch->getModifyLevel() );
		katana->cost  = 0;
		katana->level = ch->getRealLevel( );
		ch->mana -= mana;

		af.where	= TO_OBJECT;
		af.type 	= gsn_katana;
		af.level	= ch->getModifyLevel();
		af.duration	= -1;
		af.location	= APPLY_DAMROLL;
		af.modifier	= ch->getModifyLevel() / 10;
		af.bitvector	= 0;
		affect_to_obj( katana, &af );

		af.location	= APPLY_HITROLL;
		affect_to_obj( katana, &af );

		if (ch->getModifyLevel() < 70 )
			katana->value[2] = 10;
		else
		{
			katana->value[2] = 10 + (ch->getModifyLevel()-70)/7;
		}
		sprintf( buf,katana->pIndexData->extra_descr->description,ch->getNameP( ) );
		katana->extra_descr = new_extra_descr();
		katana->extra_descr->keyword =str_dup(katana->pIndexData->extra_descr->keyword );
		katana->extra_descr->description = str_dup( buf );
		katana->extra_descr->next = 0;
	
		obj_to_char(katana, ch);
		gsn_katana->improve( ch, true );
	
		act_p("�� ������� ������ �� $o2!",ch,part,0,TO_CHAR,POS_RESTING);
		act_p("$c1 ������ ������ �� $o2!",ch,part,0,TO_ROOM,POS_RESTING);
	
		extract_obj(part);
		return;
	}
	else
	{
		ch->send_to("�� ���������� ���.\n\r");
		extract_obj(part);
		ch->mana -= mana / 2;
		gsn_katana->improve( ch, false );
	}
}



/*---------------------------------------------------------------------------
 * SamuraiGuildmaster
 *--------------------------------------------------------------------------*/
void SamuraiGuildmaster::give( Character *victim, Object *obj ) 
{
    if (obj->pIndexData->vnum != OBJ_VNUM_KATANA_SWORD) {
	say_act( victim, ch, "� �� �������� ��������, $c1." );
	giveBack( victim, obj );
	return;
    }
    
    if (victim->getTrueProfession( ) != prof_samurai) {
	say_act( victim, ch, "�� �� ������������ � ������ ��������, �� �������$g���|��|��� ���� ���������� �����!" );
	giveBack( victim, obj );
	return;
    }
    
    if (!obj->extra_descr 
	|| !obj->extra_descr->description
	|| !strstr(obj->extra_descr->description, victim->getNameP( )))
    {
	say_act( victim, ch, "������� �� ���� ������ ������� � ���, ��� ��� ���� ����������� ���-�� ������, � �� �����, $c1." );
	giveBack( victim, obj );
	return;
    }

    if (!IS_WEAPON_STAT(obj, WEAPON_KATANA)) {
	if (checkPrice( victim, 100 ))
	    doFirstEnchant( victim, obj );

	giveBack( victim, obj );
	return;
    }
    
    if (!obj->behavior || obj->behavior->getType( ) != OwnedKatana::MOC_TYPE) {
	if (checkPrice( victim, 300 ))
	    doOwner( victim, obj );

	giveBack( victim, obj );
	return;
    }

    giveBack( victim, obj );
}

void SamuraiGuildmaster::tell( Character *victim, const char *speech ) 
{
    if (victim->is_npc( ))
	return;

    if (victim->getProfession( ) != prof_samurai)
	return;

    if (!is_name( "death", speech ) && !is_name( "������", speech ))
	return;
    
    if (victim->getPC( )->death < 1) {
	say_act( victim, ch, "������ ��� �� ���� �� ��������� ����, $c1." );
	return;
    }

    if (!checkPrice( victim, 50 ))
	return;

    victim->getPC( )->death -= 1;

    act( "$C1 �������� ������ � $c5.", victim, 0, ch, TO_ROOM );
    act( "$C1 �������� � ���� ������.", victim, 0, ch, TO_CHAR );
    act_p( "{B������ �������� �� ����.{x", victim, 0, ch, TO_ALL, POS_SLEEPING );
}

void SamuraiGuildmaster::giveBack( Character *victim, Object *obj )
{
    act( "$c1 ���������� $o4 $C3.", ch, obj, victim, TO_NOTVICT );
    act( "$c1 ���������� ���� $o4.", ch, obj, victim, TO_VICT );

    obj_from_char( obj );
    obj_to_char( obj, victim );
}

bool SamuraiGuildmaster::checkPrice( Character *victim, int qp )
{
    if (victim->is_npc( ))
	return false;
    
    if (victim->getPC( )->questpoints < qp) {
	say_act( victim, ch, "� ���� ��� ������������ ����� ��� �����." );
	return false;
    }

    victim->getPC( )->questpoints -= qp;
    return true;
}

void SamuraiGuildmaster::doFirstEnchant( Character *victim, Object *katana )
{
    Affect af;

    af.where	= TO_WEAPON;
    af.type	= gsn_none;
    af.level	= 100;
    af.duration	= -1;
    af.modifier	= 0;
    af.bitvector= WEAPON_KATANA;
    af.location	= APPLY_NONE;
    affect_to_obj( katana, &af );
    
    say_act( victim, ch, "��� ������ �� ����������� ����, �� ������������, ��� ���� �� ��������� �������������." );
}

void SamuraiGuildmaster::doOwner( Character *victim, Object *katana )
{
    if (katana->behavior)
	katana->behavior->unsetObj( );
	
    katana->behavior.setPointer( new OwnedKatana );
    katana->behavior->setObj( katana );
    katana->setOwner( victim->getNameP( ) );
    SET_BIT(katana->extra_flags, ITEM_NOSAC|ITEM_NOPURGE);
    SET_BIT(katana->wear_flags, ITEM_NO_SAC);

    say_act( victim, ch, "������ ����� ����� ������ ��������������. " 
                         "������ ����� �� ������� ������������ � ���." );
}

/*
 * Katana
 */
void Katana::wear( Character *ch )
{
  if (IS_WEAPON_STAT(obj,WEAPON_KATANA)
	&& obj->extra_descr
	&& obj->extra_descr->description
	&& strstr( obj->extra_descr->description, ch->getNameP( ) ) != 0)
  {
   ch->send_to("�� �������� ���� ������, ��� ����� ����!\n\r");
  }
}

bool Katana::mayFloat( ) 
{
    return true;
}

/*
 * OwnedKatana
 */
void OwnedKatana::get( Character *ch )
{
  if (ch->is_immortal())
      return;
    
  if (obj->hasOwner( ch ))
  {
    act_p("{B��������� ���� �������� ������ $o2.{x", ch, obj, 0, TO_CHAR, POS_SLEEPING);
    return;
  }

  act( "$o1 �������� �� ����� ���.", ch, obj, 0, TO_CHAR );
  act( "$o1 �������� �� ��� $c2.", ch, obj, 0, TO_ROOM );

  obj_from_char( obj );
  obj_to_room( obj, ch->in_room );
}

bool OwnedKatana::isLevelAdaptive( ) 
{
   return true; 
}

/*
 * SamuraiUniclassAdept
 */
SamuraiUniclassAdept::SamuraiUniclassAdept( )
{
    myclass.setValue( prof_samurai->getName( ) );
}

void SamuraiUniclassAdept::tell( Character *victim, const char *speech )
{
    UniclassAdept::tell( victim, speech );
}

