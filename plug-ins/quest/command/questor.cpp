/* $Id: questor.cpp,v 1.1.2.2 2009/08/31 15:14:43 rufina Exp $
 *
 * ruffina, 2005
 */

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "room.h"
#include "skill.h"
#include "skillmanager.h"

#include "wiznet.h"
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "mercdb.h"

#include "clan.h"
#include "clantypes.h"
#include "selfrate.h"
#include "occupations.h"

#include "languagemanager.h"

#include "quest.h"
#include "questregistrator.h"
#include "questexceptions.h"
#include "xmlattributequestdata.h"

#include "questor.h"
#include "def.h"

#define OBJ_VNUM_QUEST_SCROLL 28109

PROF(universal);

/*--------------------------------------------------------------------------
 * Questor
 *------------------------------------------------------------------------*/
Questor::Questor( ) 
{
}

int Questor::getOccupation( )
{
    return (1 << OCC_QUEST_MASTER);
}

bool Questor::canGiveQuest( Character *ach )
{
    return !ach->is_npc( );
}

void Questor::doRequest( PCharacter *client )  
{
    XMLAttributeQuestData::Pointer attr;
    DLString descr;
    int cha;
    
    act("$c1 ������ $C4 ���� $m �������.",client,0,ch,TO_ROOM);
    act("�� ������� $C4 ���� ���� �������.",client,0,ch,TO_CHAR);

    if (client->getAttributes( ).isAvailable( "quest" )) {
	tell_raw( client,ch,"�� � ���� ��� ���� �������!" );
	return;
    }

    attr = client->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" );
    
    if (attr->getTime( ) > 0) {
	tell_fmt( "�� ����� �����%1$G��|��|��, %1$C1, �� ��� ���� ����-������ ���.", client, ch );
	tell_raw( client, ch, "������� �����." );
	return;
    }
    
    if (client->getDescription( )) {
	descr = client->getDescription( );
	descr.stripWhiteSpace( );
    }

    if (!IS_SET( client->act, PLR_CONFIRMED )) {
	tell_raw( client, ch, "������� ������� � ����� ������������� ������ ���������.");
	tell_raw( client, ch, "���� �� ������, ��� ��� ��������, �������� help confirm." );
	return;
    } else if (descr.empty( )) {
	tell_raw( client, ch, "� �� ���� ������ ������� ����� ���������������� ��������, ��� ��!");
	wiznet( WIZ_CONFIRM, 0, 0, "%C1 is confirmed but has no description!", client );
	return;
    } 
    
    cha = client->getCurrStat( STAT_CHA );
    
    if (cha < 20 && number_percent( ) < (20 - cha) * 5) {
	tell_raw( client, ch, "������, ���-�� ���� �� ����� ������ ���� �������." );
	tell_raw( client, ch, "������� �����.");
	
	if (rated_as_guru( client ))
	    attr->setTime( 1 );
	else
	    attr->setTime( number_range(3, 6) );

	return;
    }
    
    tell_fmt( "������� ����, %1$C1!", client, ch );

    try {
	QuestManager::getThis( )->generate( client, ch );
	
	PCharacterManager::save( client );
	
	tell_raw( client, ch,  "����� ����� ����������� ����!");

    } catch (const QuestCannotStartException &e) {
	tell_raw( client, ch, "������, �� � ���� ������ ��� ��� ���� �������.");
	tell_raw( client, ch, "������� �����.");
	
	if (rated_as_guru( client ))
	    attr->setTime( 1 );
	else
	    attr->setTime( number_range(3, 6) );
    }
}

