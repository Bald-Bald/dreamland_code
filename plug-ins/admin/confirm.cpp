/* $Id: confirm.cpp,v 1.1.2.21.6.6 2009/09/05 18:30:47 rufina Exp $
 *
 * ruffina, 2003
 */

#include "confirm.h"

#include "class.h"
#include "logstream.h"

#include "pcmemoryinterface.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "race.h"
#include "xmlattributes.h"

#include "clanreference.h"
#include "wiznet.h"
#include "ban.h"
#include "interp.h"
#include "arg_utils.h"
#include "merc.h"
#include "descriptor.h"
#include "def.h"
    
CLAN(none);

/*----------------------------------------------------------------------------- 
 * 'confirm' command
 *---------------------------------------------------------------------------*/
COMMAND(Confirm, "confirm")
{
    DLString arguments = constArguments;
    DLString cmd;
    
    arguments.stripWhiteSpace( );
    cmd = arguments.getOneArgument( );
    
    if (ch->is_npc( )) {
        ch->send_to( "���� ������.\n\r" );
        return;
    }
    
    if (cmd.empty( )) {
	usage( ch );
    }
    else if (cmd.strPrefix( "request" ) 
	     || cmd.strPrefix( "���������" )) 
    {
        doRequest( ch );
    }
    else if ((cmd.strPrefix( "accept" ) 
	      || cmd.strPrefix( "�������" )) && ch->is_immortal( ))
    {
        doAccept( ch, arguments );
    }
    else if ((cmd.strPrefix( "remove" ) 
	     || cmd.strPrefix( "reject" ) 
	     || cmd.strPrefix( "���������" )) && ch->is_immortal( ))
    {
        doReject( ch, arguments );
    }
    else if ((cmd.strPrefix( "delete" ) 
	      || cmd.strPrefix( "�������" )) && ch->is_immortal( ))
    {
        doDelete( ch, arguments );
    }
    else if (arg_is_list( cmd ) && ch->is_immortal( )) 
    {
        doList( ch );
    }
    else if (arg_is_show( cmd ) && ch->is_immortal( ))
    {
        doShow( ch, arguments );
    }
    else if ((cmd.strPrefix( "unread" ) 
	      || cmd.strPrefix( "�����������" )) && ch->is_immortal( ))
    {
        doUnread( ch );
    }
    else 
    { 
	usage( ch );
    }
}

void Confirm::doRequest( Character *ch ) 
{
    XMLAttributeConfirm::Pointer attr;
    DLString descr;    

    if (IS_SET( ch->act, PLR_CONFIRMED )) {
	ch->send_to( "���� �������� ��� �����������.\n\r" );
	return;
    }
    
    if (ch->desc && banManager->checkVerbose( ch->desc, BAN_CONFIRM )) 
	return;
    
    if (ch->getDescription( )) {
	descr = ch->getDescription( );
	descr.stripWhiteSpace( );
    }
    
    if (descr.empty( )) {
	ch->send_to( "�������� ����������� '{lR������� �������������{lEhelp confirm{lx' � '{lR������� ��������{lEhelp description{lx'.\r\n" );
	return;
    }

    attr = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeConfirm>( "confirm" );
    
    ch->println( "���� �������� ���������� ����������� �� ������������." );
    wiznet( WIZ_CONFIRM, 0, 0,
            "%^C1 ������ ������������� ������ ���������.", ch );
    
    attr->update( ch ); 
    PCharacterManager::saveMemory( ch->getPC( ) );
}

void Confirm::doAccept( Character *ch, DLString& arguments ) 
{
    XMLAttributeConfirm::Pointer attr;
    PCMemoryInterface *pci;
    PCharacter *victim;
    DLString name = arguments.getOneArgument( );

    if (name.empty( )) {
	ch->send_to( "����������� ����?\r\n" );
	return;
    }

    pci = PCharacterManager::find( name );
    
    if (!pci) {
	ch->send_to( "Player not found. Misspeled name?\r\n" );
	return;
    }

    victim = pci->getPlayer( );

    if (victim && IS_SET(victim->act, PLR_CONFIRMED)) {
	ch->send_to( "���� �������� ��� �����������.\r\n" );
	return;
    }
	    
    attr = pci->getAttributes( ).findAttr<XMLAttributeConfirm>( "confirm" );

    if (!attr) { 
	if (!victim) {
	    ch->send_to( "�����, ���� �� ��������� �� ����?\r\n" );
	    return;
	} 
	
	if (!ch->isCoder( )) {
	    ch->printf( "�� %s �� ���� ������ �� ������������� ���������.\n\r", pci->getName( ).c_str( ) );
	    return;
	}

	attr = pci->getAttributes( ).getAttr<XMLAttributeConfirm>( "confirm" );
	attr->update( victim );	
    } 

    attr->responsible.setValue( ch->getNameP( ) );
    attr->reason.setValue( arguments );
    attr->accepted.setValue( true );

    PCharacterManager::saveMemory( pci );

    ch->send_to( "Ok.\r\n" );

    wiznet( WIZ_CONFIRM, 0, 0,
	    "�������� %^N1 ����������� %C5.", pci->getName( ).c_str( ), ch );

    if (victim) 
	attr->run( victim );
}

