/* $Id: group_attack.cpp,v 1.1.2.19.6.12 2010-09-01 21:20:44 rufina Exp $
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

#include "logstream.h"
#include "skillmanager.h"
#include "spelltemplate.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "material.h"
#include "fight.h"
#include "damage.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "vnum.h"
#include "act.h"
#include "def.h"

PROF(cleric);
PROF(paladin);
PROF(anti_paladin);



SPELL_DECL(BladeBarrier);
VOID_SPELL(BladeBarrier)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    try {
	int dam;

	act("��������� ������ ������� ��������� ������ $c2, ������� $C4.", ch,0,victim,TO_NOTVICT);
	act("������ ���� ��������� ��������� ������ �������, ������� $C4.", ch,0,victim,TO_CHAR);
	act("��������� ������ ������� ��������� ������ $c2, ������� ����!", ch,0,victim,TO_VICT);
	dam = dice(level,6);
	if (saves_spell(level,victim,DAM_PIERCE,ch, DAMF_SPELL))
	    dam /= 2;
	damage_nocatch(ch,victim,dam,sn,DAM_PIERCE,true, DAMF_SPELL);
	
	act("������ �� ������ ������� � $c4!",victim,0,0,TO_ROOM);
	act("������ ������ ������� � ����!",victim,0,0,TO_CHAR);
	dam = dice(level,5);
	if (saves_spell(level,victim,DAM_PIERCE,ch, DAMF_SPELL))
	    dam /= 2;
	damage_nocatch(ch,victim,dam,sn,DAM_PIERCE,true, DAMF_SPELL);

	if (number_percent() <= 55)
	    return;
	
	act("������ �� ������ ������� � $c4!",victim,0,0,TO_ROOM);
	act("������ ������ �� ������ ������� � ����!",victim,0,0,TO_CHAR);
	dam = dice(level,7);
	if (saves_spell(level,victim,DAM_PIERCE,ch, DAMF_SPELL))
	    dam /= 2;
	damage_nocatch(ch,victim,dam,sn,DAM_PIERCE,true, DAMF_SPELL);

	if (number_percent() <= 50)
	    return;
		
	act("������ �� ������ ������� � $c4!",victim,0,0,TO_ROOM);
	act("������ ������ ������� � ����!",victim,0,0,TO_CHAR);
	dam = dice(level,6);
	if (saves_spell(level,victim,DAM_PIERCE,ch, DAMF_SPELL))
	    dam /= 3;
	damage_nocatch(ch,victim,dam,sn,DAM_PIERCE,true, DAMF_SPELL);

	if (victim->fighting != 0) {
	    victim->setWaitViolence( number_bits(2) + 1 );
	    victim->position = POS_RESTING;
	}	
    }
    catch (const VictimDeathException& e) {
    }
}

SPELL_DECL(Bluefire);
VOID_SPELL(Bluefire)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	
	int dam;

	if ( !ch->is_npc() && !IS_NEUTRAL(ch) )
	{
		victim = ch;
		ch->send_to("���� {C������� �����{x ������������� ������ ����!\n\r");
	}

	if (victim != ch)
	{
		act_p("$c1 �������� {C������� ����� �����{x ������ $C2!",
			ch,0,victim,TO_NOTVICT,POS_RESTING);
		act_p("$c1 �������� {C������� ����� �����{x ������ ����!",
			ch,0,victim,TO_VICT,POS_RESTING);
		ch->send_to("�� ���������� �� ������ {C������� ����� �����{x!\n\r");
	}

	dam = dice( level, 14 );

	if ( saves_spell( level, victim,DAM_FIRE,ch, DAMF_SPELL) )
		dam /= 2;

	damage( ch, victim, dam, sn, DAM_FIRE ,true, DAMF_SPELL);

}


SPELL_DECL(Demonfire);
VOID_SPELL(Demonfire)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	
	int dam;

	if ( !ch->is_npc() && !IS_EVIL(ch) )
	{
		victim = ch;
		ch->send_to("���� {R������� ���{x ������������� ������ ����!\n\r");
	}

	if (victim != ch)
	{
		act_p("$c1 �������� ���� {R������� ���{x ������ $C2!",
			ch,0,victim,TO_NOTVICT,POS_RESTING);
		act_p("$c1 �������� ���� {R������� ���{x ������ ����!",
			ch,0,victim,TO_VICT,POS_RESTING);
		ch->send_to("�� ���������� �� ������ {R������� ���{x!\n\r");
	}

	dam = dice( level, 14 );

	if ( saves_spell( level, victim,DAM_NEGATIVE,ch, DAMF_SPELL) )
		dam /= 2;
	
	try {
	    damage_nocatch( ch, victim, dam, sn, DAM_NEGATIVE ,true, DAMF_SPELL);
	
	    if (!IS_AFFECTED(victim, AFF_CURSE))
		spell(gsn_curse, 3 * level / 4, ch,  victim);
	}
	catch (const VictimDeathException& e) {
	}

}


SPELL_DECL(DispelEvil);
VOID_SPELL(DispelEvil)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    if ( !ch->is_npc() && IS_EVIL(ch) )
	victim = ch;

    if ( IS_GOOD(victim) )
    {
	act_p( "���� �������� $c4.", victim, 0, 0, TO_ROOM,POS_RESTING);
	act_p( "���� �������� ����.", victim, 0, 0, TO_CHAR,POS_RESTING);
	return;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act_p( "$C1 �� ��������� �����.", ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }

    if (victim->hit > ( ch->getModifyLevel() * 4))
      dam = dice( level, 4 );
    else
      dam = max((int)victim->hit, dice(level,4));
    if ( saves_spell( level, victim,DAM_HOLY, ch, DAMF_SPELL ) )
	dam /= 2;
    if( ch->getTrueProfession( ) == prof_cleric ||
        ch->getTrueProfession( ) == prof_paladin ||
        ch->getTrueProfession( ) == prof_anti_paladin )
      dam *= 2;
    damage( ch, victim, dam, sn, DAM_HOLY ,true, DAMF_SPELL);
    return;

}


SPELL_DECL(DispelGood);
VOID_SPELL(DispelGood)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    if ( !ch->is_npc() && IS_GOOD(ch) )
	victim = ch;

    if ( IS_EVIL(victim) )
    {
	act_p( "$c4 �������� $s ������.", victim, 0, 0, TO_ROOM,POS_RESTING);
	act_p( "���� �������� ���� ������.", victim, 0, 0, TO_CHAR,POS_RESTING);
	return;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act_p( "$C1 �� ��������� �����.", ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }

    if (victim->hit > ( ch->getModifyLevel() * 4))
      dam = dice( level, 4 );
    else
      dam = max((int)victim->hit, dice(level,4));
    if ( saves_spell( level, victim,DAM_NEGATIVE,ch, DAMF_SPELL) )
	dam /= 2;
    if( ch->getTrueProfession( ) == prof_cleric ||
        ch->getTrueProfession( ) == prof_paladin ||
        ch->getTrueProfession( ) == prof_anti_paladin )
      dam *= 2;
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,true, DAMF_SPELL);
    return;

}

SPELL_DECL(Earthquake);
VOID_SPELL(Earthquake)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *vch;
    Character *vch_next;
    int dam;

    ch->send_to("����� ������ ��� ������ ������!\n\r");
    act( "$c1 �������� ������ � �������������.", ch, 0, 0, TO_ROOM);

    for (vch = room->people; vch != 0; vch = vch_next )
    {
	vch_next	= vch->next_in_room;

	if (DIGGED(vch) && vch->was_in_room->area == room->area)
	    if (!is_safe_nomessage( ch, vch ) && number_percent( ) < ch->getSkill( sn ) / 2)
		undig_earthquake( vch );
	
	if (ch == vch)
	    continue;

	if (is_safe_spell(ch,vch,true))
	    continue;

	if (vch->is_mirror() && number_percent() < 50) 
	    continue;

	if (is_flying( vch ))
	    continue;

	dam = level + dice(3, 8);

	switch (room->sector_type) {
	case SECT_MOUNTAIN: dam *= 4; break;
	case SECT_CITY:	    dam *= 3; break;
	case SECT_INSIDE:   dam *= 2; break;
	}

	damage( ch, vch, dam, sn, DAM_BASH, true, DAMF_SPELL );
    }

    area_message( ch, "����� ������ ������ ��� ������ ������.", true );
}



SPELL_DECL(Hellfire);
VOID_SPELL(Hellfire)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  
  int dam;

  dam = dice(level, 7);

  damage(ch,victim,dam,sn,DAM_FIRE, true, DAMF_SPELL);


}


SPELL_DECL(SeverityForce);
VOID_SPELL(SeverityForce)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    act_p("�� ���������� ��������� ����, ������������� ����� � ��� $C2.",
           ch,0,victim,TO_CHAR,POS_RESTING);
    act_p( "$c1 ��������� ��������� ����, ������������� ����� � ����� ���!.",
            ch, 0, victim, TO_VICT,POS_RESTING);

    dam = dice( level , 18 );
    damage(ch,victim,dam,sn,DAM_NONE,true);
}

TYPE_SPELL(int, SeverityForce)::getMaxRange( Character * ) const
{
    return 0;
}

SPELL_DECL(Web);
VOID_SPELL(Web)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	
	Affect af;

	if (saves_spell (level, victim,DAM_OTHER, ch, DAMF_SPELL) )
	{
		ch->send_to("�� ����������..\n\r");
		return;
	}

	if ( victim->isAffected(sn ) )
	{
		if (victim == ch)
			ch->send_to("�� � ��� � �������.\n\r");
		else
			act_p("������ ������� ��� ������� �������� $C2.",
				ch,0,victim,TO_CHAR,POS_RESTING);
		return;
	}

	af.type      = sn;
	af.level     = level;
	af.duration  = 1 + level / 30;
	af.location  = APPLY_HITROLL;
	af.modifier  = -1 * ( level / 6);
	affect_to_char( victim, &af );

	af.location  = APPLY_DAMROLL;
	af.modifier  = -1 * ( level / 6);
	affect_to_char( victim, &af );

	af.location  = APPLY_DEX;
	af.modifier  = -1 - level / 40;
	af.where     = TO_DETECTS;
	af.bitvector = ADET_WEB;
	affect_to_char( victim, &af );

	victim->send_to("������ ������� ��������� ����!\n\r");
	if ( ch != victim )
		act_p("�� ���������� $C4 ������ ��������!",
			ch,0,victim,TO_CHAR,POS_RESTING);
}

SPELL_DECL(HeatMetal);
VOID_SPELL(HeatMetal)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Object *obj_lose, *obj_next;
    int dam = 0;
    bool fail = true;

   if (!saves_spell(level + 2,victim,DAM_FIRE,ch, DAMF_SPELL))
   {
	for ( obj_lose = victim->carrying;
	      obj_lose != 0;
	      obj_lose = obj_next)
	{
	    obj_next = obj_lose->next_content;
	    if ( number_range(1,2 * level) > obj_lose->level
	    &&   !saves_spell(level,victim,DAM_FIRE,ch, DAMF_SPELL)
	    &&   material_is_typed( obj_lose, MAT_METAL )
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF))
	    {
		switch ( obj_lose->item_type )
		{
		case ITEM_ARMOR:
		if (obj_lose->wear_loc != wear_none) /* remove the item */
		{
		    if (can_drop_obj(victim,obj_lose)
		    &&  (obj_lose->weight / 10) <
			number_range(1,2 * victim->getCurrStat(STAT_DEX))
		    && obj_lose->wear_loc->remove( obj_lose, 0 ))
		    {
			act_p("$c1 ������ �� ���� � ������� $o4 �� �����!",
			        victim,obj_lose,0,TO_ROOM,POS_RESTING);
			act_p("�� ������� �� ���� � �������� $o4 �� �����!",
			       victim,obj_lose,0,TO_CHAR,POS_RESTING);
			dam += (number_range(1,obj_lose->level) / 3);
			obj_from_char(obj_lose);
			obj_to_room(obj_lose, victim->in_room);
			fail = false;
		    }
		    else /* stuck on the body! ouch! */
		    {
			act_p("$o1 �������� ���� ����!",
			       victim,obj_lose,0,TO_CHAR,POS_RESTING);
			dam += (number_range(1,obj_lose->level));
			fail = false;
		    }

		}
		else /* drop it if we can */
		{
		    if (can_drop_obj(victim,obj_lose))
		    {
			act_p("$c1 ������ �� ���� � ������� $o4 �� �����!",
			       victim,obj_lose,0,TO_ROOM,POS_RESTING);
			act_p("�� ������� �� ���� � �������� $o4 �� �����!",
			       victim,obj_lose,0,TO_CHAR,POS_RESTING);
			dam += (number_range(1,obj_lose->level) / 6);
			obj_from_char(obj_lose);
			obj_to_room(obj_lose, victim->in_room);
			fail = false;
		    }
		    else /* cannot drop */
		    {
			act_p("$o1 �������� ���� ����!",
			       victim,obj_lose,0,TO_CHAR,POS_RESTING);
			dam += (number_range(1,obj_lose->level) / 2);
			fail = false;
		    }
		}
		break;
		case ITEM_WEAPON:
		if (obj_lose->wear_loc != wear_none) /* try to drop it */
		{
		    if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
			continue;

		    if (can_drop_obj(victim,obj_lose)
		        && obj_lose->wear_loc->remove( obj_lose, 0 ))
		    {
			act_p("$o1 �������� �� ��������� ��� $c2.",
			       victim,obj_lose,0,TO_ROOM,POS_RESTING);
			victim->send_to("������ �������� �� ����� ��������� ���!\n\r");
			dam += 1;
			obj_from_char(obj_lose);
			obj_to_room(obj_lose,victim->in_room);
			fail = false;
		    }
		    else /* YOWCH! */
		    {
			victim->send_to("����������� ������ �������� ���� ����!\n\r");
			dam += number_range(1,obj_lose->level);
			fail = false;
		    }
		}
		else /* drop it if we can */
		{
		    if (can_drop_obj(victim,obj_lose))
		    {
			victim->pecho( "%1$^O1 �����������, � �� �������� %1$P2 �� �����.", obj_lose );
			victim->recho( "%1$^O1 �����������, � %2$C1 ������� %1$P2 �� �����.", obj_lose, victim );
			dam += (number_range(1,obj_lose->level) / 6);
			obj_from_char(obj_lose);
			obj_to_room(obj_lose, victim->in_room);
			fail = false;
		    }
		    else /* cannot drop */
		    {
			act_p("$o1 �������� ����!",
			       victim,obj_lose,0,TO_CHAR,POS_RESTING);
			dam += (number_range(1,obj_lose->level) / 2);
			fail = false;
		    }
		}
		break;
		}
	    }
	}
    }
    if (fail)
    {
	ch->send_to("���� ������� ����������� ��������.\n\r");
	victim->send_to("�� ���������� ������ ������������� �����.\n\r");
    }
    else /* damage! */
    {
	if (saves_spell(level,victim,DAM_FIRE,ch, DAMF_SPELL))
	    dam = 2 * dam / 3;
	damage(ch,victim,dam,sn,DAM_FIRE,true, DAMF_SPELL);
    }

}

