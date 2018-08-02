/* $Id: ccard.cpp,v 1.1.2.8.6.4 2009/09/05 18:30:47 rufina Exp $
 *
 * ruffina, 2005
 */

#include "ccard.h"
#include "mobiles.h"
#include "xmlattributecards.h"

#include "class.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "npcharacter.h"
#include "room.h"

#include "merc.h"
#include "arg_utils.h"
#include "handler.h"
#include "save.h"
#include "def.h"

COMMAND(CCard, "card")
{
    DLString arguments = constArguments;
    DLString cmd = arguments.getOneArgument( );
    PCharacter *pch = ch->getPC( );

    if (!pch)
	return;
    
    if (!pch->is_immortal( )) {
	pch->send_to( "��� �� ��� ����.\r\n" );
	return;
    }
    
    if (cmd.empty( )) 
	usage( pch );
    else if (cmd.strPrefix( "mob" )) 
	doMob( pch, arguments );
    else if (cmd.strPrefix( "char" )) 
	doChar( pch, arguments );
    else if (arg_is_list( cmd )) 
	doList( pch, arguments );
    else
	usage( pch );
}

void CCard::doMob( PCharacter *ch, DLString& arguments )
{
    Character *mob;
    DLString mobName = arguments.getOneArgument( );
    
    if (mobName.empty( )) {
	ch->send_to( "���� �� ������ ������� ���������?\r\n" );
	return;
    }
    
    mob = get_char_world( ch, mobName.c_str( ) );

    if (!mob) {
	ch->send_to( "Mobile not found.\r\n" );
	return;
    }
    
    CardStarterBehavior::Pointer bhv( NEW );
    bhv->setChar( mob->getNPC( ) );
    mob->getNPC( )->behavior.setPointer( *bhv );
    save_mobs( mob->in_room );

    ch->printf( "%s �� ������� [%d] ����(�) ���������.\r\n",
		mob->getNameP( '1' ).c_str( ), mob->in_room->vnum );
}

void CCard::doChar( PCharacter *ch, DLString& arguments )
{
    int level;
    PCMemoryInterface *pci;
    XMLAttributes *attributes;
    XMLAttributeCards::Pointer card;
    DLString name, arg;
    
    name = arguments.getOneArgument( );
    pci = PCharacterManager::find( name );

    if (!pci) {
	ch->send_to( "������ �� �������.\r\n" );
	return;
    }

    attributes = &pci->getAttributes( );
    card = attributes->findAttr<XMLAttributeCards>( "cards" );
    
    if (!card)
	ch->printf( "%s �� ������� � ������.\r\n", pci->getName( ).c_str( ) );
    else
	ch->printf( "%s - ��� %s �� ������.\r\n", 
		    pci->getName( ).c_str( ), card->getFace( '1' ).c_str( ) );


    arg = arguments.getOneArgument( );

    if (arg.empty( ) || !ch->isCoder( )) 
	return;
    
    if (arg == "clear" || arg == "off") {
	attributes->eraseAttribute( "cards" );
	PCharacterManager::saveMemory( pci );
	ch->println( "��(�) �������� �� ������." );	
	return;
    }

    try {
	level = arg.toInt( );
	if (level < 0 || level > 8)
	    throw Exception( );
    } catch (const Exception& ) {
	ch->send_to( "<card level> ������ ���� ������ �� 0 �� 8.\r\n" );
	return;
    }
    
    if (!card)
	card = attributes->getAttr<XMLAttributeCards>( "cards" );

    card->setLevel( level );

    if (card->getSuit( ) < 0)
	card->setSuit( card->getRandomSuit( ) );

    PCharacterManager::saveMemory( pci );
    ch->printf( "%s ���������� %s.\r\n", 
		pci->getName( ).c_str( ), card->getFace( '5' ).c_str( ) );
     
}

void CCard::doList( PCharacter *ch, DLString& arguments )
{
    int cnt;
    Character *wch;
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
   
    ch->send_to( "������ ���� ���� �� ������: \r\n������: \r\n");
    cnt = 0;
     
    for (i = pcm.begin( ); i != pcm.end( ); i++) {
	PCMemoryInterface *pci;
	XMLAttributeCards::Pointer card;

	pci = i->second;
	card = pci->getAttributes( ).findAttr<XMLAttributeCards>( "cards" ); 

	if (card) {
	    ch->printf( "%20s %s\r\n", 
			pci->getName( ).c_str( ),
			card->getFace( '1' ).c_str( ) );
	    cnt++;
	}
    }
    
    if (cnt > 0)
	ch->printf( "�����: %d ���\r\n", cnt );

    ch->printf( "\r\n����-��������:\r\n", cnt );
    cnt = 0;
	
    for (wch = char_list; wch; wch = wch->next) {
	NPCharacter *mob;

	if (!wch->is_npc( ))
	    continue;
	if (!wch->in_room)
	    continue;
    
	mob = wch->getNPC( );

	if (!mob->behavior)
	    continue;
	if (!mob->behavior.getDynamicPointer<CardStarterBehavior>( ))
	    continue;
	
	
	ch->printf( "[%5d] %-28s [%5d] %s\r\n",
		    mob->pIndexData->vnum, mob->getNameP( '1' ).c_str( ),
		    mob->in_room->vnum, mob->in_room->name );
	cnt++;
    }
    
    if (cnt > 0)
	ch->printf( "�����: %d ���\r\n", cnt );
}

void CCard::usage( PCharacter *ch )
{
    std::basic_ostringstream<char> buf;

    buf << "���������: " << endl
	<< "{Wcard list{x  - ���������� ��� ������" << endl
	<< "{Wcard mob {x<name>   -  ������� ���� ��������� ���������" << endl
	<< "{Wcard char {x<name>  - �������� ����� � ������ ��� ����� ������" << endl;
    
    if (ch->isCoder( )) {
	buf << "{Wcard char {x<name> <level> - ���������� ������ ������� � ������ (0..8)" << endl
	    << "{Wcard char {x<name> {Wclear{x - ������� ������ �� ������" << endl;
    }

    ch->send_to( buf );
}

