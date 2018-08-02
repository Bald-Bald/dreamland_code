/* $Id: invader.cpp,v 1.1.6.7.6.20 2010-09-01 21:20:44 rufina Exp $
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

#include "invader.h"
#include "clanorg.h"

#include "summoncreaturespell.h"
#include "affecthandlertemplate.h"
#include "spelltemplate.h"                                                 
#include "skillcommandtemplate.h"
#include "skill.h"
#include "skillmanager.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "act.h"
#include "gsn_plugin.h"
#include "merc.h"
#include "mercdb.h"
#include "fight.h"
#include "handler.h"
#include "vnum.h"
#include "clanreference.h"
#include "magic.h"
#include "def.h"

GSN(dispel_affects);
GSN(soul_lust);
GSN(shadow_shroud);
CLAN(invader);

/*--------------------------------------------------------------------------
 * Neere 
 *-------------------------------------------------------------------------*/
void ClanGuardInvader::actGreet( PCharacter *wch )
{
    do_say(ch, "����������� ����, ������ ������ �����.");
}
void ClanGuardInvader::actPush( PCharacter *wch )
{
    act( "$C1 ������� ���������� ����...\n\r�� ������� �������� �� ������ � ����-�� ��������.", wch, 0, ch, TO_CHAR );
    act( "$C1 ������� ������� $c4 � $c1 � ������ ����-�� ��������.", wch, 0, ch, TO_ROOM );
}
int ClanGuardInvader::getCast( Character *victim )
{
	int sn = -1;

	switch ( dice(1,16) )
	{
	case  0:
	case  1:
		sn = gsn_blindness;
		break;
	case  2:
	case  3:
		if (!victim->isAffected( gsn_spellbane ))
		    sn = gsn_dispel_affects;
		break;
	case  4:
	case  5:
		sn = gsn_weaken;
		break;
	case  6: 
	case  7:
		sn = gsn_energy_drain;
		break;
	case  8: 
	case  9:
		sn = gsn_plague;
		break;
	case 10:
	case 11:
		sn = gsn_acid_arrow;
		break;
	case 12:
	case 13:
	case 14:
		sn = gsn_acid_blast;
		break;
	case 15:
		if ( ch->hit < (ch->max_hit / 3) )
			sn = gsn_shadow_cloak;
		else
			sn = -1;
		break;
	default:
		sn = -1;
		break;
	}

	return sn;
}

/*
 * 'fade' skill command
 */

SKILL_RUNP( fade )
{
	if ( !gsn_fade->available( ch ))
	{
	    ch->send_to("���?\n\r");
	    return;
	}

	if ( MOUNTED(ch) )
	{
		ch->send_to("�� �� ������ ���������, ����� � �����.\n\r");
		return;
	}

	if ( RIDDEN(ch) )
	{
		ch->send_to("�� �� ������ ���������, ����� �� �������.\n\r");
		return;
	}

	if ( IS_AFFECTED( ch, AFF_FAERIE_FIRE) )
	{
		ch->send_to("�� �� ������ ��������, ����� ���������.\n\r");
		return;
	}

	if ( !gsn_fade->usable( ch ) )
		return;

	ch->send_to("�� ��������� ���������.\n\r");

	int k = ch->getLastFightDelay( );

	if ( k >= 0 && k < FIGHT_DELAY_TIME )
		k = k * 100 /	FIGHT_DELAY_TIME;
	else
		k = 100;

	if ( number_percent() < gsn_fade->getEffective( ch ) * k / 100 )
	{
		SET_BIT(ch->affected_by, AFF_FADE);
		gsn_fade->improve( ch, true );
	}
	else
		gsn_fade->improve( ch, false );

	return;
}



