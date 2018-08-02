/* $Id: marry.cpp,v 1.1.2.7.6.2 2007/09/21 21:24:08 rufina Exp $
 *
 * ruffina, 2003
 */

#include "marry.h"
#include "marriageexception.h"
#include "xmlattributemarriage.h"
#include "xmlattributelovers.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "infonet.h"
#include "room.h"
#include "class.h"

COMMAND(Marry, "marry")
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
	ch->send_to( "� ���� ������ �����?\r\n" );
	return;
    }
    
    brideName1 = arguments.getOneArgument( );
    brideName1.upperFirstCharacter( );
    
    if (arguments.empty( )) {
	ch->send_to( "��� ��� ��������� �� ������� ������ ����������.\r\n" );
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
	
    } catch (MarriageException e) {
	ch->send_to( e.what( ) );
	return;
    }

    bride1->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" )->spouse.setValue( brideName2 );
    bride1->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" )->wife.setValue( false );
    bride2->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" )->spouse.setValue( brideName1 );
    bride2->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" )->wife.setValue( true );

    buf << "�� ���������� " << brideName1 << " � " << brideName2 << " ����� � �����!" << endl;
    ch->send_to( buf );

    buf.str( "" );
    buf << ch->getName( ) << " ��������� ��� ����� � �����!" << endl;
    bride1->send_to( buf );
    bride2->send_to( buf );

    buf.str( "" );
    buf << ch->getName( ) << " ��������� " << brideName1 << " � " << brideName2 << " ����� � �����!" << endl;
    
    for (Character *wch = ch->in_room->people; wch; wch = wch->next_in_room) {
	if (!wch->is_npc( ) && wch != ch && wch != bride1 && wch != bride2)
	    wch->send_to( buf );
    }

    bride1->getAttributes( ).getAttr<XMLAttributeLovers>( "lovers" )->lovers.put( brideName2 );
    bride2->getAttributes( ).getAttr<XMLAttributeLovers>( "lovers" )->lovers.put( brideName1 );

    buf.str( "" );
    buf << "{C������� ����� �� $o2: {Y" 
	<< brideName1 << "{W � {Y" << brideName2 << "{W ������ ��� � ����!!!{x";
    infonet( buf.str( ).c_str( ), 0, 0 );
}

PCharacter * Marry::checkBride( Character *ch, DLString name ) {
    std::basic_ostringstream<char> buf;
    PCMemoryInterface *pcm;
    PCharacter *pch;
    XMLAttributeMarriage::Pointer attr;
    
    pcm = PCharacterManager::find( name ); 
    
    if (!pcm) {
	buf << "����� " << name << " �� ������." << endl;
	throw MarriageException( buf.str( ) );
    }

    pch = dynamic_cast<PCharacter *>( pcm );

    if (!pch) {
	buf << name << " �� ������������ � ����." << endl;
	throw MarriageException( buf.str( ) );
    }

    attr = pch->getAttributes( ).findAttr<XMLAttributeMarriage>( "marriage" );

    if (attr && !attr->spouse.getValue( ).empty( )) {
	buf << "�� " << name << " ��� ������(�) �������� ������!" << endl;
	throw MarriageException( buf.str( ) );
    }
    
    if (ch->in_room != pch->in_room) {
	buf << "������ " << name << " ��������� ������� ������ �� ����." << endl;
	throw MarriageException( buf.str( ) );
    }
    
    return pch;
}

