/* $Id: scribing.cpp,v 1.1.2.14.6.8 2008/05/27 21:30:04 rufina Exp $
 *
 * ruffina, 2004
 */

#include "scribing.h"
#include "skillreference.h"
#include "commandtemplate.h"

#include "skill.h"
#include "spell.h"
#include "pcharacter.h"
#include "object.h"
#include "dreamland.h"
#include "act.h"
#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(scribing);

/* 
 * SpellBook behavior 
 */

SpellBook::SpellBook( ) 
{
}

DLString SpellBook::extraDescription( Character *ch, const DLString &args )
{
    if (!is_name( args.c_str( ), obj->getName( ) ))
        return DLString::emptyString;

     
    std::basic_ostringstream<char> buf;
    toString( buf );
    return buf.str( );
}

void SpellBook::toString( ostringstream &buf )
{
    if (obj->value[0] == 0) 
	buf << "� ���� ����� ��� �� ����� ��������." << endl;
    else if (obj->value[1] == 0) 
	buf << "��� ����� �����." << endl;
    else {
	buf << "������ " << obj->getShortDescr( '4' ) << ", "
	    << "�� ������ �� ��������� ����� �������: " << endl << endl;
	
	for (SpellList::iterator i = spells.begin( ); i != spells.end( ); i++) {
	    buf << "������� ���������� {W" << i->first << "{x, "
		<< "���������� � ��������� {W" << i->second << "%{x" << endl;
	}
	
	if (obj->value[1] >= obj->value[0])
	    buf << endl << "������������ ��� �������� �����." << endl;
    }

    buf << endl << "������������ �������� ������������ ����������: "
        << obj->value[2] << "%." << endl;
    
}

bool SpellBook::examine( Character *ch ) 
{ 
    std::basic_ostringstream<char> buf;
    toString( buf );
    ch->send_to( buf );
    return true;
}


bool SpellBook::hasTrigger( const DLString &t )
{
    return (t == "examine");
}


/*
 * 'scribe' command
 */

/*
 * ITEM_SPELLBOOK:
 * v0 total
 * v1 used
 * v2 max quality
 */
CMDRUN( scribe )
{
    Object *book;
    SpellBook::Pointer behavior;
    Skill::Pointer skill;
    int chance, quality, sn;
    DLString arg1, arg2, arguments;
    
    chance = gsn_scribing->getEffective( ch );

    if (ch->is_npc( ) || chance <= 1) {
	ch->send_to( "�� �� �������� ���������� ������ ����������.\r\n" );
	return;
    }

    if (ch->position != POS_RESTING && ch->position != POS_SITTING) {
	ch->send_to( "���� � �������������.\r\n" );
	return;
    }
    
    /* parse args */
    arguments = constArguments;
    arg1 = arguments.getOneArgument( );
    arg2 = arguments;
    arg2.stripWhiteSpace();
    
    if (arg1.empty( ) || arg2.empty( )) {
	ch->send_to( "�������� ���� � ���?\r\n" );
	return;
    }
    
    /* check book */
    if (!( book = get_obj_carry( ch, arg1 ) )) {
	ch->send_to( "� ���� ��� ����� �����.\r\n" );
	return;
    }
    if (book->item_type != ITEM_SPELLBOOK) {
	ch->send_to( "��� �� ����� ����������.\r\n" );
	return;
    }
    if (book->value[0] == 0) {
	ch->send_to( "��� ����� ��� �������, ���� � ���������� �� �������.\r\n" );
	return;
    }
    if (book->value[1] >= book->value[0]) {
	ch->send_to( "��� �������� ����� ��� ������.\r\n" );
	return;
    }
    
    /* check spell */
    sn = SkillManager::getThis( )->unstrictLookup( arg2 );
    skill = SkillManager::getThis( )->find( sn );

    if (!skill) {
	ch->pecho( "��������� � ������, �� �� ����%G��|�|�� ��������� ������ ����������.\r\n", ch );
	return;
    }
    if (!skill->getSpell( ) || !skill->getSpell( )->isCasted( )) {
	ch->send_to( "��� �� ����������. ��������� �������� ������.:)\r\n" );
	return;
    }

    PCSkillData &data = ch->getPC( )->getSkillData( sn );
    if (data.learned <= 1) {
	ch->printf( "�� �� �������� ����������� '%s'.\r\n", skill->getNameFor( ch ).c_str( ) );
	return;
    }

    if (!skill->canForget( ch->getPC( ) )) {
	ch->printf( "�� �� ������� ��������� �� ������ ������ � ���������� '%s'.\r\n", skill->getNameFor( ch ).c_str( ) );
	return;
    }
    
    if (data.learned > book->value[2]) {
        act("�������� ���� ����� �� ��������� ������� ������ ������ � ���������� '$T'.", ch, 0, skill->getNameFor( ch ).c_str( ), TO_CHAR );
        return;
    }

    if (number_percent( ) > chance) {
        act("�� ��������� ���������������� ���� ������ � ���� �������, �� ��� ���������� �� ����.", ch, 0, 0, TO_CHAR);
        act("$c1 � ���-�� ����������, ����������� � $o4.", ch, book, 0, TO_ROOM);
        gsn_scribing->improve( ch, false );
        ch->setWaitViolence( 1 );
        return;
    }

    /* calc quality */
    quality = data.learned;
    quality -= (number_percent( ) >= chance);
    quality = URANGE( 1, quality, book->value[2] );

    /* write */
    if (!book->behavior) {
	behavior = SpellBook::Pointer( NEW );
	behavior->setObj( book );
	book->behavior.setPointer( *behavior );
    }
    else if (!( behavior = book->behavior.getDynamicPointer<SpellBook>( ))) {
	ch->send_to( "���.. ������, ��� ����� ������ ������ ������ �����.\r\n" );
	return;
    }

    book->value[1]++;
    behavior->spells[skill->getName( )] = quality;
    data.learned = 1;
    
    act( "�� �������������� ��� ���� ������ � '$T' � ���� ������� � �������� �� �� �������� $o2.", ch, book, skill->getNameFor( ch ).c_str( ), TO_CHAR );
    act( "$c1 ���-�� ���������� � $o4.", ch, book, 0, TO_ROOM );
    act( "�� ���������� ������� � ������. ������ � '$t' ��������� �� ����� ������.", ch, skill->getNameFor( ch ).c_str( ), 0, TO_CHAR ); 
    
    gsn_scribing->improve( ch, true );
    ch->setWaitViolence( 1 );
}

