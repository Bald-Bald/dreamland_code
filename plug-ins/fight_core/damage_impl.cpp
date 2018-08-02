/* $Id: damage_impl.cpp,v 1.1.2.6 2009/11/08 17:46:27 rufina Exp $
 * 
 * ruffina, 2004
 */
#include "damage_impl.h"
#include "russianstring.h"

#include "skillreference.h"
#include "spell.h"
#include "skillgroup.h"
#include "character.h"
#include "merc.h"
#include "def.h"

GSN(resistance);
GSN(mental_knife);
GSN(dragons_breath);
GROUP(draconian);

/*-----------------------------------------------------------------------------
 * Self Damage
 *---------------------------------------------------------------------------*/
SelfDamage::SelfDamage( Character *ch, int dam_type, int dam ) : Damage( ch, ch, dam_type, dam )
{
}

void SelfDamage::calcDamage( )
{
    protectRazer( );
}
    
/*-----------------------------------------------------------------------------
 * Raw Damage
 *---------------------------------------------------------------------------*/
RawDamage::RawDamage( Character *ch, Character *victim, int dam_type, int dam )
    : Damage( ch, victim, dam_type, dam )
{
}

void RawDamage::message( )
{
    if( ch == victim ) {
	msgRoom( "%^C1\6����", ch );
	msgChar( "��\5����" );
	return;
    } 

    if ( dam == 0 ) {
       msgRoom( "%^C1\6%C2", ch, victim);
       msgChar( "��\5%C2", victim);
    }
    else {
       msgRoom( "%^C1\6%C4", ch, victim );
       msgChar( "��\5%C4", victim );
    }

    msgVict( "%^C1\6����", ch );
}

bool RawDamage::canDamage( )
{
    return true;
}

/*-----------------------------------------------------------------------------
 * SkillDamage 
 *----------------------------------------------------------------------------*/
SkillDamage::SkillDamage( Character *ch, Character *victim, 
		          int sn, int dam_type, int dam, bitstring_t dam_flag )
	    : Damage( ch, victim, dam_type, dam, dam_flag )
{
    this->sn = sn;
}

int SkillDamage::msgNoSpamBit( )
{
    return CONFIG_SKILLSPAM;
}

void SkillDamage::message( )
{
    const RussianString &attack = skillManager->find(sn)->getDammsg( );

    if (immune) {
	if (ch == victim) {
	    msgRoom("%1$^O1 %2$C2 ������%1$G���|��|��� ������ %2$P4 ���%2$G���|���|��|��", &attack, ch);
	    msgChar("���� �������, � ���� ��������� � �����");
	}
	else {
	    msgRoom("%1$^O1 %2$C2 ������%1$G���|��|��� ������ %3$C2", &attack, ch, victim);
	    msgChar("%1$^T1 %1$O1 ������%1$G���|��|��� ������ %2$C2", &attack, victim);
	    msgVict("������ ���� %2$O1 %1$C2 ������%2$G���|��|���", ch, &attack);
	}
    }
    else {
	if (ch == victim) {
	    msgRoom( "%1$^O1 %2$C2\6����", &attack, ch );
	    msgChar( "%1$^T1 %1$O1\6����", &attack );
	}
	else {
	    if ( dam == 0 )
	    {
		msgRoom( "%1$^O1 %2$C2\6%3$C2", &attack, ch, victim );
		msgChar( "%1$^T1 %1$O1\6%2$C2", &attack, victim );
	    }
	    else {
		msgRoom( "%1$^O1 %2$C2\6%3$C4", &attack, ch, victim );
		msgChar( "%1$^T1 %1$O1\6%2$C4", &attack, victim );
	    }
	    msgVict( "%1$^O1 %2$C2\6����", &attack, ch );
	}
    }
}

/*
 * 'resistance' reduces 50% of non-magical damage,
 * with the historical exception for 'mental knife' spell
 */
void SkillDamage::protectResistance( )
{
    if (!victim->isAffected(gsn_resistance))
	return;

    if (sn == gsn_mental_knife) {
	dam -= victim->applyCurse( dam * 2 / 5 );
	return;
    }

    if (sn == gsn_dragons_breath) {
	return;
    }

    Skill *skill = skillManager->find( sn );
    Spell::Pointer spell = skill->getSpell( );

    if (!spell 
	    || !spell->isCasted( ) 
	    || spell->isPrayer( ch )
	    || skill->getGroup( ) == group_draconian)
    {
	dam -= victim->applyCurse( dam / 2 );
	return;
    }
}

