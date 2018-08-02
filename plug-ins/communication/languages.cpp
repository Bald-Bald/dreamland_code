/* $Id$
 *
 * ruffina, 2004
 */
#include <sstream>

#include "commandtemplate.h"
#include "pcharacter.h"
#include "race.h"
#include "merc.h"
#include "def.h"

CMDRUN( speak )
{
    RaceLanguage *lang;
    DLString arg = constArguments;
    
    if (ch->is_npc( ))
	return;
    
    arg = arg.getOneArgument( );

    if (arg.empty( )) {
	ostringstream buf, mybuf;
	int mylangs = 0;
	
	buf << "������ �� �������������� �� "
	    << ch->language->getShortDescr( ).ruscase( '6' ) << " ����� "
	    << "(" << ch->language->getName( ) << ")." << endl;
	
	for (int i = 0; i < raceLanguageManager->size( ); i++) {
	    lang = raceLanguageManager->find( i );

	    if (!lang->available( ch ))
		continue;
	    
	    if (!mybuf.str( ).empty( ))
		mybuf << ", ";

	    mybuf << lang->getShortDescr( ).ruscase( '1' ) << " (" << lang->getName( ) << ")";
	    mylangs++;
	}
	
	if (mylangs == 0) {
	}
	else if (mylangs == 1) {
	    buf <<  "�� ������ ������ " << mybuf.str( ) << " ����." << endl;
	}
	else {
	    buf << "�� ������ ����� �����: " << mybuf.str( ) << endl;
	}
    
	ch->send_to( buf );
	return;
    }
    
    lang = raceLanguageManager->findUnstrict( arg );

    if (!lang) {
	ch->pecho( "�� ������� �� �����%1$G��|�|�� �� ���� �����.", ch );
	return;
    }

    if (!lang->available( ch )) {
	ch->printf( "�� �� ������ ������������� �� %s �����.\r\n",
		     lang->getShortDescr( ).ruscase( '6' ).c_str( ) );
	return;
    }
    
    ch->printf( "������ �� �������������� �� %s �����.\r\n",
		 lang->getShortDescr( ).ruscase( '6' ).c_str( ) );
    ch->language.assign( lang );
}

