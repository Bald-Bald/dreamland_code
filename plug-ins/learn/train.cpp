/* $Id: train.cpp,v 1.1.2.2.6.10 2010-09-01 21:20:45 rufina Exp $
 *
 * ruffina, 2004
 */

#include "train.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "commandtemplate.h"
#include "attract.h"
#include "occupations.h"
#include "behavior_utils.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "merc.h"
#include "handler.h"
#include "act.h"
#include "def.h"

Trainer::Trainer( )
{
}

int Trainer::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) | (1 << OCC_TRAINER);
}

void Trainer::doGain( PCharacter *client, DLString & argument )
{
    DLString arg;
    
    arg = argument.getOneArgument( );
    
    if (arg.empty( )) {
	tell_act( client, ch, "�� ������ �������� {Y10{G ������� �� {Y1{G ������������� ������." );
	tell_act( client, ch, "�� ������ �������� {Y1{G ������������� ������ �� {Y10{G �������." );
	tell_act( client, ch, "��������� ��� ����� '{lR���������� ������{lEgain convert{lx' ��� '{lR���������� �������{lEgain revert{lx'." );
	return;
    }
    
    if (arg.strPrefix( "revert" ) || arg.strPrefix("�������")) {
	if (client->train < 1) {
	    tell_act( client, ch, "� ���� ��� ������������� ������." );
	    return;
	}

	act( "$C1 ���� ���� {Y10{x ������� � ����� �� {Y1{x ������������� ������.", client, 0, ch, TO_CHAR);
	act( "$C1 ���������� ��� $c2 ������������� ������ �� ������ ��������.", client, 0, ch, TO_NOTVICT );

	client->practice += 10;
	client->train -=1 ;
	return;
    }

    if (arg.strPrefix( "convert" ) || arg.strPrefix("������")) {
	if (client->practice < 10) {
	    tell_act( client, ch, "� ���� ��� ������������ ���������� �������." );
	    return;
	}

	act( "$C1 ���� ���� {Y1{x ������������� ������ � ����� �� {Y10{x �������.", client, 0, ch, TO_CHAR );
	act( "$C1 ���������� ��� $c2 ������ �������� �� ������������� ������.", client, 0, ch, TO_NOTVICT );

	client->practice -= 10;
	client->train +=1 ;
	return;
    }

    tell_act( client, ch, "� ���� �� ���� ������..." );
}

static bool mprog_cant_train( PCharacter *client, NPCharacter *trainer )
{
    FENIA_CALL( trainer, "CantTrain", "C", client );
    FENIA_NDX_CALL( trainer, "CantTrain", "CC", trainer, client );
    return false;
}

void Trainer::doTrain( PCharacter *client, DLString & argument )
{
    int cost, costQP; 
    DLString argStat, argQP;
    int stat;
    bool fQP;
    
    argStat = argument.getOneArgument( );
    argQP = argument.getOneArgument( );
    stat = -1;
    cost = 1;
    fQP = false;
    costQP = 150;
    
    if (mprog_cant_train( client, ch ))
	return;

    if (!argQP.empty( )) {
	if (argQP != "qp" && argQP != "��") {
	    tell_raw( client, ch, "���� ������ ������������� �� ��������� �������, ��������� '{lR����������� <��������> ��{lEtrain <stat> qp{lx'." );
	    return;
	}
	else
	    fQP = true;
    }

    if (argStat.empty( )) {
	tell_raw( client, ch, "� ���� {1{Y%d{2 �����������%s.", 
	          client->train.getValue( ),
                  GET_COUNT(client->train,"�� ������","�� ������","�� ������") );
    }
    else {
	for (int i = 0; i < stat_table.size; i++)
	    if (argStat.strPrefix( stat_table.fields[i].name )
		|| argStat.strPrefix( stat_table.fields[i].message ))
	    {
		stat = i;
		break;
	    }
    }
    
    if (stat == -1) {
	ostringstream buf;
	int cnt = 0;

	for (int i = 0; i < stat_table.size; i++)
	    if (client->perm_stat[stat_table.fields[i].value] 
		    < client->getMaxTrain( stat_table.fields[i].value))
	    {
		buf << russian_case( stat_table.fields[i].message, '4' )
		    << "(" << stat_table.fields[i].name << ") ";
		cnt++;
	    }
	
	if (cnt > 0) {
	    tell_raw( client, ch, 
	              "�� ������ �����������:%s%s", 
		      (cnt > 2 ? "\r\n" : " " ),
		      buf.str( ).c_str( ) );
	    if (client->perm_stat[STAT_CON] < client->getMaxTrain( STAT_CON ))
		tell_raw( client, ch, "�� ������ �������� ������������ �� {1{Y%d{2 ��������� ������: train con qp.", costQP );
	}	    
	else
	    /*
	     * This message dedicated to Jordan ... you big stud!
	     */
	    tell_raw( client, ch, "���� ������ ������ �����������, %s!",
		      GET_SEX(client, "��������", "����� ��������", "��������") );

	return;
    }

    if (client->perm_stat[stat_table.fields[stat].value] 
	>= client->getMaxTrain( stat_table.fields[stat].value )) 
    {
	tell_raw( client, ch, "���� �������� %s (%s) ��� �� ���������.", 
	          russian_case( stat_table.fields[stat].message, '1' ).c_str( ), 
		  stat_table.fields[stat].name );
	return;
    }
    
    if (fQP) {
	if (stat != STAT_CON) {
	    tell_raw( client, ch, "�� ������ ����������� �� qp ������ ������������." );
	    return;
	}

	if (costQP > client->questpoints) {
	    tell_raw( client, ch, "� ���� ������������ ��������� ������." );
	    return;
	}
    } else if (cost > client->train) {
	tell_raw( client, ch, "� ���� ��� ������������ ���������� ������������� ������." );
	return;
    }
    
    if (fQP)
	client->questpoints -= costQP;
    else
	client->train -= cost;

    client->perm_stat[stat_table.fields[stat].value] += 1;

    act( "�� ��������� $N4!", client, 0, stat_table.fields[stat].message, TO_CHAR );
    act( "$c1 �������� $N4!", client, 0, stat_table.fields[stat].message, TO_ROOM );
}


CMDRUN( gain )
{
    Trainer::Pointer trainer;
    DLString argument = constArguments;

    if (ch->is_npc( )) {
	ch->println( "��� ���������� ��� ����." );
	return;
    }
    
    trainer = find_people_behavior<Trainer>( ch->in_room );

    if (!trainer) {
	ch->println( "����� ����� �� ������ ���������� �� ��������." );
	return;
    }
    
    trainer->doGain( ch->getPC( ), argument );
}


CMDRUN( train )
{
    Trainer::Pointer trainer;
    DLString argument = constArguments;

    if (ch->is_npc( )) {
	ch->println( "��� ���������� ��� ����." );
	return;
    }
    
    trainer = find_attracted_mob_behavior<Trainer>( ch, OCC_TRAINER );

    if (!trainer) {
	ch->println( "����� ������ ����������� ����." );
	return;
    }
    
    trainer->doTrain( ch->getPC( ), argument );
}

