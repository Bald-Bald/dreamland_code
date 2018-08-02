
/* $Id: mkey.cpp,v 1.1.2.9.6.5 2009/09/06 21:48:28 rufina Exp $
 *
 * ruffina, 2004
 */

#include "mkey.h"

#include "logstream.h"
#include "class.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "behavior_utils.h"
#include "pcharactermanager.h"
#include "arg_utils.h"
#include "act.h"
#include "merc.h"
#include "handler.h"
#include "mercdb.h"
#include "def.h"

/*-------------------------------------------------------------------------
 * 'mkey' command
 *------------------------------------------------------------------------*/
COMMAND(MKey, "mkey")
{
    DLString arguments = constArguments;
    DLString cmd = arguments.getOneArgument( );

    if (cmd.empty( )) {
	usage( ch );
	return;
    }
    
    if (ch->is_immortal( )) {
	if (arg_oneof( cmd, "give", "grant", "����" )) {
	    doGrant( ch, arguments );
	    return;
	}
	else if (arg_oneof( cmd, "remove", "�������" )) {
	    doRemove( ch, arguments );
	    return;
	}
	else if (arg_is_show( cmd )) {
	    doShow( ch, arguments );
	    return;
	}
    }
    
    if (arg_is_list( cmd ) || arg_oneof( cmd, "buy", "������" )) {
	MansionKeyMaker::Pointer maker;

	maker = find_people_behavior<MansionKeyMaker>( ch->in_room );

	if (!maker) {
	    ch->send_to( "����� ��� ��������.\r\n" );
	    return;
	}

	if (ch->is_npc( )) {
	    ch->send_to( "�� ���������.\r\n" );
	    return;
	}
	
	if (arg_is_list( cmd ))
	    maker->doList( ch->getPC( ) );
	else
	    maker->doBuy( ch->getPC( ), arguments );

	return;
    }
	
    usage( ch );
}

void MKey::doRemove( Character *ch, DLString &arguments ) 
{
    int vnum;
    PCMemoryInterface *pci;
    DLString name = arguments.getOneArgument( );

    if (name.empty( )) {
	usage( ch );
	return;
    }

    try {
	vnum = arguments.getOneArgument( ).toInt( );
    } catch (const ExceptionBadType& e) {
	ch->send_to( "������������ vnum �����.\r\n" );
	return;
    }

    if ( (pci = PCharacterManager::find( name )) == 0) {
	ch->send_to( "������������ ���.\r\n" );
	return;
    }
    
    XMLAttributes *attributes = &pci->getAttributes( );
    XMLAttributeMansionKey::Pointer attr = attributes->getAttr<XMLAttributeMansionKey>( "mkey" );
    XMLVectorBase<XMLInteger>::iterator i;

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++)
	if (i->getValue( ) == vnum) {
	    attr->keys.erase( i );
	    PCharacterManager::saveMemory( pci );
	    ch->send_to( "Ok.\r\n" );
	    return;
	}
    
    ch->send_to( "����� ���� �� ������.\r\n" );
}

void MKey::doGrant( Character *ch, DLString &arguments ) 
{
    int vnum;
    PCMemoryInterface *pci;
    DLString name = arguments.getOneArgument( );

    if (name.empty( )) {
	usage( ch );
	return;
    }

    try {
	vnum = arguments.getOneArgument( ).toInt( );
    } catch (const ExceptionBadType& e) {
	usage( ch );
	return;
    }

    if (get_obj_index( vnum ) == 0) {
	ch->send_to( "������ ����� �� ����������.\r\n" );
	return;
    }

    if ( (pci = PCharacterManager::find( name )) == 0) {
	ch->send_to( "������������ ���.\r\n" );
	return;
    }
    
    XMLAttributes *attributes = &pci->getAttributes( );
    XMLAttributeMansionKey::Pointer attr = attributes->getAttr<XMLAttributeMansionKey>( "mkey" );
    XMLVectorBase<XMLInteger>::iterator i;

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++)
	if (i->getValue( ) == vnum) {
	    ch->send_to( "����� ���� � ���� ��� ����.\r\n" );
	    return;
	}
    
    attr->keys.push_back( vnum );
    PCharacterManager::saveMemory( pci );
    ch->send_to( "Ok.\r\n" );
}

void MKey::doShow( Character *ch, DLString &arguments ) 
{
    XMLAttributes *attributes;
    XMLAttributeMansionKey::Pointer attr;
    XMLVectorBase<XMLInteger>::iterator i;
    PCMemoryInterface *pci;
    DLString name = arguments.getOneArgument( );

    if ( (pci = PCharacterManager::find( name )) == 0) {
	ch->send_to( "������������ ���.\r\n" );
	return;
    }
    
    attributes = &pci->getAttributes( );
    attr = attributes->findAttr<XMLAttributeMansionKey>( "mkey" );

    if (!attr) {
	ch->send_to( "������ �� �������.\r\n" );
	return;
    }

    ch->printf( "%s ������� ������ �������: \r\n", pci->getName( ).c_str( ) );

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++) {
	int vnum = i->getValue( );
	OBJ_INDEX_DATA *pKeyIndex = get_obj_index( vnum );

	if (!pKeyIndex) {
	    LogStream::sendError( ) << "Wrong key vnum " << vnum << " for character " << ch->getName( ) << endl;
	    continue;
	}
	
	ch->printf( "[%-4d] %-25s [%s]\r\n", 
		    vnum, 
		    russian_case( pKeyIndex->short_descr, '1' ).c_str( ),
		    pKeyIndex->name );
    }
}


