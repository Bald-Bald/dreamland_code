
/* $Id: group_necromancy.cpp,v 1.1.2.16.4.7 2009/11/05 03:18:39 rufina Exp $
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

#include "group_necromancy.h"
#include "spelltemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "mercdb.h"
#include "magic.h"
#include "fight.h"
#include "interp.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "vnum.h"
#include "def.h"

#define MOB_VNUM_UNDEAD               18

CLAN(none);
PROF(necromancer);

NecroCreature::~NecroCreature( )
{
}

void AdamantiteGolem::fight( Character *victim )
{
    Character *master, *rch;
    int mirrors;

    BasicMobileDestiny::fight( victim );	
    
    master = ch->master;

    if (!master
	|| !master->fighting
	|| master->is_npc( )
	|| master->getTrueProfession( ) != prof_necromancer)
	return;
    
    if (master->fighting->fighting != master)
	return;

    if (ch->fighting == master)
	return;
    
    if (!ch->can_see( master ))
	return;

    if (is_safe( ch, master->fighting ) || is_safe( ch, master ))
	return;

    for (mirrors = 0, rch = ch->in_room->people; rch; rch = rch->next_in_room)
	if (rch->is_mirror( ) && rch->doppel == master)
	    mirrors++;
    
    if (number_percent( ) > 100 - mirrors * 5)
	return;

    ch->setWait( gsn_rescue->getBeats( )  );
    
    act( "�� �������� $C4!",  ch, 0, master, TO_CHAR );
    act( "$c1 ������� ����!", ch, 0, master, TO_VICT );
    act( "$c1 ������� $C4!",  ch, 0, master, TO_NOTVICT );

    stop_fighting( master->fighting, false );
    set_fighting( ch, master->fighting);
    set_fighting( master->fighting, ch );
}

SPELL_DECL_T(AdamantiteGolem, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, AdamantiteGolem)::createMobile( Character *ch, int level ) const 
{
    NPCharacter *mob = createMobileAux( ch, ch->getModifyLevel( ), 
	                             10 * (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit) + 4000,
				     (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
				     13, 9, ch->getModifyLevel( ) / 2 + 10 );
    
    for (int i = 0; i < stat_table.size; i ++)
	mob->perm_stat[i] = min( 25, 15 + ch->getModifyLevel( ) / 10 );

    mob->perm_stat[STAT_STR] += 3;
    mob->perm_stat[STAT_INT] -= 1;
    mob->perm_stat[STAT_CON] += 2;
    return mob;
}


SPELL_DECL_T(IronGolem, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, IronGolem)::createMobile( Character *ch, int level ) const 
{
    NPCharacter *mob = createMobileAux( ch, ch->getModifyLevel( ), 
	                             10 * (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit) + 1000,
				     (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
				     11, 5, ch->getModifyLevel( ) / 2 + 10 );
    
    for (int i = 0; i < stat_table.size; i ++)
	mob->perm_stat[i] = min( 25, 15 + ch->getModifyLevel( ) / 10 );

    mob->perm_stat[STAT_STR] += 3;
    mob->perm_stat[STAT_INT] -= 1;
    mob->perm_stat[STAT_CON] += 2;
    return mob;
}


SPELL_DECL_T(LesserGolem, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, LesserGolem)::createMobile( Character *ch, int level ) const 
{
    NPCharacter *mob = createMobileAux( ch, ch->getModifyLevel( ), 
	                             2 * (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit) + 400,
				     (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
				     3, 10, ch->getModifyLevel( ) / 2 );
    
    for (int i = 0; i < stat_table.size; i ++)
	mob->perm_stat[i] = min( 25, 15 + ch->getModifyLevel( ) / 10 );

    mob->perm_stat[STAT_STR] += 3;
    mob->perm_stat[STAT_INT] -= 1;
    mob->perm_stat[STAT_CON] += 2;
    return mob; 
}

SPELL_DECL_T(StoneGolem, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, StoneGolem)::createMobile( Character *ch, int level ) const 
{
    NPCharacter *mob = createMobileAux( ch, ch->getModifyLevel( ), 
	                             5 * (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit) + 2000,
				     (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
				     8, 4, ch->getModifyLevel( ) / 2 );
    
    for (int i = 0; i < stat_table.size; i ++)
	mob->perm_stat[i] = min( 25, 15 + ch->getModifyLevel( ) / 10 );

    mob->perm_stat[STAT_STR] += 3;
    mob->perm_stat[STAT_INT] -= 1;
    mob->perm_stat[STAT_CON] += 2;
    return mob;
}


SPELL_DECL_T(SummonShadow, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, SummonShadow)::createMobile( Character *ch, int level ) const 
{
    return createMobileAux( ch, ch->getModifyLevel( ), 
	                 ch->max_hit, ch->max_mana,
			 number_range(level/15, level/10),
			 number_range(level/3, level/2),
			 number_range(level/8, level/6) );
}

SPELL_DECL(AnimateDead);
VOID_SPELL(AnimateDead)::run( Character *ch, Object *obj, int sn, int level ) 
{
	NPCharacter *undead;
	Object *obj2,*next;
	MOB_INDEX_DATA *pCorpseOwner = 0;
	char buf[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char arg[MAX_STRING_LENGTH];
	char *argument;
	int i;

	if ( !(obj->item_type == ITEM_CORPSE_NPC
			|| obj->item_type == ITEM_CORPSE_PC))
	{
		ch->send_to("�� ������ ���������� ������ ����!!!\n\r");
		return;
	}

	if ( !ch->is_immortal() && obj->item_type == ITEM_CORPSE_PC )
	{
		ch->send_to("��������� ������!\n\r");
		return;
	}

	if ( ch->isAffected(sn ) )
	{
		ch->send_to("����� ������������ ������� ����� ����������� �����������.\n\r");
		return;
	}

	if ( overcharmed( ch ) )
		return;

	if ( ch->in_room && IS_SET( ch->in_room->room_flags, ROOM_NO_MOB ) )
	{
		ch->send_to("����� ���������� ������� ����.\n\r");
		return;
	}

	if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE )
		|| IS_SET(ch->in_room->room_flags, ROOM_PRIVATE )
		|| IS_SET(ch->in_room->room_flags, ROOM_SOLITARY ) )
	{
		ch->send_to("�������� ����� ����� �� ��������� ���� ������� �����.\n\r");
		return;
	}
	undead = create_mobile( get_mob_index(MOB_VNUM_UNDEAD) );

	for ( i=0; i < stat_table.size; i++ )
	{
		undead->perm_stat[i] = min(25,2 * ch->perm_stat[i]);
	}

	undead->max_hit = ch->is_npc() ? ch->max_hit : ch->getPC( )->perm_hit;
	undead->hit = undead->max_hit;
	undead->max_mana = ch->is_npc() ? ch->max_mana : ch->getPC( )->perm_mana;
	undead->mana = undead->max_mana;
	undead->alignment = ch->alignment;
	undead->setLevel( min(100, ( ch->getModifyLevel() - 2 ) ) );

	for ( i=0; i < 3; i++ )
		undead->armor[i] = interpolate(undead->getRealLevel( ),100,-100);
	undead->armor[3] = interpolate(undead->getRealLevel( ),50,-200);
	undead->gold = 0;

	SET_BIT(undead->act, ACT_UNDEAD);
	SET_BIT(undead->affected_by, AFF_CHARM);
	SET_BIT(undead->form, FORM_INSTANT_DECAY);
	undead->timer = (undead->getRealLevel( ) / 10 + 1) * 60 * 24; // 1 day per 10lev 
	undead->master = ch;
	undead->leader = ch;
	
	if (obj->value[3]) 
	    pCorpseOwner = get_mob_index( obj->value[3] );
	
	if (pCorpseOwner && pCorpseOwner->sex != SEX_EITHER) 
	    undead->setSex( pCorpseOwner->sex );
	else
	    undead->setSex( ch->getSex( ) );

	sprintf(buf, undead->getName().c_str(), obj->getName( ));
	undead->setName( buf );
	
	strcpy(buf, obj->getShortDescr( '1' ).c_str( ));
	argument = buf;
	buf3[0] = '\0';
	while (argument[0] != '\0' )
	{
		argument = one_argument(argument, arg);
		if (!( !str_cmp(arg,"����������")
				|| !str_cmp(arg,"(corpse)")
				|| !str_cmp(arg,"����") ))
		{
			if (buf3[0] == '\0')
				strcat(buf3,arg);
			else
			{
				strcat(buf3," ");
				strcat(buf3,arg);
			}
		}
	}
	sprintf(buf, undead->getShortDescr( ), buf3);
	undead->setShortDescr( buf );
	sprintf(buf, undead->getLongDescr( ), buf3);
	undead->setLongDescr( buf );

	char_to_room(undead,ch->in_room);

	for ( obj2 = obj->contains;obj2;obj2=next )
	{
		next = obj2->next_content;
		obj_from_obj(obj2);
		obj_to_char(obj2, undead);
	}
	interpret_raw( undead,"wear", "all" );

	postaffect_to_char( ch, sn, ch->getModifyLevel() / 10 );

	ch->send_to("��������� ����������� ���� �� ����������� ����!\n\r");

	sprintf(buf,"��������� ����������� ���� $c1 ���������� %s!",obj->getShortDescr( '4' ).c_str( ));
	act_p(buf,ch,0,0,TO_ROOM,POS_RESTING);

	act_p("$C1 ������� �� ���� ������������� ��������,\n\r��������� ����� ��������!",ch,0,undead,TO_CHAR,POS_RESTING);
	extract_obj (obj);
}

VOID_SPELL(AnimateDead)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	if ( victim == ch )
	    ch->send_to("����� ��������� � �� �� ����������� ������������ �� ����� �����!\n\r");
	else 
	    act_p("����� ��������� � $C1 �� ���������� ������������ �� ����� �����!",ch,0,victim,TO_CHAR,POS_RESTING);

}

/*
void NecroCreature::canEnter( Room *const room )
{
    if (!Wanderer::canEnter( room ))
	return false;

    if (IS_SET(room->room_flags, ROOM_NO_MOB))
	return false;
    
    if (room->clan != clan_none)
	return false;

    return true;
}

bool NecroCreature::startMoving( )
{
    pathToTarget( ch->in_room, 
	          get_room_index( masterRoomVnum ), 
		  ch->master->getModifyLevel( ) * 100 );
    
    makeOneStep( );

    if (ch->in_room == old_room) {
	path.clear( );
	masterRoomVnum = 0;
	masterID = 0;
	return false;
    }
    else 
	return true;
}

void NecroCreature::entry( )
{
    if (ch->in_room->vnum == masterRoomVnum) {
	masterRoomVnum = 0;
	masterID = 0;
    }

    SummonedCreature::entry( );
}

bool NecroCreature::specIdle( ) 
{
    if (SummonedCreature::specIdle( ))
	return true;
    
    if (!IS_AFFECTED(ch, AFF_CHARM)
	|| !ch->master
	|| ch->master->getID( ) != masterID
	|| !masterRoomVnum)
	return false;

    if (path.empty( ) && !startMoving( )) 
	return false;
    
    makeOneStep( );
    return true;
}


VOID_SPELL(CallCorpseRenameMePlease)::run( Character *ch, char *, int sn, int level )
{
    int countTotal = 0, countMoved = 0;

    if (ch->isAffected( sn )) {
	ch->println( "�� ������� ��������������� �� ���������� ������� �������." );
	return;
    }
    
    if (IS_SET(ch->in_room->room_flag, ROOM_SOLITARY|ROOM_PRIVATE|ROOM_NO_MOB)
	|| ch->in_room->clan != clan_none)
    {
	ch->println("���� ������ �� ������ ��������� �����.");
	return;
    }
    
    for (Character *wch = char_list; wch; wch = wch->next)
	if (wch->is_npc( )
	     && IS_AFFECTED(wch, AFF_CHARM)
	     && wch->master == ch
	     && wch->in_room != ch->in_room
	     && wch->behavior)
	{
	    NecroCreature::Pointer bhv;
	    
	    if (( bhv = wch->behavior.getDynamicPointer<NecroCreature>( ) )) {
		bhv->masterID = ch->getID( );
		bhv->masterRoomVnum = ch->in_room->vnum;
		countMoved += (bhv->startMoving( ));
		countTotal++;
	    }
	}

    if (countTotal == 0) {
	ch->println("� ���� ��� ������, ����������� ����.");
	return;
    }

    if (countMoved == 0) {
	ch->println("�� ���� �� ����� ������� ��� ����� �� ������ ��������� �� ����.");
	return;
    }
    
    ch->println("�� �������� ������������ ���� ����������� ���� ������ ��������� � ���� �����.");

    postaffect_to_char( ch, sn, level / 10 );
}
*/
