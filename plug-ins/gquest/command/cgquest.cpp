/* $Id: cgquest.cpp,v 1.1.2.1.6.8 2009/09/06 21:48:28 rufina Exp $
 *
 * ruffina, 2003
 */

#include "logstream.h"
#include "cgquest.h"
#include "globalquestmanager.h"
#include "globalquestinfo.h"
#include "globalquest.h"
#include "gqchannel.h"
#include "xmlattributeglobalquest.h"

#include "feniamanager.h"
#include "wrappermanagerbase.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "subr.h"
#include "native.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "act.h"
#include "loadsave.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

COMMAND(CGQuest, "gquest")
{
    DLString cmd;
    DLString arguments = constArguments;
    PCharacter *pch = ch->getPC( );

    arguments.stripWhiteSpace( );
    cmd = arguments.getOneArgument( );
    
    if (!pch) {
	ch->send_to( "���� ������.\n\r" );
	return;
    }
    
    if (cmd.empty( ))
	usage( pch );
    else if (arg_is_info( cmd ))
	doInfo( pch );
    else if (arg_oneof( cmd, "progress", "��������" ))
	doProgress( pch );
    else if (arg_oneof( cmd, "noexp", "��������" ))
	doNoExp( pch, arguments );
    else if (arg_oneof( cmd, "victory", "������", "������" ))
	doVictory( pch );
    else if (arg_oneof( cmd, "stat", "����������" ))
	doStat( pch );
    else if (pch->is_immortal( )) {
	if (arg_is_list( cmd ))
	    doList( pch );
	else if (arg_oneof( cmd, "start", "�����" ))
	    doStart( pch, arguments );
	else if (arg_oneof( cmd, "stop", "����" ))
	    doStop( pch, arguments );
	else if (arg_oneof( cmd, "time", "�����" ))
	    doTime( pch, arguments );
	else if (arg_oneof( cmd, "talk", "�������", "��������" ))
	    doTalk( pch, arguments );
	else if (arg_oneof( cmd, "auto", "����" ))
	    doAuto( pch, arguments );
	else if (pch->isCoder( )) {
	    if (arg_oneof( cmd, "set", "����������" ))
		doSet( pch, arguments );
	    else if (arg_oneof( cmd, "read", "��������" ))
		doRead( pch, arguments );
	    else
		usage( pch );
	}
	else 
	    usage( pch );
    }
    else
	usage( pch );

}

bool CGQuest::gqprog( PCharacter *ch, Scripting::IdRef &id )
{
    static Scripting::IdRef ID_TMP( "tmp" ), ID_GQUEST( "gquest" );
    Scripting::Register tmpGQuest;
    Scripting::RegisterList regList;

    if (!FeniaManager::wrapperManager)
	return false;

    try {
	tmpGQuest = *(*Scripting::Context::root[ID_TMP])[ID_GQUEST];
	regList.push_front( FeniaManager::wrapperManager->getWrapper( ch ) );
	return tmpGQuest[id]( regList ).toBoolean( );
    }
    catch (const Scripting::Exception &e) {
	LogStream::sendWarning( ) << "gquest: " << e.what( ) << endl;
	return false;
    }
}

bool CGQuest::gqprog_info( PCharacter *ch )
{
    static Scripting::IdRef ID_INFO( "info" );
    return gqprog( ch, ID_INFO );
}

bool CGQuest::gqprog_progress( PCharacter *ch )
{
    static Scripting::IdRef ID_PROGRESS( "progress" );
    return gqprog( ch, ID_PROGRESS );
}

void CGQuest::doInfo( PCharacter *ch ) 
{
    GlobalQuestInfo::Pointer gqi;
    GlobalQuest::Pointer gq;
    GlobalQuestManager::RunList::iterator i;
    GlobalQuestManager *manager = GlobalQuestManager::getThis( );
    GlobalQuestManager::RunList &rl = manager->getRunning( );
    
    for (i = rl.begin( ); i != rl.end( ); i++) {
	ostringstream buf;

	gq = i->second;
	gqi = manager->findGlobalQuestInfo( gq->getQuestID( ) );
	
	if (gq->isHidden( ))
	    continue;

        buf << GQChannel::BOLD << "\"" << gqi->getQuestName( ) << "\"" << GQChannel::NORMAL << endl;
	gq->getQuestDescription( buf );
	buf << GQChannel::NORMAL;
	gq->report( buf, ch );
	GQChannel::pecho( ch, buf );
    }

    gqprog_info( ch );
}