SPELL_DECL(EvilSpirit);
VOID_SPELL(EvilSpirit)::run( Character *ch, Room *room, int sn, int level ) 
{ 
 AREA_DATA *pArea = room->area;
 Affect af,af2;

 if (IS_RAFFECTED(room, AFF_ROOM_ESPIRIT)
	|| room->isAffected( sn) )
  {
   ch->send_to("��� ���� ��������� ��� ��������� ���� �����.\n\r");
   return;
  }

 if ( ch->isAffected(sn ) )
    {
      ch->send_to("� ���� ������������ �������.\n\r");
      return;
    }

  if (IS_SET(room->room_flags, ROOM_LAW)
	|| IS_SET(room->area->area_flag,AREA_HOMETOWN) )
    {
      ch->send_to("������ ���� � ���� ������� �� ���� ������� ���.\n\r");
      return;
    }

    af2.where     = TO_AFFECTS;
    af2.type      = sn;
    af2.level	  = ch->getModifyLevel();
    af2.duration  = level / 5;
    af2.modifier  = 0;
    af2.location  = APPLY_NONE;
    af2.bitvector = 0;
    affect_to_char( ch, &af2 );

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 25;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_ESPIRIT;

    for (map<int, Room *>::iterator i = pArea->rooms.begin( ); i != pArea->rooms.end( ); i++)
    {
	room = i->second;
	room->affectTo( &af );
	act("������� ������������ ��� ��������� � ���� ���.", room->people,0,0,TO_ALL);
    }
}

SPELL_DECL(EyesOfIntrigue);
VOID_SPELL(EyesOfIntrigue)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
	Character *victim;

	if (( victim = get_char_world_doppel( ch, target_name ) ) == 0 || DIGGED(victim))
	{
		ch->send_to("���� �� ����� ���������� ������ ���������.\n\r");
		return;
	}

	if(is_safe_nomessage(ch,victim) 
	    || (victim->is_npc( ) && IS_SET(victim->act, ACT_NOEYE)))
	{
		ch->send_to("������, ���� ��� ���������.\n\r");
		return;
	}

	if( victim->isAffected(gsn_golden_aura ) )
	{
		if(saves_spell(level, victim, DAM_OTHER, ch, DAMF_SPELL))
		{
			ch->send_to("� ���� �� ������� ���� ��������� �������.\n\r");
			return;
		}

		victim->pecho( "�� ��� ���� �������� �������, �� ������� �� ���� ������� �������� ����.\n\r"
			       "...� ����� ���� {W����{x ������� ��� - {D%#^C1{x.", ch );
	}
	
	do_look_auto( ch, victim->in_room );
}




SPELL_DECL(Nightfall);
VOID_SPELL(Nightfall)::run( Character *ch, Room *room, int sn, int level ) 
{ 
  Character *vch;
  Object  *light;
  Affect af;

  if( ch->isAffected(sn ) ) {
    ch->send_to("� ���� �� ���������� ������� ��� �������� ��� ������.\n\r");
    return;
  }

	if (IS_SET(room->room_flags,ROOM_SAFE))
	{
		ch->send_to("��� ������ ���� ����� ������, ��� ���.\n\r");
		return;
	}

  for( vch = room->people; vch != 0; vch = vch->next_in_room )
    for( light = vch->carrying; light != 0; light = light->next_content )
      if( light->item_type == ITEM_LIGHT && !is_same_group( ch, vch ) ) {
        damage_to_obj( vch, light, light, light->condition / 2 + number_range( 1, light->condition ) );
	  }

  for( light = room->contents;light != 0; light=light->next_content )
    if (light->item_type == ITEM_LIGHT ) {
      damage_to_obj( vch, light, light, light->condition / 2 + number_range( 1, light->condition ) );
    }

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 2;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char( ch, &af );

}

SPELL_DECL_T(Nightwalker, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, Nightwalker)::createMobile( Character *ch, int level ) const 
{
    return createMobileAux( ch, ch->getModifyLevel( ), 
	                 (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit), 
			 ch->max_mana,
			 number_range(level/15, level/10),
			 number_range(level/3, level/2),
			 0 );
}


