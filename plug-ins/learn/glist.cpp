/* $Id: glist.cpp,v 1.1.2.2.10.8 2009/09/05 18:30:47 rufina Exp $
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"
#include "skill.h"
#include "skillgroup.h"
#include "skillmanager.h"
#include "helpmanager.h"
#include "pcharacter.h"
#include "comm.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"

GROUP(clan);

CMDRUN( glist )
{
    ostringstream buf;
    SkillGroup *group;
    HelpArticle::Pointer help;
    DLString argument = constArguments;
    
    if (!ch->getPC( ))
	return;

    if (argument.empty( )) {
	buf << "��� ������:" << endl << endl;;
	
	for (int gn = 0; gn < skillGroupManager->size( ); gn++) {
	    group = skillGroupManager->find( gn );
	    buf << fmt( 0, "    %-17s   %-25s",
	                group->getName( ).c_str( ),
			group->getRussianName( ).c_str( ) )
	        << endl;
	}

	buf << endl
	    << "��� ��������� ������� ������ ������ ��������� '{y{lR������������{lEglist{lx {D<{w������{D>{w'."
	    << endl;
    }
    else {
	group = skillGroupManager->findUnstrict( argument );
	
	if (!group) {
	    ch->println("����������� ������� ������.");
	    return;
	}
	
	group->show( ch->getPC( ), buf );
    }
    
    page_to_char( buf.str( ).c_str( ), ch );
}


