/* $Id: mlove.cpp,v 1.1.2.14.6.5 2009/11/04 03:24:33 rufina Exp $
 * ruffina, 2003
 * ideas and beta-testing with Filths >8) 
 */

#include "logstream.h"
#include "commandtemplate.h"

#include "xmllovers.h"
#include "xmlattributelovers.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "pcharactermanager.h"

#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

#define MLOVE_DAZE(ch)	(ch)->wait = std::max((ch)->wait, 25);

static bool mprog_makelove( Character *ch, Character *victim )
{
    FENIA_CALL( ch, "MakeLove", "C", victim );
    FENIA_NDX_CALL( ch->getNPC( ), "MakeLove", "CC", ch, victim );
    return false;
}

CMDRUN( mlove )
{
	DLString arguments = constArguments;
	DLString arg;
	std::basic_ostringstream<char> str;
	Character *victim;

	if (IS_AFFECTED(ch,AFF_CHARM)) {
	    act_p("... �� ������ �� ���������.", ch, 0, 0, TO_CHAR, POS_RESTING);  
	    act_p("$c1 ���������� - ������ �� ���������.", ch, 0, ch->master, TO_VICT, POS_RESTING);
	    return;
	}

	if (arguments.empty( )) {
	    if (ch->getSex( ) == SEX_MALE)
		act_p("�� ����� �� ������ ������������: ���� ������ �����?", ch, 0, 0, TO_CHAR, POS_RESTING);
	    else 
		act_p("���� �����, ���� ��������.. ���� �����, ���� ��������?", ch, 0, 0, TO_CHAR, POS_RESTING);

	    act_p("$c1 �������� � ���������� ����� �� ����� � �������..��������!", ch, 0, 0, TO_ROOM, POS_RESTING);
	    return;
	}
	
	arg = arguments.getOneArgument( );

	if ( (victim = get_char_room(ch, arg.c_str())) == 0 ) {
	    ch->send_to("������ ����� ������� ����-�� ���������.\n\r");
	    return;
	}

	if (ch == victim) {
	    ch->move -= ch->move / 4;
	    ch->mana -= ch->mana / 4;

	    ch->send_to("��! �� ������ ����! ���, ���..!\n\r");
	    act_p("������� $c1 � ����$g��|��|� ���� ���������� ������ �����������.", ch, 0, 0, TO_ROOM, POS_RESTING);
	    MLOVE_DAZE(ch);
	    return;
	}

	if (ch->position == POS_FIGHTING) {
	    if (ch->getSex( ) == SEX_MALE)
		act_p("������ ������, ���� �� ��������!", ch, 0, 0, TO_CHAR, POS_RESTING);
	    else 
		act_p("��, �� ����������!", ch, 0, 0, TO_CHAR, POS_RESTING);
	    
	    act_p("$c1 ������������ ����������: '{gMake love, not war!{x'", ch, 0, 0, TO_ROOM, POS_RESTING);
	    return;
	}

	if (victim->position <= POS_STUNNED) {
	    act_p("$M ������ '���-�� ���'.. ������.", ch, 0, victim, TO_CHAR, POS_RESTING);
	    return;
	}
	else if (victim->position == POS_SLEEPING) {
	    act_p("�����, ����� $S ��� ������ ���������?", ch, 0, victim, TO_CHAR, POS_RESTING);
	    act_p("$c1 �������� ������ $C2 � ���, � ����, �� ���-�� $s �������. ��������, $S ����?", ch, 0, victim, TO_NOTVICT, POS_RESTING);
	    return;
	}
	else if (victim->position == POS_FIGHTING) {
	    act_p("$M ������ ������ �� �� ����.", ch, 0, victim, TO_CHAR, POS_RESTING);
	    return;
	}
	
	if (mprog_makelove( ch, victim ))
	    return;

	if (ch->is_npc()) {
	    ch->send_to("���� ������.\n\r");
	    return;
	}
	
	if (!victim->is_npc()) {
	    XMLAttributeLovers::Pointer pointer;
	    
	    XMLAttributes* attributes = &victim->getPC( )->getAttributes( );
	    XMLAttributes::iterator ipos = attributes->find( "lovers" );	
	
	    if (ipos != attributes->end( ) &&
		!(pointer = ipos->second.getDynamicPointer<XMLAttributeLovers>( ))->lovers.empty( ))
	    {
		if (pointer->lovers.isPresent( ch->getName() ) ) {
		    ch->mana -= ch->mana / 4;
		    victim->mana -= victim->mana / 4;
		    ch->move -= ch->move / 4;
		    victim->move -= victim->move / 4;
 
		    act_p("�� �������� � $C2 ������ � �������� ����������� � $Y �������.", ch, 0, victim, TO_CHAR, POS_RESTING);
		    act_p("$c1 ������� � ���� ������ � �������� ���������� � ����� �������. ��, ��! ���, ���!", ch, 0, victim, TO_VICT, POS_RESTING);
		    act_p("$c1 ������� � $C2 ������ � �������� ���������� � $Y �������.", ch, 0, victim, TO_NOTVICT, POS_RESTING);
		    
		    MLOVE_DAZE(victim);
		    MLOVE_DAZE(ch);
		    
		    return;
		}
	    }
	}
	
	act_p("�$G��|�|�� ���� �� �����.", ch, 0, victim, TO_CHAR, POS_RESTING);
	act_p("$c1 �������� �������� �� ���� ����������, �� �� ���������� $s.", ch, 0, victim, TO_VICT, POS_RESTING);
	act_p("$c1 �������� �������� �� $C2 ����������, �� $C1 ��������� $s.", ch, 0, victim, TO_NOTVICT, POS_RESTING);
}


