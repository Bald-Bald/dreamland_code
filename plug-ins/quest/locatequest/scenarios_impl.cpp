/* $Id$
 *
 * ruffina, 2004
 */
#include "scenarios_impl.h"
#include "locatequest.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "interp.h"
#include "mercdb.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*-----------------------------------------------------------------------------
 * LocateMousesScenario
 *----------------------------------------------------------------------------*/
void LocateMousesScenario::getLegend( PCharacter *hero, LocateQuest::Pointer quest, ostream &buf )
{
    buf << russian_case( quest->customerName.getValue( ), '1' ) << " "
	<< "�������� �� ����� ��������, ������� ��������� " << russian_case( quest->itemMltName.getValue( ), '4' ) << " "
	<< "�� �� ��������." << endl
	<< "������� �������� �����, �� ��� ����� ���������� ����, ���� �� ��������� �� "
	<< "���� �� {Y" << quest->total << "{x ����" << GET_COUNT(quest->total, "�", "�", "") << "." << endl;
}

void LocateMousesScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest )
{
    act("$c1, ��������� ������, ��������� ���� ���������.", ch, 0, hero, TO_VICT);    
    act("$c1, ��������� ������, ��������� ��������� $C3.", ch, 0, hero, TO_NOTVICT);    
    tell_raw( hero, ch, "������ ����, ����� �� ��� ��� ��������. ��� � �� ������ �� �������!");
    tell_act( hero, ch, "��������� �� �������� {W$n4{G. ���� �� ���, �������, �� ��������.", 
	      quest->itemMltName.c_str( ) );
    tell_raw( hero, ch, "�� �������� ������ ���� �� {W%d{G ����. ��� � �����������.",
              quest->total.getValue( ) );
}

bool LocateMousesScenario::applicable( PCharacter *ch )
{
    return !IS_EVIL(ch);
}

/*-----------------------------------------------------------------------------
 * LocateSecretaryScenario
 *----------------------------------------------------------------------------*/
void LocateSecretaryScenario::getLegend( PCharacter *hero, LocateQuest::Pointer quest, ostream &buf )
{
    buf << russian_case( quest->customerName.getValue( ), '1' ) << " "
        << "������ ���� ������� ����� " << russian_case( quest->itemMltName.getValue( ), '2' )
	<< ", ������� ������ ���������� �� ������������. ����� �� ���� {Y"
	<< quest->total << "{x ����" << GET_COUNT(quest->total, "�", "�", "") << "." << endl;
}

void LocateSecretaryScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest )
{
    act("$c1 ������� �� ���� ������ ���������� �� ����� �������.", ch, 0, hero, TO_VICT);    
    act("$c1 ������� �� $C4 ������ ���������� �� ����� �������.", ch, 0, hero, TO_NOTVICT);    
    tell_act( hero, ch, "��������� �������. � ����� �������� ����� ���������� ������ � ���� ����� {W$n2{G � ���������� �� ������������!",
	      quest->itemMltName.c_str( ) );
    tell_raw( hero, ch, "���� � �� �� ������, ���� ������, � �� �� � ������.");
    act("$c1 ������� �����������.", ch, 0, 0, TO_ROOM);
    tell_raw( hero, ch, "����� �� ���� {W%d{G. ����������, ����� �� � ������� ���! �� ��� ��������� �������!",
              quest->total.getValue( ) );
}

bool LocateSecretaryScenario::applicable( PCharacter *ch )
{
    return !IS_EVIL(ch);
}

/*-----------------------------------------------------------------------------
 * LocateAlchemistScenario
 *----------------------------------------------------------------------------*/
void LocateAlchemistScenario::getLegend( PCharacter *hero, LocateQuest::Pointer quest, ostream &buf )
{
    buf << "� ����������� " << russian_case( quest->customerName.getValue( ), '2' ) << " "
        << "������� ���������� " << russian_case( quest->itemMltName.getValue( ), '4' ) << ", "
	<< "� ���������� {Y" << quest->total << "{x ����" << GET_COUNT(quest->total, "�", "", "") << "." << endl
	<< russian_case( quest->customerName.getValue( ), '1' ) << " ������ ���� ���������� ������� ��." << endl;
}

void LocateAlchemistScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest )
{
    act("$c1 �������� ������ �� �������� � �������������� � ����.", ch, 0, hero, TO_VICT);    
    act("$c1 �������� ������ �� �������� � �������������� � $C2.", ch, 0, hero, TO_NOTVICT);    
    tell_raw(hero, ch, "������� � ���-�� ������ �� � ��� ����������..");
    act("$c1 ������ � ���-��, ����������� � ���� �����.", ch, 0, 0, TO_ROOM);
    tell_act(hero, ch, "��, ��� ���.. � ���� ����������� ��������� �����, � {W$n4{G ���������� � ������ �������.",
	     quest->itemMltName.c_str( ));
    tell_raw(hero, ch, "�� ���� ���������, �� ����� {W%d{G. �����, ����� ���� �������.",
            quest->total.getValue( ));
    act("$c1 ����� ������������ � ������.", ch, 0, 0, TO_ROOM);
}

/*-----------------------------------------------------------------------------
 * LocateTorturerScenario
 *----------------------------------------------------------------------------*/
void LocateTorturerScenario::getLegend( PCharacter *hero, LocateQuest::Pointer quest, ostream &buf )
{
    buf << "��������� �� ����� " << russian_case( quest->customerName.getValue( ), '3' ) 
        << " ������ �����, �������� �� �� ������� �� " << quest->targetArea << "." << endl
	<< "����� �� ���� {Y" << quest->total << "{x ����" << GET_COUNT(quest->total, "�", "", "") << "." << endl
	<< russian_case( quest->customerName.getValue( ), '1' ) << " ������ ���� ������� �� � ������ ���." << endl;
}

void LocateTorturerScenario::actTellStory( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest )
{
    tell_act(hero, ch, "������$G��|�|�� � �� ���� ����� ����������� �������� ��������������. "
                    "�������� ���� '�� �����', ��������� ��� ����.. ��, �� ���� ���������.");
    tell_act(hero, ch, "�� ������ ��������� �������� ��� �� ���� �� {W$t{G ����. "
	           "�� ��� ��� �������� ���������� ������ ��������, � ��� ���������� ����� "
	           "�� ��� ��� �������� ���-�� �� ������.",
	    quest->targetArea.c_str( ));
    tell_raw(hero, ch, "����� ��� {W%d{G �������. ��������� �� ����, ���� �� ������������� "
                   "����� ������� �����, ��� � ���� ������������.",
            quest->total.getValue( ));
}

bool LocateTorturerScenario::applicable( PCharacter *ch )
{
    return !IS_GOOD(ch);
}

