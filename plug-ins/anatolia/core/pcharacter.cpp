/* $Id$
 *
 * ruffina, 2004
 */
#include "clanreference.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "pcrace.h"
#include "room.h"

#include "wiznet.h"
#include "infonet.h"
#include "act.h"
#include "stats_apply.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

CLAN(none);
PROF(samurai);
PROF(universal);

/*
 *  Experience
 */
void PCharacter::gainExp( int gain )
{
    if (level >= LEVEL_HERO - 1)
        return;

    if (level > 19  && !IS_SET( act, PLR_CONFIRMED )) {
        send_to("�� ������ �� ������ �������� ����, ���� ���� �� ����������� ����.\n\r"
	        "�������� '{lR������� �������������{lEhelp confirm{lx'.\n\r");
        return;
    }

    if (level >= PK_MIN_LEVEL && IS_SET(in_room->room_flags, ROOM_NEWBIES_ONLY)) {
	println("�� �� ������ ������ �������� ���� � ���� ����.");
	return;
    }

    if (IS_SET(act,PLR_NO_EXP)) {
        send_to("�� �� ������ �������� ����, ���� ���� ��� �� ������ ����������.\n\r");
        return;
    }
    
    if (attributes.isAvailable( "noexp" ))
	return;

    exp = max( getExpPerLevel( level ), exp + gain ); 

    while (level < LEVEL_HERO - 1 && getExpToLevel( ) <= 0) {
	
	act_p("{C�� �����$g���|�|��� ���������� ������!!!{x", this, 0, 0, TO_CHAR, POS_DEAD);
        setLevel( level + 1 );

        /* added for samurais by chronos */
        if (getProfession( ) == prof_samurai && level == 10)
            wimpy = 0;

        infonet("{C��������� ����� �� $o2: {W$C1 �����$G���|�|��� ��������� ������� ����������.{x", this, 0);

        ::wiznet( WIZ_LEVELS, 0, 0, 
	          "%1$^C1 �����%1$G���|�|��� %2$d ������!", this, getRealLevel( ) );

        advanceLevel( );
	save( );
    }
}

#ifndef FIGHT_STUB
/*
 * Advancement stuff.
 */
void PCharacter::advanceLevel( )
{
    ostringstream buf;
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;
    int add_train;

    last_level = age.getTrueHours( );
    
    add_hp = (getCurrStat(STAT_CON) * getProfession( )->getHpRate( )) / 100;

    add_mana = number_range(getCurrStat(STAT_INT)/2 + 10,
	    2 * getCurrStat(STAT_INT) + getCurrStat(STAT_WIS)/5 - 10);

    add_mana = (add_mana * getProfession( )->getManaRate( )) / 100;

    add_move = number_range( 1, (getCurrStat(STAT_CON) + getCurrStat(STAT_DEX))/6 );

    add_prac = get_wis_app( this ).practice;

    add_hp = max( 3, add_hp );
    add_mana = max( 3, add_mana );
    add_move = max( 6, add_move );
    add_train = getRealLevel( ) % 5 == 0 ? 1 : 0;

    if (getSex( ) == SEX_FEMALE) {
	add_hp   -= 1;
	add_mana += 2;
    }

    add_hp += remorts.getHitPerLevel( level );
    add_mana += remorts.getManaPerLevel( level ); 

    max_hit 	+= add_hp;
    max_mana	+= add_mana;
    max_move	+= add_move;
    practice	+= add_prac;
    train       += add_train;

    perm_hit	+= add_hp;
    perm_mana	+= add_mana;
    perm_move	+= add_move;
    
    buf << "{C�� ���������: "
        << "{Y" << add_hp << "{C/" << max_hit << " {lR��������{lEhp{lx, "
	<< "{Y" << add_mana << "{C/" << max_mana << " {lR����{lEmana{lx, "
	<< "{Y" << add_move << "{C/" << max_move << " {lR��������{lEmove{lx, "
        <<  endl <<  "              " 
	<< "{Y" << add_prac << "{C/" << practice << " {lR��������{lEprac{lx";

    if (add_train > 0)
        buf << ", {Y" << add_train << "{C/" << train << " {lR����������{lEtrain{lx";

    if (getProfession( ) == prof_universal) {
	int sp_gain;

	sp_gain = 200 + getRace( )->getPC( )->getSpBonus( ) + remorts.getSkillPointsPerLevel( level );
	max_skill_points += sp_gain; 	
#if 0        
	buf << "," << endl << "              "
	    << "{Y" << sp_gain << "{C/" << max_skill_points << " {lR����� ������{lEskill points{lx"; 
#endif        
    }
    
    buf << ".{x";
    println( buf.str( ).c_str( ) );

    updateSkills( );
}

#else
void PCharacter::advanceLevel( ) { }
#endif

