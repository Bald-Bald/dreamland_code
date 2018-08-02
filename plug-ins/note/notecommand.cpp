/* $Id: notecommand.cpp,v 1.1.2.17.6.14 2009/08/16 02:50:31 rufina Exp $
 *
 * ruffina, 2004
 */

#include "locale.h"
#include "logstream.h"
#include "notemanager.h"
#include "notethread.h"
#include "note.h"
#include "noteattrs.h"
#include "listfilter_val.h"    
#include "notehooks.h"

#include "class.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "object.h"
#include "xmlpcstringeditor.h"

#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "arg_utils.h"
#include "comm.h"
#include "ban.h"
#include "descriptor.h"
#include "infonet.h"
#include "act.h"
#include "def.h"

const DLString & NoteThread::getRussianName( ) const
{
    return rusCommand;
}

short NoteThread::getLevel( ) const
{
    return min( readLevel.getValue( ), writeLevel.getValue( ) );
}

bool NoteThread::properOrder( Character *ch )
{
    return false;
}    

const DLString& NoteThread::getHint( ) const
{
    return hint;
}

void NoteThread::run( Character* cch, const DLString& constArguments ) 
{
    XMLAttributeNoteData::Pointer attr;
    PCharacter *ch;
    DLString arguments = constArguments;
    DLString cmd = arguments.getOneArgument( );
    
    if (cch->is_npc( ))
	return;

    ch = cch->getPC( );

    if (cmd.empty( )) {
	if (canRead( ch ))
	    cmd = "read";
	else {
	    ch->pecho( "�� �� ������ ������ {W%N4{x.", rusNameMlt.c_str( ) );
	    return;
	}
    }
    
    if (cmd.strPrefix( "dump" ) && ch->isCoder( )) {
	dump( );
	ch->println( "Ok." );
	return;
    }

    if (cmd.strPrefix( "webdump" ) && ch->isCoder( )) {
	doWebDump( ch );
	return;
    }

    if (cmd.strPrefix( "hooksdump" ) && ch->isCoder( )) {
	doHooksDump( ch, arguments );
	return;
    }

    if (canRead( ch )) {
	if (arg_oneof( cmd, "read", "������" )) {
	    doRead( ch, arguments );
	    return;
	}
	else if (arg_oneof( cmd, "copy", "����������" ) && !arg_oneof_strict( cmd, "�" )) {
	    doCopy( ch, arguments );
	    return;
	}
	else if (arg_is_list( cmd )) {
	    doList( ch, arguments );
	    return;
	}
	else if (arg_oneof( cmd, "catchup", "���������" )) {
	    doCatchup( ch );
	    return;
	}
	else if (arg_oneof( cmd, "uncatchup", "�����������" )) {
	    doUncatchup( ch, arguments );
	    return;
	}
    }

    if (!canWrite( ch )) {
	ch->pecho( "� ���� ������������ ���������� ��� ��������� {W%N2{x.", rusNameMlt.c_str( ) );
	return;
    }

    if (!IS_SET(ch->act, PLR_CONFIRMED) && ch->getRemorts( ).size( ) == 0) {
	ch->println("�� �� ������ ������ ��������, ���� ���� �� ����������� ����." );
	return;
    }

    if (ch->getRealLevel( ) < 3 && ch->getRemorts( ).size( ) == 0) {
	ch->println("���� ����� ������� 3 ������, ����� ���������� ������.");
	return;
    }

    if (canRead( ch )) {
	if (arg_oneof( cmd, "remove", "delete", "�������" )) {
	    doRemove( ch, arguments );
	    return;
	}
    }

    static const DLString nopost( "nopost" );
    if (ch->getAttributes( ).isAvailable( nopost ) || banManager->check( ch->desc, BAN_COMMUNICATE )) {
	ch->println( "���� ������ ���� ����������� ������ ���������." );
	return;
    }

    attr = ch->getAttributes( ).getAttr<XMLAttributeNoteData>( "notedata" );
    
    if (canRead( ch ) && arg_oneof( cmd, "forward", "�������������" )) {
	doForward( ch, attr, arguments );
	return;
    }
    else if (cmd == "+") {
	doLinePlus( ch, attr, arguments );
	return;
    }
    else if (arg_oneof( cmd, "paste", "��������" )) {
	doPaste( ch, attr );
	return;
    }
    else if (cmd == "-") {
	doLineMinus( ch, attr );
	return;
    }
    else if (arg_oneof( cmd, "subject", "����" )) {
	doSubject( ch, attr, arguments );
	return;
    }
    else if (arg_oneof_strict( cmd, "to", "�" ) || arg_oneof( cmd, "����" )) {
	doTo( ch, attr, arguments );
	return;
    }
    else if (arg_oneof_strict( cmd, "from", "��" )) {
	if (doFrom( ch, attr, arguments ))
	    return;
    }
    else if (arg_oneof( cmd, "clear", "cancel", "��������", "��������" )) {
	if (doClear( ch, attr ))
	    return;
    }
    else if (arg_oneof( cmd, "show", "��������" )) {
	if (doShow( ch, attr ))
	    return;
    }
    else if (arg_oneof( cmd, "post", "send", "�������", "���������" )) {
	if (doPost( ch, attr ))
	    return;
    }
    else {
	ch->println("�������� �������, ������ {W? {lR������ ���������{lEnote syntax{x.");
	return;
    }
    
    echo( ch, msgNoCurrent, "�� �� ������ �������� ������." );
}

