/* $Id: outsider.cpp,v 1.1.6.1.10.3 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2005
 */
/***************************************************************************
 * ��� ����� �� ���� ��� 'Dream Land' ����������� Igor {Leo} � Olga {Varda}*
 * ��������� ������ � ��������� ����� ����, � ����� ������ ������ ��������:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    � ��� ���������, ��� ��������� � ����� � ���� MUD                    *
 ***************************************************************************/

#include "outsider.h"

#include "pcharacter.h"
#include "npcharacter.h"

#include "handler.h"
#include "act.h"
#include "def.h"


/*--------------------------------------------------------------------------
 * Outsider's ClanGuard 
 *-------------------------------------------------------------------------*/
void ClanGuardOutsider::actGreet( PCharacter *wch )
{
    do_say(ch, "����������� ����, ������ ����������� �������!");
}
void ClanGuardOutsider::actPush( PCharacter *wch )
{
    act( "$C1 �������� �������� ������� �� ����...\n\r�� ��������, ����������� �� ���� �������� � � ������ ������ ����...", wch, 0, ch, TO_CHAR );
    act( "$C1 ������� �� $c4 � $c1, �� �������� ����� �������, ���������� � ������ � ��������.", wch, 0, ch, TO_ROOM );
}
void ClanGuardOutsider::actIntruder( PCharacter *wch )
{
}

