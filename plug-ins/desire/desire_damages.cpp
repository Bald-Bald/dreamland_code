/* $Id$
 *
 * ruffina, 2004
 */
#include "desire_damages.h"
#include "character.h"

#include "damageflags.h"
#include "merc.h"
#include "def.h"

HungerDamage::HungerDamage( Character *ch, int dam )
    : SelfDamage( ch, DAM_NONE, dam )
{
}

void HungerDamage::message( )
{
    msgRoom( "�� ������ %C1\6����", ch );
    msgChar( "�� ������ ��\5����" );
}

ThirstDamage::ThirstDamage( Character *ch, int dam )
    : SelfDamage( ch, DAM_NONE, dam )
{
}

void ThirstDamage::message( )
{
    msgRoom( "�� ����� %C1\6����", ch );
    msgChar( "�� ����� ��\5����" );
}