void NoteThread::doWebDump( PCharacter *ch ) const
{
    // TODO remove boilerplate 
    setlocale(LC_TIME, "ru_RU.KOI8-R");

    if (name == "news" || name == "change") {
        NoteList::const_iterator i;
        XMLListBase<WebNote> webNotes( true );
	NoteThread::Pointer thread = NoteManager::getThis( )->findThread( "news" );
        for (i = thread->getNoteList( ).begin( ); i != thread->getNoteList( ).end( ); i++) {
            const Note *note = *i;
            if (note->isNoteToAll( )) {
                WebNote webnote;
                webnote.subject.setValue( note->getSubject( ).colourStrip( ) );
                webnote.from.setValue( note->getFrom( ).colourStrip( ) );
                webnote.date.setValue( note->getDate( ).getTimeAsString( "%d %b %Y") );
                webnote.id.setValue( note->getID( ) );
                webnote.text.setValue( note->getText( ).colourStrip( ) );
                webNotes.push_back( webnote );
            }
        }

	thread = NoteManager::getThis( )->findThread( "change" );
        for (i = thread->getNoteList( ).begin( ); i != thread->getNoteList( ).end( ); i++) {
            const Note *note = *i;
            if (note->isNoteToAll( )) {
                WebNote webnote;
                webnote.subject.setValue( note->getSubject( ).colourStrip( ) );
                webnote.from.setValue( note->getFrom( ).colourStrip( ) );
                webnote.date.setValue( note->getDate( ).getTimeAsString( "%d %b %Y") );
                webnote.id.setValue( note->getID( ) );
                webnote.text.setValue( note->getText( ).colourStrip( ) );
                webNotes.push_back( webnote );
            }
        }

        saveXML( &webNotes, "all-web" );
        ch->println( "All news and changes dumped to all-web.xml." );
        return;
    }

    if (name == "story") {
        NoteList::const_iterator i;
        XMLListBase<Note> xmlNotes( true );
        
        for (i = xnotes.begin( ); i != xnotes.end( ); i++)
            xmlNotes.push_back( **i );

        saveXML( &xmlNotes, "all-web" );
        ch->println( "All stories dumped to all-web.xml." );
        return;
    }

    ch->println( "The 'webdump' command is not supported for this note thread." );
}

bool NoteThread::doShow( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note;

    if (( note = attr->findNote( this ) )) {
	ostringstream buf;
	note->toStream( buf );
	page_to_char( buf.str( ).c_str( ), ch );
	return true;
    }

    return false;
}

bool NoteThread::doClear( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note;

    if (( note = attr->findNote( this ) )) {
	attr->clearNote( this );
	ch->println( "Ok." );
	return true;
    }

    return false;
}

void NoteThread::doSubject( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    XMLNoteData *note = attr->makeNote( ch, this );
    note->setSubject( arguments );
    ch->println( "Ok." );
}