/*
 * 'memorize' command
 */
CMDRUN( memorize )
{
    std::basic_ostringstream<char> buf;
    Object *book;
    SpellBook::Pointer behavior;
    SpellBook::SpellList::iterator i;
    Skill::Pointer skill;
    int  chance, quality;
    DLString arg1, arg2, arguments;

    chance = gsn_scribing->getEffective( ch );

    if (ch->is_npc( ) || chance <= 1) {
	ch->send_to( "�� �� �������� ���������� ������ ������� ����������.\r\n" );
	return;
    }

    /* parse args */
    arguments = constArguments;
    arg1 = arguments.getOneArgument( );
    arg2 = arguments;
    arg2.stripWhiteSpace( );
    
    if (arg1.empty( ) || arg2.empty( )) {
	ch->send_to( "�������� ������ � ���?\r\n" );
	return;
    }

    /* check book */
    if (!( book = get_obj_carry( ch, arg1 ) )) {
	ch->send_to( "� ���� ��� ����� �����.\r\n" );
	return;
    }
    if (book->behavior) {
	behavior = book->behavior.getDynamicPointer<SpellBook>( );
    }
    if (!behavior || book->item_type != ITEM_SPELLBOOK) {
	ch->send_to( "��� �� ����� ����������.\r\n" );
	return;
    }

    /* find the spell */
    for (i = behavior->spells.begin( ); i != behavior->spells.end( ); i++)
	if (arg2.strPrefix( i->first )) {
	    skill = SkillManager::getThis( )->find( i->first );
	    
	    if (!skill) {
		ch->printf( "������, ��� ������ ������� ���������� '%s' ������� �����-�� ����.", 
		            i->first.c_str( ) );
		return;
	    }
	    break;
	}

    if (!skill) {
	ch->send_to( "�� �� ������ ����� ������� ����� ���������� � �����.\r\n" );
	return;
    }

    /* can learn? */
    if (!skill->available( ch )) {
	ch->send_to( "�� ������� �� ������, ��� ������������ ��� �������.\r\n" );
	return;
    }
    
    if (number_percent( ) > chance) {
        act("�� ��������� ������������ �������, �� ��� ���������� �� ����.", ch, 0, 0, TO_CHAR);
        act("$c1 � ���-�� ����������, ����������� � $o4.", ch, book, 0, TO_ROOM);
        gsn_scribing->improve( ch, false );
        ch->setWaitViolence( 1 );
        return;
    }

//    if (!skill->canPractice( ch->getPC( ), buf )) {
//	ch->send_to( buf );
//	return;
//   }
    
    /* move from book to brains */
    quality = i->second;
    quality -= (number_percent( ) >= chance) * (number_percent( ) >= book->value[2]);
    ch->getPC( )->getSkillData( skill->getIndex( ) ).learned = std::max( 1, quality );
    behavior->spells.erase( i );
    book->value[1]--;
    
    act( "�� ��������������� ������� '$t' � ���� ������� ��.", ch, skill->getNameFor( ch ).c_str( ), 0, TO_CHAR );
    act( "$c1 ������� � $o4 � ���-�� ������.", ch, book, 0, TO_ROOM );
    act( "������� ������� �� ������� $o2 � ������ ������������ � ����� ������.", ch, book, 0, TO_CHAR );
    
    gsn_scribing->improve( ch, true );
    ch->setWaitViolence( 1 );
}


