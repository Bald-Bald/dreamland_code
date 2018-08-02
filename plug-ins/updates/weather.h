/* $Id: weather.h,v 1.1.2.2 2008/02/23 13:41:51 rufina Exp $
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
#ifndef __WEATHER_H__
#define __WEATHER_H__

#include <sstream>
using namespace std;

class DLString;

void sunlight_update( );
void weather_update( );
void weather_init( );
void make_date( ostringstream & );
DLString time_of_day( );
int hour_of_day( );
DLString sunlight( );
DLString calendar_month( );

#endif
