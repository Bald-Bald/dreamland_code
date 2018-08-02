/* $Id$
 *
 * ruffina, 2004
 */
#include "lifeprice.h"
#include "pcharacter.h"

/*----------------------------------------------------------------------
 * LifePrice 
 *---------------------------------------------------------------------*/
const DLString LifePrice::LIFE_ONE = "������|��|��|��|��|��|�� ����|�|�|�|�|��|�";
const DLString LifePrice::LIFE_MANY = "������|��|��|��|��|���|�� ����|�|��|��|�|���|��";

DLString LifePrice::toCurrency( ) const
{
    return LIFE_MANY;
}

DLString LifePrice::toString( Character * ) const
{
    DLString str;
    int p = points.getValue( ) / Remorts::POINT_PER_LIFE;
    
    str << (p == 0 ? "���" : p == 1 ? "����" : DLString( p ))
        << " "
	<< (p == 0 ? LIFE_ONE.ruscase( '2' ) : p == 1 ? LIFE_ONE : LIFE_MANY);

    return str;
}

bool LifePrice::canAfford( Character *ch ) const
{
    if (ch->is_npc( ))
	return false;
    else
	return ch->getPC( )->getRemorts( ).points >= points.getValue( );
}

void LifePrice::induct( Character *ch ) const
{
    if (!ch->is_npc( ))
	ch->getPC( )->getRemorts( ).points += points.getValue( );
}

void LifePrice::deduct( Character *ch ) const
{
    if (!ch->is_npc( ))
	ch->getPC( )->getRemorts( ).points -= points.getValue( );
}

void LifePrice::toStream( Character *ch, ostringstream &buf ) const
{
    buf << toString( ch ).ruscase( '4' );
}

