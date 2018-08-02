/* $Id: effects.h,v 1.1.2.2 2008/04/14 20:12:35 rufina Exp $
 *
 * ruffina, 2004
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
#ifndef EFFECTS_H
#define EFFECTS_H

#include "bitstring.h"

void    acid_effect     (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    cold_effect     (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    fire_effect     (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    poison_effect   (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    shock_effect    (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    sand_effect     (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    scream_effect   (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );

#endif
