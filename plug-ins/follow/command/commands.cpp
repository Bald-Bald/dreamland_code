/* $Id: commands.cpp,v 1.1.2.8 2010-09-01 21:20:44 rufina Exp $ 
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

#include "commandtemplate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "clanreference.h"

#include "follow_utils.h"
#include "loadsave.h"
#include "mercdb.h"
#include "act.h"
#include "merc.h"
#include "def.h"

CLAN(battlerager);
Character * follower_find_nosee( Character *ch, const char *cargument );

static bool check_mutual_induct( Character *ch, Character *victim, ClanReference &clan )
{
    if (ch->is_npc( ))
	return true;
    
    bool isClanMember = (ch->getClan( ) == clan);
    bool isNotAllowed = (!isClanMember && !clan->canInduct( ch->getPC( ) ));

    for (Character *gch = char_list; gch; gch = gch->next) {
	if (gch->is_npc( ))
	    continue;

	if (!is_same_group( gch, victim ))
	    continue;

	if (isClanMember && !clan->canInduct( gch->getPC( ) ))
	    return false;

	if (isNotAllowed && gch->getClan( ) == clan)
	    return false;
    }

    return true;
}

CMDRUN( follow )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    Character *victim;
    DLString arg;
    DLString arguments = constArguments;;
    
    arg = arguments.getOneArgument( );

    if (arg.empty( )) {
	ch->send_to( "��������� �� ���?\n\r");
	return;
    }

    if ( ( victim = get_char_room( ch, arg.c_str( ) ) ) == 0 ) {
	ch->send_to( "�� �� �������� ����� �����.\n\r");
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != 0 ) {
	act_p( "�� ���� ������� ��������� �� $C5!", ch, 0, ch->master, TO_CHAR,POS_RESTING );
	return;
    }

    if (victim == ch)
    {
	if ( ch->master == 0 )
	{
	    ch->send_to( "�� ��� �������� �� �����.\n\r");
	    return;
	}
	ch->stop_follower();
	return;
    }
    
    if (!check_mutual_induct( ch, victim, clan_battlerager )) {
	act("�� �� ������� ��������� �� $C5.", ch, 0, victim, TO_CHAR);
	return;
    }

    if( !victim->is_npc() &&
        IS_SET( victim->act, PLR_NOFOLLOW ) &&
        !ch->is_immortal() ) 
    {
	act_p("$C1 �� ������ ������ � ���-����.\n\r",
             ch,0,victim, TO_CHAR,POS_RESTING);
        return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);

    if (ch->master != 0)
	ch->stop_follower();

    ch->add_follower( victim );
}

CMDRUN( group )
{
    Character *victim;
    DLString argument = constArguments;
    DLString arg = argument.getOneArgument( );

    if (arg.empty( ))
    {
	Character *leader;

	leader = (ch->leader != 0) ? ch->leader : ch;
	ch->printf( "������ %s:\n\r", ch->sees(leader,'2').c_str( ) );

	for (Character *gch = char_list; gch != 0; gch = gch->next )
	    if (is_same_group( gch, ch )) {
		if (gch->is_npc( ))
		    ch->pecho( "[%3d    ] %-16.16s{x %5d/%-5d hp %5d/%-5d mana %4d/%-4d mv",
			gch->getRealLevel( ),
			ch->sees( gch, '1' ).c_str( ),
			gch->hit.getValue( ), gch->max_hit.getValue( ), 
			gch->mana.getValue( ), gch->max_mana.getValue( ),
			gch->move.getValue( ), gch->max_move.getValue( ) );
		else
		    ch->pecho( "[%3d %3s] %-16.16s %5d/%-5d hp %5d/%-5d mana %4d/%-4d mv %5d xp",
			gch->getRealLevel( ), 
			gch->getProfession( )->getWhoNameFor( ch ).c_str( ),
			ch->sees( gch, '1' ).c_str( ),
			gch->hit.getValue( ), gch->max_hit.getValue( ), 
			gch->mana.getValue( ), gch->max_mana.getValue( ),
			gch->move.getValue( ), gch->max_move.getValue( ),
			gch->getPC( )->getExpToLevel( ) );
            }

	return;
    }

    if ( ( victim = get_char_room( ch, arg.c_str( ) ) ) == 0 )
    {
	ch->send_to( "����� ��� �����.\n\r");
	return;
    }

    if (victim == ch) {
	ch->println( "� �����?" );
	return;
    }

    if ( ch->master != 0 || ( ch->leader != 0 && ch->leader != ch ) )
    {
	ch->send_to( "�� �� �������� �� ���-�� ���!\n\r");
	return;
    }

    if (victim->master != ch) {
	act_p( "$C1 �� ������� �� �����.", ch, 0, victim, TO_CHAR,POS_RESTING );
	return;
    }

    if (IS_AFFECTED(victim,AFF_CHARM)) {
	ch->send_to("�� �� ������ ��������� ����������� �������� �� ����� ������.\n\r");
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM)) {
	act_p("�� ������ ������ ������� ��� ������, ��� �� ������ �������� $s!",ch,0,victim,TO_VICT,POS_RESTING);
	return;
    }


    if (is_same_group( victim, ch )) {	
	guarding_nuke( ch, victim );

	victim->leader = 0;
	act_p( "$c1 ��������� $C4 �� $s ������.",ch,0,victim,TO_NOTVICT,POS_RESTING);
	act_p( "$c1 ��������� ���� �� $s ������.",ch,0,victim,TO_VICT,POS_SLEEPING);
	act_p( "�� ���������� $C4 �� ����� ������.",ch,0,victim,TO_CHAR,POS_SLEEPING);
	
	guarding_assert( victim );
	return;
    }

    if ( abs(ch->getModifyLevel() - victim->getModifyLevel()) > 8)
    {
	act_p( "$C1 �� ����� �������������� � ������ $c2.",
		ch,0,victim,TO_NOTVICT,POS_RESTING );
	act_p( "�� �� ������ �������������� � ������ $c2.",
		ch,0,victim,TO_VICT,POS_SLEEPING );
	act_p( "$C1 �� ����� �������������� � ����� ������.",
		ch,0,victim,TO_CHAR,POS_SLEEPING );
	return;
    }

    if (IS_GOOD(ch)
	&& IS_EVIL(victim)
	&& ( ch->getClan() != victim->getClan()
	     || ch->getClan( )->isDispersed( ) ))
    {
	act_p("�� ������� ���$G���|���|��� ��� ������ $c2.", ch, 0, victim,
		TO_VICT,POS_SLEEPING);
	act_p("$C1 ������� ���$G���|���|��� ��� ����� ������!", ch, 0, victim,
		TO_CHAR,POS_SLEEPING);
	return;
    }

    if ( IS_GOOD(victim)
	    && IS_EVIL(ch)
	    && ( ch->getClan() != victim->getClan()
		    || ch->getClan()->isDispersed( ) ) )
    {
	act_p("�� ������� ����$G���|���|��� ��� ������ $c2!", ch, 0, victim,
		TO_VICT,POS_SLEEPING);
	act_p("$C1 ������� ����$G���|���|��� ��� ����� ������!", ch, 0, victim,
		TO_CHAR,POS_SLEEPING);
	return;
    }

    if (!victim->is_npc( ) 
	&& (ch->getClan( )->isEnemy( *victim->getClan( ) )
	    || victim->getClan( )->isEnemy( *ch->getClan( ) )))
    {
	act_p("�� �� ���������� ���� $c2, ��� �� ������ �������������� � $s ������?!", ch,
		0, victim,TO_VICT,POS_SLEEPING);
	act_p("�� �� ���������� ���� $C2, ��� �� ������ ���������� $M �������������� � ����� ������?!",
		ch, 0, victim, TO_CHAR,POS_SLEEPING);
	return;
    }

    if (!check_mutual_induct( ch, victim, clan_battlerager )) {
	act_p("�� �� ������� �������� � ������ $C2.", ch, 0, victim, TO_VICT, POS_SLEEPING);
	act("$C1 �� ������ �������� � ���� ������.", ch, 0, victim, TO_CHAR);
	return;
    }

    victim->leader = ch;
    act_p( "$C1 �����������$G���|��|��� � ������ $c2.", ch, 0, victim,TO_NOTVICT, POS_RESTING);
    act_p( "�� �����������$G���|��|��� � ������ $c2.", ch, 0, victim,TO_VICT, POS_SLEEPING);
    act_p( "$C1 �����������$G���|��|��� � ����� ������.", ch, 0, victim, TO_CHAR, POS_SLEEPING);
}


CMDRUN( nuke )
{
    Character *victim;
    DLString argument = constArguments;
    DLString arg = argument.getOneArgument( );

    if (arg.empty( )) {
	ch->println( "��� ���������� �� ����� �� ������ ����������?" );
	return;
    }
    
    if ( !( victim = follower_find_nosee( ch, arg.c_str( ) ) )) {
	ch->println( "����� ����� �������������� ��� ������ � ����� ������." );
	return;
    }

    if (ch == victim) {
	ch->println( "�� ���� �� �������." );
	return;
    }
    
    if (is_same_group( victim, ch )) {
	guarding_nuke( ch, victim );
	victim->leader = 0;
	guarding_assert( victim );
    }
    
    act( "$c1 ��������� $C4 �� ����� $s ��������������.",ch,0,victim,TO_NOTVICT);
    act_p( "$c1 ��������� ���� �� ����� $s ��������������.",ch,0,victim,TO_VICT,POS_SLEEPING);
    act_p( "�� ���������� $C4 �� ����� ����� ��������������.",ch,0,victim,TO_CHAR,POS_SLEEPING);
    victim->stop_follower( );
}


/*
 * 'Split' originally by Gnort, God of Chaos.
 */
