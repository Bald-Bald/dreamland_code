/* $Id$
 *
 * ruffina, 2004
 */
#include <iomanip>

#include "healer.h"
#include "behavior_utils.h"
#include "attract.h"
#include "occupations.h"
#include "commandtemplate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "clanreference.h"

#include "merc.h"
#include "handler.h"
#include "magic.h"
#include "act.h"
#include "mercdb.h"
#include "def.h"

CLAN(battlerager);

/*------------------------------------------------------------------------
 * Healer 
 *-----------------------------------------------------------------------*/
Healer::Healer( ) : healPets( false )
{
}

int Healer::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) | (1 << OCC_HEALER);
}

bool Healer::canServeClient( Character *client )
{
    if (client->is_npc( ) && !healPets) {
	say_act( client, ch, "� �� ���� �������� ��������." );
	return false;
    }

    if (IS_GHOST( client )) {
	say_act( client, getKeeper( ), "� ������� ����, � �� ����, $c1. ������ ����� - ����� � ���������." );
	return false;
    }

    if (getKeeper( )->fighting) {
	say_act( client, getKeeper( ), "������� �������, $c1, ��� ������ �� �� ����." );
	return false;
    }
    
    if ((!client->is_npc( ) && client->getClan( ) == clan_battlerager)
	|| (client->is_npc( ) && client->master && client->master->getClan( ) == clan_battlerager)) 
    {
	act( "$C1 ������������ ������ ������� � �����, ����� �� $c4.", client, 0, getKeeper( ), TO_NOTVICT );
        client->send_to("����������: �� BattleRager, � �� ���������� ���!\n\r");
        return false;
    }

    return true;
}

void Healer::msgListEmpty( Character *client )
{
    say_act( client, getKeeper( ), "������, $c1, � ����� �� ����� ���� ������." );
}

void Healer::msgListRequest( Character *client ) 
{
    act( "$c1 ������ $C4 ����������, ����� ������� $E ����� ��������.", client, 0, getKeeper( ), TO_NOTVICT );
    act( "�� ������� $C4 ����������, ����� ������� $E ����� ��������.", client, 0, getKeeper( ), TO_CHAR );
}

void Healer::msgListBefore( Character *client ) 
{
    tell_dim( client, getKeeper( ), "� ��������� ��������� ����������: " );
}

void Healer::msgListAfter( Character *client )
{
    tell_dim( client, getKeeper( ), "��������� 'heal <��� ����������>', � � ������ ���� �� ��������� ����." );
}

void Healer::msgArticleNotFound( Character *client ) 
{
    say_act( client, getKeeper( ), "� �� ���� ������ ����������, $c1." );
    tell_act( client, getKeeper( ), "��������� 'heal', ����� ������� ������ ��������� ��� ����������." );
}

void Healer::msgArticleTooFew( Character *client, Article::Pointer )
{
    say_act( client, getKeeper( ), "� ���� �������� ������ ���� ������� �� ���� ���." );    
}

void Healer::msgBuyRequest( Character *client )
{
    act( "�� ������� $C4 � ������.", client, 0, getKeeper( ), TO_CHAR );
    act( "$c1 ������ $C4 � ������.", client, 0, getKeeper( ), TO_NOTVICT );
}

/*------------------------------------------------------------------------
 * HealService
 *-----------------------------------------------------------------------*/
void HealService::toStream( Character *client, ostringstream &buf ) const
{
    buf << "  {c" << setiosflags( ios::left ) << setw( 11 ) << name << "{x: "
	<< setw( 30 ) << descr << "   " << setiosflags( ios::right ) << setw( 8 );
	
    price->toStream( client, buf );

    buf << resetiosflags( ios::right ) << endl;
}

bool HealService::visible( Character * ) const
{
    return true;
}

bool HealService::available( Character *, NPCharacter * ) const
{
    return true;
}

bool HealService::matches( const DLString &argument ) const
{
    return !argument.empty( ) && argument.strPrefix( name.getValue( ) );
}

int HealService::getQuantity( ) const
{
    return 1;
}

void HealService::purchase( Character *client, NPCharacter *healer, const DLString &, int )
{
    Character *payer;

    payer = client->is_npc( ) && client->master ? client->master : client;

    if (!price->canAfford( payer )) {
	if (payer == client)
	    say_act( client, healer, "� ���� �� ������� $n2 �������� ��� ������.", price->toCurrency( ).c_str( ) );
	else
	    say_act( client, healer, "� ������ ������� �� ������� $n2 �������� ��� ������.", price->toCurrency( ).c_str( ) );
	return;
    }
    
    price->deduct( payer );
    payer->setWaitViolence( 1 );
    heal( client, healer );
}

/*------------------------------------------------------------------------
 * SpellHealService 
 *-----------------------------------------------------------------------*/
void SpellHealService::heal( Character *client, NPCharacter *healer )
{
    ::spell( spell, healer->getModifyLevel( ), healer, client, FSPELL_VERBOSE );
}

/*------------------------------------------------------------------------
 * ManaHealService 
 *-----------------------------------------------------------------------*/
void ManaHealService::heal( Character *client, NPCharacter *healer )
{
    act( "$c1 �������� '$T'.", healer, 0, words.getValue( ).c_str( ), TO_ROOM );

    if (enhanced) {
	client->mana += 300;
	client->send_to( "�������� ����� ��������� ���� ����.\n\r" );
    }
    else {
	client->mana += dice( 2, 8 ) + healer->getModifyLevel( ) / 3;
	client->send_to( "�� �������� �������� �����.\n\r" );
    }

    client->mana = std::min( client->mana, client->max_mana );
}

/*------------------------------------------------------------------------
 * 'heal' command 
 *-----------------------------------------------------------------------*/
CMDRUN( heal )
{
    DLString argument = constArguments;
    Healer::Pointer healer;
    
    healer = find_attracted_mob_behavior<Healer>( ch, OCC_HEALER );

    if (!healer) {
	ch->send_to( "����� ������ ���� ��������.\r\n" );
	return;
    }

    if (ch->is_npc( ) && !ch->master) {
	ch->send_to( "������, ���� ����� ����������� �� �����.\r\n" );
	return;
    }

    if (argument.empty( )) 
	healer->doList( ch );
    else
	healer->doBuy( ch, argument );
}