void NoteThread::doTo( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    ostringstream buf;

    XMLNoteData *note = attr->makeNote( ch, this );
    note->setRecipient( arguments );
    
    if (!Note::parseRecipient( ch, arguments, buf ))
	echo( ch, msgToError, "{R���� ��������� ����� �� �������!{x" );
    else
	echo( ch, msgToOk, "���� ��������� �������:\r\n", buf.str( ) );
}

void NoteThread::doLinePlus( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    XMLNoteData *note = attr->makeNote( ch, this );

    if (!ch->isCoder( ) && note->getBodySize( ) > 4096) 
	echo( ch, msgTooLong, "������� ������� ���������." );
    else {
	note->addLine( arguments );
	ch->println( "Ok." );
    }
}

void NoteThread::doLineMinus( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note = attr->makeNote( ch, this );

    if (note->isBodyEmpty( )) 
	ch->println( "������ ������ �������." );
    else {
	note->delLine( );
	ch->println( "Ok." );
    }
}

void NoteThread::doPaste( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note = attr->makeNote( ch, this );

    const Editor::reg_t &reg = ch->getAttributes()
	.getAttr<XMLAttributeEditorState>("edstate")->regs[0];

    note->clearBody( );
    
    for(Editor::reg_t::const_iterator i = reg.begin(); i != reg.end(); i++)
	note->addLine( *i );

    ch->println( "Ok." );
}

void NoteThread::doCatchup( PCharacter *ch ) const
{
    ch->getAttributes( ).getAttr<XMLAttributeLastRead>( "lastread" )->updateStamp( this );
    ch->println( "Ok." );
}

void NoteThread::doCopy( PCharacter *ch, DLString &arguments ) const
{
    const Note *note;
    DLString arg = arguments.getOneArgument( );
   
    if (arg.empty( )) {
        XMLAttributeNoteData::Pointer attr = ch->getAttributes( ).getAttr<XMLAttributeNoteData>( "notedata" );
        XMLNoteData *note = attr->findNote( this );
    	if (!note) {
		ch->println("�� �� ������������ ���������, ���������� ������.");
		return;
	}

        ostringstream buf;
	note->linesToStream( buf );
	ch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")
	    ->regs[0].split(buf.str( ));
	ch->println( "������� ��������� ����������� � �����." );
	return;
    }
 
    if (arg.isNumber( )) {
	try {
	    note = getNoteAtPosition( ch, arg.toInt( ) );

	    if (note) {
		ostringstream ostr;
		note->toStream(arg.toInt( ), ostr);
		ch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")
		    ->regs[0].split(ostr.str( ));
		ch->println( "Ok." );
	    } else
		ch->pecho( "��� ����� %N2 ��� �� ��������.", rusNameMlt.c_str( ) );
		
	} catch (const ExceptionBadType& e) {
	    ch->println( "������������ ����� ������." );
	}
    }
    else {
	ch->println( "����������� ����� �����?" );
    }
}

void NoteThread::doRead( PCharacter *ch, DLString &arguments ) const
{
    const Note *note;
    DLString arg = arguments.getOneArgument( );
    
    if (arg.empty( ) || arg_oneof( arg, "next", "���������", "������" )) {
	note = getNextUnreadNote( ch );
	
	if (note) 
	    showNoteToChar( ch, note );
	else
	    ch->pecho( "� ���� ��� ������������� %N2.", rusNameMlt.c_str( ) );
    }
    else if (arg.isNumber( )) {
	try {
	    note = getNoteAtPosition( ch, arg.toInt( ) );

	    if (note)
		showNoteToChar( ch, note );
	    else
		ch->pecho( "��� ����� %N2 ��� �� ��������.", rusNameMlt.c_str( ) );
		
	} catch (const ExceptionBadType& e) {
	    ch->println( "������������ ����� ������." );
	}
    }
    else {
	ch->println( "�������� ����� �����?" );
    }
}

