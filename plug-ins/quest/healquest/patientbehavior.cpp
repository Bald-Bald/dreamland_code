/* $Id: patientbehavior.cpp,v 1.1.2.13.6.5 2008/04/04 21:29:04 rufina Exp $
 *
 * ruffina, 2003
 */

#include "patientbehavior.h"
#include "healquest.h"

#include "pcharacter.h"
#include "npcharacter.h"

void PatientBehavior::deadFromIdiot( PCMemoryInterface *pcm )
{
    if (!getQuest( ) || !quest->isComplete( ))
	pcm->getPlayer( )->pecho( "{Y�� ������%G��|�|�� �������� ����� �� ���� �������� >8){x",  pcm->getPlayer( ) );
    else
	pcm->getPlayer( )->println("{Y������ �����, ����� �������?{x");
}

void PatientBehavior::deadFromSuicide( PCMemoryInterface *pcm )
{
    deadFromKill( pcm, ch );
}

void PatientBehavior::deadFromKill( PCMemoryInterface *pcm, Character *killer )
{
    if (pcm->isOnline( )) 
	pcm->getPlayer( )->println( "{Y������� ���������� �� ��� ���� ��� ����� ������.{x" );
}

bool PatientBehavior::spell( Character *caster, int sn, bool before ) 
{
    if (!getQuest( ) || quest->isComplete( ))
	return false;

    if (before) {
	if (quest->maladies.hasKey( sn ) && ourHero( caster ))
	    healInfect( caster->getPC( ) );
	
	quest->maladies.setAttempts( sn );
    }
    else {
	if (ourHero( caster )) {
	    if (quest->maladies.checkSuccess( sn, ch )) {
		if (quest->isComplete( )) 
		    healComplete( caster->getPC( ) );
		else
		    healSuccess( caster->getPC( ) );
	    }
	}
	else {
	    if (quest->maladies.checkSuccessOther( sn, ch )) {
		if (quest->isComplete( ))
		    healOtherComplete( getHeroMemory( ) );	
		else
		    healOtherSuccess( getHeroMemory( ) );
	    }
	}
    }

    return false;
}

void PatientBehavior::healInfect( PCharacter *pch )
{
    pch->println( "{Y��� � �� �������! ���� ������ �������.{x" );
}

void PatientBehavior::healSuccess( PCharacter *pch )
{
    pch->pecho( "{Y�� ������%G��|�|�� �������� �� ������ �� �������.{x", pch );
}

void PatientBehavior::healComplete( PCharacter *pch )
{
    pch->println("���� ������� {Y���������{x!");
    pch->println("������� �� ��������������� � ������� ���� ������� �� ����, ��� ������� �����!");
}

void PatientBehavior::healOtherSuccess( PCMemoryInterface *pcm )
{
    if (pcm->isOnline( )) {
	pcm->getPlayer( )->println( "{Y����-�� ������� �������� ������ �������� �� ������ �� �������.{x" );
	pcm->getPlayer( )->println( "��� ������������ �������� �� ����� ������� ��������������." );
    }
}

void PatientBehavior::healOtherComplete( PCMemoryInterface *pcm )
{
    if (pcm->isOnline( )) {
	pcm->getPlayer( )->println( "{Y���-�� ������ ��������� ������� ������ ��������.{x" );
	pcm->getPlayer( )->println( "������� � �������� �� ������������ ������." );
    }
}

bool PatientBehavior::canCancel( Character *caster )
{
    return MobQuestBehavior::canCancel( caster );
}

