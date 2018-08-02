/* $Id: group_beguiling.cpp,v 1.1.2.15.6.11 2009/08/16 02:50:31 rufina Exp $
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
 
#include "group_beguiling.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "merc.h"
#include "mercdb.h"
#include "occupations.h"
#include "handler.h"
#include "act.h"
#include "vnum.h"
#include "def.h"


PROF(none);

#define OBJ_VNUM_MAGIC_JAR		93


SPELL_DECL(AttractOther);
VOID_SPELL(AttractOther)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    if  ( ch->getSex( ) == victim->getSex( ) )
    {
	ch->send_to("�������� ����� ��������� ����� ����������� ����!\n\r");
	return;
    }
    spell(gsn_charm_person, level,ch,victim);
    return;
}

SPELL_DECL(CharmPerson);
VOID_SPELL(CharmPerson)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	Affect af;

	if (is_safe(ch,victim) || overcharmed(ch) )
	{
		return;
	}

	if ( victim == ch )
	{
		ch->send_to("�� ��������� ���� ��� ������!\n\r");
		return;
	}

	if ( !IS_AWAKE(victim) || !victim->can_see(ch) )
	{
		ch->send_to("���� ������ �� ����� ����.\n\r");
		return;		
	}
	
	if ( (number_percent() > 50) && !victim->is_npc() )	
	{
		ch->pecho("������, �� �� ���%1$G��|��|�� �������������%1$G��|��|��, ��� ���� �������.", ch );
		return;
	}
	

	if ( IS_AFFECTED(victim, AFF_CHARM)
		|| IS_AFFECTED(ch, AFF_CHARM)
		|| ( ch->getSex( ) == SEX_MALE &&  level < victim->getModifyLevel() )
		|| ( ch->getSex( ) == SEX_FEMALE &&  level < ( victim->getModifyLevel()  ) )
		|| IS_SET(victim->imm_flags,IMM_CHARM)
		|| saves_spell( level, victim,DAM_CHARM, ch, DAMF_SPELL )
		|| (victim->is_npc( ) 
		     && victim->getNPC( )->behavior 
		     && IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_SHOPPER)))
		|| victim->is_immortal() )
	{
		ch->send_to("�� ����������...\n\r");
		return;
	}

	if ( victim->master )
		victim->stop_follower( );

	victim->add_follower( ch );

	victim->leader = ch;

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level	 = ch->getRealLevel( );
	af.duration  = number_fuzzy( level / 5 );
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = AFF_CHARM;
	affect_to_char( victim, &af );

	act_p( "$c1 ����������� ����!!!", ch, 0, victim, TO_VICT,POS_RESTING);

	if ( ch != victim )
		act_p("$C1 � ��������� ������� �� ����.",ch,0,victim,TO_CHAR,POS_RESTING);

}

AFFECT_DECL(CharmPerson);
VOID_AFFECT(CharmPerson)::remove( Character *victim ) 
{
    DefaultAffectHandler::remove( victim );
    
    victim->stop_follower( );

    if(victim->is_npc() 
	&& victim->position == POS_SLEEPING
	&& !IS_AFFECTED(victim, AFF_SLEEP))
	victim->position = victim->getNPC()->default_pos;
}
    


SPELL_DECL(ControlUndead);
VOID_SPELL(ControlUndead)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	

	if  ( !victim->is_npc() || !IS_SET(victim->act,ACT_UNDEAD) )
	{
		act_p("$C1 �� ����$g��|�|�� �� ������ ��������.",ch,0,victim,TO_CHAR,POS_RESTING);
		return;
	}
	spell(gsn_charm_person,level,ch,victim);
	return;

}



SPELL_DECL(LovePotion);
VOID_SPELL(LovePotion)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 50;
  affect_join(ch, &af);

  ch->send_to("���� ��� � ����� ��������� �� ����-������.\n\r");

}

AFFECT_DECL(LovePotion);
VOID_AFFECT(LovePotion)::look( Character *ch, Character *victim, Affect *paf ) 
{
    Affect af;

    DefaultAffectHandler::look( ch, victim, paf );
    
    if (ch == victim || ch->is_immortal( ))
	return;

    affect_strip( ch, paf->type );

    if (ch->master)
	ch->stop_follower( );

    ch->add_follower( victim );
    ch->leader = victim;

    af.where     = TO_AFFECTS;
    af.type      = gsn_charm_person;
    af.level     = ch->getModifyLevel( );
    af.duration  = number_fuzzy( victim->getModifyLevel( ) / 4);
    af.bitvector = AFF_CHARM;
    affect_to_char(ch, &af);

    act("������� $c1 �������� ��� �������������?", victim, 0, ch, TO_VICT);
    act("$C1 ������� �� ���� � �����������.", victim, 0, ch, TO_CHAR);
    act("$C1 ������� �� $c4 � �����������.", victim, 0, ch, TO_NOTVICT);
}

/*
 * magic jar behavior
 */
