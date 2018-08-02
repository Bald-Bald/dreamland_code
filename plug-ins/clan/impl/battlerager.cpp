/* $Id: battlerager.cpp,v 1.1.6.10.4.18 2010-09-01 21:20:44 rufina Exp $
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

#include "battlerager.h"
#include "xmlattributerestring.h"

#include "commandtemplate.h"
#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"

#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"
#include "magic.h"
#include "fight.h"
#include "vnum.h"
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "interp.h"
#include "def.h"

using std::max;
using std::min;

Object * bodypart_create( int vnum, Character *ch, Object *corpse );

#define OBJ_VNUM_BATTLE_PONCHO       26

/*
 * poncho behavior
 */
void BattleragerPoncho::wear( Character *ch ) 
{
    Affect af;
    short level = ch->getModifyLevel( );

    if (ch->isAffected(gsn_haste ) || ch->isAffected(gsn_transform )) 
	return;

    af.where = TO_AFFECTS;
    af.type = gsn_haste;
    af.duration = -2;
    af.level = level;
    af.bitvector = AFF_HASTE;
    af.location = APPLY_DEX;
    af.modifier = 1 + ( level >= 18 ) + ( level >= 30 ) + ( level >= 45 );
    affect_to_char(ch, &af);
}

void BattleragerPoncho::remove( Character *ch )
{
    if (ch->isAffected(gsn_haste))
	affect_strip(ch, gsn_haste);
}

PersonalBattleragerPoncho::~PersonalBattleragerPoncho( )
{
}


/*
 * 'chop' skill command
 * 'chop leg|arm|head|����|����|������ <corpse>'
 */
CMDRUNP( chop )
{
    Object *corpse, *axe;
    bitstring_t part;
    DLString args = argument, argPart, argBody;
    int vnum;

    if (!gsn_trophy->available( ch )) {
	ch->println( "���?" );
	return;
    }

    if (!gsn_trophy->usable( ch ))
	return;

    argPart = args.getOneArgument( );
    argBody = args.getOneArgument( );

    if (argPart.empty( )) {
	ch->println( "����� ����� ���� �� ������ �������� - ����, ������ ��� ����?" );
	return;
    }

    if (arg_oneof( argPart, "leg", "����", "����" )) {
	part = PART_LEGS;
	vnum = OBJ_VNUM_SLICED_LEG;
    }
    else if (arg_oneof( argPart, "arm", "����", "����" )) {
	part = PART_ARMS;
	vnum = OBJ_VNUM_SLICED_ARM;
    }
    else if (arg_oneof( argPart, "head", "������", "������" )) {
	part = PART_HEAD;
	vnum = OBJ_VNUM_SEVERED_HEAD;
    }
    else {
	ch->println( "�� ������ �������� ������ ����, ������ ��� ����." );
	return;
    }

    if (argBody.empty( )) {
	ch->println( "�� ������ ����� �� ������ �������� �����?" );
	return;
    }

    if (!( corpse = get_obj_here( ch, argBody.c_str( ) ) )) {
	ch->println( "����� ��� ������ �����." );
	return;
    }

    if (corpse->item_type != ITEM_CORPSE_PC && corpse->item_type != ITEM_CORPSE_NPC) {
	ch->println( "��� �� ����." );
	return;
    }

    if (!IS_SET(corpse->value[2], part)) {
	ch->pecho( "� ����� ����� ���� %s.", 
		   part_flags.messages( part, '2' ).c_str( ) );
	return;
    }
    
    if (!( axe = get_wield( ch, false ) )) {
	ch->println( "��� ������ ������?" );
	return;
    }

    if (axe->value[3] != DAMW_SLASH && axe->value[3] != DAMW_CHOP && axe->value[3] != DAMW_SLICE) {
	ch->println( "����� ������� �������� ��� ������." );
	return;
    }

    REMOVE_BIT(corpse->value[2], part);
    ch->setWait( gsn_trophy->getBeats( ) / 2 );

    DLString what = part_flags.messages( part, '4' );

    if (number_percent( ) > 2 * gsn_trophy->getEffective( ch ) / 3) {
	ch->pecho( "�� ������ ������� ������, ��������� %s ����� � �������� ������.",
		   what.c_str( ) );
	ch->recho( "%^C1 ������ ����� ������� �� %O3.", ch, corpse ); 
	gsn_trophy->improve( ch, false );
	return;
    }

    ch->pecho( "�� ��������� %s �� %O2.", what.c_str( ), corpse );
    ch->recho( "%^C1 �������� %s �� %O2.", ch, what.c_str( ), corpse );
    bodypart_create( vnum, 0, corpse );
    gsn_trophy->improve( ch, true );
}

