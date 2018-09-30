/* $Id: socialbase.cpp,v 1.1.2.3.6.8 2009/08/16 02:50:31 rufina Exp $
 * 
 * ruffina, 2004
 */
/* 
 *
 * sturm, 2003
 */

#include "socialbase.h"

#include "russianstring.h"
#include "skillreference.h"
#include "character.h"
#include "room.h"

#include "merc.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"

GSN(improved_invis);

SocialBase::SocialBase( ) 
{
}

SocialBase::~SocialBase( )
{
}

short SocialBase::getLog( ) const
{
    return LOG_NORMAL;
}

bool SocialBase::matches( const DLString& argument ) const
{
    if (argument.empty( )) 
	return false;

    if (argument.strPrefix( getName( ) )) 
	return true;
    
    if (argument.strPrefix( getRussianName( ) )) 
	return true;

    return false;
}

bool SocialBase::properOrder( Character * )
{
    return true;
}

bool SocialBase::dispatchOrder( const InterpretArguments &iargs )
{
    return dispatch( iargs );
}

bool SocialBase::dispatch( const InterpretArguments &iargs )
{
    Character *ch = iargs.ch;

    if (!ch->is_npc( )) {
	if (IS_SET(ch->act, PLR_FREEZE)) {
	    ch->pecho("�� ��������� ��������%G��|�|��!", ch);
	    return false;
	}

	if (IS_SET( ch->comm, COMM_NOEMOTE )) {
	    ch->pecho("�� ����-������%G���|��|���!", ch);
	    return false;
	}

	if (IS_SET( ch->comm, COMM_AFK )) {
	    ch->send_to( "����� ������� �� {WAFK{x\n\r" );
	    return false;
	}
    }
    
    if (!checkPosition( ch )) 
	return false;
    
    visualize( ch );
    return true;
}

static const void *victimOrSelf(Character *ch, Character *victim)
{
    static RussianString self("�||���||���||���||���||����||���");
    if (ch == victim)
        return &self;
    else
        return victim;
}

void SocialBase::run( Character *ch, const DLString &constArguments )
{
    Character *victim, *victim2;
    int pos;
    DLString argument = constArguments;

    DLString firstArgument =  argument.getOneArgument( );
    DLString secondArgument = argument.getOneArgument( );
    pos = getPosition( );
    victim = 0;
    victim2 = 0;

    if (firstArgument.empty( )) // ����� ��� ����������
    {
        act( getNoargOther( ).c_str( ), ch, 0, victim, TO_ROOM );
        act_p( getNoargMe( ).c_str( ), ch, 0, victim, TO_CHAR,pos );
    }
    else if (( victim = get_char_room( ch, firstArgument ) ) != 0) // ������ �������� �� ������� ���������
    { 
        victim2 = victim;
        // See if 2-victim syntax is supported by this social. Find second victim.
        if (!getArgMe2( ).empty( ) && !secondArgument.empty( )) {
            victim2 = get_char_room( ch, secondArgument );
        }

        if ( !victim2 ) { // �� ������ �������� �� ������� ���������
            if ( victim == ch )
                ch->pecho( "�� ������ ������ ���� �����, ��� ����� %s?", secondArgument.c_str( ));
            else
                ch->pecho( "�� ������ ������ %1$C4 �����, ��� ����� %s?", victim, secondArgument.c_str( ));
            return;
        }

        if (victim == ch && victim2 == ch) { // ���������� ������� �� ����
            act( getAutoOther( ).c_str( ), ch, 0, victim, TO_ROOM );
            act_p( getAutoMe( ).c_str( ), ch, 0, victim, TO_CHAR, pos );
        }
        else if (victim2 == victim) { // ���������� ������� �� ������, � �.�. ���� ��� ��������� - ���� � �� �� ������
	    act( getArgOther( ).c_str( ), ch, 0, victim, TO_NOTVICT );
	    act_p( getArgMe( ).c_str( ), ch, 0, victim, TO_CHAR, pos );
	    act( getArgVictim( ).c_str( ), ch, 0, victim, TO_VICT );
        } else {
            // Output to actor and both victims. Substitute actor name with "self" if it matches victim.
	    const void *arg1 = victimOrSelf(ch, victim);
	    const void *arg2 = victimOrSelf(ch, victim2);

            ch->pecho( getArgMe2( ).c_str( ), ch, arg1, arg2 );
            if (victim != ch ) victim->pecho( getArgVictim2( ).c_str( ), ch, arg1, arg2 );
            if (victim2 != ch ) victim2->pecho( getArgVictim2( ).c_str( ), ch, arg2, arg1 );

            // Output to everyone else in the room.
            for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
                if (rch != ch && rch != victim && rch != victim2)
                    rch->pecho( getArgOther2( ).c_str( ), ch, arg1, arg2 );
        }
    }

    reaction( ch, victim, firstArgument );
    if (victim2 && victim2 != victim)
        reaction( ch, victim2, secondArgument );
}

bool SocialBase::checkPosition( Character *ch )
{
    if (ch->position >= getPosition( ))
	return true;

    switch (ch->position.getValue( )) {
    case POS_DEAD:
	ch->send_to("���� ������! �� {R����{x.\n\r");
	break;

    case POS_INCAP:
    case POS_MORTAL:
	ch->send_to("���� �� ����� �� ����! �� � ������� ���������.\n\r");
	break;

    case POS_STUNNED:
	ch->send_to("�� �� � ��������� ������� ���.\n\r");
	break;

    case POS_SLEEPING:
	ch->send_to("�� ���? ��� ����� ������� ����������...\n\r");
	break;

    case POS_RESTING:
	ch->send_to( "���... �� ���� �� ���������...\n\r" );
	break;

    case POS_SITTING:
	ch->send_to( "����? ��� ����� ������� ��������...\n\r" );
	break;

    case POS_FIGHTING:
	act_p( "���� �� �� ����, �� �� ����������!", ch, 0, 0, TO_CHAR, POS_FIGHTING );
	break;
    }

    return false;
}

void SocialBase::visualize( Character *ch )                                        
{
    if (IS_AFFECTED( ch, AFF_HIDE|AFF_FADE ))  {
	REMOVE_BIT( ch->affected_by, AFF_HIDE|AFF_FADE );
	ch->send_to("�� �������� �� ����.\n\r");
	act_p( "$c1 ������� �� ����.", ch, 0, 0, TO_ROOM,POS_RESTING);
    }

    if (IS_AFFECTED(ch, AFF_IMP_INVIS)) {
	affect_strip(ch,gsn_improved_invis);
	act("�� ����������� �����$g�|��|�� ��� ����������.", ch, 0, 0, TO_CHAR);
	act("$c1 ���������� �����$g�|��|�� ��� ����������.\n\r", ch,0,0,TO_ROOM);
    }
}