void CGQuest::doProgress( PCharacter *ch ) 
{
    GlobalQuestInfo::Pointer gqi;
    GlobalQuest::Pointer gq;
    GlobalQuestManager::RunList::iterator i;
    GlobalQuestManager *manager = GlobalQuestManager::getThis( );
    GlobalQuestManager::RunList &rl = manager->getRunning( );
    
    for (i = rl.begin( ); i != rl.end( ); i++) {
	ostringstream buf;

	gq = i->second;
	gqi = manager->findGlobalQuestInfo( gq->getQuestID( ) );

	if (gq->isHidden( ))
	    continue;

	buf << GQChannel::NORMAL << "����� "<< GQChannel::BOLD << "\""<< gqi->getQuestName( ) << "\""
	    << GQChannel::NORMAL << " (��� ";
	    
	if (gq->hasLevels( ))
	    buf << GQChannel::BOLD << gq->getMinLevel( ) 
		<< "-" << gq->getMaxLevel( ) << GQChannel::NORMAL;
	else
	    buf << "����";
	
	buf << " �������)" << endl;
	gq->progress( buf );
	GQChannel::pecho( ch, buf );
    }

    gqprog_progress( ch );
}

void CGQuest::doNoExp( PCharacter *ch, DLString& arguments ) 
{
    std::basic_ostringstream<char> buf;
    XMLAttributeGlobalQuest::Pointer attribute;
   
    try {
	attribute = ch->getAttributes( ).getAttr<XMLAttributeGlobalQuest>( "gquest" ); 
    } catch (Exception e) {
	LogStream::sendError( ) << e.what( ) << endl;
	return;
    }

    if (!arguments.empty( )) {
	if (arg_is_yes( arguments )) {
	    attribute->setNoExp( true );
	} else if (arg_is_no( arguments )) {
	    attribute->setNoExp( false );
	} else {
	    ch->println("��������� '{lEgquest noexp yes{lR������ �������� ��{lx' ��� '{lEgquest noexp no{lR������ �������� ���{lx'.");
	    return;
	}

	PCharacterManager::saveMemory( ch );
    }

    if (attribute->getNoExp( ) == true) {
	ch->send_to("�� �� ������ �������� ���� ��� ������� �� ���������� ������.\r\n" );
    } else {
	ch->send_to("�� ������ �������� ���� ��� ������� �� ���������� ������.\r\n" );
    }
}

void CGQuest::doVictory( PCharacter *ch )
{
    ostringstream buf;
    XMLAttributeGlobalQuest::Pointer gqAttr;
    int cnt = 0;

    buf << "���� ������ � ���������� �������:" << endl;

    gqAttr = ch->getAttributes( ).findAttr<XMLAttributeGlobalQuest>( "gquest" );
    if (gqAttr) {
	GlobalQuestManager::RegistryList::iterator i;
	GlobalQuestManager::RegistryList& reg = GlobalQuestManager::getThis( )->getRegistry( );

	for (i = reg.begin( ); i != reg.end( ); i++) {
	    GlobalQuestInfo::Pointer gqi = i->second;
	    int vct = gqAttr->getVictories( gqi->getQuestID( ) );

	    if (vct > 0) {
		cnt++;
		buf << GQChannel::BOLD << "\"" << gqi->getQuestName( ) << "\"" << GQChannel::NORMAL << endl
		    << "    " << vct << endl << endl;
	    }
	}
    }
    
    if (cnt == 0) 
	buf << "    �� �����, ���" << endl;

    GQChannel::pecho( ch, buf );
}

void CGQuest::doStat( PCharacter *ch )
{
    ostringstream buf;
    XMLAttributeStatistic::Statistic stat;
    XMLAttributeStatistic::Statistic::iterator s;
    static DLString pad = "          ";
   
    stat = XMLAttributeStatistic::gatherAll( "gquest" );

    buf << "������ ������������� ���� ����: " << endl;

    for (s = stat.begin( ); s != stat.end( ); s++) {
	XMLAttributeStatistic::StatRecordList::iterator r;
	GlobalQuestInfo::Pointer gqi;
	int last = 0, cnt = 0;
	
	gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( s->first );

	if (!gqi)
	    continue;

	XMLAttributeStatistic::StatRecordList &records = s->second;
	
	buf << GQChannel::BOLD << "\"" << gqi->getQuestName( ) << "\"" << GQChannel::NORMAL << endl;

	for (r = records.begin( ); r != records.end( ) && cnt < 10; cnt++) {
	    last = r->second;
	    buf << pad << GQChannel::BOLD 
		<< dlprintf( "%4d", last ) << GQChannel::NORMAL << " ";

	    for ( ; r != records.end( ) && r->second == last; r++)
		buf << r->first << " ";

	    buf << endl;
	}
	
	buf << endl;
    }
    
    GQChannel::pecho( ch, buf );
}