void Questor::doComplete( PCharacter *client, DLString &args ) 
{
    ostringstream msg;
    int time;
    Quest::Reward::Pointer r;
    Quest::Pointer quest;
    XMLAttributes *attributes;
    XMLAttributeQuestData::Pointer qdata;
    bool fExpReward, fScrollGiven;
    DLString arg = args.getOneArgument( );

    act("$c1 ����������� $C4 � ���������� �������.",client,0,ch,TO_ROOM);
    act("�� ������������ $C4 � ���������� �������.",client,0,ch,TO_CHAR);

    attributes = &client->getAttributes( );
    quest = attributes->findAttr<Quest>( "quest" );

    if (!quest) {
	if (client->getAttributes( ).isAvailable( "quest" )) 
	    tell_raw( client, ch, "���� ������� ���������� ���������." );
	else
	    tell_fmt( "���� ����� ������� �������� (request) �������, %1$C1.", client, ch );
	    
	return;
    }
   
    if (!quest->isComplete( )) {
	tell_raw( client, ch,  "������� �� ���������! �� � ���� ��� �������� ������� �������!");
	return;
    }

    tell_raw( client, ch,  "���������� � ����������� �������!" );

    if (quest->hint.getValue( ) > 0) {
	tell_raw( client, ch,  "� ����������, ��� ��� �������� ���������� ���� ����.");
	msg << "�� �� ������������� � ��� ����";
    }
    else {
	msg << "� ������� � ��� ����";
    }
    
    fExpReward = (!arg.empty( ) && (arg.strPrefix( "experience" ) || arg.strPrefix("����")));
    msg << " {Y%3$d{G "
	<< (fExpReward ? "���%3$I�|�|�� �����" : "�������%3$I��|��|�� �����%3$I��|��|�")
	<< " � {Y%4$d{G �����%4$I��|��|�� ����%4$I��|��|�.";
    
    r = quest->reward( client, ch );
    tell_fmt( msg.str( ).c_str( ), client, ch, 
              fExpReward ? r->exp : r->points,
	      r->gold );

    client->gold += r->gold;

    if (fExpReward) {
	client->gainExp( r->exp );
    }
    else {
	client->questpoints += r->points;

	if (r->clanpoints > 0) {
	    ClanData *cd = client->getClan( )->getData( );
	    
	    if (cd && cd->getBank( )) {
		tell_fmt( "��� {Y%3$d{G �������%3$I��|��|�� �����%3$I��|��|� ������ �� ���� ������ �����.",
			  client, ch, r->clanpoints );
		
		cd->getBank( )->questpoints += r->clanpoints;
		cd->save( );
	    }
	}
    }

    if (r->prac > 0) {
	tell_fmt( "���� �������! �� ��������� {Y%3$d{G �����%3$I�|�|� ��������!",
		  client, ch, r->prac );
	client->practice += r->prac;
    }
    
    if (chance( r->wordChance ))
	rewardWord( client );
   
    if (client->getProfession( ) == prof_universal) 
        r->scrollChance *= 2;
    fScrollGiven = chance( r->scrollChance );
    if (fScrollGiven)
	rewardScroll( client );

    quest->wiznet( "complete", "%s = %d Gold = %d Prac = %d WordChance = %d ScrollChance = %d %s",
	           (fExpReward ? "Exp" : "Qp"), (fExpReward ? r->exp : r->points), 
		   r->gold, r->prac, r->wordChance, r->scrollChance, (fScrollGiven ? "" : "*") );
    
    time = quest->getNextTime( client );
    qdata = attributes->getAttr<XMLAttributeQuestData>( "questdata" );
    qdata->setTime( time );
    qdata->rememberVictory( quest->getName( ) );
    
    attributes->eraseAttribute( "quest" );
    PCharacterManager::save( client );
}


void Questor::doCancel( PCharacter *client )  
{
    int time;
    XMLAttributes *attributes;
    Quest::Pointer quest;
    
    act_p("$c1 ������ $C4 �������� $S �������.",client,0,ch,TO_ROOM,POS_RESTING);
    act_p("�� ������� $C4 �������� $S �������.",client,0,ch,TO_CHAR,POS_RESTING);
    
    attributes = &client->getAttributes( );
    quest = attributes->findAttr<Quest>( "quest" );
    
    if (!quest) {
	if (attributes->isAvailable( "quest" ))
	    tell_raw( client, ch, "���� ������� ���������� ��������." );
	else
	    tell_raw( client, ch,  "�� � ���� ��� �������!");

	return;
    }
    
    if (rated_as_guru( client )) {
	tell_raw( client, ch, "������, %s, �� ��� ���� �� ������� ������� ������� ��� ����.", client->getNameP( ) );
	return;
    }
    
    if ( client->questpoints < 3 )  {
	tell_raw( client, ch,  "� ���� ������������ ��������� ������ ��� ������ �������.");
	return;
    }
    
    quest->wiznet( "cancel" );

    time = quest->getCancelTime( client );
    quest->setTime( client, time );
    attributes->eraseAttribute( "quest" );
    client->questpoints -= 3;
    PCharacterManager::save( client );

    tell_raw( client, ch,  "�� ������� {Y3{G ��������� �������.");
    tell_fmt( "����� {Y%3$d{G ����%3$I��|��|� �� ������� �������� ����� �������.",
              client, ch, time );
}
	    