/*
 * 'trophy' skill command
 */
SKILL_RUNP( trophy )
{
    int trophy_vnum;
    Object *trophy;
    Affect af;
    Object *part;
    char arg[MAX_INPUT_LENGTH];
    short level;
    int mana = gsn_trophy->getMana( );

    argument = one_argument( argument, arg );

    if (!gsn_trophy->available( ch )) {
	ch->println( "���?" );
	return;
    }

    if (!gsn_trophy->usable( ch ))
	return;

    if (ch->isAffected(gsn_trophy))
    {
	ch->println( "�� � ���� ��� ���� ���� ������!" );
	return;
    }

    if (ch->mana < mana)
    {
	ch->println( "�� ������� ����, ���� ������������������." );
	return;
    }

    if (arg[0] == '\0')
    {
	ch->println( "��� ������ �� ������ ���������� � ������?" );
	return;
    }

    if ( ( part = get_obj_carry( ch, arg ) ) == 0 )
    {
	ch->println( "� ���� ���� ����� ����� ����." );
	return;
    }


    switch (part->pIndexData->vnum) {
    case OBJ_VNUM_SLICED_ARM:
    case OBJ_VNUM_SLICED_LEG:
    case OBJ_VNUM_SEVERED_HEAD:
    case OBJ_VNUM_TORN_HEART:
    case OBJ_VNUM_GUTS:
	trophy_vnum = OBJ_VNUM_BATTLE_PONCHO;
    break;
    case OBJ_VNUM_BRAINS:
	ch->println( "� ������ �� ���� ������ �� ������ ��?" );
	return;
    default:
	ch->println( "�� �� ������ ���������� ��� � ������!" );
	return;
    }

    if (part->from[0] == '\0')
    {
	ch->println( "��� �����-�� ������������ ����� ����." );
	return;
    }

    if (part->level < ch->getModifyLevel( ) - 20) {
	ch->println( "��� ����� ���� ������� ���� ��� ������." );
	return;
    }

    if (number_percent( ) > (gsn_trophy->getEffective( ch )/3)*2)
    {
	ch->println( "���� ������� �� �������, � �� ���������� ���." );
	extract_obj(part);
	return;
    }

    ch->setWait( gsn_trophy->getBeats( ) );

    if (!ch->is_npc() && number_percent() < gsn_trophy->getEffective( ch ))
    {
	af.where  = TO_AFFECTS;
	af.type	= gsn_trophy;
	af.level	= ch->getModifyLevel();
	af.duration	= ch->getModifyLevel() / 2;
	af.modifier	= 0;
	af.bitvector 	= 0;

	af.location	= 0;
	affect_to_char(ch,&af);

	if ( trophy_vnum != 0 )
	{
	    level = min(part->level + 5, MAX_LEVEL);

	    trophy = create_object( get_obj_index( trophy_vnum ), level );
	    trophy->timer = ch->getModifyLevel() * 2;
	    trophy->fmtShortDescr( trophy->getShortDescr( ), part->from );
	    trophy->fmtDescription( trophy->getDescription( ), part->from );
	    dress_created_item( gsn_trophy, trophy, ch, argument );

	    trophy->cost  = 0;
	    trophy->level = ch->getRealLevel( );
	    ch->mana     -= mana;
	    af.where	= TO_OBJECT;
	    af.type 	= gsn_trophy;
	    af.level	= level;
	    af.duration	= -1;
	    af.location	= APPLY_DAMROLL;
	    af.modifier   = ch->applyCurse( ch->getModifyLevel( ) / 5 );
	    af.bitvector	= 0;
	    affect_to_obj( trophy, &af );

	    af.location	= APPLY_HITROLL;
	    af.modifier   = ch->applyCurse( ch->getModifyLevel( ) / 5 );
	    af.bitvector	= 0;
	    affect_to_obj( trophy, &af );

	    af.location	= APPLY_INT;
	    af.modifier	= level > 20 ? -2 : -1;
	    affect_to_obj( trophy, &af );

	    af.location	= APPLY_STR;
	    af.modifier	= level > 20 ? 2 : 1;
	    affect_to_obj( trophy, &af );

	    trophy->value[0] = ch->getModifyLevel();
	    trophy->value[1] = ch->getModifyLevel();
	    trophy->value[2] = ch->getModifyLevel();
	    trophy->value[3] = ch->getModifyLevel();


	    obj_to_char(trophy, ch);
	    gsn_trophy->improve( ch, true );

	    act_p("�� �������������� ����� �� $o2!",ch,part,0,TO_CHAR,POS_RESTING);
	    act_p("$c1 ������������� ����� �� $o2!",ch,part,0,TO_ROOM,POS_RESTING);

	    extract_obj(part);
	    return;
	}
    }
    else
    {
	ch->println( "�� ���������� ���." );
	extract_obj(part);
	ch->mana -= mana / 2;
	gsn_trophy->improve( ch, false );
    }
}

