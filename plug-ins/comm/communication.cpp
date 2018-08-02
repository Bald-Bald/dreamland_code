/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"

#include "commandtemplate.h"
#include "commonattributes.h"
#include "replay.h"

#include "pcharacter.h"
#include "room.h"
#include "save.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "interp.h"
#include "handler.h"
#include "comm.h"
#include "def.h"

CMDRUNP( deaf )
{
	if (IS_SET(ch->comm,COMM_DEAF))
	{
		ch->send_to("�� ����� ������ �������� � ���-����.\n\r");
		REMOVE_BIT(ch->comm,COMM_DEAF);
	}
	else
	{
		ch->send_to("� ����� ������� �� �� ������ �� � ��� ��������.\n\r");
		SET_BIT(ch->comm,COMM_DEAF);
	}
}

CMDRUNP( quiet )
{
	if (IS_SET(ch->comm,COMM_QUIET))
	{
		ch->send_to("����� ������� �������� ��������.\n\r");
		REMOVE_BIT(ch->comm,COMM_QUIET);
	}
	else
	{
		ch->send_to("� ����� ������� �� ������ ������� ������ ��, ��� ���������� � �������.\n\r");
		SET_BIT(ch->comm,COMM_QUIET);
	}
}


/**
 * Syntax:
 * replay help          ������ ?     -- ��� �������
 * replay                            -- ��������� ����������� �� ����� ���, ���������� ��� ����� ��� � AFK
 * replay private       ������       -- ��������� X ������ ���������
 * replay near          �����        -- ��������� X ��������� � ������ ����� ��� � ��� �� ����, ��������� �����
 * replay public        ���������    -- ��������� X ��������� � ����� �������, ����������� ����, �������� ������
 * replay all           ���          -- ��� ����������� ��������� � ��������������� �������
 * replay <keyword> -N               -- ��������� N ��������� � ������ ��������� TODO
 *
 * X = 10, MAX for all = 100
 *
 * After fight with autostore:
 * ���� ���� ������� %d ���������. ��������� 'replay|����������' ��� ���������.
 * 
 * On returning from afk:
 * ����� AFK|��� ��������. ��������� 'replay|����������' ��� ��������� %d ������ ���������, 
 * ���������� �� ����� ����������, 'replay help' ��� �������.
 * ���
 * ����� AFK|��� ��������. ������ ��������� �� ����� ������ ���������� �� ����.
 * ��������� 'replay help' ��� ������� � ���, ��� ����������� ��� �������� ���������.
 * 
 * On reconnect:
 * ���������� �������������. ��������� 'replay|����������' ��� ��������� ����������� ���������.
 *
 * On replay with no args:
 *  - if tells > 0: show separately stored tells for fight|afk|reconnect and delete them
 *  - show 'see also' 
 *
 * On 'replay private':
 * ����������� ������ ���������:
 *  - show tells, pager private
 *  - show 'see also' 
 *
 * On 'replay near':
 * ����������� ��������� ����� � �����:
 *  - show say, social&emote, gtalk, yell
 *  - show 'see also' 
 *
 * On 'replay public':
 * ����������� ��������� � ����� �������:
 *  - show clantalk, grats, ooc, ic, gossip, pager public
 *  - show 'see also' 
 *
 *
 * TODO:
 * timestamps like in notes
 * replay -N
 *
 */

static bool replay_tells( ostringstream &buf, PCharacter *ch )
{
    XMLStringListAttribute::iterator i;
    XMLStringListAttribute::Pointer tells
		= ch->getAttributes( ).findAttr<XMLStringListAttribute>( "tells" );
    
    if (!tells || tells->size( ) == 0)
	return false;

    for (i = tells->begin( ); i != tells->end( ); i++)
	buf << " ** " << *i << endl;
    
    ch->getAttributes( ).eraseAttribute( "tells" );
    return true;
}

static void replay_summary( PCharacter *ch )
{
    ostringstream buf;

    buf << "��������� ������� {y{lEreplay{lR�����{lx ?{x ��� �������," << endl
        << "{y{lEreplay priv{lR���������� ����{x ��� ��������� ��������� ������ ���������," << endl
        << "{y{lEreplay near{lR���������� �����{x ��� ��������� ��������� ����� � �����," << endl
        << "{y{lEreplay pub{lR���������� ���{x ��� ��������� ��������� ��������� � ����� �������," << endl
        << "{y{lEreplay all{lR���������� ���{x ��� ��������� ���� ���������." << endl
        << "���������� ��������� -N ������ ��������� N ��������� � ������ ���������." << endl;

    ch->send_to( buf );
}

static void replay_hint( PCharacter *ch )
{
    ch->println( "����� ������ {y{lR���������� ?{lEreplay help{lx{x." );
}