void CGQuest::doList( PCharacter *ch ) 
{
    char buf[MAX_STRING_LENGTH];
    GlobalQuestManager::RegistryList::iterator i;
    GlobalQuestManager::RegistryList& reg = GlobalQuestManager::getThis( )->getRegistry( );

    sprintf( buf, "%s������ ���������� ������� ���� ����\r\n", GQChannel::NORMAL );
    ch->send_to( buf );
   
    sprintf( buf, "%s%-10s %-10s %s %-4s %s %7s %9s %s%s\r\n",
	    GQChannel::BOLD, "��������", "ID", "A", "idle", "R", "������", "�����", "��������", GQChannel::NORMAL );
    ch->send_to( buf );
    
    for (i = reg.begin( ); i != reg.end( ); i++) {
	GlobalQuestInfo::Pointer gqip = i->second;
	GlobalQuest::Pointer gq = GlobalQuestManager::getThis( )->findGlobalQuest( gqip->getQuestID( ) );

	sprintf( buf, "%s%-10s %-10s ",
		GQChannel::NORMAL,
		gqip->getQuestName( ).c_str( ),
		gqip->getQuestID( ).c_str( ));

	if (gqip->getAutostart( ))
	    sprintf( buf + strlen(buf), "* %-4d", (int)(gqip->getWaitingTime( ) / 60));
	else 
	    strcat( buf, "      " );

	if (gq) {
	    sprintf( buf + strlen(buf), " * " );

	    if (gq->hasLevels( ))
		sprintf( buf + strlen(buf), "%3d-%-3d",
			 gq->getMinLevel( ), gq->getMaxLevel( ));
	    else
		sprintf( buf + strlen(buf), "%7s", "" );
	    
	    sprintf( buf + strlen(buf), " %4d/%-4d",
			             gq->getElapsedTime( ), gq->getTotalTime( ));
	}
	else 
	    sprintf(buf + strlen(buf), "%-2s %7s %9s", "", "", "");
	
	sprintf(buf + strlen(buf), " %s{x\r\n", gqip->getQuestShortDescr( ).c_str( ) );
	ch->send_to( buf );
    }

    ch->send_to( "\r\n����: "
	         "A - ���������, "
	         "idle - ����� ����� ������������, "
		 "R - running{x\r\n" );
}

void CGQuest::doStart( PCharacter *ch, DLString& arguments ) 
{
    ostringstream buf;
    GlobalQuestInfo::Pointer gqi;
    GlobalQuestInfo::Config config;
    
    if (arguments.empty( )) {
	ch->send_to( "������� ID ����������� ������.\r\n" );
	return;
    }
    
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( arguments.getOneArgument( ) );

    if (!gqi) {
	ch->send_to( "������������ ID.\r\n" );
	return;
    }

    if (!gqi->parseArguments( arguments, config, buf ))
	ch->send_to( buf );
    else
	try {
	    gqi->tryStart( config );
	}
	catch (const Exception &e) {
	    ch->send_to( e.what( ) );
	}
}

void CGQuest::doStop( PCharacter *ch, DLString& arguments ) 
{
    GlobalQuest::Pointer gq;
    
    if (arguments.empty( )) {
	ch->send_to( "������� ID ����������� ������.\r\n" );
	return;
    }
    
    gq = GlobalQuestManager::getThis( )->findGlobalQuest( arguments.getOneArgument( ) );
    if (!gq) {
	ch->send_to( "������������ ID, ���� ����� �� �������.\r\n" );
	return;
    }
    
    try {
	gq->scheduleDestroy( );
    }  catch (const Exception &e) {
	ch->send_to( e.what( ) );
	return;
    }
    ch->println( "���������� ����� ����������." );
}

