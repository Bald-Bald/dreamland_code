/* $Id$
 *
 * ruffina, 2004
 */
#include <sstream>

#include "class.h"
#include "regexp.h"

#include "mysocial.h"
#include "pcharacter.h"
#include "comm.h"
#include "arg_utils.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"

/*----------------------------------------------------------------------
 * 'mysocial' command
 *---------------------------------------------------------------------*/
COMMAND(MySocial, "mysocial")
{
    DLString arg = constArguments, cmd, name;
    XMLAttributeCustomSocials::Pointer attr;

    if (ch->is_npc( ))
	return;
	
    cmd = arg.getOneArgument( );
    name = arg.getOneArgument( );
    
    if (cmd.empty( )) {
	usage( ch );
	return;
    }

    attr = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeCustomSocials>( "socials" );

    if (arg_is_list( cmd ))
	doList( ch, attr );
    else if (arg_is_help( cmd ))
	usage( ch );
    else if (name.empty( )) 
	ch->println( "����� ��� �������." );
    else if (arg_oneof( cmd, "del", "�������" ))
	doDelete( ch, attr, name );
    else if (arg_is_show( cmd ))
	doShow( ch, attr, name );
    else if (arg.empty( )) 
	ch->println( "�����, ����� ��� ��������� �������� � ���. ������ '{lR������ ?{lEmysoc help{lx'." );
    else {
	CustomSocial::Pointer social;
	XMLAttributeCustomSocials::iterator i = attr->find( name );
	
	if (i == attr->end( )) {
	    social.construct( );
	    (**attr)[name] = social;
	}
	else
	    social = i->second;

	social->setName( name );
	
	if (arg_oneof( cmd, "rus", "���" ))
	{
	    social->setRussianName( arg );
	}
	else if (arg_oneof( cmd, "noarg_other", "�������_������" ))
	{
	    if (hasMeOnly( ch, arg )) 
		social->setNoargOther( arg );
	    else
		return;
	}
	else if (arg_oneof( cmd, "noarg_me", "�������_�" ))
	{
	    if (hasNoVariables( ch, arg ))
		social->setNoargMe( arg );
	    else
		return;
	}
	else if (arg_oneof( cmd, "auto_me", "������_�" ))
	{
	    if (hasNoVariables( ch, arg ))
		social->setAutoMe( arg );
	    else
		return;
	}
	else if (arg_oneof( cmd, "auto_other", "������_������" ))
	{
	    if (hasMeOnly( ch, arg ))
		social->setAutoOther( arg );
	    else
		return;
	}
	else if (arg_oneof( cmd, "arg_victim", "������_������" ))
	{
	    if (hasMeOnly( ch, arg ))
		social->setArgVictim( arg );
	    else
		return;
	}
	else if (arg_oneof( cmd, "arg_me", "������_�" ))
	{
	    if (hasVictOnly( ch, arg ))
		social->setArgMe( arg );
	    else
		return;
	}
	else if (arg_oneof( cmd, "arg_other", "������_������" ))
	{
	    if (hasBoth( ch, arg ))
		social->setArgOther( arg );
	    else
		return;
	}
	else {
	    ch->println("��� ������ ���� ���������. ������ '{lR������ ?{lEmysoc help{lx'.");
	    return;
	}

	ch->send_to("Ok.\r\n");
    }
}

bool MySocial::hasMeOnly( Character *ch,  const DLString &arg )
{
    static RegExp pat( "^\\$c[1-6] [^\\$]*$", true );
    
    if (!pat.match( arg )) {
	ch->send_to( "��������� ������ ���������� � ������ ����� (� ������ ������) � �� ��������� ������ ����������. ��������� $c1, $c2...$c6.\r\n" );
	return false;
    }
    else 
	return true;
}

bool MySocial::hasNoVariables( Character *ch, const DLString &arg )
{
    static RegExp pat( "^[^\\$]*$" );

    if (!pat.match( arg )) {
	ch->send_to( "������� ����� ��������! � ���� ��������� ������ ������������ ����������.\r\n" );
	return false;
    }

    return true;
}

bool MySocial::hasVictOnly( Character *ch, const DLString &arg )
{
    static RegExp pat( "^.*\\$C[1-6].*$|"
                       "^[^\\$]*$", true );
    
    if (!pat.match( arg )) {
	ch->send_to( "� ���� ��������� ����� ����������� ������ ��� ������ � ������ ������ ($C1, .. $C6).\r\n" );
	return false;
    }

    return true;
}

bool MySocial::hasBoth( Character *ch, const DLString &arg )
{
    static RegExp pat( "^\\$c[1-6] |"
                       "^\\$c[1-6] [^\\$]*$|"
		       "^\\$c1[1-6] .*\\$C[1-6][^\\$]*$", true );
    
    if (!pat.match( arg )) {
	ch->println( "��������� ������ ���������� � ������ ����� � ������ ������ ($c1, .. $c6) \r\n"
	             "� ����� ���� ����� ��������� ��� ������ ��� ������ ($C1, ... $C6)." ); 
	return false;
    }
    else 
	return true;
}