void MKey::usage( Character *ch ) 
{
    std::basic_ostringstream<char> buf;

    buf << "{W{lR����� ������{lEmkey list{lx{w" << endl
        << "     - �������� ������ ����� ������" << endl
        << "{W{lR����� ������{lEmkey buy{lx{w <��� �����>" << endl
	<< "     - ���������� ����" << endl;
    
    if (ch->is_immortal( )) 
	buf << "{W{lR����� ��������{lEmkey show{lx{w <victim>" << endl
	    << "     - �������� ������ ������ ������" << endl
	    << "{W{lR����� ����{lEmkey give{lx{w <victim> <key vnum>" << endl
	    << "     - ���� ���� � �������� ������ ������" << endl
	    << "{W{lR����� �������{lEmkey remove{lx{w <victim> <key vnum>" << endl
	    << "     - ������� ����" << endl;

    ch->send_to( buf );
}

/*-------------------------------------------------------------------------
 * MansionKeyMaker 
 *------------------------------------------------------------------------*/
void MansionKeyMaker::toStream( Character *client, ostringstream &buf ) 
{
    XMLAttributeMansionKey::Pointer attr;
    XMLVectorBase<XMLInteger>::iterator i;
    
    if (client->is_npc( ))
	return;

    attr = client->getPC( )->getAttributes( ).findAttr<XMLAttributeMansionKey>( "mkey" );
    
    if (!attr) 
	return;

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++) {
	int vnum = i->getValue( );
	OBJ_INDEX_DATA *pKeyIndex = get_obj_index( vnum );

	if (!pKeyIndex) 
	    LogStream::sendError( ) << "Wrong key vnum " << vnum << " for character " << client->getName( ) << endl;
	else	
	    buf << "     * " << russian_case( pKeyIndex->short_descr, '1' )
		<< " ({c" << pKeyIndex->name << "{x)" << endl;
    }
}

void MansionKeyMaker::msgListEmpty( Character *client ) 
{
    say_act( client, getKeeper( ), "� ���� ��� ������ �� �� ������ ����." );
}

void MansionKeyMaker::msgListBefore( Character *client ) 
{
    tell_dim( client, getKeeper( ), "� ���� ���������� ��� ���� ����� �����: " );
}

void MansionKeyMaker::msgListAfter( Character *client ) 
{
    client->send_to( "\r\n" );
    tell_dim( client, getKeeper( ), "�� ���� ������ � �������� $n4.", price->toString( client ).c_str( ) );
}

void MansionKeyMaker::msgArticleNotFound( Character *client ) 
{
}

bool MansionKeyMaker::canServeClient( Character * )
{
    return true;
}

void MansionKeyMaker::msgListRequest( Character *client )
{
    act( "$c1 ������ $C4 �������� ������ ������.", client, 0, getKeeper( ), TO_ROOM );
    act( "�� ������� � $C4 �������� ������ ������.", client, 0, getKeeper( ), TO_CHAR );
}

void MansionKeyMaker::msgBuyRequest( Character *client )
{
}

void MansionKeyMaker::msgArticleTooFew( Character *, Article::Pointer )
{
}

Article::Pointer 
MansionKeyMaker::findArticle( Character *client, DLString &arg )
{
    MansionKeyArticle::Pointer article;
    int vnum ;
    
    if (client->is_npc( ))
	return article;

    if (!( vnum = findKeyVnum( client->getPC( ), arg ) ))
	return article;

    article.construct( );
    article->setPrice( price );
    article->setVnum( vnum );
    return article;
}

int MansionKeyMaker::findKeyVnum( PCharacter *client, const DLString& arg ) 
{
    XMLVectorBase<XMLInteger>::iterator i;
    XMLAttributeMansionKey::Pointer attr;

    attr = client->getAttributes( ).findAttr<XMLAttributeMansionKey>( "mkey" );

    if (!attr || attr->keys.empty( )) {
	tell_act( client, getKeeper( ), "������, $c1, �� ���� �� ����������� �� ������ �����. " );
	return 0;
    }

    if (arg.empty( )) {
	tell_act( client, getKeeper( ), "����� ������ ���� �� ������ ������?" );
	return 0;
    }

    for (i = attr->keys.begin( ); i != attr->keys.end( ); i++) {
	OBJ_INDEX_DATA *pKeyIndex = get_obj_index( i->getValue( ) );

	if (!pKeyIndex)
	    continue;

	if (!is_name( arg.c_str( ), pKeyIndex->name )) 
	    continue;
	
	return i->getValue( );
    }

    tell_act( client, getKeeper( ), "������, $c1, �� ���� �� ����������� ���� � ����� ������." );
    return 0;
}

/*-------------------------------------------------------------------------
 * MansionKeyArticle
 *------------------------------------------------------------------------*/
void MansionKeyArticle::purchase( Character *client, NPCharacter *maker, const DLString &, int ) 
{
    Object *key;
    
    if (!price->canAfford( client )) {
	tell_act( client, maker, 
	          "� ���� ������������ $n2, ����� �������� ��� ������, $c1.", 
	          price->toCurrency( ).c_str( ) );
	return;
    }
    
    price->deduct( client );
    key = create_object( get_obj_index( vnum ), 1 );
    obj_to_char( key, client );

    act( "$C1 ������� ���� $o4.", client, key, maker, TO_CHAR );
    act( "$C1 ������� $c3 $o4." , client, key, maker, TO_ROOM );
}

bool MansionKeyArticle::available( Character *client, NPCharacter *maker ) const
{
    return true;
}

int MansionKeyArticle::getQuantity( ) const
{
    return 1;
}
