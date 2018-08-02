/* $Id: class_warlock.cpp,v 1.1.2.11.6.16 2009/09/01 22:29:51 rufina Exp $
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
#include "class_warlock.h"

#include "playerattributes.h"

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "desire.h"
#include "object.h"

#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"
#include "magic.h"
#include "fight.h"
#include "vnum.h"
#include "handler.h"
#include "effects.h"
#include "damage_impl.h"
#include "act.h"
#include "merc.h"
#include "interp.h"
#include "def.h"



#define OBJ_VNUM_FIRE_SHIELD	92

/*
 * 'blink' skill command
 */

SKILL_RUNP( blink )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument , arg);

    if (!ch->is_npc() && !gsn_blink->usable( ch ))
    {
	ch->send_to("���?\n\r");
	return;
    }

    if (arg[0] == '\0' )
    {
	ch->printf("�� ����� ��� �� {W%s��������{x.\n\r",
		    IS_SET(ch->act, PLR_BLINK_ON) ? "" : "�� ");
	return;
    }

    if (arg_is_switch_on( arg ))
	{
	    ch->println("�� ������ �������, ��������� �� ����.");
	    SET_BIT(ch->act,PLR_BLINK_ON);
	     return;
	}

    if (arg_is_switch_off( arg ))
	{
	 REMOVE_BIT(ch->act,PLR_BLINK_ON);
	 ch->println("�� ������ �� ������ �������, ��������� �� ����.");
	 return;
	}
    
    ch->println("����� {lR��� ��� ����{lEon ��� off{lx � �������� ���������."); 
}

SPELL_DECL(Disintegrate);
VOID_SPELL(Disintegrate)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	int dam=0;

	if ( victim->fighting )
	{
		ch->send_to("�� �� ������ ���������������.. ������ ������� ������ ��������.\n\r");
		return;
	}

	short chance = 50;

	if ( !victim->is_npc() )
		chance /= 2;

	if ( saves_spell(level,victim,DAM_MENTAL,ch, DAMF_SPELL) )
		chance = 0;

	ch->setWait( skill->getBeats( ) );
	
	if ( !ch->is_immortal()
		&& ( victim->is_immortal()
			|| number_percent() > chance ) )
	{
		dam = dice( level , 24 ) ;
		damage(ch, victim , dam , sn, DAM_MENTAL, true, DAMF_SPELL);
		return;
	}

	act_p("$C1 ����������� �������� ����� {R###��������� ����������###{x ����!",
		victim, 0, ch, TO_CHAR, POS_RESTING);
	act_p("$c1 ����������� �������� ����� {R###��������� ����������###{x $C4!",
		ch, 0, victim, TO_NOTVICT, POS_RESTING);
	act_p("����������� �������� ����� �� {R###��������� �����������###{x $C4!",
		ch, 0, victim, TO_CHAR, POS_RESTING);
	victim->send_to("���� {R�����{x!\n\r");

	act_p("���� ������ �� ����������!\n\r", victim, 0, 0, TO_CHAR,POS_RESTING);
	act_p("$c1 ������ �� ����������!\n\r", victim, 0, 0, TO_ROOM,POS_RESTING);

	victim->send_to("{Y������������ ���� ���������� ���� � �����!{x\n\r");
	
	group_gain( ch, victim );
	raw_kill( victim, -1, ch, FKILL_REABILITATE | FKILL_PURGE | FKILL_MOB_EXTRACT );
	pk_gain( ch, victim );
	
	victim->hit  = 1;
	victim->mana = 1;
}


