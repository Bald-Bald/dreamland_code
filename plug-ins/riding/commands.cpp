/* $Id$
 *
 * ruffina, 2004
 */
#include "commands.h"
#include "commandtemplate.h"
#include "xmlattributetrust.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "skillreference.h"

#include "loadsave.h"
#include "move_utils.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(riding);

/*
 * 'mount' command
 */
CMDRUN( mount )
{
    XMLAttributeTrust::Pointer trust;
    Character *horse;
    int moveCost = 1;
    DLString arg, args = constArguments;

    if (IS_SET( ch->form, FORM_CENTAUR )) {
	ostringstream buf;
	
	if (ch->is_npc( ) || IS_AFFECTED(ch, AFF_CHARM)) {
	    ch->println( "�� �� � ��� �� ����." );
	    return;
	}
	
	trust = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeTrust>( "mount" );

	if (trust->parse( constArguments, buf )) 
	    ch->send_to( "������ �� ���� ������ " );
	    
	ch->println( buf.str( ) );
	return;
    }
    
    if (MOUNTED(ch)) {
	ch->println( "�� ��� ������." );
	return;
    }

    if (RIDDEN(ch)) {
	ch->pecho( "�� ���� ���%G�|���|� �������!", ch );
	return;
    }

    arg = args.getOneArgument( );

    if (( horse = get_char_room( ch, arg.c_str( ) ) ) == NULL) {
	ch->println( "���� �� ������ ��������?" );
	return;
    }
    
    if (MOUNTED(horse) || horse == ch) {
	ch->pecho( "�� �� � �����!" );
	return;
    }
    
    if (RIDDEN(horse)) {
	ch->pecho( "%1$^C1 ��� ������%1$G��|�|��.", horse );
	return;
    }
    
    if (!horse->is_npc( )) { /* pc-mounts like centaurs */
        if (!IS_SET(horse->form, FORM_CENTAUR)) {
            act("$c1 �������� ���������� ������ �� $C4.", ch, 0, horse, TO_NOTVICT);
            act("$c1 �������� ���������� ������ �� ����.", ch, 0, horse, TO_VICT);
            act("�� ��������� �������� $C4, �� ����� �� �������, ��� �� � $X ��������..", ch, 0, horse, TO_CHAR);
            return;
        }
	
	trust = horse->getPC( )->getAttributes( ).findAttr<XMLAttributeTrust>( "mount" );
	if (!trust || !trust->check( ch )) {
            act("$c1 �������� �������� $C4. $C1 ������ ������� �� $c4.", ch, 0, horse, TO_NOTVICT);
            act("$c1 �������� �������� ����, �� ������� ���� ������� ������, ���������������.", ch, 0, horse, TO_VICT);
            act("$C1 �� ������, ����� �� $Z ��������.", ch, 0, horse, TO_CHAR);
            return;
        }
    }
    else if (!ch->isCoder( )) { /* other rideable beasts */
	if (!IS_SET(horse->act, ACT_RIDEABLE)) {
            act("$c1 �������� ���������� ������ �� $C4, �� �������������.", ch, 0, horse, TO_NOTVICT);
	    ch->println("���� ��� ����� ������� �� ������������ ��� �������� ����.");
	    return;
	}
	
	if (horse->getModifyLevel( ) - ch->getModifyLevel( ) > 5) {
            act("$c1 �������� �������� $C4, �� ����� ���� �� �������.", ch, 0, horse, TO_NOTVICT);
	    ch->println("���� �� ������ ����� ���������� � ���� ��������.");
	    return;
	}
    }
    
    if ((horse->is_npc( ) || IS_AFFECTED(horse, AFF_CHARM))
	&& horse->master 
	&& horse->master != ch) 
    {
	ch->pecho("� %C2 ��� ���� ������, � ��� ���� - �� ��!", horse );
	return;
    }
    
    if (horse->position < POS_STANDING) {
	ch->pecho("%1$^C1 ����%1$G��|��|�� ��� ������ ������ �� ����.", horse );
	return;
    }

    if (ch->move < moveCost) {
	ch->println("� ���� �� ������� ��� ���� ������� ����.");
	return;
    }
    
    /* horrible XXX unless riding skills are available for all */
    if (horse->is_npc( ) 
	    && ((horse->getNPC( )->pIndexData->vnum >= 50000
	           && horse->getNPC( )->pIndexData->vnum <= 51000)
		|| (horse->getNPC( )->pIndexData->vnum >= 550
		   && horse->getNPC( )->pIndexData->vnum <= 560)))
    {
    }
    else if (horse->is_npc( ) 
	     && number_percent( ) > gsn_riding->getEffective( ch ) 
	     && !ch->isCoder( )) 
    {
	act( "���� �� ������� ���������� �������� $C4.", ch, 0, horse, TO_CHAR );
	act( "$c1 �������� �������� ����, �� ���������� ���� �� �������.", ch, 0, horse, TO_VICT );
	act( "$c1 �������� �������� $C4, �� ���������� ���� �� �������.", ch, 0, horse, TO_NOTVICT );
	
	ch->setWait( gsn_riding->getBeats( ) );
	gsn_riding->improve( ch, false );
	return;
    }

    ch->mount = horse;
    ch->riding = true;
    horse->mount = ch;
    horse->riding = false;

    act( "�� ������������ �� $C4.", ch, 0, horse, TO_CHAR );
    act( "$c1 ����������� ���� �� �����.", ch, 0, horse, TO_VICT );
    act( "$c1 ����������� �� ����� $C2.", ch, 0, horse, TO_NOTVICT );
    
    gsn_riding->improve( ch, true);

#if 0
    Object *obj;
    HorseSaddle::Pointer saddle;
    HorseBridle::Pointer bridle;
    Skill *ridingSkill = &*gsn_horseback_riding;
    if (!horse->is_npc( ) && horse->getRace( ) != race_centaur) {
	ch->println("�������� mlove.");
	return;
    }

    if (horse->is_npc( )) {
	Rideable::Pointer rideable;
	
	if (horse->getNPC( )->behavior)
	    rideable = horse->getNPC( )->behavior.getDynamicPointer<Rideable>( );

	if (!rideable) {
	    ch->println("���� ��� ����� ������� �� ������������ ��� �������� ����.");
	    return;
	}
	
	ridingSkill = rideable->getRidingSkill( );
    }

    if (abs( ch->size - horse->size ) > 1) {
	ch->println("������ ����-�� ����� ����������� �������.");
	return;
    }

    if (horse->is_npc( ) 
	&& horse->getModifyLevel( ) - ch->getModifyLevel( ) > ch->getModifyLevel / 10)
    {
	ch->println("���� �� ������ ����� ���������� � ���� ��������.");
	return;
    }

    if (!( obj = wear_horse->find( horse ) ) 
	|| !obj->behavior
	|| !( sadle = obj->behavior.getDynamicPointer<HorseSaddle>( ) ))
    {
	ch->pecho("��� �� ����������� ��������? �� %C6 ��� �����.", horse);
	return;
    }

    if (( obj = wear_head->find( horse ) )
	&& obj->behavior
	&& ( bridle = obj->behavior.getDynamicPointer<HorseBridle>( ) ))
    {
	if (bridle->isTethered( )) {
	    ch->pecho("���� ����� ��� ������ �������� %C4.", horse );
	    return;
	}
    }
    else if (horse->is_npc( )) {
	ch->pecho("��� �� ����������� ��������? �� %C6 ��� �������.", horse );
	return;
    }

    if ((horse->is_npc( ) || IS_AFFECTED(horse, AFF_CHARM))
	&& horse->master 
	&& horse->master != ch) 
    {
	ch->pecho("� %C2 ��� ���� ������, � ��� ���� - �� ��!", horse );
	return;
    }
    
    if (horse->position < POS_STANDING) {
	ch->pecho("%1$^C1 ����%1$G��|��|�� ��� ������ ������ �� ����.", horse );
	return;
    }

    if (ch->move < moveCost) {
	ch->println("� ���� �� ������� ��� ���� ������� ����.");
	return;
    }

    ch->move -= moveCost;
    ch->setWait( ridingSkill->getBeats( ) );
    
    if (number_percent( ) > gsn_carry_rider->getEffective( horse )) {
	act( "$C1 ���������, � �� �������.", ch, 0, horse, TO_CHAR );
	act( "$c1 �������� �������� ���� �� �����, �� �� ����������, � $e ������.", ch, 0, horse, TO_VICT );
	act( "$c1 �������� �������� �� ����� $C3, �� $e ��������� � $c1 ������.", ch, 0, horse, TO_NOTVICT );

	gsn_carry_rider->improve( horse, false );
	ch->position = POS_SITTING;
	return;
    }

    if (number_percent( ) > ridingSkill->getEffective( ch )) {
	act( "���� �� ������� ���������� �������� $C4.", ch, 0, horse, TO_CHAR );
	act( "$c1 �������� �������� ����, �� ����� ���� �� �������.", ch, 0, horse, TO_VICT );
	act( "$c1 �������� �������� $C4, �� ����� ���� �� �������.", ch, 0, horse, TO_NOTVICT );

	ridingSkill->improve( ch, false );
	return;
    }
    
    ch->mount = horse;
    ch->riding = true;
    horse->mount = ch;
    horse->riding = false;

    act( "�� ������������ �� $C4.", ch, 0, horse, TO_CHAR );
    act( "$c1 ����������� ���� �� �����.", ch, 0, horse, TO_VICT );
    act( "$c1 ����������� �� ����� $C2.", ch, 0, horse, TO_NOTVICT );

    ridingSkill->improve( ch, true );
#endif    
}

