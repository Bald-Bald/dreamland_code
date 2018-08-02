/* $Id: cquest.cpp,v 1.1.4.8.6.7 2009/02/15 01:44:56 rufina Exp $
 *
 * ruffina, 2003
 */

#include "class.h"

#include "behavior_utils.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "room.h"

#include "feniamanager.h"
#include "wrappermanagerbase.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "subr.h"
#include "native.h"

#include "attract.h"
#include "occupations.h"
#include "act.h"
#include "merc.h"
#include "handler.h"
#include "mercdb.h"

#include "cquest.h"
#include "quest.h"
#include "questregistrator.h"
#include "questmanager.h"
#include "questor.h"
#include "questtrader.h"
#include "xmlattributequestdata.h"
#include "def.h"

COMMAND(CQuest, "quest")
{
    Questor::Pointer questman;
    DLString arguments = constArguments;
    DLString cmd = arguments.getOneArgument( );
    PCharacter *pch = ch->getPC( );
	
    if (!pch)
	return;

    if (IS_GHOST( pch )) {
	pch->send_to("����������� ������ ���������� ���������.\r\n");
	return;
    }

    if (cmd.empty( )) {
	usage( pch );
	return;
    }
    else if (arg_is_info( cmd )) {
	doInfo( pch );
	return;
    }
    else if (arg_oneof( cmd, "points", "����" )) {
	doPoints( pch );
	return;
    }
    else if (arg_is_time( cmd )) {
	doTime( pch );
	return;
    }
    else if (arg_oneof( cmd, "stat", "����������" )) {
	doStat( pch );
	return;
    }
    else if (arg_oneof( cmd, "set", "����������" ) && pch->isCoder( )) {
	doSet( pch, arguments );
	return;
    }
    
    if (arg_is_list( cmd ) || arg_oneof( cmd, "buy", "������" ) || arg_oneof( cmd, "trouble", "�������" )) { 
	QuestTrader::Pointer trader;

	trader = find_attracted_mob_behavior<QuestTrader>( pch, OCC_QUEST_TRADER );

	if (!trader) {
	    pch->send_to( "����� ��� �������� ���������� �������.\r\n" );
	    return;
	}
	
	if (arg_is_list( cmd ))
	    trader->doList( pch );
	else if (arg_oneof( cmd, "buy", "������" ))
	    trader->doBuy( pch, arguments );
	else
	    trader->doTrouble( pch, arguments );

	return;
    }
    
    questman = find_attracted_mob_behavior<Questor>( pch, OCC_QUEST_MASTER );

    if (!questman) {
	pch->send_to("�� �� ������ ������� ����� �����.\n\r");
	return;
    }
    
    if (!questman->canGiveQuest( pch ))
	return;
    
    if (arg_oneof( cmd, "request", "���������", "��������" )) 
	questman->doRequest( pch );
    else if (arg_oneof( cmd, "complete", "�����", "���������" )) 
	questman->doComplete( pch, arguments );
    else if (arg_oneof( cmd, "cancel", "��������" )) 
	questman->doCancel( pch ); 
    else if (arg_oneof( cmd, "find", "�����" )) 
	questman->doFind( pch ); 
    else 	
	usage( pch ); 
}

bool CQuest::gprog_questinfo( PCharacter *ch )
{
    static Scripting::IdRef ID_TMP( "tmp" ), ID_QUEST( "quest" ), ID_INFO( "info" );
    Scripting::Register tmpQuest;
    Scripting::RegisterList regList;

    if (!FeniaManager::wrapperManager)
	return false;

    try {
	tmpQuest = *(*Scripting::Context::root[ID_TMP])[ID_QUEST];
	regList.push_front( FeniaManager::wrapperManager->getWrapper( ch ) );
	return tmpQuest[ID_INFO]( regList ).toBoolean( );
    }
    catch (const Scripting::Exception &e) {
	LogStream::sendWarning( ) << "quest: " << e.what( ) << endl;
	return false;
    }
}

void CQuest::doInfo( PCharacter *ch ) 
{
    std::basic_ostringstream<char> buf;
    int time;
    Quest::Pointer quest;
    
    quest = ch->getAttributes( ).findAttr<Quest>( "quest" );
    time = ch->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" )->getTime( );

    if (!quest) {
	if (ch->getAttributes( ).isAvailable( "quest" )) 
	    buf << "���� ������� ���������� �� ���������, �� ��������." << endl;
    } else {
	quest->info( buf, ch );
	buf << "� ���� {Y" << time << "{x �����" << GET_COUNT(time, "�", "�", "")
	    << " �� ���������� �������." << endl;
    }
    
    if (!gprog_questinfo( ch ) && buf.str( ).empty( ))
	buf << "� ���� ������ ��� �������." << endl;

    ch->send_to( buf );
}

void CQuest::doPoints( PCharacter *ch )  
{
    char buf [MAX_STRING_LENGTH];
    int points = ch->questpoints;

    sprintf( buf, "� ���� {Y%d{x �������%s ������%s.\n\r", 
	     points, GET_COUNT(points, "��", "��", "��"), GET_COUNT(points, "�", "�", ""));
    ch->send_to(buf);
}

