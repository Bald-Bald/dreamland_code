/* $Id: cwizlist.cpp,v 1.1.2.3.6.3 2008/05/23 01:16:12 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * ��� ����� �� ���� ��� 'Dream Land' ����������� Igor {Leo} � Olga {Varda}*
 * ��������� ������ � ��������� ����� ����, � ����� ������ ������ ��������:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    Zadvinsky Alexandr   {Kiddy}                                         *
 *    � ��� ���������, ��� ��������� � ����� � ���� MUD                    *
 ***************************************************************************/

#include "cwizlist.h"
#include "class.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "comm.h"
#include "merc.h"
#include "def.h"

CWizlist::GodLevelName CWizlist::names[] =
{
  { "Implementors", 110, "{W" },
  { "Creators",     109, "{C" },
  { "Supremacies",  108, "{C" },
  { "Deities",      107, "{C" },
  { "Gods",         106, "{C" },
  { "Immortals",    105, "{G" },
  { "DemiGods",     104, "{G" },
  { "Angels",       103, "{G" },
  { "Avatars",      102, "{G" }
};

CWizlist::SwordLine CWizlist::swordLines[] = {
 { "       |XX::XXXX|        ",    12 },
 { "/<<>>\\/<<>>\\/<<>>\\/<<>>\\",  1 },
 { "\\<<>>/\\<<>>/\\<<>>/\\<<>>/" , 1 },
 { "      |  .// \\.  |       ",   -1 }
};

const int CWizlist::textLine = 28;

void CWizlist::initSwords( )
{
    cSwordLine = lineCounter = 0;
}

/* ��������� � ������ ������ ������, ���������� � 2-� ������ ������ */
void CWizlist::writeSwordLine( std::ostream &buf, char *str, char * color )
{
    int clen, i;

    if ((int) strlen(str) > textLine)        /* ������� �����, ����� �� ���������� */
	str[textLine] = '\0';

    clen = (textLine-strlen(str))/2; /* ������� ������� ����� �������*/

    buf << swordLines[cSwordLine].name.c_str( ); /* ������ ��� */

    if ( color && color[0] != '\0' )
	buf << color;

    for( i=0;i<clen;++i)             /* ������ ������� � ����� */
	buf << " ";
    
    buf << str;

    if ( color && color[0] != '\0' )
	buf << "{x";

    clen = textLine-(clen+strlen( str )); /* ������� ������� ����� ������ */
    for( i=0;i<clen;++i)                  /* � ������ �� */
	buf << " ";

    buf << swordLines[cSwordLine].name.c_str( ); /* ������ ������ ��� */
    buf << "\n\r";

    /* ���������, �� ���� �� ���������� � ���������� ������� ���� */
    if ( (++lineCounter >= swordLines[cSwordLine].count) && (swordLines[cSwordLine].count>0) ) {
	lineCounter = 0;
	cSwordLine++;
    }
}

/* ��������� � ����� ������ �������� ����� ����� */
void CWizlist::writeSwordPixels( std::ostream &res, char pixel, int count ) 
{
    char buf[textLine+1];
    int i;

    if (count>textLine)
	count = textLine;

    for (i=0;i<count;++i)
	buf[i] = pixel;

    buf[count] = '\0';
    writeSwordLine( res, buf, 0 );
}

COMMAND(CWizlist, "wizlist")
{
    do_help( ch, "wizlist" );

#if 0
    int cLevel;
    char bbuf[128];
    std::basic_ostringstream<char> buf;

    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
    GodList gods;
    GodList::iterator j;

    for (i = pcm.begin( ); i != pcm.end( ); i++) 
	if (i->second->getLevel( ) > LEVEL_HERO)
	    gods.push_back( i->second );

    gods.sort( CompareGods( ) );
    j = gods.begin( );

    initSwords();

    /* ������ ��������� */
    buf <<
    "        ________            **********************           ________\n\r"
    "      /+_+_+_+_+_\\       ** The gods of Dream Land **      /+_+_+_+_+_\\\n\r"
    "      \\__________/          **********************         \\__________/\n\r";


    /* ��� ������� ������ ����� */
    for ( cLevel = 0; cLevel < ( int )( sizeof(names)/sizeof(GodLevelName) ); cLevel ++ ) {
	writeSwordLine( buf, "", 0 );

	sprintf( bbuf, "%s [%d]", names[cLevel].name.c_str( ), names[cLevel].level );
	writeSwordLine( buf, bbuf, 0 );
	writeSwordPixels( buf, '*', names[cLevel].name.length( )+5 );
	writeSwordLine( buf, "", 0 );

	/* ������� ���� ����� ����� ������ */
	for ( ; j != gods.end( ) && (*j)->getLevel( ) >= names[cLevel].level; j++) {
	    sprintf( bbuf, "%s", (*j)->getName( ).c_str( ) );
	    writeSwordLine( buf, bbuf, names[cLevel].color );
	}
    }

    /* ������ ������� ����� */
    buf <<
    "       \\        /                                           \\        /\n\r"
    "        \\      /                                             \\      /\n\r"
    "         \\    /                                               \\    /\n\r"
    "          \\  /                                                 \\  /\n\r"
    "           \\/                                                   \\/\n\r" ;
  
    /* ������ ��� ��� �� ����� */
    page_to_char( buf.str( ).c_str( ), ch );
#endif    
}