SPELL_DECL(Scream);
VOID_SPELL(Scream)::run( Character *ch, Room *room, int sn, int level ) 
{ 
	Character *vch, *vch_next;
	int dam=0,hp_dam,dice_dam;
	int hpch;

        if ( ch->isAffected(sn ) )
	{
	    ch->send_to("�� ��������� ��������, �� ������ ���� ���������� �� ������ �����.");
	    act_p("$c1 ������!",ch,0,0,TO_ROOM,POS_RESTING);
	    return;
	}

	act_p("$c1 ������������ ������, �������� ��� ������!",
		ch,0,0,TO_ROOM,POS_RESTING);
	act_p("�� ������������ �������, �������� ��� ������.",
		ch,0,0,TO_CHAR,POS_RESTING);

	hpch = max( 10, (int)ch->hit );
	if ( ch->is_npc() )
		hpch /= 6;
	hp_dam  = number_range( hpch/9+1, hpch/5 );
	dice_dam = dice(level,20);
	dam = max(hp_dam + dice_dam /10 , dice_dam + hp_dam /10);

	scream_effect(room,level,dam/2,TARGET_ROOM, DAMF_SPELL);

	for (vch = room->people; vch != 0; vch = vch_next)
	{
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch,vch,true))
			continue;

		if ( is_safe(ch, vch) )
			continue;

		if ( ch != vch && !saves_spell(level,vch,DAM_SOUND,ch, DAMF_SPELL))
			vch->setWaitViolence( 2 );
			
		if (saves_spell(level,vch,DAM_SOUND,ch, DAMF_SPELL))
			scream_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_SPELL);
		else
			scream_effect(vch,level,dam,TARGET_CHAR, DAMF_SPELL);
	}

}


SPELL_DECL(Shielding);
VOID_SPELL(Shielding)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (saves_spell( level, victim, DAM_OTHER,ch, DAMF_SPELL)) {
	act_p("������ ����� ����������� $C4, �� ��� ������ ��������.",
               ch, 0, victim, TO_CHAR,POS_RESTING );
	victim->send_to("������ ����� ����������� ����, �� ��� ������ ��������.\n\r");
	return;
    }

    if (!victim->isAffected(sn)) {
	af.type    = sn;
	af.level   = level;
	af.duration = level / 20;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = 0;
	affect_to_char(victim, &af );
	if (ch != victim)
	    act_p("�� �������� ����� ���������� ���� ������ $C2.", ch, 0, victim, TO_CHAR,POS_RESTING);
	victim->send_to("���������� ���� ������� ����� ������ ����.\n\r");
    }
    else {
	af.type	= sn;
	af.level    = level;
	af.duration = level / 15;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = 0;
	affect_join( victim, &af );

	victim->send_to("���������� ���� ��������� ��������� ���� �� �������� ����.\n\r");
	if (ch != victim)
	    act_p("���������� ���� ��������� ��������� $C4 �� �������� ����.", ch, 0, victim, TO_CHAR,POS_RESTING);
    }
}

SPELL_DECL(ShockingTrap);
VOID_SPELL(ShockingTrap)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af;

    if ( room->isAffected( sn ))
    {
	ch->send_to("������� ��� ��������� �������� �������.\n\r");
	return;
    }

    if ( ch->isAffected(sn))
    {
	ch->send_to("��� ���������� �������������� ������ �������.\n\r");
	return;
    }

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 40;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_SHOCKING;
    room->affectTo( &af );

    postaffect_to_char( ch, sn, level / 10 );

    ch->send_to("������� ����������� �������� �������, ��������� ����������� ������.\n\r");
    act_p("$c1 ���������� ����������� ������, �������� ������� �������� �������.",
           ch,0,0,TO_ROOM,POS_RESTING);
}

struct ShockingTrapDamage : public SelfDamage {
    ShockingTrapDamage( Character *ch, int dam ) : SelfDamage( ch, DAM_LIGHTNING, dam )
    {
    }
    virtual void message( ) {
	msgRoom( "������� �����, ����������� ���������,\6%C4", ch );
	msgChar( "������� �����, ����������� ���������,\6����", ch );
    }
};
	
AFFECT_DECL(ShockingTrap);
VOID_AFFECT(ShockingTrap)::entry( Room *room, Character *ch, Affect *paf )
{
    if (!is_safe_rspell(paf->level,ch)) {
	try {
	    ShockingTrapDamage( ch, dice(paf->level,4)+12 ).hit( true );
	}
	catch (const VictimDeathException &) {
	}
	room->affectRemove( paf);
    }
}