void CQuest::doTime( PCharacter *ch ) 
{
    std::basic_ostringstream<char> buf;
    int time;
    Quest::Pointer quest;
    
    quest = ch->getAttributes( ).findAttr<Quest>( "quest" );
    time = ch->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" )->getTime( );
    
    if (!quest) {
	if (ch->getAttributes( ).isAvailable( "quest" )) 
	    buf << "���� ������� ���������� �� ���������, �� ��������." << endl;
	else {
	    buf << "� ���� ������ ��� �������." << endl;
	    
	    if (time > 1) 
		buf << "�� ����, ��� �� ����� ������� �������� �������, {Y"
		    << time <<  "{x �����"
		    << GET_COUNT(time, "�", "�", "") << endl;
	    else if (time == 1) 
		buf <<"�������� ������ ������ �� ����, ��� �� ����� ������� �������� �������." << endl;
	}
	
    }
    else {
	 buf << "� ���� {Y" << time << "{x �����" << GET_COUNT(time, "�", "�", "")
	     << " �� ���������� �������." << endl;
    }

    ch->send_to( buf );
}

void CQuest::doSet( PCharacter *ch, DLString& arguments )
{
    int count;
    PCMemoryInterface *pci;
    DLString name, questID, number;
    bool plus;
    XMLAttributeQuestData::Pointer attr;
    QuestRegistratorBase::Pointer qbase;
	
    name = arguments.getOneArgument( );
    questID = arguments.getOneArgument( ); 
    number = arguments.getOneArgument( ); 
    plus = false;
    
    if (name == "clear") {
	PCharacterMemoryList::const_iterator i;
	const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
	
	for (i = pcm.begin( ); i != pcm.end( ); i++) {
	    XMLAttributeQuestData::Pointer attr;
	    PCMemoryInterface *pc;

	    pc = i->second;
	    attr = pc->getAttributes( ).findAttr<XMLAttributeQuestData>( "questdata" );

	    if (attr) {
		XMLAttributeStatistic::Victories::const_iterator j;
		const XMLAttributeStatistic::Victories &v = attr->getVictories( );

		for (j = v.begin( ); j != v.end( ); j++)
		    attr->setVictories( j->first, 0 );

		PCharacterManager::saveMemory( pc );
	    }
	}
	
	return;
    }
    
    if (name.empty( ) || questID.empty( ) || number.empty( )) {
	ch->send_to("�������������: quest set <player> <quest id> [+]<num. of victories>\r\n");
	return;
    }

    pci = PCharacterManager::find( name );

    if (!pci) {
	ch->printf("%s: ��� �� �������.\r\n", name.c_str( ));
	return;
    }
    
    qbase = QuestManager::getThis( )->findQuestRegistrator( questID );

    if (!qbase) {
	ch->send_to("������������ ID.\r\n");
	return;
    }
    
    try {
	if (number.at( 0 ) == '+') {
	    plus = true;
	    number.erase( 0, 1 );
	}
	
	count = number.toInt( );
    } catch (const ExceptionBadType&) {
	ch->send_to("�������� ���������� �����.\r\n");
	return;
    }
   
    attr = pci->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" );
    
    if (plus)
	count += attr->getVictories( qbase->getName( ) );

    attr->setVictories( qbase->getName( ), count );
    PCharacterManager::saveMemory( pci );
    ch->send_to("Done.\r\n");
}

void CQuest::doStat( PCharacter *ch )
{
    ostringstream buf;
    XMLAttributeStatistic::Statistic stat;
    XMLAttributeStatistic::Statistic::iterator s;
    
    stat = XMLAttributeStatistic::gatherAll( "questdata" );

    buf << "{w������ ������������� ���� ����: {x" << endl;

    for (s = stat.begin( ); s != stat.end( ); s++) {
	XMLAttributeStatistic::StatRecordList::iterator r;
	QuestRegistratorBase::Pointer qb;
	int last = 0, cnt = 0;
	
	qb = QuestManager::getThis( )->findQuestRegistrator( s->first );

	if (!qb)
	    continue;

	XMLAttributeStatistic::StatRecordList &records = s->second;
	
	buf << "{W\"" << qb->getShortDescr( ) << "\"{x" << endl;

	for (r = records.begin( ); r != records.end( ) && cnt < 5; cnt++) {
	    last = r->second;
	    buf << dlprintf( "          {W%4d{w ", last ); 

	    for ( ; r != records.end( ) && r->second == last; r++)
		buf << r->first << " ";

	    buf << "{x" << endl;
	}
	
	buf << endl;
    }

    ch->send_to( buf );
}

void CQuest::usage( PCharacter *ch ) 
{
    ostringstream buf;

    buf << "����� ���� �� ��������� ������:" << endl << "    "
        << "{lR���� ���� ����� ��������� ����� ������ ������ ������� ����� �������� ����{lEpoints info time request complete list buy trouble find cancel stat{lx." 
        << endl
        << "����� ������ {W{lR? ������{lE? quests{x." << endl;
    ch->send_to( buf );
}

    
