/* $Id$
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
#ifndef _ACT_MOVE_H_
#define _ACT_MOVE_H_

#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"

void open_door_extra ( Character *ch, int door, void *pexit );
bool open_portal( Character *, Object * );
void open_door( Character *, int );

#endif