VOID_AFFECT(ShockingTrap)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "������ ��������� �� ����������� �������, ��� ��������� ��� {W%1$d{x ��%1$I�|��|���.",
		   paf->duration )
	<< endl;
}

SPELL_DECL(WitchCurse);
VOID_SPELL(WitchCurse)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (victim->isAffected(gsn_witch_curse)) {
	ch->println("���� ��������� ��� ������� ��������.");
	return;
    }

    ch->hit -=(2 * level);

    af.where		= TO_AFFECTS;
    af.type             = gsn_witch_curse;
    af.level            = level;
    af.duration         = 24;
    af.location         = APPLY_HIT;
    af.modifier         = - level;
    af.bitvector        = 0;
    affect_to_char(victim,&af);

    act("$C1 ������$G��|�|�� �� ���� ������.", ch, 0, victim, TO_CHAR);
    act("$C1 ������$G��|�|�� �� ���� ������.", ch, 0, victim, TO_NOTVICT);
    act("�� ������$G��|�|�� �� ���� ������.",  ch, 0, victim, TO_VICT);
}

AFFECT_DECL(WitchCurse);
VOID_AFFECT(WitchCurse)::update( Character *ch, Affect *paf ) 
{
    Affect witch;
    
    DefaultAffectHandler::update( ch, paf );

    act_p("��������� ����� ����������� �������� ����� � $c2.",
	  ch,0,0,TO_ROOM,POS_RESTING);
    ch->send_to("��������� ����� ����������� �������� � ���� �����.\n\r");

    if (paf->level <= 1)
	return;

    witch.where = paf->where;
    witch.type  = paf->type;
    witch.level = paf->level;
    witch.duration = paf->duration;
    witch.location = paf->location;
    witch.modifier = paf->modifier * 2;
    witch.bitvector = 0;

    affect_remove(ch, paf);
    affect_to_char( ch ,&witch);
    ch->hit = min(ch->hit,ch->max_hit);

    if (ch->hit < 1) {
	affect_strip(ch,gsn_witch_curse);
	damage_nocatch(ch,ch,20,gsn_witch_curse,DAM_NONE,false);
    }
}


SPELL_DECL(LightningShield);
VOID_SPELL(LightningShield)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af;

    if ( room->isAffected( sn ))
    {
	ch->send_to("��� ������� ��� �������� �����.\n\r");
	return;
    }

    if ( ch->isAffected(sn))
    {
	ch->send_to("��� ���������� �������������� ������ �������.\n\r");
	return;
    }

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 40;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_L_SHIELD;
    room->affectTo( &af );

    postaffect_to_char( ch, sn, level / 10 );

    ch->in_room->owner = str_dup( ch->getNameP( ) );
    ch->send_to("������� ����������� ��������.\n\r");
    act_p("$c1 �������� ���� ��������.",ch,0,0,TO_ROOM,POS_RESTING);
    return;

}

AFFECT_DECL(LightningShield);
VOID_AFFECT(LightningShield)::entry( Room *room, Character *ch, Affect *paf )
{
    Character *vch;

    for (vch=room->people;vch;vch=vch->next_in_room)
	if (room->isOwner(vch)) 
	    break;

    if ( !vch ) {
	bug("Owner of lightning shield left the room.",0);
	free_string(room->owner);
	room->owner = str_dup("");	
	room->affectStrip(paf->type);
    }
    else if (!ch->is_immortal( )) {
	ch->send_to("�������� ��� ������� ��������� ����.\n\r");
	act_p("$C1 ������� � �������.",vch,0,ch,TO_CHAR,POS_RESTING);
	interpret_raw( vch, "wake" );

	if (!is_safe_rspell(paf->level,ch)) {
	    damage( vch,ch,dice(paf->level,4)+12, paf->type,DAM_LIGHTNING, true, DAMF_SPELL);
	    free_string(room->owner);
	    room->owner = str_dup("");	
	    room->affectRemove(paf);
	}
    }
}

