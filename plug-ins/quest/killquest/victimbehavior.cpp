/* $Id: victimbehavior.cpp,v 1.1.2.11.6.2 2007/09/29 19:34:05 rufina Exp $
 *
 * ruffina, 2003
 */

#include "victimbehavior.h"

#include "pcharacter.h"

void VictimBehavior::deadFromHunter( PCMemoryInterface *pcm )
{
    pcm->getPlayer( )->println(
	"���� ������� {Y���������{x!\n"
	"������� �� ��������������� � ������� ���� ������� �� ����, ��� ������� �����!" );
}

void VictimBehavior::deadFromSuicide( PCMemoryInterface *pcm )
{
    if (pcm->isOnline( ))
	pcm->getPlayer( )->println( "{Y������ ������ ����� �������.{x" );
}

void VictimBehavior::deadFromOther( PCMemoryInterface *pcm, Character *killer )
{
    killer->println("{Y����������! �� ����� ��� ���� �������� �������.{x");

    if (pcm->isOnline( ))
	pcm->getPlayer( )->println( "{Y���-�� ������ �������� ���������� ���� �������.{x" );
}

void VictimBehavior::show( Character *victim, std::basic_ostringstream<char> &buf ) 
{
    if (ourHero( victim ))
	buf << "{R[����] {x";
}