void MagicJar::get( Character *ch )
{
    if (!ch->is_npc( ) && strstr(obj->getName( ), ch->getNameP( )) != 0) {
	act("��� ��� �����!",ch,obj,0,TO_CHAR);
	extract_obj(obj);
    }
    else
	act("�� ��������%g��|�|�� ������� ����.",ch,obj,0,TO_CHAR);
} 

bool MagicJar::extract( bool fCount )
{
    Character *wch;

    for (wch = char_list; wch != 0 ; wch = wch->next)
    {
	if (wch->is_npc()) 
	    continue;

	if (strstr(obj->getName( ),wch->getNameP( )) != 0)
	{
	    if (IS_SET( wch->act, PLR_NO_EXP )) {
		REMOVE_BIT(wch->act,PLR_NO_EXP);
		wch->send_to("���� ���� ������������ � ����.\n\r");
	    }

	    break;
	}
    }

    return ObjectBehavior::extract( fCount );
}

bool MagicJar::quit( Character *ch, bool count )
{
    extract_obj( obj );
    return true;
}

bool MagicJar::area( )
{
    Character *carrier = obj->carried_by;
    
    if (!carrier
	|| carrier->is_npc( )
	|| IS_SET(carrier->in_room->room_flags, 
	          ROOM_SAFE|ROOM_SOLITARY|ROOM_PRIVATE)
	|| !carrier->in_room->guilds.empty( ))
    {
	extract_obj( obj );
	return true;
    }
    else
	return false;
}
    
SPELL_DECL(MagicJar);
VOID_SPELL(MagicJar)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Object *vial;
    Object *jar;
    char buf[MAX_STRING_LENGTH];

    if (victim == ch)
	{
	ch->send_to("���� ���� ������ � �����!\n\r");
	return;
	}

    if (victim->is_npc())
	{
	ch->send_to("���� ����� ���������� ������������ ����!.\n\r");
	return;
	}

    if ( IS_SET( victim->act, PLR_NO_EXP ) )
    {
	ch->send_to("���� ������ ���������� ���-�� ������...\n\r");
	return;
    }


    if (saves_spell(level ,victim,DAM_MENTAL, ch, DAMF_SPELL))
       {
        ch->send_to("���� ������� ����������� ��������.\n\r");
        return;
       }

    for( vial=ch->carrying; vial != 0; vial=vial->next_content )
	if ( vial->pIndexData->vnum == OBJ_VNUM_POTION_VIAL )
	    break;

    if (  vial == 0 )  {
	ch->send_to("� ���� ��� ������� ������, ���� �������� � ���� ��� ����������.\n\r");
	return;
    }
    
    extract_obj(vial);

    jar	= create_object(get_obj_index(OBJ_VNUM_MAGIC_JAR), 0);
    jar->setOwner(ch->getNameP( ));
    jar->from = str_dup(ch->getNameP( ));
    jar->level = ch->getRealLevel( );

    jar->fmtName( jar->getName( ), victim->getNameP( ));
    jar->fmtShortDescr( jar->getShortDescr( ), victim->getNameP( ));
    jar->fmtDescription( jar->getDescription( ), victim->getNameP( ));

    sprintf( buf,jar->pIndexData->extra_descr->description, victim->getNameP( ) );
    jar->extra_descr = new_extra_descr();
    jar->extra_descr->keyword = str_dup( jar->pIndexData->extra_descr->keyword );
    jar->extra_descr->description = str_dup( buf );
    jar->extra_descr->next = 0;

    jar->level = ch->getRealLevel( );
    jar->cost = 0;
    obj_to_char( jar , ch );

    SET_BIT(victim->act,PLR_NO_EXP);
    act("��� $C2 ������ ������� � ������ � ��������� � ����� ������.", ch, 0, victim, TO_CHAR);
    act("$c1 {R������� ���� ��� � ������.{x", ch, 0, victim, TO_VICT);
}

