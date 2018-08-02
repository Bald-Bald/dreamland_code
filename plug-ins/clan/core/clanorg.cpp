/* $Id$
 *
 * ruffina, 2004
 */
#include "clanorg.h"
#include "commonattributes.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "logstream.h"
/*----------------------------------------------------------------------------
 * ClanOrder
 *---------------------------------------------------------------------------*/
bool ClanOrder::canInduct( PCMemoryInterface * ) const
{
    return true;
}

const DLString & ClanOrder::getTitle( PCMemoryInterface * ) const
{
    return DLString::emptyString;
}

/*----------------------------------------------------------------------------
 * ClanOrgs
 *---------------------------------------------------------------------------*/
const DLString ClanOrgs::ATTR_NAME = "orden";

ClanOrder::Pointer ClanOrgs::findOrder( const DLString &oname ) const
{
    ClanOrder::Pointer null;

    if (oname.size( ) == 0)
	return null;

    const_iterator i = find( oname );
    if (i == end( ))
	return null;

    return i->second;
}

ClanOrder::Pointer ClanOrgs::findOrder( PCMemoryInterface *pci ) const
{
    return findOrder( getAttr( pci ) );
}

bool ClanOrgs::hasAttr( PCMemoryInterface *pci )
{
    return pci->getAttributes( ).isAvailable( ATTR_NAME );
}

void ClanOrgs::setAttr( PCMemoryInterface *pci, const DLString &value )
{
    pci->getAttributes( ).getAttr<XMLStringAttribute>( ATTR_NAME )->setValue( value );
}

void ClanOrgs::delAttr( PCMemoryInterface *pci )
{
    pci->getAttributes( ).eraseAttribute( ATTR_NAME );
}

const DLString & ClanOrgs::getAttr( PCMemoryInterface *pci )
{
    XMLStringAttribute::Pointer ord;
    
    ord = pci->getAttributes( ).findAttr<XMLStringAttribute>( ATTR_NAME );

    if (ord)
	return ord->getValue( );
    else
	return DLString::emptyString;
}

/*
 * command actions
 */
void ClanOrgs::doList( PCharacter *pch ) const
{
    ostringstream buf;

    for (const_iterator i = begin( ); i != end( ); i++)
	buf << dlprintf("   %-15s (%s)\n\r",
	                i->second->shortDescr.c_str( ),
	                i->first.c_str( ) );

    pch->send_to( buf );
}

void ClanOrgs::doMembers( PCharacter *pch ) const
{
    ostringstream buf;
    ClanOrder::Pointer ord = findOrder( pch );

    if (!ord) {
	pch->pecho( "�� �� �������� � %O6.", &name );
	return;
    }
    
    buf << "\n\r{W���         ����        �����         �������   ������{x\n\r";
    
    const PCharacterMemoryList& list = PCharacterManager::getPCM( );
    for (PCharacterMemoryList::const_iterator pos = list.begin( ); pos != list.end( ); pos++) {
	PCMemoryInterface *pcm = pos->second;

	if (pcm->getClan( ) != pch->getClan( )
	    || pcm->getLevel( ) >= LEVEL_IMMORTAL
	    || getAttr( pcm ) != ord->name)
	    continue;

	player_fmt( "%-10n %-10R %-12P %b %-3l   %-15t\r\n", pcm, buf );
    }

    pch->send_to( buf );
}


void ClanOrgs::doSelfInduct( PCharacter *pch, DLString &arg ) const
{
    ClanOrder::Pointer ord;

    if (!pch->getClan( )->isLeader( pch )) {
	pch->pecho( "������� ���� � %O4 ����� ������ �����.", &name );
	return;
    }
    
    if (!( ord = findOrder( arg ) )) {
	pch->pecho( "%1$^O1 �����%1$G��|�|�� �������.", &name );
	return;
    }
    
    setAttr( pch, ord->name );
    pch->pecho( "�� ��������� � %s.", ord->shortDescr.c_str( ) );
}