VOID_AFFECT(LightningShield)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "����� ���������� ��������� ���, ������� ������������� ��� {W%1$d{x ��%1$I�|��|���.",
		   paf->duration )
	<< endl;
}

VOID_AFFECT(LightningShield)::leave( Room *room, Character *ch, Affect *paf )
{
    if (room->isOwner(ch)) {
	free_string(room->owner);
	room->owner = str_dup("");
	room->affectStrip( paf->type );
    }
}

/*
 * energy shield behavior
 */
bool EnergyShield::isColdShield( ) const
{
  return (obj->extra_descr
		&& obj->extra_descr->description
		&& strstr( obj->extra_descr->description, "cold" ) != 0 );
}
bool EnergyShield::isFireShield( ) const
{
  return (obj->extra_descr
		&& obj->extra_descr->description
		&& strstr( obj->extra_descr->description, "fire" ) != 0 );
}
void EnergyShield::wear( Character *ch )
{
    if (!ch->isAffected(gsn_make_shield)) {
	if (isColdShield( ))
	    ch->send_to("���� ���������������� ������ ����������.\n\r");
	else if (isFireShield( )) 
	    ch->send_to("���� ���������������� ���� ����������.\n\r");
    }
}

void EnergyShield::equip( Character *ch )
{
    Affect af;
    
    if (ch->isAffected(gsn_make_shield))
	return;

    af.where = TO_RESIST;
    af.type = gsn_make_shield;
    af.duration = -2;
    af.level = ch->getModifyLevel();

    if (isColdShield( ))
	af.bitvector = RES_COLD;
    else if (isFireShield( )) 
	af.bitvector = RES_FIRE;
    else
	return;
   
    affect_to_char(ch, &af);
}

void EnergyShield::remove( Character *ch )
{
    if (!ch->isAffected(gsn_make_shield))
	return;

    affect_strip(ch, gsn_make_shield);

    if (isColdShield( ))
	ch->send_to("���� ���������������� ������ ���������� ����.\n\r");
    else  if (isFireShield( ))
	ch->send_to("���� ���������������� ���� ���������� ����.\n\r");
}

/*
 * 'make shield' spell
 */
SPELL_DECL(MakeShield);
VOID_SPELL(MakeShield)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    Object *fire;

    target_name = one_argument( target_name, arg );

    if (!(!str_cmp(arg,"cold") || !str_cmp(arg,"fire"))) {
	ch->send_to("������: �� ���� ��� ������ �� ����� ���� ��������.\n\r");
	return;
    }
	
    fire	= create_object(get_obj_index(OBJ_VNUM_FIRE_SHIELD), 0);
    fire->setOwner(ch->getNameP( ));
    fire->from = str_dup(ch->getNameP( ));
    fire->level = ch->getRealLevel( );
    fire->fmtShortDescr( fire->getShortDescr( ), arg );
    fire->fmtDescription( fire->getDescription( ), arg );

    sprintf( buf, fire->pIndexData->extra_descr->description, arg );
    fire->extra_descr = new_extra_descr();
    fire->extra_descr->keyword =
	      str_dup( fire->pIndexData->extra_descr->keyword );
    fire->extra_descr->description = str_dup( buf );
    fire->extra_descr->next = 0;

    fire->level = ch->getRealLevel( );
    fire->cost = 0;
    fire->timer = 5 * ch->getModifyLevel();

    if (IS_GOOD(ch))
	 SET_BIT(fire->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL));
    else if (IS_NEUTRAL(ch))
	 SET_BIT(fire->extra_flags,(ITEM_ANTI_GOOD | ITEM_ANTI_EVIL));
    else if (IS_EVIL(ch))
	 SET_BIT(fire->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_GOOD));	
	 
    obj_to_char( fire, ch);
    ch->send_to("�� �������� �������������� ���.\n\r");
}

