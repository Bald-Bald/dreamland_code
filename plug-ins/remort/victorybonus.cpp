/* $Id$
 *
 * ruffina, 2004
 */
#include "victorybonus.h"
#include "xmlattributestatistic.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*
 * Koschey
 */
void Koschey::greet( Character *victim )
{
    if (victim->is_npc( ))
	return;

}

bool Koschey::command( Character *victim, const DLString &cmdName, const DLString &cmdArgs )
{
    if (victim->is_npc( ))
	return false;

    if (cmdName == "buy") {
	doBuy( victim, cmdArgs.quote( ) );
	return true;
    }

    if (cmdName == "list") {
	doList( victim );
	return true;
    }

    if (cmdName == "sell" || cmdName == "value") {
	tell_dim( victim, ch, "��� ���� ������ ����� �� � ����." );
	return true;
    }
    
    return false;
}

bool Koschey::canServeClient( Character * )
{
    return true;
}

void Koschey::msgArticleTooFew( Character *client, Article::Pointer )
{
    say_act( client, ch, "� ������ ��������� - ������� ����." );
}

void Koschey::msgListEmpty( Character *client )
{
    say_act( client, ch, "������ ������, $c1, �������-���������." );
}

void Koschey::msgListAfter( Character *client )
{
    tell_dim( client, ch, "��� ���. ���� ��, �� ��� ����$g��|��|��, � ���� ���� ������ ����� �� ����!" );
    act( "$C1 ���-�� ��������� ����� ������ ����, ��������� � $c4.", client, 0, ch, TO_NOTVICT );
}

void Koschey::msgListBefore( Character *client )
{
    act( "$C1 ��������� �� ���� ������������ ������.", client, 0, ch, TO_CHAR );
    act( "$C1 ��������� �� $c4 ������������ ������.", client, 0, ch, TO_ROOM );
    act( "$C1 ��������� ������� ���������� '{g��� ����� ������� ���� ���������:{x'", client, 0, ch, TO_CHAR );
}

void Koschey::msgBuyRequest( Character *client )
{
    act( "$c1 ��������� � $C5.", client, 0, ch, TO_NOTVICT );
}

void Koschey::msgArticleNotFound( Character *client )
{
    act( "$C1 � ������ ���������� '{g$c1, �� ���� ����� ����$g��|��|�� ������?!{x'", client, 0, ch, TO_ALL );
}

/*
 * VictoryPrice
 */
const int VictoryPrice::COUNT_PER_LIFE = 500;
const DLString VictoryPrice::CURRENCY_NAME = "�����|�||��|�|���|��";

DLString VictoryPrice::toCurrency( ) const
{
    return CURRENCY_NAME;
}

DLString VictoryPrice::toString( Character * ) const
{
    DLString str;

    str << count << " " << GET_COUNT( count, "������", "������", "�����" );
    return str;
}

bool VictoryPrice::canAfford( Character *ch ) const
{
    XMLAttributeStatistic::Pointer attr;
    int avail;
    
    if (ch->is_npc( ))
	return false;
	
    attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeStatistic>( "questdata" );
    if (!attr)
	return false;

    avail = min( (int)(Remorts::MAX_BONUS_LIFES
                           - ch->getPC( )->getRemorts( ).size( )) * COUNT_PER_LIFE,
	         attr->getAllVictoriesCount( ) );

    return avail - attr->getVasted( ) >= count.getValue( );
}

void VictoryPrice::deduct( Character *ch ) const
{
    if (!ch->is_npc( )) {
	XMLAttributeStatistic::Pointer attr;
	
	attr = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeStatistic>( "questdata" );
	attr->setVasted( attr->getVasted( ) + count.getValue( ) );
    }
}

void VictoryPrice::induct( Character *ch ) const
{
}

void VictoryPrice::toStream( Character *ch, ostringstream &buf ) const
{
    buf << toString( ch );
}