void NoteThread::doHooksDump( PCharacter *ch, DLString &arguments ) const
{
    const Note *note;
    DLString arg = arguments.getOneArgument( );
    
    if (arg.empty( ) || !arg.isNumber( )) {
        ch->printf( "Syntax: %s hooksdump <number>\r\n", getName( ).c_str( ) );
        return;
    }

    try {
        note = getNoteAtPosition( ch, arg.toInt( ) );

        if (!note) {
            ch->pecho( "��� ����� %N2 ��� �� ��������.", rusNameMlt.c_str( ) );
            return;
        }

        noteHooks->processNoteMessage( *this, *note );
        ch->println( "Ok." );
            
    } catch (const ExceptionBadType& e) {
        ch->println( "������������ ����� ������." );
    }
}

void NoteThread::doList( PCharacter *ch, DLString &argument ) const
{
    ostringstream buf;
    NoteList mynotes;
    NoteList::const_iterator i;
    time_t stamp = getStamp( ch );
    
    for (i = xnotes.begin( ); i != xnotes.end( ); i++)
	if ((*i)->isNoteTo( ch ) || (*i)->isNoteFrom( ch ))
	    mynotes.push_back( *i );

    try {
	int cnt, last = mynotes.size( ) - 1;
	
	for (cnt = 0, i = mynotes.begin( ); i != mynotes.end( ); i++, cnt++) {
	    bool hidden = isNoteHidden( *i, ch, stamp );

	    if (listfilter_parse( ch, cnt, last, *i, hidden, argument.c_str( ) ))
		buf << "[" << cnt 
		    << (hidden ? " " : "N") << "] " 
		    << (*i)->getFrom( ) << ": " << (*i)->getSubject( ) << "{x" 
		    << endl;
	}
    }
    catch (const Exception &e) {
	ch->println( DLString( e.what( ) ) + "" );
	return;
    }
    
    if (buf.str( ).empty( ))
	ch->println( "�� ������� �� ������ ���������." );
    else
	page_to_char( buf.str( ).c_str( ), ch );
}

void NoteThread::doRemove( PCharacter *ch, DLString &arguments )
{
    const Note *note;
    DLString arg = arguments.getOneArgument( );
    
    if (!arg.isNumber( )) {
	ch->println( "������� ����� �����?" );
	return;
    }

    try {
	note = getNoteAtPosition( ch, arg.toInt( ) );

	if (note) {
	    if (ch->get_trust( ) < CREATOR && note->getAuthor( ) != ch->getName( ))
		ch->println( "�� �� ������ ������� ��� ���������, �.�. �� ��������� ��� �������." );
	    else {
		LogStream::sendNotice( ) 
		    << getName( ) << " remove: " 
		    << ch->getName( ) << ", id " << note->getID( ) << endl;
		remove( note );
		ch->println( "Ok." );
	    }
	}
	else
	    ch->pecho( "��� ����� %N2 ��� �� ��������.", rusNameMlt.c_str( ) );
    } 
    catch (const ExceptionBadType& e) {
	ch->println( "������������ ����� ������." );
    }
}

void NoteThread::doUncatchup( PCharacter *ch, DLString &arguments ) const
{
    XMLAttributeLastRead::Pointer attr;
    DLString arg = arguments.getOneArgument( );
    
    attr = ch->getAttributes( ).getAttr<XMLAttributeLastRead>( "lastread" );

    if (!arg.isNumber( )) {
	attr->setStamp( this, 0 );
	ch->printf( "��� %s �������� ��� �������������.\r\n", nameMlt.getValue( ).c_str( ) );
	return;
    }

    try {
	int vnum = arg.toInt( );
	const Note *note = getNoteAtPosition( ch, vnum );
	
	if (note) {
	    attr->setStamp( this, note->getID( ) );
	    ch->printf( "��� %s, ������� � ������ %d, �������� ��� �������������.\r\n", 
			nameMlt.getValue( ).c_str( ), vnum );
	}
	else
	    ch->pecho( "��� ����� %N2 ��� �� ��������.", rusNameMlt.c_str( ) );
    }
    catch (const ExceptionBadType& e) {
	ch->println( "������������ ����� ������." );
    }
}

