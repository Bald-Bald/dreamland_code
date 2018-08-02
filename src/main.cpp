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

#include <iostream>

#include "logstream.h"
#include "exception.h"
#include "dreamland.h"

int main( int argc, char* argv[] )
{
    try
    {
	DreamLand dl;

	if (argc > 1)
	    dl.setConfigFilePath( argv[1] );
	
	dl.load( );

	try {
	    dl.run( );
	} catch (const Exception &e1) {
	    e1.printStackTrace( LogStream::sendFatal( ) );
	}

	dl.save( );
	
    } catch (const Exception &e1) {
	e1.printStackTrace( LogStream::sendFatal( ) );
	return 1;
    }

    return 0;
}