void Questor::doFind( PCharacter *client ) 
{
    ostringstream buf;
    Quest::Pointer quest;
    
    act_p("$c1 ������ ������ � $C2.",client,0,ch,TO_ROOM,POS_RESTING);
    act_p("�� ������� ������ � $C2.",client,0,ch,TO_CHAR,POS_RESTING);

    quest = client->getAttributes( ).findAttr<Quest>( "quest" );

    if (!quest) {
	if (client->getAttributes( ).isAvailable( "quest" ))
	    tell_raw( client, ch, "��� ������������ ������� ������ �� �����." );
	else
	    tell_raw( client, ch,  "�� � ���� ��� �������.");

	return;
    }
     
    if (quest->help( client, ch )) 
	return;

    if (rated_as_guru( client )) {
	tell_fmt( "������, �� ���� �������� ������ ���� ����%1$G��|��|�.", client, ch);
	quest->wiznet( "find", "failure, guru mode" );
	return;
    }
    
    quest->helpMessage( buf );
    
    if (!makeSpeedwalk( ch->in_room, quest->helpLocation( ), buf )) 
    {
	tell_fmt( "������, %1$C1, �� � ����� �� ���� ���� ������.", client, ch );
	quest->wiznet( "find", "failure, broken path" );
	return;
    }

    tell_raw( client, ch, "� ������ ����, �� ������� ����� �� ��� ������.");
    tell_raw( client, ch, buf.str( ).c_str( ) );
    tell_raw( client, ch,  "�� �����! ��� ������ � ���� ���� ��������� � ������.");
    tell_raw( client, ch,  "� �� ������� ��������� ����� �� ����� ����.");
    
    quest->hint++;
    quest->wiznet( "find", "success, attempt #%d", quest->hint.getValue( ) );
}

bool Questor::canWander( Room *const room, EXIT_DATA *exit )
{
    return exit->u1.to_room->isCommon( );
}

bool Questor::canWander( Room *const room, EXTRA_EXIT_DATA *eexit )
{
    return true;
}

bool Questor::canWander( Room *const room, Object *portal )
{
    return true;
}

void Questor::rewardWord( PCharacter *client )
{
    Word word;
    
    languageManager->getRandomWord( word, client );

    if (!word.empty( )) {
	tell_raw( client, ch, 
		  "� ������� � ������ � ����� �������� ������� �������� "
		  "� ������� ���� ����� {1{Y%s{2.", word.toStr( ) );
	::wiznet( WIZ_LANGUAGE, 0, 0, "%^C1 ������ ����� '%s' (%s).", client, word.toStr( ), word.effect.getValue( ).c_str( ) );
    }	
}

void Questor::rewardScroll( PCharacter *client )
{
    int sn, count;
    int learned, maximum;
    vector<int> skills;
    Object *scroll;
    QuestScrollBehavior::Pointer bhv;

    for (sn = 0; sn < skillManager->size( ); sn++) {
	Skill *skill = skillManager->find( sn );

	if (!skill->usable( client, false ))
	    continue;
	
	learned = skill->getLearned( client );
	maximum = skill->getMaximum( client );
	
	if (learned >= maximum)
	    continue;
	
	if (number_percent( ) > learned * 100 / maximum) 
	    continue;

	skills.push_back( sn );
    }
    
    if (skills.empty( ))
	return;

    bhv.construct( );
    count = number_range( 1, 2 );

    for (sn = 0; sn < count && !skills.empty( ); sn++) {
	sn = number_range( 0, skills.size( ) - 1 );
	bhv->addSkill( skills[sn], number_range( 1, 3 ) );	
	skills.erase( skills.begin( ) + sn );
    }
    
    scroll = create_object( get_obj_index( OBJ_VNUM_QUEST_SCROLL ), 0 );
    scroll->behavior.setPointer( *bhv );
    bhv->setObj( scroll );
    bhv->setOwner( client );
    bhv->createDescription( client );

    obj_to_char( scroll, client );
    tell_raw( client, ch, "����� ����, � ������ ���� ������, ����������� ������ �������, "
	                  "�� ������� ����������������� ���� ������." );
    act( "$C1 ���� ���� $o4.", client, scroll, ch, TO_CHAR );
    act( "$C1 ���� $c3 $o4.", client, scroll, ch, TO_ROOM );
}

