/* $Id: divorce.cpp,v 1.1.2.5.10.1 2007/06/26 07:18:03 rufina Exp $
 *
 * ruffina, 2003
 */

#include "divorce.h"
#include "marriageexception.h"
#include "xmlattributemarriage.h"
#include "xmlattributelovers.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "class.h"

COMMAND(Divorce, "divorce")
{
    std::basic_ostringstream<char> buf;
    PCharacter *bride1, *bride2;
    DLString arguments = constArguments;
    DLString brideName1, brideName2;
    
    if (!ch->is_immortal( )) {
	ch->send_to( "��� �� ��� ����.\r\n" );
	return;
    }

    if (arguments.empty( )) {
	ch->send_to( "� ���� ��������� �����?\r\n" );
	return;
    }
    
    brideName1 = arguments.getOneArgument( );
    brideName1.upperFirstCharacter( );
   
    if (arguments.empty( )) { 
	try {
	    bride1 = checkBride( ch, brideName1 );
	    divorceWidow( bride1 );	
	    buf << brideName1 << " - ��������� �������." << endl;
	    ch->send_to( buf );

	} catch (MarriageException e) {
	    ch->send_to( e.what( ) );
	    return;
	}
	return;
    }

    brideName2 = arguments.getOneArgument( );
    brideName2.upperFirstCharacter( );

    if (brideName2 == brideName1) {
	ch->send_to( "��� ��� ���?\r\n" );
	return;
    }

    if (ch->getName( ) == brideName1 || ch->getName( ) == brideName2) {
	ch->send_to( "������� ����-�� ������ ����.\r\n" );
	return;
    }

    try {
	bride1 = checkBride( ch, brideName1 );
	bride2 = checkBride( ch, brideName2 );

	divorce( bride1, brideName2 );
	divorce( bride2, brideName1 );
	
    } catch (MarriageException e ) {
	ch->send_to( e.what( ) );
	return;
    }

    buf << brideName1 << " � " << brideName2 << " ������������." << endl;
    ch->send_to( buf );
}

void Divorce::divorce( PCharacter *pch, DLString name ) {
    std::basic_ostringstream<char> buf;
    XMLAttributeMarriage::Pointer attr;
    
    attr = pch->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" );

    if (attr->spouse.getValue( ) != name) {
	buf << name << " � " << pch->getName( ) << " �� ������." << endl;
	throw MarriageException( buf.str( ) );
    }
    
    attr->history.push_back( XMLString( attr->spouse ));
    attr->spouse.setValue( "" );
    buf << "�� ���������� ���� ��������� ���������." << endl;
    pch->send_to( buf );
}

void Divorce::divorceWidow( PCharacter *pch ) {
    std::basic_ostringstream<char> buf;
    XMLAttributeMarriage::Pointer attr;
    
    attr = pch->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" );

    if (PCharacterManager::find( attr->spouse.getValue( ) )) {
	buf << pch->getName( ) << " ��� �� �������(�)." << endl;
	throw MarriageException( buf.str( ) );
    }

    attr->history.push_back( XMLString( attr->spouse ) );
    attr->spouse.setValue( "" );
    buf << "�� ���������� ���� ��������� ���������." << endl;
    pch->send_to( buf );
}
    
PCharacter * Divorce::checkBride( Character *ch, DLString name ) {
    std::basic_ostringstream<char> buf;
    PCMemoryInterface *pcm;
    PCharacter *pch;
    XMLAttributeMarriage::Pointer attr;
    
    if ((pcm = PCharacterManager::find( name )) == NULL) {
	buf << "����� " << name << " �� ������." << endl;
	throw MarriageException( buf.str( ) );
    }

    if ((pch = dynamic_cast<PCharacter *>( pcm )) == NULL) {
        buf << name << " �� ������������ � ����." << endl;
        throw MarriageException( buf.str( ) );
    }

    attr = pch->getAttributes( ).findAttr<XMLAttributeMarriage>( "marriage" );

    if (!attr || attr->spouse.getValue( ).empty( )) {
	buf << "�� " << name << " �� ������(�) �������� ������!" << endl;
	throw MarriageException( buf.str( ) );
    }
    
    if (ch->in_room != pch->in_room) {
	buf << "������ " << name << " ��������� ������� ������ �� ����." << endl;
	throw MarriageException( buf.str( ) );
    }
    
    return pch;
}

