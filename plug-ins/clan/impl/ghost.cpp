/* $Id: ghost.cpp,v 1.1.6.3.6.5 2010-09-01 21:20:44 rufina Exp $
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

#include "ghost.h"

#include "pcharacter.h"
#include "npcharacter.h"

#include "act.h"
#include "gsn_plugin.h"
#include "handler.h"
#include "mercdb.h"
#include "magic.h"
#include "merc.h"
#include "def.h"

GSN(dispel_affects);

/*--------------------------------------------------------------------------
 * Sharu-Gorul 
 *-------------------------------------------------------------------------*/
void ClanGuardGhost::actGreet( PCharacter *wch )
{
    do_say(ch, "����������� ����, ������������ �� ������.");
}
void ClanGuardGhost::actPush( PCharacter *wch )
{
    act( "$C1 ������� �� ���� ���������� ������.\n\r� ��� �� �� ����������, ��� ����� ���������� ���� ����������� ���� ���.", wch, 0, ch, TO_CHAR );
    act( "$C1 ������� �� $c4 ���������� ������ � $c1 ��������� ��������.", wch, 0, ch, TO_ROOM );
}
int ClanGuardGhost::getCast( Character *victim )
{
    int sn = -1;
    
    switch ( dice(1,20) ) {
    case  0: 
	if (!victim->isAffected( gsn_spellbane ))
	    sn = gsn_dispel_affects;
	break;
    case  1: sn = gsn_blindness;      break;
    case  2: sn = gsn_weaken;         break;
    case  3: sn = gsn_blindness;      break;
    case  4: sn = gsn_acid_arrow;   break;
    case  5: sn = gsn_fireball;     break;
    case  6: sn = gsn_energy_drain;   break;
    case  7:
    case  8:
    case  9: sn = gsn_acid_blast;       break;
    case 10: sn = gsn_plague;	   break;
    case 11: sn = gsn_acid_blast;         break;
    case 12:
    case 13: sn = gsn_lightning_breath;  break;
    case 14:
    case 15: sn = gsn_acid_blast;	break;
    default: sn = -1;     break;
    }
    
    return sn;
}

bool ClanGuardGhost::spec_cast( Character *victim )
{
    return ::spell( getCast( victim ), 
	            ch->getRealLevel( ), 
		    ch, 
		    victim, 
		    FSPELL_VERBOSE | FSPELL_BANE );
}