/*
 * 'mortal strike' skill command
 */
SKILL_DECL( mortalstrike );
BOOL_SKILL( mortalstrike )::run( Character *ch, Character *victim )
{
    Object *wield;
    int chance;

    if (gsn_mortal_strike->usable( ch, false )
	&& (chance = gsn_mortal_strike->getEffective( ch )) > 1
	&& (wield = get_eq_char(ch,wear_wield)) != 0
	&& wield->level > ( victim->getModifyLevel() - 5 ))
    {
	chance += 1 + chance / 30;
	chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) / 2;
	if ( number_percent( ) < chance )
	{
	    int dam;
	    act_p("{R���� ������������ ���� � ���� ��������� ������ $C4 �����!{x",
		ch,0,victim,TO_CHAR,POS_RESTING);
	    act_p("{R������������ ���� $c2 � ���� ��������� ������ $C4 �����!{x",
		ch,0,victim,TO_NOTVICT,POS_RESTING);
	    act_p("{R������������ ���� $c2 � ���� ��������� ������ ���� �����!{x",
		ch,0,victim,TO_VICT,POS_DEAD);
	    dam = ( victim->hit + 1 ) * ch->getPC( )->curse / 100;
	    damage(ch,victim,(victim->hit + 1),gsn_mortal_strike,DAM_NONE, true);
	    gsn_mortal_strike->improve( ch, true, victim );
	    return true;
	}
	else
	    gsn_mortal_strike->improve( ch, false, victim );
    }

    return false;
}

/*
 * 'bloodthirst' skill command
 */

