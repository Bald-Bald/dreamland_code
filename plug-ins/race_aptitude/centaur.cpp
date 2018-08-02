/* $Id$
 *
 * ruffina, 2004
 */
#include "skillreference.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"

#include "dreamland.h"
#include "onehit.h"
#include "damage_impl.h"
#include "act_move.h"
#include "magic.h"
#include "fight.h"
#include "handler.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(rear_kick);
GSN(enhanced_damage);

/*----------------------------------------------------------------------------
 * Rear kick 
 *---------------------------------------------------------------------------*/
class RearKickOneHit: public OneHit, public SkillDamage {
public:
    RearKickOneHit( Character *ch, Character *victim );
    
    virtual void init( );
    virtual void calcTHAC0( );
    virtual void calcDamage( );
    void damBase( );
};

RearKickOneHit::RearKickOneHit( Character *ch, Character *victim )
	    : Damage( ch, victim, 0, 0 ), OneHit( ch, victim ),
	      SkillDamage( ch, victim, gsn_rear_kick, 0, 0, DAMF_WEAPON )
{
}

void RearKickOneHit::init( )
{
    dam_type = attack_table[DAMW_HOOVES].damage;
    skill = 20 + ch->getSkill( sn );
}

void RearKickOneHit::damBase( )
{
    int ave, level = ch->getModifyLevel( );
    
         if (level >= 100) ave = level - 12;
    else if (level >= 40)  ave = level - 10;
    else if (level >= 35)  ave = level -  8;
    else if (level >= 30)  ave = level -  6;
    else if (level >= 25)  ave = level -  5;
    else                   ave = level;
    
    dam = ave * skill / 100;                   // as weapon with skill bonus
}

void RearKickOneHit::calcDamage( )
{
    damBase( ); 
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
    damApplyPosition( );
    dam = ( ch->getModifyLevel( ) < 50)
	? ( ch->getModifyLevel( ) / 10 + 1) * dam + ch->getModifyLevel( )
	: ( ch->getModifyLevel( ) / 10 ) * dam + ch->getModifyLevel( );
    damApplyDamroll( );

    OneHit::calcDamage( );
}

void RearKickOneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
    thacApplySkill( );
    thac0 -= 10 * (100 - gsn_rear_kick->getEffective( ch ));
}

/*
 * 'rear kick' skill command
 */
SKILL_DECL( rearkick );
BOOL_SKILL( rearkick )::run( Character *ch, Character *victim ) 
{
    if (!IS_AWAKE( victim ))
	return false;

    if (number_percent( ) > 33
	|| number_percent( ) >= gsn_rear_kick->getEffective( victim )) 
    {
	gsn_rear_kick->improve( victim, false, ch );
	return false;
    }
   
    try {
	RearKickOneHit( victim, ch ).hit( );
	gsn_rear_kick->improve( victim, true, ch );
	
	yell_panic( victim, ch,
		    "��������! ���-�� ������ ���� ������� �� ������!",
		    "��������! %1$^C1 �����%1$G��|�|�� ���� ������� �� ������!" );
    }
    catch (const VictimDeathException& e) {                                     
    }

    return true;
}