bool NoteThread::doPost( PCharacter *ch, XMLAttributeNoteData::Pointer attr )
{
    XMLNoteData *notedata;

    if (!( notedata = attr->findNote( this ) ))
	return false;

    if (notedata->getRecipient( ).empty( )) {
	echo( ch, msgNoRecepient, "������� �������� ������." );
	return false;
    }

    Note note;
    notedata->commit( &note );
    note.godsSeeAlways = godsSeeAlways;
    attach( &note );

    echo( ch, msgSent, "������ ����������." );
    notify( ch, note );
    LogStream::sendNotice( ) 
	<< getName( ) << " post: " 
	<< ch->getName( ) << ", id " << note.getID( ) << endl;

    noteHooks->processNoteMessage( *this, note );
    attr->clearNote( this );
    return true;
}

bool NoteThread::doFrom( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    if (ch->get_trust( ) >= GOD) {
	XMLNoteData *note = attr->makeNote( ch, this );
	DLString arg = arguments;
	
	arg = arg.getOneArgument( );
	arg.colourstrip( );

	if (PCharacterManager::find( arg )) {
	    ch->println( "�������� ���������� ���������!" );
	}
	else {
	    note->setFrom( arguments );
	    ch->println( "Ok." );
	}

	return true;
    }

    return false;
}

void NoteThread::doForward( PCharacter *ch, XMLAttributeNoteData::Pointer attr, DLString &arguments ) const
{
    const Note *orig;
    XMLNoteData *note;
    ostringstream buf;
    DLString arg = arguments.getOneArgument( );
    
    if (!arg.isNumber( )) {
	ch->println( "������������� ����� �����?" );
	return;
    }

    try {
	orig = getNoteAtPosition( ch, arg.toInt( ) );
    } 
    catch (const ExceptionBadType& e) {
	ch->println( "������������ ����� ������." );
	return;
    }

    if (!orig) {
	ch->pecho( "��� ����� %N2 ��� �� ��������.", rusNameMlt.c_str( ) );
	return;
    }

    note = attr->makeNote( ch, this );
    orig->toForwardStream( buf );
    note->addLine( buf.str( ) );
    ch->println( "Ok." );
}

void NoteThread::notify( PCharacter *ch, const Note &note ) const
{
    Descriptor *d;
    NoteManager::Threads::const_iterator i;
    NoteManager::Threads &threads = NoteManager::getThis( )->getThreads( );

    for (d = descriptor_list; d; d = d->next) {
	Object *pager;
	std::basic_ostringstream<char> buf0;
	PCharacter *victim;
	bool fFirst = true;
	
	if (d->connected != CON_PLAYING || !d->character)
	    continue;

	victim = d->character->getPC( );
	
	if (victim == ch)
	    continue;
	
	if (!canRead( victim ))
	    continue;

	if (!note.isNoteTo( victim ))
	    continue;
	 
	if (!( pager = get_pager( victim ) ))
	    continue;

	buf0 << "{C����� ����� �� $o2: {W� ����:";
	
	for (i = threads.begin( ); i != threads.end( ); i++) {
	    const char *c1, *c2, *c5;
	    const NoteThread &th = **i->second;
	    int count = th.countSpool( victim );
	    int gender = th.gender.getValue( );
	    
	    if (count > 0) {
		c1 = (gender == SEX_FEMALE ? "��" : gender == SEX_MALE ? "��" : "��");
		c2 = (gender == SEX_FEMALE ? "��" : "��");
		c5 = "��";
		
		if (fFirst) 
		    fFirst = false;
		else
		    buf0 << "                                       ";

		buf0 << " {Y" << count << "{W �����������"
		     << GET_COUNT(count, c1, c2, c5) << " " 
		     << GET_COUNT(count, 
				    russian_case( th.rusName.getValue( ), '1' ),
				    russian_case( th.rusName.getValue( ), '2' ),
				    russian_case( th.rusNameMlt.getValue( ), '2' ) )
		     << " (" << th.name << ").{x" << endl;
	    }
	}
	
	act_p( buf0.str( ).c_str( ), victim, pager, 0, TO_CHAR, POS_DEAD );
    }
}

void NoteThread::echo( PCharacter *ch, const DLString &msg, const DLString &defaultMsg, const DLString &arg ) const
{
    ostringstream buf;
    buf << (msg.empty( ) ? defaultMsg : msg) << arg << endl;
    ch->send_to( buf );
}

const Flags & NoteThread::getExtra( ) const
{
    return extra;
}
