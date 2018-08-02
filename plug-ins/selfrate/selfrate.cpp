/* $Id: selfrate.cpp,v 1.1.2.5 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2004
 */
#include "so.h"
#include "commandtemplate.h"
#include "selfrate.h"

#include "pcharacter.h"

#include "infonet.h"
#include "merc.h"
#include "def.h"

bool rated_as_newbie( PCMemoryInterface* pcm )
{
    return pcm->getAttributes( ).getAttr<XMLAttributeSelfRate>( "selfrate" )->rate == 0;
}
bool rated_as_expert( PCMemoryInterface* pcm )
{
    return pcm->getAttributes( ).getAttr<XMLAttributeSelfRate>( "selfrate" )->rate == 1;
}
bool rated_as_guru( PCMemoryInterface* pcm )
{
    return pcm->getAttributes( ).getAttr<XMLAttributeSelfRate>( "selfrate" )->rate == 2;
}



CMDRUN( selfrate )
{
    int rate;
    XMLAttributeSelfRate::Pointer attr;
    DLString args = constArguments;
    DLString arg = args.getOneArgument( );
    PCharacter *pch = ch->getPC( );
    
    if (!pch) {
	ch->send_to( "�� ������� ��������.\r\n" );
	return;
    }

    if (IS_AFFECTED(pch, AFF_CHARM)) {
	pch->send_to( "������ �� ���������.\r\n" );
	return;
    }
	
    attr = pch->getAttributes( ).getAttr<XMLAttributeSelfRate>( "selfrate" );

    if (arg.empty( )) {
	pch->printf( "���� ������� ���������: {W%s{x.\r\n", attr->getRateAlias( ).c_str( ) );
	return;
    }

    if (arg == "newbie" ) 
	rate = 0;
    else if (arg == "expert" || arg == "experienced" )
	rate = 1;
    else if (arg == "guru" )
	rate = 2;
    else {
	pch->printf( "������, ��� ��: newbie, expert ��� guru.\r\n" );
	return;
    }

    if (rate == attr->rate) 
	pch->printf( "�� �� � ��� %s!\r\n", attr->getRateAlias( ).c_str( ));
    else if (rate < attr->rate) 
	pch->send_to( "�� �� ������ �������� ������ ������ ������ ���������.\r\n" );
    else {
	char buf[256];
	
	attr->rate = rate;
	pch->printf( "�����������! ������ �� {W%s{x.\r\n", attr->getRateAlias( ).c_str( ));
	sprintf( buf, "{C������������� ����� �� $o2: {W$C1 ������ %s!{x", attr->getRateAlias( ).c_str( ) );
	infonet( buf, pch, 0 );
    }
}


XMLAttributeSelfRate::XMLAttributeSelfRate( ) 
{
}

DLString XMLAttributeSelfRate::getRateAlias( ) const
{
    switch (rate.getValue( )) {
    case 0: return "newbie";
    case 1: return "expert";
    case 2: return "guru";
    default: return "unknown!";
    }
}

bool XMLAttributeSelfRate::handle( const WhoisArguments &args )
{
    DLString l;
    
    l << "������� ���� {W" << getRateAlias( ) << "{x";
    args.lines.push_back( l );
    return true;
}

extern "C"
{
    SO::PluginList initialize_selfrate( ) {
	SO::PluginList ppl;
	
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeSelfRate> >( ppl );
	
	return ppl;
    }
}