void CGQuest::doTime( PCharacter *ch, DLString& arguments ) 
{
    GlobalQuest::Pointer gq;
    
    if (arguments.empty( )) {
	ch->send_to( "������� ID ����������� ������.\r\n" );
	return;
    }
    
    gq = GlobalQuestManager::getThis( )->findGlobalQuest( arguments.getOneArgument( ) );
    if (!gq) {
	ch->send_to( "������������ ID, ���� ����� �� �������.\r\n" );
	return;
    }
    
    try {
        int newTotalTime = arguments.getOneArgument( ).toInt( );
        int minTotalTime = gq->getElapsedTime( ) + 1;
        if (newTotalTime < minTotalTime) {
            ch->printf( "�������� �����, ������� %d �����.\r\n", minTotalTime );
            return;
        }

        gq->suspend( );
        gq->setTotalTime( newTotalTime );
        gq->resume( );
        ch->printf( "����� ����� ������ %d �����, �� ����� �������� %d �����.\r\n",
                gq->getTotalTime( ), gq->getRemainedTime( ) );
    }  catch (const Exception &e) {
	ch->send_to( e.what( ) );
	return;
    }
}

void CGQuest::doTalk( PCharacter *ch, DLString& arguments ) 
{
    GlobalQuestInfo::Pointer gqi;
    DLString id, arg = arguments;
    
    if (arguments.empty( )) {
	ch->send_to( "������� ���?\r\n" );
	return;
    }
   
    id = arguments.getOneArgument( );
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( id );

    if (gqi) {
	if (arguments.empty( ))
	    ch->send_to( "������� ���?\r\n" );
	else 
	    GQChannel::gecho( *gqi, arguments );
    }
    else 
	GQChannel::gecho( arg );
    
}

void CGQuest::doAuto( PCharacter *ch, DLString& arguments ) 
{
    std::basic_ostringstream<char> buf;
    DLString id, on;
    GlobalQuestInfo::Pointer gqi;
    bool autostart;
    int time = 0;

    if (arguments.empty( )) {
	ch->send_to( "������� ID ����������� ������.\r\n" );
	return;
    }

    id = arguments.getOneArgument( );
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( id );

    if (!gqi) {
	ch->send_to( "������������ ID.\r\n" );
	return;
    }

    if (arguments.empty( )) {
	autostart = !gqi->getAutostart( );
    }
    else {
	on = arguments.getOneArgument( );
	
	if (arg_is_yes( on ) || arg_is_switch_on( on ))
	    autostart = true;
	else if (arg_is_no( on ) || arg_is_switch_off( on ))
	    autostart = false;
	else {
	    ch->println( "������������ ��������: ������ {lR���{lEon{lx ��� {lR����{lEoff{lx, {lR��{lEyes{lx ��� {lR���{lEno{lx." );
	    return;
	}

	if (!arguments.empty( )) {
	    try {
		time = arguments.getOneArgument( ).toInt( );
	    } catch (ExceptionBadType e) { 	
	    } 
	    
	    if (time <= 0) {
		ch->send_to( "������������ �����.\r\n" );
		return;
	    }
	}
    }

    gqi->setAutostart( autostart );
    time = ( time ? time : 180 );
    gqi->setWaitingTime( time * 60 );

    if (autostart) 
	buf << "����� " << id << " ����� ���������� �������������"
	    << " � ���������� � " << time << " �����." << endl;
    else
	buf << "����� " << id << " �� ����� ���������� �������������." << endl;

    ch->send_to( buf );
}
    
void CGQuest::doRead( PCharacter *ch, DLString& arguments ) 
{
    GlobalQuestInfo::Pointer gqi;
    
    if (arguments.empty( )) {
	ch->send_to( "������� ID ����������� ������.\r\n" );
	return;
    }
    
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( arguments.getOneArgument( ) );
    if (!gqi) {
	ch->send_to( "������������ ID.\r\n" );
	return;
    }
    
    try {
	GlobalQuestManager::getThis( )->load( *gqi );
    }  catch (const Exception &e) {
	ch->send_to( e.what( ) );
	return;
    }
    
    ch->send_to( "������������ ����������� ������ ���������.\r\n" );
}