SKILL_RUNP( bloodthirst )
{
    int chance, hp_percent;

    if (!gsn_bloodthirst->available( ch ))
    {
	ch->println( "�� �� ������ ��� ����� �����." );
	return;
    }

    if (!gsn_bloodthirst->usable( ch ))
      return;

    chance = gsn_bloodthirst->getEffective( ch );

    if (IS_AFFECTED(ch,AFF_BLOODTHIRST) || ch->isAffected(gsn_bloodthirst) )
    {
	ch->println( "�� ��� ����� ������� �����." );
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	ch->println( "�� ������� ���������, ���� ������� �����." );
	return;
    }

    if (ch->fighting == 0)
      {
	ch->println( "��� ����� �� ������ ���������." );
	return;
      }

    /* modifiers */

    hp_percent = ch->applyCurse( HEALTH(ch) );
    chance += ch->applyCurse( 25 - hp_percent / 2 );

    if (number_percent() < chance)
    {
	Affect af;

	ch->setWaitViolence( 1 );


	ch->println( "�� ������� {r�����!{x" );
	act_p("����� $c2 ���������� ����������� �����.",
               ch,0,0,TO_ROOM,POS_RESTING);
	gsn_bloodthirst->improve( ch, true );

        af.where	= TO_AFFECTS;
	af.type		= gsn_bloodthirst;
	af.level	= ch->getModifyLevel();
	af.duration	= 2 + ch->getModifyLevel() / 18;
	af.modifier	= ch->applyCurse( 5 + ch->getModifyLevel( ) / 4 );
	af.bitvector 	= AFF_BLOODTHIRST;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= ch->applyCurse( -min( ch->getModifyLevel( ) - 5, 35 ) );
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	ch->setWaitViolence( 3 );

	ch->println( "�� ��� �� ���������� ���� ����������, �� ��� ������ ��������." );
	gsn_bloodthirst->improve( ch, false );
    }
}


/*
 * 'spellbane' skill command
 */

SKILL_RUNP( spellbane )
{
	Affect af;
	
	if (!gsn_spellbane->usable( ch ))
	    return;

	if (ch->isAffected(gsn_spellbane))
	{
		ch->println( "�� ��� ��������� ����������." );
		return;
	}

	ch->setWait( gsn_spellbane->getBeats( )  );

	af.where	= TO_AFFECTS;
	af.type		= gsn_spellbane;
	af.level	= ch->getModifyLevel();
	af.duration	= ch->getModifyLevel() / 3;
	af.location	= APPLY_SAVING_SPELL;
	af.modifier	= ch->applyCurse( -ch->getModifyLevel( ) / 4 );
	af.bitvector	= 0;

	affect_to_char(ch,&af);

	act_p("��������� � ����� �������� ����.",ch,0,0,TO_CHAR,POS_RESTING);
	act_p("$c1 �������������� ������ ���� ��������� � �����.", ch,0,0,TO_ROOM,POS_RESTING);
}

/*
 * 'resistance' skill command
 */

SKILL_RUNP( resistance )
{
	int mana = gsn_resistance->getMana( );

	//if (!gsn_resistance->available( ch ))
	//	return;

	if (!gsn_resistance->usable( ch ))
		return;

	if (ch->isAffected(gsn_resistance))
	{
		ch->println( "�� ��� �������. ������ ��� ������." );
		return;
	}

	if ( ch->mana < mana )
	{
		ch->println( "� ���� ������������ ������� ��� �����." );
		return;
	}

	ch->setWait( gsn_resistance->getBeats( )  );

	if ((!ch->is_npc() && number_percent() < gsn_resistance->getEffective( ch ))
	  || ch->is_npc() )
    {
      Affect af;

      af.where	= TO_AFFECTS;
      af.type 	= gsn_resistance;
      af.level 	= ch->getModifyLevel();
      af.duration = ch->getModifyLevel() / 6;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = 0;

      affect_to_char(ch,&af);
      ch->mana -= mana;

      act_p("�� ���������� ���� ������!",ch,0,0,TO_CHAR,POS_RESTING);
      act_p("$c1 �������� ��������.",ch,0,0,TO_ROOM,POS_RESTING);
      gsn_resistance->improve( ch, true );
    }
  else
    {
      ch->mana -= mana / 2;

     ch->println( "�� ���������� ���� �������, �� ��� ��� �������." );
      act_p("$c1 ������ ���������, ������� ��������� ������.",
	     ch,0,0,TO_ROOM,POS_RESTING);
      gsn_resistance->improve( ch, false );
    }

}


/*
 * 'truesight' skill command
 */