SPELL_DECL(ShadowCloak);
VOID_SPELL(ShadowCloak)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;
    DLString msgChar, msgVict, orgCh, orgVict;
    
    if (ch->is_npc( ) || victim->is_npc( ) || ch->getClan( ) != victim->getClan( )) {
	ch->println("��� ���������� �� ������ ���������� ������ �� ����� ������ �����.");
	return;
    }
    
    orgCh = ClanOrgs::getAttr( ch->getPC( ) );
    orgVict = ClanOrgs::getAttr( victim->getPC( ) );

    if (orgCh != orgVict) {
	ch->println("��� ���������� �� ������ ���������� ������ �� ����� ����� �����������.");
	return;
    }

    if (victim->isAffected( gsn_soul_lust )) {
	ch->pecho( ch == victim ? 
			  "����� ��� ��� ����� � ����." :
			  "����� ��� ��� ����� � %C6.", victim );
	return;
    }

    if (victim->isAffected( sn ) || victim->isAffected( gsn_shadow_shroud )) {
	ch->pecho( ch == victim ? 
			  "���������� ������ ��� �������� ����." : 
			  "���������� ������ ��� �������� %C4.", victim );
	return;
    }

    if (orgCh == "killers") {
	msgVict = "���������� ������ ��������� ����. �� ������������ �� ����.";
	msgChar = "%2$C1 ����������� �����.";
	sn = gsn_shadow_shroud;
    }
    else if (orgVict == "adepts") {
	msgVict = "� ���� ���������� �����, �������� ��� �������.";
	msgChar = "� %2$C6 ���������� �����, �������� ��� �������.";
	sn = gsn_soul_lust;
    }
    else {
	msgVict = "���������� ������ ��������� ����.";
	msgChar = "���������� ������ ��������� %2$C4.";
    }

    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;

    af.where     = TO_DETECTS;
    af.bitvector = DETECT_GOOD | DETECT_FADE;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = ch->applyCurse( 0 - level / 9 );
    affect_to_char( victim, &af );

    af.where     = TO_AFFECTS;
    af.bitvector = IS_AFFECTED(victim, AFF_PROTECT_GOOD) ? 0 : AFF_PROTECT_GOOD;
    af.modifier  = ch->applyCurse( -level * 5 / 2 );
    af.location  = APPLY_AC;
    affect_to_char( victim, &af );
    
    victim->pecho( msgVict.c_str( ), ch, victim );
    if (ch != victim)
	ch->pecho( msgChar.c_str( ), ch, victim );
}




SPELL_DECL(Shadowlife);
VOID_SPELL(Shadowlife)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	Affect af;

	if (victim->is_npc())
	{
		ch->send_to("����������� ����� ��� � �������...\n\r");
		return;
	}

	if (ch->isAffected(sn))
	{
		ch->send_to("� ���� ������������ �������, ����� ������� ����.\n\r");
		return;
	}

	if ( victim->isAffected(gsn_golden_aura )
		&& saves_spell(level, victim, DAM_OTHER, ch, DAMF_SPELL) )
	{
		ch->send_to("���� �������� ������� ��� ��� ����, �� �� �����.\n\r");
		victim->send_to("����� ���� ���������� ���� ����, �� ��� �� ��������.\n\r");
		return;
	}

  act_p("�� ����� ����� ���� $C2!",ch, 0, victim, TO_CHAR,POS_RESTING);
  act_p("$c1 ���� ����� ���� $C2!",ch,0,victim,TO_NOTVICT,POS_RESTING);
  act_p("$c1 ���� ����� ����� ����!", ch, 0, victim, TO_VICT,POS_RESTING);

  victim->getPC()->shadow	= ch->getModifyLevel() / 10;

    postaffect_to_char( ch, sn, 24 );
}