void Confirm::doReject( Character *ch, DLString& arguments ) 
{
    XMLAttributeConfirm::Pointer attr;
    PCMemoryInterface *pci;
    PCharacter *victim;
    DLString name = arguments.getOneArgument( );

    if (name.empty( )) {
	ch->send_to( "Unconfirm whom?\r\n" );
	return;
    }

    if (arguments.empty( )) {
	ch->send_to( "����� ������� ������� ������.\r\n" );
	return;
    }

    pci = PCharacterManager::find( name );
    
    if (!pci) {
	ch->send_to( "Player not found. Misspeled name?\r\n" );
	return;
    }

    try {
	attr = pci->getAttributes( ).findAttr<XMLAttributeConfirm>( "confirm" );

    } catch (Exception e) {
	LogStream::sendError( ) << e.what( ) << endl;
	ch->send_to( "���������� �� ����������� ��������.\r\n" );
	return;
    }

    victim = pci->getPlayer( );
    
    if (!attr) { 
	if (!victim) { 
	    ch->send_to( "�����, ���� �� ��������� �� ����?\r\n" );
	    return;
	} else { 
	    attr = pci->getAttributes( ).getAttr<XMLAttributeConfirm>( "confirm" );
	    attr->update( victim );	
	}
    }

    attr->responsible.setValue( ch->getNameP( ) );
    attr->reason.setValue( arguments );
    attr->accepted.setValue( false );

    PCharacterManager::saveMemory( pci );

    ch->send_to( "Ok.\r\n" );

    wiznet( WIZ_CONFIRM, 0, 0,
	    "%^C1 ���������� � ������������� ��������� %^N1.", ch, pci->getName( ).c_str( ) );

    if (victim) 
	attr->run( victim );
}

void Confirm::doDelete( Character *ch, DLString& arguments ) 
{
    XMLAttributeConfirm::Pointer attr;
    PCMemoryInterface *pci;
    DLString name = arguments.getOneArgument( );

    if (name.empty( )) {
	ch->send_to( "��� ������ �������?\r\n" );
	return;
    }

    pci = PCharacterManager::find( name );
    
    if (!pci) {
	ch->send_to( "Player not found. Misspeled name?\r\n" );
	return;
    }

    attr = pci->getAttributes( ).findAttr<XMLAttributeConfirm>( "confirm" );
    
    if (!attr) {
	ch->send_to( "���� ����� �� ������� ������ �� �������������.\r\n" );
	return;
    }
    
    pci->getAttributes( ).eraseAttribute( "confirm" );
    PCharacterManager::saveMemory( pci );

    ch->send_to( "Ok.\r\n" );
}

void Confirm::doList( Character *ch ) 
{
    char buf[MAX_STRING_LENGTH];
    PCharacterMemoryList::const_iterator i;
    XMLAttributeConfirm::Pointer attr;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
   
    sprintf( buf, "%-15s %-20s %-9s %s\r\n", "Name", "Date", "State", "Responsible" );
    ch->send_to( buf );
     
    for (i = pcm.begin( ); i != pcm.end( ); i++) {
	DLString rp; 
	attr = i->second->getAttributes( ).findAttr<XMLAttributeConfirm>( "confirm" );
	
	if (!attr)
	    continue;
	
	rp = attr->responsible.getValue( );
	sprintf( buf, "%-15s %-20s %-9s %s\r\n",
		i->second->getName( ).c_str( ),
		attr->date.getTimeAsString("%H:%M %b %d" ).c_str( ),
		rp == "" ? "" :  (attr->accepted.getValue( ) ? "accepted" : "rejected"),
		rp.c_str( ) );
		
	ch->send_to( buf );
    }
}