SKILL_RUNP( truesight )
{
    int mana = gsn_truesight->getMana( );

  if (!gsn_truesight->available( ch ))
  {
    ch->println( "���?" );
    return;
  }

  if (!gsn_truesight->usable( ch ))
    return;

  if (ch->isAffected(gsn_truesight))
    {
      ch->println( "���� ����� ��������� �����, ��������� ��� ��������." );
      return;
    }

  if (ch->mana < mana)
    {
      ch->println( "� ���� �� ������� ������� ��� �����." );
      return;
    }

  ch->setWait( gsn_truesight->getBeats( )  );

  if (!ch->is_npc() && number_percent() < gsn_truesight->getEffective( ch ))
    {
      Affect af;

      af.where  = TO_DETECTS;
      af.type 	= gsn_truesight;
      af.level 	= ch->getModifyLevel();
      af.duration = ch->getModifyLevel() / 2 + 5;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = DETECT_HIDDEN;
      affect_to_char(ch, &af);

      af.bitvector = DETECT_INVIS;
      affect_to_char(ch, &af);

      af.bitvector = DETECT_IMP_INVIS;
      affect_to_char(ch,&af);

// ������ ! ������.
//      af.bitvector = ACUTE_VISION;
//      affect_to_char(ch,&af);

      af.bitvector = DETECT_MAGIC;
      affect_to_char(ch,&af);

      ch->mana -= mana; 

      act_p("�� ����� �������� ������!",ch,0,0,TO_CHAR,POS_RESTING);
      act_p("$c1 ������� ����� �����.",ch,0,0,TO_ROOM,POS_RESTING);
      gsn_truesight->improve( ch, true );
    }
  else
    {
      ch->mana -= mana / 2;

     ch->println( "�� ����� �������� ������, �� �� ������ ������ ������." );
      act_p("$c1 ����� ������� ������, �� ������ ������ �� ��������.",
	     ch,0,0,TO_ROOM,POS_RESTING);
      gsn_truesight->improve( ch, false );
    }

}


/*
 * 'bandage' skill command
 */

SKILL_RUNP( bandage )
{
	int heal;

	if ( !gsn_bandage->usable( ch ) )
		return;

	if ( gsn_bandage->getEffective( ch ) == 0)
	{
		ch->println( "���?" );
		return;
	}

	if ( IS_AFFECTED(ch,AFF_REGENERATION) || ch->isAffected(gsn_bandage) )
	{
		act_p("�� ��� ��������$g��|�|�� ���� ����!",ch,0,0,TO_CHAR,POS_RESTING);
		return;
	}

	if (SHADOW(ch))
	{
		ch->println( "��� ��� �������� ��������� ��������� �� ������� - ��������� ����������� ����." );
		act_p("$c1 �������� ����������� ���� ����������� ����\n\r...������ ����-�� ����� ������.",
			ch, 0, 0, TO_ROOM,POS_RESTING);
		return;
	}

	int skill = ch->is_npc() ? 100 : gsn_bandage->getEffective( ch );
	if ( number_percent() < skill )
	{
		Affect af;

		ch->setWaitViolence( 1 );

		ch->println( "�� ������������ ������� �� ���� ����!" );
		act_p("$c1 ������������ ���� ����.",ch,0,0,TO_ROOM,POS_RESTING);
		gsn_bandage->improve( ch, true );

		heal = ch->applyCurse( dice(4, 8 ) + ch->getModifyLevel() / 2 );
		ch->hit = min( ch->hit + heal, (int)ch->max_hit );
		update_pos( ch );
		ch->println( "���� ���������� �����!" );

		af.where	= TO_AFFECTS;
		af.type		= gsn_bandage;
		af.level	= ch->getModifyLevel();
		af.duration	= ch->getModifyLevel() / 10;
		af.modifier	= ch->applyCurse( min( 15, ch->getModifyLevel( ) / 2 ) );
		af.bitvector 	= AFF_REGENERATION;
		af.location	= 0;
		affect_to_char(ch,&af);
	}
	else
	{
		ch->setWaitViolence( 1 );

		ch->println( "�� ��������� ���������� ���� ����, �� ������ �� ��������� ����." );
		gsn_bandage->improve( ch, false );
	}
}