void CGQuest::doSet( PCharacter *ch, DLString &arguments )
{
    int count;
    PCMemoryInterface *pci;
    DLString name, questID, number;
    bool plus;
    XMLAttributeGlobalQuest::Pointer attr;
	
    name = arguments.getOneArgument( );
    questID = arguments.getOneArgument( ); 
    number = arguments.getOneArgument( ); 
    plus = false;

    if (name.empty( ) || questID.empty( ) || number.empty( )) {
	ch->send_to("�������������: gquest set <player> <quest id> <num. of victories>\r\n");
	return;
    }
    
    pci = PCharacterManager::find( name );

    if (!pci) {
	ch->send_to("������� ��� ��������� � ���������.\r\n");
	return;
    }
    
    if (!GlobalQuestManager::getThis( )->findGlobalQuestInfo( questID )) {
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
    
    attr = pci->getAttributes( ).getAttr<XMLAttributeGlobalQuest>( "gquest" );
    
    if (plus)
	count += attr->getVictories( questID );

    attr->setVictories( questID, count );
    PCharacterManager::saveMemory( pci );
    ch->send_to("Done.\r\n");
}

void CGQuest::usage( PCharacter *ch ) 
{
    std::basic_ostringstream<char> buf;

    buf << "{W{lEgquest{lR������{lx {lEinfo{lR���� {lx       {w - ���������� � ������� ���������� �������" << endl
	<< "{W{lEgquest{lR������{lx {lEprogress{lR�������� {lx   {w - �������� ������� ��������� ���������� �������" << endl
	<< "{W{lEgquest{lR������{lx {lEstat{lR���� {lx       {w - �������� ���������� �����" << endl
	<< "{W{lEgquest{lR������{lx {lEvictory{lR������  {lx    {w - �������� ���� ������" << endl
	<< "{W{lEgquest{lR������{lx {lEnoexp{lR��������{lx {lEyes  {lR�� {lx{w - �� �������� ���� � �������" << endl
	<< "{W{lEgquest{lR������{lx {lEnoexp{lR��������{lx {lEno   {lR���{lx{w - �������� ���� � �������" << endl;

    if (!ch->is_immortal( )) {
	ch->send_to( buf );
	return;
    }

    buf << "{W{lEgquest{lR������{lx {lElist{lR������{lx       {w - ������ ���� ������������ ��������" << endl
	<< "{W{lEgquest{lR������{lx {lEstart{lR�����{lx <id> [<min_level> <max_level>] [<time>] [<arg>] [<playerCnt>]" << endl
	<< "                  {w - ������ �������:" << endl
	<< "                  {w - <id> ��� �������, ������ ��. �� {lEgquest list{lR������ ������{lx" << endl
	<< "                  {w - <levels> ��������� ������� ������� ��� ������� ���� gangsters" << endl
	<< "                  {w - <time> ��������� ������������ � �������, �� ��������� 30" << endl
	<< "                  {w - <arg> ��������� ��� ��������, ���� ��� �������������� �������" << endl
	<< "                  {w - <playerCnt> ��������� ������ ������ ��� ����� ������ ����� ���-�� �������" << endl
	<< "{W{lEgquest{lR������{lx {lEstop{lR����{lx <id>  {w - ���������� ��� ����������� ������" << endl
	<< "{W{lEgquest{lR������{lx {lEtime{l�����{lx <id> <time>{w - ���������� ����� ����������� ������ � <time> �����" << endl
	<< "{W{lEgquest{lR������{lx {lEtalk{lR��������{lx <text>{w - ������� ��������� � ����� [Global Quest]" << endl
	<< "{W{lEgquest{lR������{lx {lEtalk{lR��������{lx <id> <text>{w - ��������� � ����� [Global Quest: <��� ������>]" << endl
	<< "{W{lEgquest{lR������{lx {lEauto{lR����{lx <id> [{lEon|off{lR���|����{lx] [<time>]" << endl
	<< "                  {w - ���./����. ���������� � ���������� � <time> �����" << endl;

    if (!ch->isCoder( )) {
	ch->send_to( buf );
	return;
    }
    
    buf << "{W{lEgquest{lR������{lx {lEset{lR����������{lx <player> <id> [+]<count>" << endl
	<< "                  {w - ���������� ���� ���������� ����� �� ����� ���� ������." << endl
	<< "{W{lEgquest{lR������{lx {lEread{lR��������{lx <id>  {w - �������� ������������ ������ (���������� �������)" << endl;
    ch->send_to( buf );
}