SPELL_DECL(Holycross);
VOID_SPELL(Holycross)::run( Character *ch, Object *grave, int sn, int level ) 
{ 
    int dam;
    PCMemoryInterface *pcm;
    PCharacter *victim;

    if ((ch->getTrueProfession( ) != prof_cleric && ch->getTrueProfession( ) != prof_paladin)
	|| IS_EVIL(ch)) 
    {
	ch->send_to("�� �� �������� ���� �����.\r\n");
	return;
    }
    
    if (grave->pIndexData->vnum != OBJ_VNUM_GRAVE) {
	ch->send_to("���� �� ���������, ��� �� ������.\r\n");
	return;
    }

    pcm = PCharacterManager::find( DLString( grave->getOwner( )));

    if (!pcm || (victim = dynamic_cast<PCharacter *>( pcm )) == 0 || !DIGGED(victim)) {
	ch->send_to("���.. � ������-�� ��������..\r\n");
	LogStream::sendError( ) << "Unexistent grave owner: " << grave->getOwner( )<< endl;
	return;
    }

    if (number_percent( ) > ch->getSkill( sn )) {
	act_p("$c1 ������� � ������ �����, �� �� ������ �� ���.", ch, 0, 0, TO_ROOM, POS_RESTING);
	act_p("�� �������� � ������ �����, �� �� ������ �� ���.", ch, 0, 0, TO_CHAR, POS_RESTING);
	return;
    }
    
    act_p("$c1 ������� � ������ ��������� �����!", ch, 0, 0, TO_ROOM, POS_RESTING);
    act_p("�� �������� � ������ ��������� �����!", ch, 0, 0, TO_CHAR, POS_RESTING);
    act_p("��-��� ����� ��������� ����������� ���� �����!", ch, 0, 0, TO_ALL, POS_RESTING);
    
    undig( victim );
    dam = dice(level, 20);
    damage(ch, victim, dam, sn, DAM_HOLY, true, DAMF_SPELL); 

}