AFFECT_DECL(EvilSpirit);
VOID_AFFECT(EvilSpirit)::update( Character *ch, Affect *paf ) 
{
    DefaultAffectHandler::update( ch, paf );

    if (ch->getClan( ) == clan_invader)
	return;
    
    Spell::Pointer spell = gsn_mental_knife->getSpell( );

    if (spell)
	spell->run( ch, ch, gsn_mental_attack, ch->getModifyLevel( ) );
}

VOID_AFFECT(EvilSpirit)::update( Room *room, Affect *paf ) 
{
    Affect af;
    Character *vch;

    af.where	= TO_AFFECTS;
    af.type 	= gsn_evil_spirit;
    af.level 	= paf->level;
    af.duration	= number_range(1,(af.level/30));
    af.location	= APPLY_NONE;
    af.modifier	= 0;
    af.bitvector= 0;

    for ( vch = room->people; vch; vch = vch->next_in_room )
    {
	if ( !saves_spell(vch->getModifyLevel() + 2, vch, DAM_MENTAL, 0, DAMF_SPELL)
		&& !vch->is_immortal()
		&& !is_safe_rspell(vch->getModifyLevel() + 2, vch)
		&& !vch->isAffected(gsn_evil_spirit) && number_bits(3) == 0 )
	{
	    vch->send_to("���� ���� ���������� �����.\n\r");
	    act_p("���� ���� ���������� $c1.",vch,0,0,TO_ROOM,POS_RESTING);
	    affect_join(vch,&af);
	}
    }
}

VOID_AFFECT(EvilSpirit)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "���� ���� ���������� ����� �� {W%1$d{x ��%1$I�|��|���.", paf->duration )
	<< endl;
}



/*-----------------------------------------------------------------
 * 'darkleague' command 
 *----------------------------------------------------------------*/
COMMAND(CDarkLeague, "darkleague")
{
    PCharacter *pch;
    const ClanOrgs *orgs;
    DLString arguments, cmd, arg;

    if (ch->is_npc( ))
	return;

    pch = ch->getPC( );
    
    if (pch->getClan( ) != clan_invader) {
	pch->println( "�� �� ������������ � ����� �����������." );
	return;
    }
    
    if (!( orgs = clan_invader->getOrgs( ) )) {
	pch->println( "�������� �����." );
	return;
    }

    if (!pch->getClan( )->isRecruiter( pch )) {
	pch->println( "����� ���������� ������������." );
	return;
    }

    arguments = constArguments;
    cmd = arguments.getOneArgument( );
    arg = arguments.getOneArgument( );
    
    if (cmd.empty( )) {
	doUsage( pch );
    }
    else if (arg_is_list( cmd )) {
	orgs->doList( pch );
    }
    else if (arg_oneof( cmd, "induct", "�������" )) {
	if (arg_is_self( arg ))
	    orgs->doSelfInduct( pch, arguments );
	else
	    orgs->doInduct( pch, arg );
    }
    else if (arg_oneof( cmd, "remove", "�������", "����" )) {
	if (arg_is_self( arg ))
	    orgs->doSelfRemove( pch );
	else
	    orgs->doRemove( pch, arg );
    }
    else if (arg_oneof( cmd, "members", "�����" )) {
	orgs->doMembers( pch );
    }
    else {
	doUsage( pch );
    }
}

void CDarkLeague::doUsage( PCharacter *pch )
{
    ostringstream buf;

    buf << "��� �����������: " << endl
        << "{wdarkleague list{x            - ���������� ������ �����" << endl
	<< "{wdarkleague members{x         - ���������� ������ ������ ������" << endl
	<< "{wdarkleague remove self{x     - ����� �� ������" << endl
	<< "{wdarkleague induct <{Dname{w>{x - ������� ����-�� � ������" << endl
	<< "{wdarkleague remove <{Dname{w>{x - ������� ����-�� �� ������" << endl
	<< endl
	<< "��� ������: " << endl
	<< "{wdarkleague induct self <{Dname{w>{x - ������� ���� � ������" << endl;

    pch->send_to( buf );
}