/*--------------------------------------------------------------------------
 * QuestScrollBehavior 
 *------------------------------------------------------------------------*/
void QuestScrollBehavior::createDescription( PCharacter *ch )
{
    ostringstream bufInfo, bufEmpty, bufSkill;
    XMLMapBase<XMLInteger>::iterator s;
    
    bufEmpty << "�� ������� � ����� ������ �� ������� ����������, ��� ������� �� ������� ������� ���, " << endl
	     << "��� ���������� ���-���� ���������." << endl;

    bufInfo << "�� ������� � ����� ������ �� ������� ����������, ����������� ����������� ��������." << endl
	    << "������ �������� ���������� ��������, �, ��-��������, ��� �� ��������� �������������� ������ �������." << endl
	    << "�� ������� ����� �� �������� �� ���������, ��� ��� �������� ";
	    
    for (s = skills.begin( ); s != skills.end( ); s++) {
	Skill * skill = skillManager->findExisting( s->first );

	if (!skill) 
	    continue;

	if (!bufSkill.str( ).empty( ))
	    bufSkill << " � ";
	
	switch (number_range( 1, 3 )) {
	case 1: bufSkill << "������ ���������� '" << skill->getNameFor( ch ) << "'"; break;
	case 2: bufSkill << "����������� ���� ����� ���� � ��������� '" << skill->getNameFor( ch ) << "'"; break;
	case 3: bufSkill << "���-��� ����� � '" << skill->getNameFor( ch ) << "'"; break;
	}
    }

    if (bufSkill.str( ).empty( ))
	obj->addExtraDescr( obj->getName( ), bufEmpty.str( ) );
    else {
	bufInfo << bufSkill.str( ) << "." << endl;
	obj->addExtraDescr( obj->getName( ), bufInfo.str( ) );
    }
}

void QuestScrollBehavior::addSkill( int sn, int count )
{
    skills[skillManager->find( sn )->getName( )] = count;
}

void QuestScrollBehavior::setOwner( PCharacter *pch )
{
    ownerName = pch->getName( );
    ownerID = pch->getID( );
}

bool QuestScrollBehavior::isOwner( Character *ch ) const
{
    return !(ownerName.getValue( ) != ch->getName( ) 
	    || ownerID.getValue( ) != ch->getID( )
	    || ch->is_npc( ));
}

bool QuestScrollBehavior::hasTrigger( const DLString &t )
{
    return (t == "examine");
}

bool QuestScrollBehavior::examine( Character *ch )
{
    ostringstream buf, tmpbuf;
    Skill *skill;
    XMLMapBase<XMLInteger>::iterator s;
    
    if (!isOwner( ch )) {
	act("������, ����������� � $o6, ���������� ����.", ch, obj, 0, TO_CHAR);
	return true;
    }
    
    act("�� ����������� �������� ����� �� $o6.", ch, obj, 0, TO_CHAR);
    
    for (s = skills.begin( ); s != skills.end( ); s++) {
	if (s->second <= 0)
	    continue;

	skill = skillManager->findExisting( s->first );

	if (!skill) 
	    continue;

	if (!skill->canPractice( ch->getPC( ), tmpbuf )) {
	    buf << "�� �� ������ ������ �������� ���� �������� � '" << skill->getNameFor( ch ) << "'." << endl;
	    continue;
	}

	PCSkillData &data = ch->getPC( )->getSkillData( skill->getIndex( ) );
	
	if (data.learned >= skill->getMaximum( ch )) {
	    buf << "��������� '" << skill->getNameFor( ch ) << "' ��� ������� ����� � ������������." << endl;
	}
	else {
	    buf << "�� ������� ���-��� ����� �� ��������� '" << skill->getNameFor( ch ) << "'!" << endl;
	    data.learned = URANGE( data.learned.getValue( ), 
		                   data.learned + s->second,
		                   skill->getMaximum( ch ));
	    s->second = 0;
	}
    }
    
    if (buf.str( ).empty( ))
	buf << "������, ����� �� ���� ������ �������� ����." << endl;

    ch->send_to( buf );
    return true;
}