CMDRUN( split )
{
    char buf[MAX_STRING_LENGTH];
    DLString arg1, arg2;
    Character *gch;
    int members;
    int amount_gold = 0, amount_silver = 0;
    int share_gold, share_silver;
    int extra_gold, extra_silver;
    DLString argument = constArguments;
    
    arg1 = argument.getOneArgument( );
    arg2 = argument.getOneArgument( );

    if (arg1.empty( ))
    {
	ch->send_to( "���������? �������?\n\r");
	return;
    }

    amount_silver = atoi( arg1.c_str() );

    if (!arg2.empty())
	amount_gold = atoi(arg2.c_str());

    if ( amount_gold < 0 || amount_silver < 0)
    {
	ch->send_to( "����� ������ ��� �� ����������.\n\r");
	return;
    }

    if ( amount_gold == 0 && amount_silver == 0 )
    {
	ch->send_to( "�� �� ���� �� ����� ������, �� ������ �� ���� �� ������.\n\r");
	return;
    }

    if ( ch->gold <  amount_gold || ch->silver < amount_silver)
    {
	ch->send_to( "� ���� ��� �������, ���� ����������.\n\r");
	return;
    }

    members = 0;
    for ( gch = ch->in_room->people; gch != 0; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	ch->send_to( "������ ������� ���� ���.\n\r");
	return;
    }
	
    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    share_gold   = amount_gold / members;
    extra_gold   = amount_gold % members;

    if ( share_gold == 0 && share_silver == 0 )
    {
 	ch->send_to( "����� �����.\n\r");
	return;
    }

    ch->silver	-= amount_silver;
    ch->silver	+= share_silver + extra_silver;
    ch->gold 	-= amount_gold;
    ch->gold 	+= share_gold + extra_gold;

    if (share_silver > 0)
    {
	sprintf(buf,
	    "�� ������ %d ���������%s. �� ��������� %d �������.\n\r",
 	    amount_silver,GET_COUNT(amount_silver,"�� ������","�� ������","�� �����"),
            share_silver + extra_silver);
	ch->send_to(buf);
    }

    if (share_gold > 0)
    {
	sprintf(buf,
	    "�� ������ %d �����%s. �� ��������� %d ������.\n\r",
	     amount_gold,GET_COUNT(amount_silver,"�� ������","�� ������","�� �����"),
             share_gold + extra_gold);
	ch->send_to(buf);
    }

    if (share_gold == 0)
    {
	sprintf(buf,"$c1 ����� %d ���������%s. �� ��������� %d �������.",
		amount_silver,GET_COUNT(amount_silver,"�� ������","�� ������","�� �����"),
                share_silver);
    }
    else if (share_silver == 0)
    {
	sprintf(buf,"$c1 ����� %d �����%s. �� ��������� %d ������.",
		amount_gold,GET_COUNT(amount_silver,"�� ������","�� ������","�� �����"),
                share_gold);
    }
    else
    {
	sprintf(buf,"$c1 ����� %d ������� � %d ������, ���� ���� %d ������� � %d ������.\n\r",
	 amount_silver,amount_gold,share_silver,share_gold);
    }

    for ( gch = ch->in_room->people; gch != 0; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act_p( buf, ch, 0, gch, TO_VICT,POS_RESTING );
	    gch->gold += share_gold;
	    gch->silver += share_silver;
	}
    }

}

CMDRUN( nofollow )
{
    if (ch->is_npc())
	return;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )  {
	ch->send_to( "�� �� ������ �������� ������ ����������.\n\r");
	return;
    }

    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      ch->send_to("������ �� ���������� ��������� �� �����.\n\r");
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
      ch->send_to("������ �� �� ���������� ��������� �� �����.\n\r");
      SET_BIT(ch->act,PLR_NOFOLLOW);
      ch->die_follower( );
    }
}

