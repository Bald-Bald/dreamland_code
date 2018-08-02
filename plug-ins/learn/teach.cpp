/* $Id: teach.cpp,v 1.1.2.5.6.3 2008/02/24 17:24:38 rufina Exp $
 *
 * ruffina, 2004
 */

#include "teach.h"
#include "practice.h"
#include "commandtemplate.h"
#include "skill.h"
#include "skillmanager.h"
#include "pcharacter.h"
#include "act.h"
#include "handler.h"
#include "merc.h"

CMDRUN( teach )
{
    XMLAttributes * attributes;
    
    if (ch->is_npc( ) || ch->getRealLevel( ) < LEVEL_HERO - 1) {
	ch->send_to("���� ���������� ������� ������ �����.\n\r");       
	return;
    }
    
    attributes = &ch->getPC( )->getAttributes( );
    
    if (attributes->isAvailable( "teacher" )) {
	attributes->eraseAttribute( "teacher" );
	act_p("�� ��������$g��|�|�� ������� ������.", ch, 0, 0, TO_CHAR, POS_DEAD);
    }
    else {
	attributes->getAttr<XMLAttributeTeacher>( "teacher" );
	ch->send_to("������ �� ������ ������� ������ ����, ��� �� ������ � ������������.\r\n" );
    }
}

CMDRUN( learn )
{
    ch->println( "��������� 'prac <skill>', ���� ����� � ������-��������." );
}

XMLAttributeTeacher::XMLAttributeTeacher( ) 
{
}

XMLAttributeTeacher::~XMLAttributeTeacher( ) 
{
}