/*--------------------------------------------------------------------------
 * Wiseman 
 *-------------------------------------------------------------------------*/
ClanHealerBattlerager::ClanHealerBattlerager( ) : healPets( false )
{
}

void ClanHealerBattlerager::speech( Character *wch, const char *speech )
{
    if (!speech[0] || str_cmp( speech, "aid me wiseman" ))
	return;
    
    if ((wch->is_npc( ) && (!wch->master 
		            || wch->master->getClan( ) != clan
			    || !healPets))
	|| (!wch->is_npc( ) && wch->getClan( ) != clan)) 
    {
	do_say(ch, "� �� ���� �������� ����.");
	return;
    }

    if (!IS_AFFECTED(wch,AFF_BLIND) && !IS_AFFECTED(wch,AFF_PLAGUE)
	 && !IS_AFFECTED(wch,AFF_POISON) && !IS_AFFECTED(wch,AFF_CURSE) )
    {
	do_say(ch, "�� �� ���������� � ���� ������.");
	return;
    }

    act_p("$c1 ���� ���� �������� �����, ��������� ������ ���.",
           ch,0,wch,TO_VICT,POS_RESTING);
    act_p("�� �������� �������� �����.",ch,0,wch,TO_VICT,POS_RESTING);
    act_p("�� ��������� �������� ����� $C3.",ch,0,wch,TO_CHAR,POS_RESTING);
    act_p("$C1 ������� �������� �����, ������ �����.",ch,0,wch,TO_CHAR,POS_RESTING);
    act_p("$c1 ���� �������� ����� $C3.",ch,0,wch,TO_NOTVICT,POS_RESTING);
    act_p("$C1 ������� �������� �����, ������� $m ��$g��|�|�� $c1.",ch,0,wch,TO_NOTVICT,POS_RESTING);

    wch->is_npc( ) ? wch->master->setWaitViolence( 1 ) : wch->setWaitViolence( 1 );

    if (IS_AFFECTED(wch,AFF_BLIND))
	::spell( gsn_cure_blindness, ch->getModifyLevel( ), ch, wch );

    if (IS_AFFECTED(wch,AFF_PLAGUE))
	::spell( gsn_cure_disease, ch->getModifyLevel( ), ch, wch );

    if (IS_AFFECTED(wch,AFF_POISON))
	::spell( gsn_cure_poison, ch->getModifyLevel( ), ch, wch );

    if (IS_AFFECTED(wch,AFF_CURSE))
	::spell( gsn_remove_curse, ch->getModifyLevel( ), ch, wch );
}

/*--------------------------------------------------------------------------
 * Powerman 
 *-------------------------------------------------------------------------*/
bool ClanGuardBattlerager::specFight( )
{
    Character *victim;

    if ( !ch->isAffected(gsn_spellbane) )
	    interpret( ch, "spellbane" );

    if (!( victim = getVictim( ) ))
	return true;

    if ( number_percent() < 33 )
    {
	    act("�� �������� ������� ���� ������������ ����!",ch,0,0,TO_CHAR);
	    act("$c1 ������� ������� ���� ������������ ����!",ch,0,0,TO_ROOM);
	    one_hit( ch, victim );
	    one_hit( ch, victim );
	    one_hit( ch, victim );
    }

    if ( !ch->isAffected(gsn_resistance) )
	    interpret( ch, "resistance" );

    if ( ch->hit < (ch->max_hit /3) && !IS_AFFECTED(ch, AFF_REGENERATION) )
	    interpret( ch, "bandage" );

    return true;
}

void ClanGuardBattlerager::actGreet( PCharacter *wch )
{
    do_say(ch, "����� ����������, ������� ����.");
}

void ClanGuardBattlerager::actPush( PCharacter *wch )
{
    act( "$C1 ���������� ���� ������� ������������...", wch, 0, ch, TO_CHAR );
    act( "$C1 ���������� $c3 ������������...\n\r$c1 - ��� ������ �����.", wch, 0, ch, TO_ROOM );
}


