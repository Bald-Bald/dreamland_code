/* $Id: genericnotes.cpp,v 1.1.2.8.6.2 2008/04/10 00:06:02 rufina Exp $
 *
 * ruffina, 2005
 */
#include "notethread.h"
#include "note.h"
#include "dreams.h"

#include "xmlattributeplugin.h"
#include "pcharacter.h"
#include "clanreference.h"
#include "act.h"
#include "def.h"

CLAN(ruler);

NoteThreadRegistrator * NoteThreadRegistrator::first = 0;

NOTE_DECL(note);
VOID_NOTE(note)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: � ���� {W8{x ������������� ����� ('{hc{y{lR������{lEnote{x').
    buf << fmt( 0, "� ���� {W%1$d{x ����������%1$I���|���|��� ���%1$I���|���|�� ('{hc{y{lR������{lEnote{x').", count ) << endl;
}

NOTE_DECL(news);
VOID_NOTE(news)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: {W5{x �������� ������� ���� ('{hc{y{lR�������{lEnews{x').
    buf << fmt( 0, "{W%1$d{x ������%1$I�|�|�� �����%1$I��|��|�� ���� ('{hc{y{lR�������{lEnews{x').", count ) << endl;
}

NOTE_DECL(change);
VOID_NOTE(change)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: �� ��������� ����� ��������� {W2{x ��������� ('{hc{y{lR���������{lEchange{x').
    buf << fmt( 0, "�� ��������� ����� ��������%1$I�|�|� {W%1$d{x �������%1$I��|��|�� ('{hc{y{lR���������{lEchange{x').", count ) << endl;
}

NOTE_DECL(story);
VOID_NOTE(story)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: {W10{x ����� ������� ������� ��������� ('{hc{y{lR�������{lEstory{x').
    buf << fmt( 0, "{W%1$d{x ���%1$I��|��|�� �����%1$I��|��|�� �����%1$I��|��|�� ��������� ('{hc{y{lR�������{lEstory{x').", count ) << endl;
}

NOTE_DECL(idea);
VOID_NOTE(idea)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: � ���� {W1{x ������������� ���� ('{hc{y{lR����{lEidea{x').
    buf << fmt( 0, "� ���� {W%1$d{x �����������%1$I��|��|�� ���%1$I�|�|� ('{hc{y{lR����{lEidea{x').", count ) << endl;
}

NOTE_DECL(penalty);
VOID_NOTE(penalty)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: ������������ {W8{x ��������� � ���� �������� ('{hc{y{lR���������{lEpenalty{x').
    buf << fmt( 0, "�����������%1$I�|�|� {W%1$d{x �������%1$I��|��|�� � ���� �������� ('{hc{y{lR���������{lEpenalty{x').", count ) << endl;
}


NOTE_DECL(crime);
VOID_NOTE(crime)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: ���� ������� {W1{x ��������� � ������������� ('{hc{y{lR������������{lEcrime{x').
    buf << fmt( 0, "���� �����%1$I��|��|�� {W%1$d{x �������%1$I��|��|�� � �����������%1$I�|��|�� ('{hc{y{lR������������{lEcrime{x').", count ) << endl;
}
TYPE_NOTE(bool, crime)::canWrite( const PCharacter *ch ) const 
{
    if (!NoteThread::canWrite( ch ))
	return false;
	
    return (const_cast<PCharacter *>(ch))->getClan( ) == clan_ruler;
}

NOTE_DECL(qnote);
VOID_NOTE(qnote)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: ���� ������� {W2{x ��������� � ���������� ������� ('{hc{y{lR���������{lEqnote{x').
    buf << fmt( 0, "���� �����%1$I��|��|�� {W%1$d{x �������%1$I��|��|�� � ��������%1$I��|��|�� �����%1$I�|��|�� ('{hc{y{lR���������{lEqnote{x').", count ) << endl;
}

extern "C"
{
    SO::PluginList initialize_generic_notes( ) 
    {
	SO::PluginList ppl;

	NoteThreadRegistrator::registrateAll( ppl );

	Plugin::registerPlugin<DreamThread>( ppl );
	Plugin::registerPlugin<DreamManager>( ppl );
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeDream> >( ppl );

	return ppl;
    }
}
