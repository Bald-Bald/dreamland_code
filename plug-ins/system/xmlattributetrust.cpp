/* $Id$
 *
 * ruffina, 2004
 */
#include "xmlattributetrust.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "clanreference.h"

XMLAttributeTrust::XMLAttributeTrust( )
                    : all( false ), 
		      clansAllow( clanManager ), clansDeny( clanManager )
{
}

XMLAttributeTrust::~XMLAttributeTrust( )
{
}

bool XMLAttributeTrust::check( Character *ch ) const
{
    if (ch->is_npc( )) {
	if (ch->master)
	    return check( ch->master );
	else 
	    return all.getValue( );
    }

    if (all.getValue( )) 
	return !checkDeny( ch->getPC( ) );
    else 
	return checkAllow( ch->getPC( ) );
}

bool XMLAttributeTrust::checkDeny( PCharacter *ch ) const
{
    if (playersDeny.hasElement( ch->getName( ) ))
	return true;

    if (clansDeny.isSet( *(ch->getClan( )) ))
	return true;

    return false;
}

bool XMLAttributeTrust::checkAllow( PCharacter *ch ) const
{
    if (playersAllow.hasElement( ch->getName( ) ))
	return true;

    if (clansAllow.isSet( *(ch->getClan( )) ))
	return true;

    return false;
}

bool XMLAttributeTrust::parse( const DLString &constArguments, ostringstream &buf )
{
    DLString args = constArguments;
    DLString cmd = args.getOneArgument( );
    bool fAllow;
    PCMemoryInterface *pci;
    Clan *clan;
    
    if (cmd.empty( )) {
	buf << "����� ���� �� ��������: {lR������, ��������� ��� ���������{lElist, allow ��� deny{lx.";
	return false;
    }
    
    if (cmd.strPrefix( "list" ) || cmd.strPrefix( "������" )) {
	if (all) {
	    buf << "��������� ����";
	    
	    if (!clansDeny.empty( ))
		buf << endl << "   ����� ������ ����� " << clansDeny.toString( );

	    if (!playersDeny.empty( ))
		buf << endl << "   ����� ��������� " << playersDeny.toString( );
	}
	else { 
	    buf << "��������� ����";

	    if (!clansAllow.empty( ))
		buf << endl << "   ����� ������ ����� " << clansAllow.toString( );

	    if (!playersAllow.empty( ))
		buf << endl << "   ����� ��������� " << playersAllow.toString( );
	}

	return true;
    }

    if (cmd.strPrefix( "allow" ) || cmd.strPrefix( "���������" ))
	fAllow = true;
    else if (cmd.strPrefix( "deny" ) || cmd.strPrefix( "���������" ))
	fAllow = false;
    else {
	buf << "����� ���� �� ��������: {lR������, ��������� ��� ���������{lElist, allow ��� deny{lx.";
	return false;
    }

    if (args.empty( )) {
	buf << "���� ������ �� ������ " 
	    << (fAllow ? "���������" : "���������") << " ��� ������?";
	return false;
    }
    
    if (args == "all" || args == "���" || args == "����") {
	if (fAllow)
	    buf << "������ ��������� ���� (����� ���, ���� ��������� ����).";
	else
	    buf << "������ ��������� ���� (����� ���, ���� ��������� ����).";

	all = fAllow;
	return true;
    }

    pci = PCharacterManager::find( args );
    clan = clanManager->findExisting( args );

    if (pci) {
	if (fAllow) {
	    if (all)
		playersDeny.remove( pci->getName( ) );
	    else
		playersAllow.add( pci->getName( ) );
	    buf << "������ ��������� ��������� " << pci->getName( ) << ".";
	}
	else {
	    if (all)
		playersDeny.add( pci->getName( ) );
	    else
		playersAllow.remove( pci->getName( ) );
	    buf << "������ ��������� ��������� " << pci->getName( ) << ".";
	}
	return true;
    }

    if (clan) {
	if (fAllow) {
	    if (all)
		clansDeny.remove( *clan );
	    else
		clansAllow.set( *clan );
	    buf << "������ ��������� ������ ����� " << clan->getShortName( ) << ".";
	}
	else {
	    if (all)
		clansDeny.set( *clan );
	    else
		clansAllow.remove( *clan );
	    buf << "������ ��������� ������ ����� " << clan->getShortName( ) << ".";
	}
	return true; 
    }

    buf << "���� ��� �������� � ����� ������ �� �������.";
    return false;
}

