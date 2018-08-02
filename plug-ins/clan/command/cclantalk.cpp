/* $Id: cclantalk.cpp,v 1.1.6.4.6.10 2008/07/26 19:12:32 rufina Exp $
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


#include "cclantalk.h"
#include "commandtemplate.h"
#include "logstream.h"
#include "replay.h"

#include "pcharacter.h"
#include "clanreference.h"

#include "dreamland.h"
#include "gsn_plugin.h"
#include "descriptor.h"
#include "mudtags.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

CLAN(none);

/* TODO reimplement clantalk using 'communication' interfaces */

static void garble( const char *src, char *dst )
{

    for (; *src; src++, dst++)
    {
	    char ch = *src;

	    if( ch >= 'a' && ch <= 'z' )
		    *dst= 'a' + number_range( 0, 25 );
	    else if( ch >= 'A' && ch <= 'Z' )
		    *dst = 'A' + number_range( 0, 25 );
	    else if( ch >= '�' && ch <= '�' )
		    *dst = '�' + number_range( 0, '�' - '�' );
	    else if( ch >= '�' && ch <= '�' )
		    *dst = '�' + number_range( 0, '�' - '�' );
	    else
		    *dst = ch;
    }

    *dst = 0;
}


static bool check_soap( Character *ch )
{
    static const DLString soap( "soap" );
    
    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master)
	return check_soap(ch->master);
    
    if (ch->is_npc( ))
	return false;
    
    if (!ch->getPC( )->getAttributes( ).isAvailable( soap )) 
	return false;
    
    act("$c1 ������� ��� ��� {R�{Y�{G�{C�{M�{R�{G�{Y�{C�{M�{Y�{C�{x ������� ������.", ch, 0, 0, TO_ROOM);
    act("�� �������� ��� ��� {R�{Y�{G�{C�{M�{R�{G�{Y�{C�{M�{Y�{C�{x ������� ������.", ch, 0, 0, TO_CHAR);
    return true;
}


CMDRUN( cb )
{
    Descriptor *d;
    DLString argument = constArguments;
    ostringstream act_buf;
    DLString act_str;

    if (ch->getClan( ) == clan_none) {
	ch->send_to("�� �� ������������ �� � ������ �����.\n\r");
	return;
    }

    if (!ch->getClan( )->hasChannel( )) {
	ch->send_to("�� ���� ������ ��� ����.\n\r");
	return;
    }
    
    if (argument.empty( )) {
	TOGGLE_BIT(ch->comm, COMM_NOCB);

	if (IS_SET(ch->comm, COMM_NOCB))
	    ch->println("� ����� ������� �� �� ������� �������� ���������.");
	else
	    ch->println("�� ����� ������� �������� ���������.");
	return;
    }
    
    REMOVE_BIT(ch->comm, COMM_NOCB);

    if (check_soap(ch))
	return;


    act_buf << "{" << ch->getClan( )->getColor( ) << "["
            << ch->getClan( )->getShortName( ) << "] " 
            << "%1$C1: %2$s {x";
    act_str = act_buf.str( );
    
    if (dreamland->hasOption( DL_LOG_COMM ))
	LogStream::sendNotice( ) 
	    << "[" << ch->getClan( )->getShortName( ) << "] " 
	    << ch->getName( ) << ": " << argument << endl;

    if (!ch->isAffected(gsn_deafen)) {
        DLString message = fmt( ch, act_str.c_str( ), ch, argument.c_str( ) );
        ch->println( message );
        if (ch->getPC( )) {
            remember_history_public( ch->getPC( ), message );
        }
    }

    for (d = descriptor_list; d != 0; d = d->next) 
	if (d->connected == CON_PLAYING
	        && d->character
                && d->character != ch
		&& (d->character->getClan( ) == ch->getClan( ))
		&& !IS_SET(d->character->comm, COMM_NOCB)
		&& !d->character->isAffected(gsn_deafen))
	{
	    char msg_str[MAX_INPUT_LENGTH];

	    if (ch->isAffected(gsn_garble)) {
		ostringstream out;
		mudtags_convert_nocolor( argument.c_str( ), out, d->character );
		garble( out.str( ).c_str( ), msg_str );
	    }
	    else
		strcpy( msg_str, argument.c_str( ) );

            DLString message = fmt( d->character, act_str.c_str( ), ch, msg_str ); 
            d->character->pecho( message.c_str( ) );

            if (d->character->getPC( ))
                remember_history_public( d->character->getPC( ), message );
	}
}

void clantalk( Clan &clan, const char *format, ... )
{
    va_list ap;
    char msg[MAX_STRING_LENGTH];
    ostringstream buf;
    
    va_start( ap, format );
    vsprintf( msg, format, ap );
    va_end( ap );

    buf << "{" << clan.getColor( ) 
	<< "[" << clan.getShortName( ) << "] : "
	<< msg << "{x" << endl;

    for (Descriptor *d = descriptor_list; d != 0; d = d->next) 
	if (d->connected == CON_PLAYING
		&& d->character
		&& (d->character->getClan( ) == clan)
		&& !IS_SET(d->character->comm, COMM_NOCB)
		&& !d->character->isAffected(gsn_deafen))
	{
	    d->character->send_to( buf );
	}
}