/*
 * 'dismount' command
 */
CMDRUN( dismount )
{
    /*
     * jump off the horse 
     */
    if (!ch->mount) {
	ch->println( "�� ��� �����, �� ��� ����� ������ ���!" );
	return;
    }
    
    if (MOUNTED(ch)) {
	act( "�� ������������ �� ����� $C2.", ch, 0, ch->mount, TO_CHAR );
	act( "$c1 ����������� � ����� �����.", ch, 0, ch->mount, TO_VICT );
	act( "$c1 ���������� � $C2.", ch, 0, ch->mount, TO_NOTVICT );
    }
    else {
	act( "�� ����������� $C4 �� �����.", ch, 0, ch->mount, TO_CHAR );
	act( "$c1 ���������� ���� �� �����.", ch, 0, ch->mount, TO_VICT );
	act( "$c1 ���������� $C4 �� �����.", ch, 0, ch->mount, TO_NOTVICT );
    }
    
    ch->dismount( );

#if 0    
    Character *victim;
    Object *wield;
    int chance, learned;
    DLString arg, args = constArguments;
    
    /*
     * dismount victim 
     */
    if (( learned = gsn_dismount->getEffective( ch ) ) <= 1) {
	ch->println("�� �� �������� ���������� ���������� ���������� � ������.");
	return;
    }
    
    arg = args.getOneArgument( );

    if (( victim = get_char_room( ch, arg.c_str( ) ) ) == NULL) {
	ch->println( "���� �� ������ ������� � ������?" );
	return;
    }
    
    if (!MOUNTED(victim)) {
	ch->pecho("�� %C1 �� � �����.", victim);
	return;
    }

    if (victim == ch) {
	ch->println("������� ����? �����, ����� ���������?");
	return;
    }
    
    if (is_safe( ch, victim ))
	return;

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	ch->pecho("�� ���� %C1 - ���� ������� ������!", victim);
	return;
    }
    
    chance = 1;
    chance += (ch->getCurrStat(STAT_STR) - victim->getCurrStat(STAT_STR)) * 3;
    chance += (ch->getCurrStat(STAT_DEX) - victim->getCurrStat(STAT_DEX)) * 2;

    if (( wield = wear_wield->find( ch ) ))
	switch (wield->value[0]) {
	case WEAPON_POLEARM: 
	    chance += 20;
	    break;
	case WEAPON_DAGGER:
	    chance -= 40;
	    break;
	case WEAPON_SPEAR:
	    chance += 10;
	    break;
	}
    else
	chance -= 30;
    
    if (!victim->can_see( ch ))
	chance += 20;
    else if (victim->fighting && victim->fighting != ch)
	chance += 10;
    
    chance = chance * learned / 100;
    chance = URANGE( 1, chance, 100 );
    
    if (number_percent( ) < chance) {
	gsn_dismount->improve( ch, true, victim );		
    }
#endif    
}