void Confirm::doShow( Character *ch, DLString& argument ) 
{
    std::basic_ostringstream<char> buf;
    XMLAttributeConfirm::Pointer attr;
    PCMemoryInterface *pci;
    DLString name = argument.getOneArgument( );

    pci = PCharacterManager::find( name );

    if (!pci) {
	ch->send_to( "Player not found. Misspeled name?\r\n" );
	return;
    }

    attr = pci->getAttributes( ).findAttr<XMLAttributeConfirm>( "confirm" );
    
    if (!attr) {
	ch->send_to( "���� ����� �� ������� ������ �� �������������.\r\n" );
	return;
    }

    buf << "{W" << pci->getName( ) << "{x, " 
	<< "level " << pci->getLevel( ) << ", "
	<< sex_table.name( pci->getSex( ) ) << " "
	<< pci->getRace( )->getName( ) << " "
	<< pci->getProfession( )->getName( ).c_str( );

    if (pci->getClan( ) != clan_none)
	buf << ", clan " << pci->getClan( )->getShortName( );
    
    buf << endl;

    if (!attr->responsible.getValue( ).empty( )) {
	if (attr->accepted.getValue( ))
	    buf << "Accepted by ";
	else
	    buf << "Rejected by ";

	buf << "{C" << attr->responsible.getValue( ) << "{x" << endl;

	if (!attr->reason.getValue( ).empty( ))
	    buf << "Reason: " << attr->reason.getValue( ) << endl;
    }

    buf << endl << attr->description.getValue( ) << endl;
    ch->send_to( buf );
}

void Confirm::doUnread( Character *ch ) 
{
    std::basic_ostringstream<char> buf;
    PCharacterMemoryList::const_iterator i;
    XMLAttributeConfirm::Pointer attr;
    int total = 0, processed = 0;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
    
    for (i = pcm.begin( ); i != pcm.end( ); i++) {
	attr = i->second->getAttributes( ).findAttr<XMLAttributeConfirm>( "confirm" );
	
	if (!attr)
	    continue;
	
	if (attr->responsible.getValue( ) != "")
	    processed++;

	total++;
    }
    
    if (total) {
	buf << "���� ������� " << total-processed 
	    << " ����" << GET_COUNT( total-processed, "��", "��", "��" )
	    << " �� ������������� ���������. ����� ������: " << total << "." << endl;
	ch->send_to( buf );
    }
}

void Confirm::usage( Character *ch ) 
{
    std::basic_ostringstream<char> buf;
   
    buf << "������:" << endl
        << "{lEconfirm{lR�������������{lx {lErequest{lR���������{x  - ������� ������ �� ������������� ���������" << endl;

    if (!ch->is_immortal( )) {
	ch->send_to( buf );
	return;
    }

    buf << endl << "��� �����������:" << endl
        << "confirm list     - �������� ������ ������" << endl
	<< "confirm unread   - ���������� ������� ������" << endl
	<< "confirm accept <player> [<reason>] - ����������� ��������" << endl
	<< "confirm reject <player> <reason>   - �������� � �������������" << endl
	<< "confirm delete <player>            - ������� ������ �� ������" << endl
	<< "confirm show   <player>            - �������� ������ ������" << endl;

    ch->send_to( buf );
}

/*----------------------------------------------------------------------------- 
 * XMLAttributeConfirm
 *---------------------------------------------------------------------------*/

void XMLAttributeConfirm::run( Character *ch ) 
{
    std::basic_ostringstream<char> buf;
    
    if (responsible.getValue( ).empty( )) 
	return;
    
    if (accepted.getValue( )) {
	buf << "{W���� �������� ����������� ������.{x" << endl;
	SET_BIT( ch->act, PLR_CONFIRMED );
    }
    else {
	buf << "{R������ ��������� �������� � �������������.{x" << endl;
	REMOVE_BIT( ch->act, PLR_CONFIRMED );
    }
    
    if (!reason.getValue( ).empty( )) 
	buf << reason.getValue( ) << endl;
    
    ch->send_to( buf );

    ch->getPC( )->getAttributes( ).eraseAttribute( "confirm" );
}

void XMLAttributeConfirm::update( Character *ch ) 
{
    if (ch->getDescription( ))
	description.setValue( ch->getDescription( ) );

    date.setTime( Date::getCurrentTime( ) );
    responsible.setValue( "" );
    reason.setValue( "" );
    accepted.setValue( false );
}

/*---------------------------------------------------------------------------- 
 * XMLAttributeConfirmListenerPlugin
 *---------------------------------------------------------------------------*/

void XMLAttributeConfirmListenerPlugin::run( int oldState, int newState, Descriptor *d ) 
{
    std::basic_ostringstream<char> buf;
    Character *ch;
    XMLAttributeConfirm::Pointer attr;
    
    if (newState != CON_PLAYING)
	return;
    
    ch = d->character;
    if (!ch)
	return;

    if (ch->is_immortal( ))
	interpret_raw( ch, "confirm", "unread" );

    attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeConfirm>( "confirm" );
    
    if (!attr)
	return;
    
    attr->run( ch );

}