void ClanOrgs::doInduct( PCharacter *pch, DLString &arg ) const
{
    PCMemoryInterface *victim;

    if (!pch->getClan( )->isRecruiter( pch )) {
	pch->println( "����� ���������� ������������." );
	return;
    }

    if (!hasAttr( pch )) {
	pch->pecho( "�� �� ��������� ������ %O2.", &name );
	return;
    }
    
    if (!( victim = PCharacterManager::find( arg ) )) {
	pch->println( "������ ��� � ����� ������. ����� ��� ���������." );
	return;
    }

    if (victim->getClan( ) != pch->getClan( )) {
	pch->pecho( "�� %s �� ����������� � ������ �����!", victim->getName( ).c_str( ) );
	return;
    }
    
    if (hasAttr( victim )) {
	if (getAttr( victim ) != getAttr( pch )) 
	    pch->pecho( "%1$s ��� ������� � ����%2$G��|��|�� %2$O6.", 
	                 victim->getName( ).c_str( ), &name );
	else
	    pch->pecho( "%1$s � ��� ������� � ���%2$G��|��|�� %2$O6.", 
	                 victim->getName( ).c_str( ), &name );
	
	return;
    }
    
    ClanOrder::Pointer ord = findOrder( pch );

    if (!ord) {
	pch->pecho( "%1$^O1, � �����%1$G���|���|�� �� ������������, �� ����������!", &name );
	return;
    }

    if (!ord->canInduct( victim )) {
	pch->pecho( "%s �� ����� �������� � %s.", 
		    victim->getName( ).c_str( ), ord->shortDescr.c_str( ) );
	return;
    }
    
    setAttr( victim, ord->name );
    pch->pecho( "�� ���������� %s � %s!", 
                 victim->getName( ).c_str( ), ord->shortDescr.c_str( ) );
    
    if (victim->isOnline( ))
	victim->getPlayer( )->pecho( "%s ��������� ���� � %s!",
				     pch->getName( ).c_str( ),
				     ord->shortDescr.c_str( ) );
    else
	PCharacterManager::saveMemory( victim );
}

void ClanOrgs::doSelfRemove( PCharacter *pch ) const
{
    if (!hasAttr( pch ))
	pch->println( "�� � ��� ����� �� ��������." );
    else {
	delAttr( pch );
	pch->pecho( "�� ��������� ���%1$G�|�|� %1$^O4.", &name );
    }
}

void ClanOrgs::doRemove( PCharacter *pch, DLString &arg ) const
{
    PCMemoryInterface *victim;

    if (!pch->getClan( )->isRecruiter( pch )) {
	pch->println( "����� ���������� ������������." );
	return;
    }
    
    if (!hasAttr( pch )) {
	pch->pecho( "�� �� ��������� ������ %1$O2.", &name );
	return;
    }
    
    if (!( victim = PCharacterManager::find( arg ) )) {
	pch->println( "������ ��� � ����� ������. ����� ��� ���������." );
	return;
    }

    if (victim->getClan( ) != pch->getClan( )) {
	pch->pecho( "�� %s �� ����������� � ����� �����!", victim->getName( ).c_str( ) );
	return;
    }

    if (getAttr( victim ) != getAttr( pch )) {
	pch->pecho( "%1$s �� ������� � ���%2$G��|��|�� %2$O6!", 
	            victim->getName( ).c_str( ), &name );
	return;
    }
    
    delAttr( victim );
    pch->pecho( "%1$s �������� ���%2$G�|�|� %2$O4.", 
                 victim->getName( ).c_str( ), &name );

    if (victim->isOnline( ))
	victim->getPlayer( )->pecho( "%s ��������� ���� �� %^O2!",
				     pch->getName( ).c_str( ), &name );
    else
	PCharacterManager::saveMemory( victim );
}