SPELL_DECL(MysteriousDream);
VOID_SPELL(MysteriousDream)::run( Character *ch, Room *room, int sn, int level ) 
{ 
  Affect af;

  if (IS_SET(room->room_flags, ROOM_LAW))
    {
      ch->send_to("������������ ���� ����������������� ����� �����.\n\r");
      return;
    }
    if ( room->isAffected( sn ))
    {
     ch->send_to("��� ������� ��� ��������� ���������� �����.\n\r");
     return;
    }

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 15;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_SLEEP;
    room->affectTo( &af );

    ch->send_to("������� ������������ � ����� ������ ����� ��� ���.\n\r");
    act_p("������� ������������ � ����� ���������� ����� ��� ������ ������.\n\r",
           ch,0,0,TO_ROOM,POS_RESTING);


}

AFFECT_DECL(MysteriousDream);
VOID_AFFECT(MysteriousDream)::entry( Room *room, Character *ch, Affect *paf )
{
     act_p("{y� ������� �������� �����-�� �����.{x",ch, 0, 0, TO_CHAR, POS_SLEEPING);
}

VOID_AFFECT(MysteriousDream)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "������ �����, ���������� � �������, ��������� ����� {W%1$d{x ��%1$I�|��|���.",
		   paf->duration )
	<< endl;
}

VOID_AFFECT(MysteriousDream)::update( Room *room, Affect *paf )
{
    Affect af;
    Character *vch;

    af.where	= TO_AFFECTS;
    af.type 	= gsn_sleep;
    af.level 	= paf->level - 1;
    af.duration	= number_range(1,((af.level/10)+1));
    af.location	= APPLY_NONE;
    af.modifier	= -5;
    af.bitvector= AFF_SLEEP;

    for (vch = room->people; vch != 0; vch = vch->next_in_room) {
	if ( !saves_spell(af.level - 4,vch,DAM_CHARM, 0, DAMF_SPELL)
		&& !is_safe_rspell(paf->level,vch)
		&& !(vch->is_npc() && IS_SET(vch->act,ACT_UNDEAD) )
		&& !IS_AFFECTED(vch,AFF_SLEEP) && number_bits(3) == 0 )
	{
	    if ( IS_AWAKE(vch) )
	    {
		vch->send_to("�� ���������...\n\r");
		act_p("$c1 ��������.",vch,0,0,TO_ROOM,POS_RESTING);
		vch->position = POS_SLEEPING;
	    }

	    affect_join(vch,&af);
	}
    }
}


SPELL_DECL(Sleep);
VOID_SPELL(Sleep)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (victim->is_npc() && IS_SET(victim->act,ACT_UNDEAD))
    ||   level < victim->getModifyLevel()
    ||   saves_spell( level-4, victim,DAM_CHARM, ch, DAMF_SPELL ) )
    {
	ch->println("�� ����������...");
    	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1 + level/10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
        act("�� ���������� ���� ����� ����$g��|��|��.... �� ���������..", victim, 0, 0, TO_CHAR);
	act("$c1 ��������.", victim, 0, 0, TO_ROOM);
	victim->position = POS_SLEEPING;
    }
    return;

}

AFFECT_DECL(Sleep);
VOID_AFFECT(Sleep)::remove( Character *victim ) 
{
    DefaultAffectHandler::remove( victim );

    if(victim->is_npc() && victim->position == POS_SLEEPING)
	victim->position = victim->getNPC()->default_pos;
}


SPELL_DECL(Terangreal);
VOID_SPELL(Terangreal)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (victim->is_npc())
	return;

    af.where		= TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
	victim->send_to("��������� ����� ���������� ���������� �� ����.\n\r");
	act_p( "$c1 ���������� �������� ����.",
                victim, 0, 0, TO_ROOM,POS_RESTING);
	victim->position = POS_SLEEPING;
    }

    return;

}

