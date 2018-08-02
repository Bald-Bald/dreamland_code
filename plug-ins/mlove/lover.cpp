/* $Id: lover.cpp,v 1.1.2.10.10.3 2009/09/06 21:48:28 rufina Exp $
 * ruffina, 2003
 */

#include "lover.h"
#include "xmlattributemarriage.h"
#include "xmlattributelovers.h"
#include "xmllovers.h"

#include "logstream.h"
#include "class.h"

#include "pcharacter.h"
#include "pcharactermanager.h"

#include "arg_utils.h"
#include "merc.h"
#include "act.h"
#include "def.h"

const DLString Lover::XMLAttributeLoverString = "XMLAttributeLovers";

COMMAND(Lover, "lover")
{
    DLString arguments = constArguments;
    DLString cmd = arguments.getOneArgument( );
    
    if (ch->is_npc()) {
	ch->send_to("���� ������.\n\r");
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM)) {
	act_p("... �� ������ �� ���������.", ch, 0, 0, TO_CHAR, POS_RESTING);  
	act_p("$c1 ���������� - ������ �� ���������.", ch, 0, ch->master, TO_VICT, POS_RESTING);
	return;
    }
    
    if (cmd.empty( ))
	usage( ch );
    else if (arg_is_list( cmd ))
	list( ch, arguments );
    else if (arg_oneof( cmd, "add", "��������" ))
	add( ch, arguments );
    else if (arg_oneof( cmd, "del", "�������" ))
	del( ch, arguments );
    else 
	usage( ch );
	
}

void Lover::list( Character* ch, DLString arguments) 
{
    std::basic_ostringstream<char> str;
    XMLAttributes *attributes;
    XMLAttributeLovers::Pointer pointer;

    attributes = &ch->getPC( )->getAttributes( );
    XMLAttributes::iterator ipos = attributes->find( "lovers" );
	
    if (ipos == attributes->end( ) ||
	(pointer = ipos->second.getDynamicPointer<XMLAttributeLovers>( ))->lovers.empty( )) 
    {
	str << "�� ������ �� ������." << endl;
    }
    else {
	str << "{W- {R���������{W -" << endl;
	
	for (XMLLovers::iterator loverpos = pointer->lovers.begin( );
	     loverpos != pointer->lovers.end( );
	     loverpos++) 
	{
	    str << loverpos->first << endl;
	}
	
	str << "{x";
   }
    
    ch->send_to( str );
}

void Lover::add( Character* ch, DLString arguments)
{
    DLString name = arguments.getOneArgument();
    std::basic_ostringstream<char> str;

    if (PCMemoryInterface* pci = PCharacterManager::find( name )) {
	ch->getPC( )->getAttributes( ).getAttr<XMLAttributeLovers>( "lovers" )->
			    lovers.put( pci->getName( ) );

	str << "�� ������� ���� ������ "<<  pci->getName( ) << "." << endl;
    }
    else {
	str << "����� ���." << endl;
    }

    ch->send_to( str );
}

void Lover::del( Character* ch, DLString arguments)
{
    DLString name = arguments.getOneArgument( );
    std::basic_ostringstream<char> str;
    XMLAttributes *attributes;
    XMLAttributeLovers::Pointer pointer;

    attributes = &ch->getPC( )->getAttributes( );
    XMLAttributes::iterator ipos = attributes->find( "lovers" );
	
    if (ipos == attributes->end( ) ||
	(pointer = ipos->second.getDynamicPointer<XMLAttributeLovers>( ))->lovers.empty( )) 
    {	
	str << "�� ���� ������ ���������� � ��� ����!" << endl;
    }
    else {
	name.toLower( ).upperFirstCharacter( );

	if (pointer->lovers.isPresent( name )) {
	    XMLAttributeMarriage::Pointer mattr;
	    
	    mattr = attributes->findAttr<XMLAttributeMarriage>( "marriage" );

	    if (mattr && mattr->spouse.getValue( ) == name) {
		str << "������ ������ ������� ����." << endl;
	    } else {
		pointer->lovers.erase( name );
		str << "���� ������ ������ �� �������� ��� ���� " << name << "." << endl;
	    }
	}
	else 
	    str << "�� � ��� �� ������� �������� � " << name << ". " << endl;	
    }
    
    ch->send_to( str );
}

void Lover::usage( Character* ch ) 
{
    std::basic_ostringstream<char> str;
    
    str << "{lR��������� ������{lElover list{lx - �������� ������ ����������" << endl
        << "{lR��������� ��������{lElover add{lx <������> - ������ ���� ������ ����-��" << endl
	<< "{lR��������� �������{lElover del{lx <������> - ������� ����-�� �� ������ ����������" << endl;

    ch->send_to( str );
}

