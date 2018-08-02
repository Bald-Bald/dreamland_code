/* $Id$
 *
 * ruffina, 2004
 */
#include <algorithm>
#include "class.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "commandmanager.h"
#include "mercdb.h"
#include "handler.h"
#include "merc.h"

#include "security.h"
#include "olc.h"
#include "onlinecreation.h"

#include "def.h"

/*
 * XMLAttributeOLC
 */
XMLVnumRange::XMLVnumRange( ) 
{
}

XMLVnumRange::XMLVnumRange( int min, int max, int s ) 
	    : minVnum( min ), maxVnum( max ), security( s )
{
}

void XMLAttributeOLC::removeInterval( int a, int b )
{
    RangeList::iterator i;
    RangeList nlist;

    if (a > b)
//	a ^= b ^= a ^= b;
        swap(a, b);

    for (i = vnums.begin( ); i != vnums.end( ); i++) {
	if (a < i->minVnum && b > i->maxVnum) {
	    nlist.push_back( *i );
	}
	else {
	    if (b < i->maxVnum)
		nlist.push_back( XMLVnumRange( b + 1, i->maxVnum, i->security ) );
	    if (a > i->minVnum) 
		nlist.push_back( XMLVnumRange( i->minVnum, a - 1, i->security ) );
	}
    }
    
    vnums = nlist;
}

bool XMLAttributeOLC::isOverlapping( int a, int b )
{
    RangeList::iterator i;
    
    if (a > b)
        swap(a, b);

    for (i = vnums.begin( ); i != vnums.end( ); i++) {
	if (a >= i->minVnum && a <= i->maxVnum)
	    return true;
	if (b >= i->minVnum && b <= i->maxVnum)
	    return true;
    }

    return false;
}

void XMLAttributeOLCPlugin::initialization( ) 
{
    Class::regMoc<XMLVnumRange>( );
    Class::regMoc<XMLAttributeOLC>( );
    XMLAttributePlugin::initialization( );
}

void XMLAttributeOLCPlugin::destruction( ) 
{
    XMLAttributePlugin::destruction( );
    Class::unregMoc<XMLVnumRange>( );
    Class::unregMoc<XMLAttributeOLC>( );
}

/*
 * security commands and functions
 */
CMD(security, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
	"Set char security.")
{
    char buf[MAX_STRING_LENGTH];
    Character *victim;
    
    if(ch->getPC()->getSecurity() < 100) {
	ch->send_to("��� �� ��� ����.\n\r");
	return;
    }
    argument = one_argument(argument, buf);
    
    if(!*buf) {
	ch->send_to("Usage: security <player> <#security level>\n\r");
	return;
    }
    victim = get_char_world(ch, buf);

    if(!victim || victim->is_npc()) {
	ch->send_to("Char not found.\n\r");
	return;
    }

    argument = one_argument(argument, buf);
    if(!*buf || !is_number(buf)) {
	ch->send_to("Security must be a number.\n\r");
	return;
    }
    
    victim->getPC()->setSecurity(atoi(buf));
    
    ch->send_to("Security set.\n\r");
}

CMD(olcvnum, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
	"Set/Show/Remove allowed vnum ranges for char.")
{
    PCMemoryInterface *victim;
    XMLAttributeOLC::Pointer attr;
    DLString arg1, arg2, arg3, arg4, arg5;
    DLString arguments = argument;
    
    if(ch->getPC( )->getSecurity() < 100) {
	ch->send_to( "��� �� ��� ����.\n\r" );
	return;
    }
    
    arg1 = arguments.getOneArgument( );
    arg2 = arguments.getOneArgument( );
    
    if (!arg1.empty( )) {
	victim = PCharacterManager::find( arg1 );

	if (!victim) {
	    ch->send_to( "Char not found, misspelled name?\r\n" );
	    return;
	}

	if (arg2 == "show") {
	    std::basic_ostringstream<char> buf;

	    attr = victim->getAttributes( ).findAttr<XMLAttributeOLC>( "olc" );
	    
	    if (!attr) {
		buf << victim->getName( ) << " doesnt own any vnum range" << endl;
	    }
	    else {
		XMLAttributeOLC::RangeList::iterator i;
		
		for (i = attr->vnums.begin( ); i != attr->vnums.end( ); i++)
		    buf << "Vnums {W" << i->minVnum << "{x - {W" << i->maxVnum 
			<< "{x, security {W" << i->security << "{x" << endl;
	    }
	    
	    ch->send_to( buf );
	    return;
	}
	if (arg2 == "del") {
	    arg3 = arguments.getOneArgument( );
	    arg4 = arguments.getOneArgument( );

	    attr = victim->getAttributes( ).findAttr<XMLAttributeOLC>( "olc" );

	    if (attr) {
		try {
		    attr->removeInterval( arg3.toInt( ), arg4.toInt( ) );
		} catch (const ExceptionBadType &e) {
		    ch->send_to( "Vnum ranges must be numbers.\r\n" );
		    return;
		}
		
		ch->send_to( "Ok.\r\n" );
	    }
	    else
		ch->send_to( "No vnum ranges found.\r\n" );
	    
	    return;
	}
	if (arg2 == "set") {
	    int a, b;

	    arg3 = arguments.getOneArgument( );
	    arg4 = arguments.getOneArgument( );
	    arg5 = arguments.getOneArgument( );
    
	    try {
		attr = victim->getAttributes( ).getAttr<XMLAttributeOLC>( "olc" );
		a = arg3.toInt( );
		b = arg4.toInt( );

		if (attr->isOverlapping( a, b )) {
		    ch->send_to( "Vnums overlap existing ranges. Try olcvnum <name> show.\r\n" );
		    return;
		}
		
		attr->vnums.push_back( XMLVnumRange( a, b, arg5.toInt( ) ) );
		PCharacterManager::saveMemory( victim );
		ch->send_to("Vnum range granted.\n\r");
		return;
		
	    } catch (const ExceptionBadType& e) {
	    }
	}
    } 

    ch->send_to( "Usage: olcvnum <player> set <min vnum> <max vnum> <security level>\r\n" 
		 "       olcvnum <player> show\r\n" 
		 "       olcvnum <player> del <min vnum> <max vnum>\r\n" );

}
