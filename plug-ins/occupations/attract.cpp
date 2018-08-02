/* $Id$
 *
 * ruffina, 2004
 */
#include "attract.h"

#include "class.h"
#include "logstream.h"
#include "commandtemplate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "act.h"
#include "interp.h"
#include "loadsave.h"
#include "merc.h"
#include "occupations.h"
#include "mercdb.h"
#include "def.h"


/*-------------------------------------------------------------------
 * 'attract' command
 *------------------------------------------------------------------*/
CMDRUN( attract )
{
    NPCharacter *target;
    Character *vch;
    XMLAttributeAttract::Pointer attr;
    DLString arguments = constArguments;
    DLString targetName = arguments.getOneArgument( );
    int occupation;
    
    if (targetName.empty( )) {
	ch->println( "��� �������� �� ������ ��������?" );
	return;
    }
    
    vch = get_char_room( ch, targetName );
    
    if (vch== NULL) {
	ch->println( "����� ����� ���." );
	return;
    }

    if (!IS_AWAKE(vch)) {
	ch->pecho("�������, ���� %P1 ���������.", vch);
	return;
    }

    if (!vch->is_npc( ) || ch->is_npc( )) {
	act("$c1 ����� ������, ������� �������� ���� ��������.", ch, 0, vch, TO_VICT);
	act("�� ������ ������, ������� �������� �������� $C2.", ch, 0, vch, TO_CHAR);
	act("$c1 ����� ������, ������� �������� �������� $C2.", ch, 0, vch, TO_NOTVICT);
	return;
    }
    
    target = vch->getNPC( );
    occupation = (target->behavior ? target->behavior->getOccupation( ) : OCC_NONE);

    act("�� ������� $C4 �������� �� ���� ��������.", ch, 0, target, TO_CHAR);
    act("$c1 ������ $C4 �������� �� $x ��������.", ch, 0, target, TO_NOTVICT);
    act("$c1 ������ ���� �������� �� $x ��������.", ch, 0, target, TO_VICT);

    if (occupation == OCC_NONE) {
	say_act( ch, target, "� ����� �� ����� ���� ���� �����$G��|���|���." );
	return;
    }
    
    act("$C1 �������������� � ���� �������.", ch, 0, target, TO_CHAR);
    act("$C1 �������������� � $c3.", ch, 0, target, TO_NOTVICT);
    act("�� ��������������� � $c3.", ch, 0, target, TO_VICT);

    attr = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeAttract>( "attract" );
    attr->addTarget( target, occupation );
}

/*-------------------------------------------------------------------
 * XMLAttributeAttract
 *------------------------------------------------------------------*/
XMLAttributeAttract::XMLAttributeAttract( )
{
    targets.resize( OCC_MAX, 0 );
}

XMLAttributeAttract::~XMLAttributeAttract( )
{
}

void XMLAttributeAttract::addTarget( NPCharacter *target, int occupation )
{
    for (unsigned int i = 0; i < targets.size( ); i++)
	if (occupation & (1 << i)) 
	    targets[i] = target->getID( );
}

NPCharacter * XMLAttributeAttract::findTarget( PCharacter *pch, int occType )
{
    long long id = targets[occType];

    if (id != 0) {
	Character *rch;

	for (rch = pch->in_room->people; rch; rch = rch->next_in_room)
	    if (rch->is_npc( ) && rch->getID( ) == id)
		return rch->getNPC( );

	targets[occType] = 0;
    }

    return NULL;
}

/*-------------------------------------------------------------------
 * utils 
 *------------------------------------------------------------------*/
NPCharacter * find_attracted_mob( Character *ch, int occType )
{
    XMLAttributeAttract::Pointer attr;
    PCharacter *pch;
    NPCharacter *mob = NULL;
    
    if (( pch = ch->getPC( ) )
        && ( attr = pch->getAttributes( ).findAttr<XMLAttributeAttract>( "attract" ) )
	&& ( mob = attr->findTarget( pch, occType ) ))
    {
	return mob;
    }

    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
	if (( mob = rch->getNPC( ) )
	    && mob->behavior 
	    && IS_SET(mob->behavior->getOccupation( ), (1 << occType)))
	{
	    return mob;
	}

    return NULL;
}