void MySocial::doList( Character *ch, XMLAttributeCustomSocials::Pointer attr )
{
    XMLAttributeCustomSocials::iterator i;
    ostringstream buf;
    
    if (attr->empty( )) {
	act_p("�� ���� �� �������$g��|�|�� �� ������ ������������ �������.", ch, 0, 0, TO_CHAR, POS_DEAD);
	return;
    }
    
    bool fRus = ch->getConfig( )->rucommands;

    buf << "{W----------------+--------------------------------------------------------------{x" << endl;
      
    for (i = attr->begin( ); i != attr->end( ); i++) {
	CustomSocial::Pointer c = i->second;
	
	buf << dlprintf( "{c%-14s{x | %-11s ",
	                  c->getName( ).c_str( ),
			  c->getRussianName( ).c_str( ) ) << endl;

	if (!c->getNoargMe( ).empty( ))
	    buf << dlprintf( "%-14s | ", fRus ? "�������_�" : "noarg_me" )
		<< c->getNoargMe( ) << endl;
	if (!c->getNoargOther( ).empty( ))
	    buf << dlprintf( "%-14s | ", fRus ? "�������_������" : "noarg_other" )
		<< c->getNoargOther( ) << endl;
	if (!c->getArgOther( ).empty( ))
	    buf << dlprintf( "%-14s | ", fRus ? "������_������" : "arg_other" )
		<< c->getArgOther( ) << endl;
	if (!c->getArgMe( ).empty( ))
	    buf << dlprintf( "%-14s | ", fRus ? "������_�" : "arg_me" )
		<< c->getArgMe( ) << endl;
	if (!c->getArgVictim( ).empty( ))
	    buf << dlprintf( "%-14s | ", fRus ? "������_������" : "arg_victim" )
		<< c->getArgVictim( ) << endl;
	if (!c->getAutoMe( ).empty( ))
	    buf << dlprintf( "%-14s | ", fRus ? "������_�" : "auto_me" )
		<< c->getAutoMe( ) << endl;
	if (!c->getAutoOther( ).empty( ))
	    buf << dlprintf( "%-14s | ", fRus ? "������_������" : "auto_other" )
		<< c->getAutoOther( ) << endl;

	buf << "{W----------------+--------------------------------------------------------------{x" << endl;
    }

    page_to_char( buf.str( ).c_str( ), ch );
}

void MySocial::doDelete( Character *ch, XMLAttributeCustomSocials::Pointer attr, const DLString &name )
{
    XMLAttributeCustomSocials::iterator i;

    i = attr->find( name );

    if (attr->end( ) == i)
	ch->send_to( "������ � ����� ������ �� ������.\r\n" );
    else {
	attr->erase( i );
	ch->send_to( "Ok.\r\n" );
    }
}

void MySocial::doShow( Character *ch, XMLAttributeCustomSocials::Pointer attr, const DLString &name )
{
    ostringstream buf;
    CustomSocial::Pointer c;
    XMLAttributeCustomSocials::iterator i = attr->find( name );
    
    if (i == attr->end( )) {
	ch->send_to( "������ � ����� ������ �� ������.\r\n" );
	return;
    }

    c = i->second;

    buf << "{c" << c->getName( ) << "{x";

    if (!c->getRussianName( ).empty( ))
	buf << "({c" << c->getRussianName( ) << "{x)";

    buf << endl;

    buf << "{c��� ������������� ��� ���������{x: " << endl
	<< "�� ������� ({lR�������_�{lEnoarg_me{lx):  " << c->getNoargMe( ) << endl  
	<< "���������� ������ ({lR�������_������{lEnoarg_other{lx):  " << c->getNoargOther( ) << endl 
	<< endl
	<< "{c��� ������������� �� ����-��{x: " << endl
	<< "�� ������� ({lR������_�{lEarg_me{lx):  " << c->getArgMe( ) << endl
	<< "������ ������ ({lR������_������{lEarg_victim{lx):  " << c->getArgVictim( ) << endl 
	<< "��� ��������� ������ ({lR������_������{lEarg_other{lx):  " << c->getArgOther( ) << endl 
	<< endl
	<< "{c��� ������������� �� ������ ����{x: " << endl
	<< "�� ������� ({lR������_�{lEauto_me{lx):  " << c->getAutoMe( ) << endl 
	<< "���������� ������ ({lR������_������{lEauto_other{lx):  " << c->getAutoOther( ) << endl;

    page_to_char( buf.str( ).c_str( ), ch );
}


void MySocial::usage( Character *ch )
{
    ostringstream buf;
    
    buf << "{W{lR��������� ������{lEmysocial list{lx{w" << endl
	<< "      - �������� ������ ��������" << endl
	<< "{W{lR��������� ��������{lEmysocial show{lx{w <��������>" << endl
	<< "      - �������� ����������� �������" << endl
	<< "{W{lR��������� �������{lEmysocial del{lx{w <��������>" << endl
	<< "      - ������� ������" << endl
	<< "{W{lR��������� ���{lEmysocial rus{lx{w <��������> <�������>" << endl
	<< "      - ��������� ������� �������" << endl
	<< "{W{lR��������� �������_������|�������_�{lEmysocial noarg_other|noarg_me{lx{w <��������> <������>" << endl
	<< "      - ������ ������, ������� ������������ ��� �������� ����" << endl
	<< "{W{lR��������� ������_������|������_�{lEmysocial auto_other|auto_me{lx{w <��������> <������>" << endl
	<< "      - ������ ������, ������� ������������ �� ������ ����" << endl
	<< "{W{lR��������� ������_������|������_�|������_������{lEmysocial arg_other|arg_me|arg_victim{lx{w <��������> <������>" << endl
	<< "      - ������ ������, ������� ������������ �� ����-�� �������" << endl
	<< endl
	<< "����������� ������ � '{lR? ���������{lEhelp mysocial{lx'." << endl;

    ch->send_to( buf );
}

