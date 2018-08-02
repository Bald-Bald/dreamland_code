/* $Id: xmlattributecards.cpp,v 1.1.2.7.6.1 2007/06/26 07:09:27 rufina Exp $
 *
 * ruffina, 2005
 */

#include "xmlattributecards.h"

#include "class.h"

#include "pcharacter.h"
#include "merc.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

/*--------------------------------------------------------------------------
 * XMLAttributeCards
 *--------------------------------------------------------------------------*/

const XMLAttributeCards::CardLevelFace XMLAttributeCards::levelFaces [] =
{
    { "�������|�|�|�|�|��|�",        SEX_FEMALE },
    { "������|�|�|�|�|��|�",         SEX_FEMALE },
    { "��������|�|�|�|�|��|�",       SEX_FEMALE },
    { "������|�|�|�|�|��|�",         SEX_FEMALE },
    { "������|�|�|�|�|��|�",         SEX_FEMALE },
    { "���|��|���|���|���|����|���", SEX_MALE   },
    { "���|�|�|�|�|��|�",            SEX_FEMALE },
    { "�����|�|�|�|�|��|�",          SEX_MALE   },
    { "���||�|�|�|��|�",             SEX_MALE   },
};

const XMLAttributeCards::CardSuitFace XMLAttributeCards::suitFaces [] =
{
    { "����|�|��|��|��|��|��", "������|��|���|���|���|��|��", "������|��|��|��|��|��|��" },
    { "���|�|||||",    "�����|��|���|���|���|��|��",  "�����|��|��|��|��|��|��"  },
    { "����|�|||||",   "������|��|���|���|���|��|��", "������|��|��|��|��|��|��" },
    { "�����",  "������|��|���|���|���|��|��", "������|��|��|��|��|��|��" },
};

int XMLAttributeCards::getMaxLevel( )
{
    return sizeof(levelFaces) / sizeof(*levelFaces) - 1;
}

int XMLAttributeCards::getRandomSuit( ) 
{
    return number_bits( 2 );
}

XMLAttributeCards::XMLAttributeCards( ) : level( -1 ), suit( -1 )
{
}

XMLAttributeCards::~XMLAttributeCards( ) 
{
}

bool XMLAttributeCards::isTrump( ) const
{
    return getSuit( ) == getTrump( );
}
    
DLString XMLAttributeCards::getFace( char needcase ) const
{
    DLString face, suit;
    
    if (getLevelFace( ).gender == SEX_MALE)
	suit = russian_case( getSuitFace( ).male, needcase );
    else
	suit = russian_case( getSuitFace( ).female, needcase );
    
    face = suit + " " + russian_case( getLevelFace( ).name, needcase );
    return face;
}

bool XMLAttributeCards::handle( const DeathArguments &args )
{
    PCharacter *pkiller;
    Pointer card;
    
    if (!args.killer || args.killer->is_npc( ) || args.killer == args.pch)
	return false;

    pkiller = args.killer->getPC( );

    card = pkiller->getAttributes( ).getAttr<XMLAttributeCards>( "cards" );
    
    if (card->level < level || isTrump( )) {
	if (card->level < getMaxLevel( )) {
	    card->level++;

	    if (card->suit < 0)
		card->suit = getRandomSuit( );

	    act( "{c�� ���$g��|�|�� $t �� ������.{x", pkiller, getFace( '4' ).c_str( ), 0, TO_CHAR );
	    act( "{c������ �� $t!{x", pkiller, card->getFace( '1' ).c_str( ), 0, TO_CHAR );
	}
    
	level--;

	if (level >= 0)
	    args.pch->printf( "{c�� ����������� %s.{x\r\n", getFace( '5' ).c_str( ) );
	else {
	    args.pch->send_to( "{c�� ��������� �� ������!{x\r\n" );
	    args.pch->getAttributes( ).eraseAttribute( "cards" );
	    return false;
	}
    }

    return false;
}

int XMLAttributeCards::getTrump( ) 
{
    static const int PRIME1 = 37;
    static const int PRIME2 = 1048583;
    unsigned int h;

    h = time_info.day;
    h = h * PRIME1 ^ time_info.year;
    h = h * PRIME1 ^ time_info.month;
    h %= PRIME2;
    h %= 4;

    return h;
}