static void replay_help( PCharacter *ch )
{
    ostringstream buf;
    int c = DEFAULT_REPLAY_SIZE;

    buf << "�������������:" << endl
        << "    {W{lR���������� ?        {lEreplay ?        {lx{x  - ��� �������" << endl
        << "    {W{lR����������          {lEreplay          {lx{x  - ������� ���������, ����������� �� ����� ���, ���������� ��� ����� ��� � AFK" << endl
        << "    {W{lR���������� ������   {lEreplay private  {lx{x  - ������� ��������� " << c << " ������ ���������" << endl
        << "    {W{lR���������� �����    {lEreplay near     {lx{x  - ������� ��������� " << c << " ��������� � ������ ����� � �����, � ����� ��������� �����" << endl
        << "    {W{lR���������� �����    {lEreplay public   {lx{x  - ������� ��������� " << c << " ��������� � ����� �������, ����������� ����, �������� ������" << endl
        << "    {W{lR���������� ���      {lEreplay all      {lx{x  - ��������� " << c << " ����������� ��������� � ��������������� �������" << endl
        << "    {W{lR���������� <���> -N {lEreplay <���> -N {lx{x  - ������ ��������� N ��������� � ������ ���������" << endl;
    ch->send_to( buf );
}

CMDRUNP( replay )
{
    PCharacter *pch;
    DLString arguments( argument );
    DLString arg = arguments.getOneArgument( );
    DLString arg2 = arguments.getOneArgument( );
    int limit = DEFAULT_REPLAY_SIZE;

    if (ch->is_npc( )) {
	ch->println("�� �� ������ ������������ ������� replay.");
        return;
    }

    pch = ch->getPC( );

    // Replay stored tells, show command summary.
    if (arg.empty( )) {
        ostringstream buf;

        if (replay_tells( buf, pch )) {
            pch->println( "���������, ���������� �� ����� ������ ����������:" );
            pch->send_to( buf );
        } else {
            pch->println( "��� ���������, ���������� �� ����� ����������, ��� ���������." );
        }

        replay_hint( pch );
        return;
    }

    // Display command syntax.
    if (arg_is_help( arg )) {
        replay_help( pch );
        return;
    }

    // Support -N syntax
    if (arg2.size( ) > 1 && arg2.at(0) == '-') {
        try {
            Integer newLimit( arg2.substr( 1 ) );
            if (newLimit > 0)
                limit = newLimit;
        } catch( ExceptionBadType &e) {
        }
    }

    // Display all messages in chronological order.
    if (arg_is_all( arg )) {
        ostringstream buf;

        if (!replay_history_all( buf, pch, limit )) {
            pch->println( "�� ��������� ����� ����� ������ �� �������." );
        } else {
            pch->println( "��� ����������� ��������� � ��������������� �������:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }

    // Display private messages only.
    if (arg_oneof( arg, "������", "������������", "private", "personal" )) {
        ostringstream buf;

        if (!replay_history_private( buf, pch, limit )) {
            pch->println( "�� ��������� ����� ���� ����� ������ �� �������." );
        } else {
            pch->println( "����������� ������ ���������:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }

    // Display public messages only.
    if (arg_oneof( arg, "�����", "���������", "public" )) {
        ostringstream buf;

        if (!replay_history_public( buf, pch, limit )) {
            pch->println( "�� ��������� ����� ����� ������ �� ������� � ����� �������." );
        } else {
            pch->println( "����������� ��������� � ����� �������:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }

    // Display nearby messages and socials.
    if (arg_oneof( arg, "�����", "nearby" )) {
        ostringstream buf;

        if (!replay_history_near( buf, pch, limit )) {
            pch->println( "����� � ����� ������ �� �����������." );
        } else {
            pch->println( "����������� ��������� ����� � �����:" );
            page_to_char( buf.str( ).c_str( ), pch );
        }

        replay_hint( pch );
        return;
    }


    // Argument not recognized.
    pch->println( "������������ ��������." );
    replay_summary( pch ); 
}


CMDRUNP( afk )
{
    PCharacter *pch = ch->getPC( );
    
    if (ch->is_npc( )) {
	ch->send_to( "����� �� ����?!\r\n" );
	return;
    }
    
    if (IS_SET(pch->comm,COMM_AFK))
    {
	XMLStringListAttribute::Pointer tells
		    = pch->getAttributes( ).findAttr<XMLStringListAttribute>( "tells" );

	if(tells && tells->size( ) > 0)
	{
            pch->pecho( "����� AFK ��������. ���� ������� {R%1$d{x �������%1$I��|��|��.\r\n"
                         "��������� ������� {y{lR����������{lEreplay{x, ����� %1$I���|��|�� ���������.", tells->size( ) );
	}
	else
	{
            pch->pecho( "����� AFK ��������. ��������� �� ����.\r\n"
                        "��������� {y{lR���������� ?{lEreplay help{x ��� ������� � ���, ��� ����������� ��� �������� ���������." );
	}

	REMOVE_BIT(pch->comm,COMM_AFK);
	pch->getAttributes( ).eraseAttribute( "afk" );	
    }
    else
    {
	SET_BIT(pch->comm,COMM_AFK);
	
	if (argument[0] != '\0') {
	    pch->getAttributes( ).getAttr<XMLStringAttribute>( "afk" )->setValue( argument );
	    pch->printf("�� � ������ AFK: {c%s.{x\r\n", argument);
	}
	else
	    pch->send_to("����� AFK �������.\n\r");
    }
}



