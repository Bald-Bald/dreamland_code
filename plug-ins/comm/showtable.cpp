/* $Id: showtable.cpp,v 1.1.2.9 2009/09/11 11:24:54 rufina Exp $
 *
 * ruffina, 2004
 */
/*
 *
 * sturm, 2003
 */
#include "commandflags.h"
#include "commandtemplate.h"
#include "commandmanager.h"
#include "pcharacter.h"
#include "comm.h"
#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

/*
 * 'commands'
 */
enum {
    FCMD_TABLE = (A),
    FCMD_ALIASES = (B),
    FCMD_HINTS = (C),
    FCMD_WIZARD = (D),
    FCMD_SHOW_FIRST_RUS = (E),
    FCMD_IMPORTANT = (F)
};

static void show_aliases( Command::Pointer cmd, ostringstream &buf, int flags = 0 )
{
    XMLStringList::const_iterator a, r;

    const XMLStringList &aliases = cmd->getAliases( ); 
    for (a = aliases.begin( ); a != aliases.end( ); a++) 
	buf << *a << " ";

    const XMLStringList &russian = cmd->getRussianAliases( );
    if (!russian.empty( )) {
	r = russian.begin( );

	if (!IS_SET(flags, FCMD_SHOW_FIRST_RUS))
	    r++;
	
	for ( ; r != russian.end( ); r++) 
	    buf << *r << " ";
    }
}

static void show_matched_commands( Character *ch, const DLString &arg )
{
    ostringstream buf;
    list<Command::Pointer>::const_iterator c;
    const CommandList &commands = commandManager->getCommands( );
    bool found = false;

    if (arg.empty( )) {
        ch->println("�������������: {y{lR������� �����{lEcommand show{lx{D ��������{x.");
        return;
    }

    buf << "������� ����� �������:" << endl;

    for (c = commands.getCommands( ).begin( ); c != commands.getCommands( ).end( ); c++) {
	ostringstream aliases;
	Command::Pointer cmd = *c;

	if (!cmd->visible( ch ))
	    continue;
	
	if (!cmd->matches( arg ) && !cmd->matchesAlias( arg ))
	    continue;
        
        found = true;
	show_aliases( cmd, aliases );

	buf << fmt( 0, "{c%-12s {x: {c%s %s{x\r\n%-12s   %s\r\n",
		       cmd->getName( ).c_str( ),
		       cmd->getRussianName( ).c_str( ),
		       aliases.str( ).c_str( ),
		       " ",
		       cmd->getHint( ).c_str( ) );
    }

    if (found)
        page_to_char( buf.str( ).c_str( ), ch );
    else
        ch->printf("�� ������� �� ����� �������, ������������ � '%s'.\r\n", arg.c_str( ));
}

static void show_commands( Character *ch, int flags )
{
    ostringstream buf;
    list<Command::Pointer>::const_iterator c;
    const CommandList &commands = commandManager->getCommands( );

    if (IS_SET(flags, FCMD_ALIASES|FCMD_HINTS)) {
	buf << fmt( 0, "%-12s | %-17s| %s", 
		    "��-���������", "��-������", 
		    IS_SET(flags, FCMD_ALIASES) ? "��������" : "�������" ) 
	    << endl
	    << "-------------+------------------+--------------------------------------------" 
	    << endl;

        for (c = commands.getCommands( ).begin( ); c != commands.getCommands( ).end( ); c++) {
	    ostringstream other;
	    Command::Pointer cmd = *c;
	    
	    if (!cmd->visible( ch ))
		continue;
	    
	    if (cmd->getLevel( ) >= LEVEL_HERO)
		continue;
	    
	    if (IS_SET(flags, FCMD_ALIASES)) 
		show_aliases( cmd, other );
	    else 
		other << cmd->getHint( );

	    buf << fmt( 0, "{c%-12s {x: %-17s: %s",
			   cmd->getName( ).c_str( ),
			   cmd->getRussianName( ).c_str( ),
			   other.str( ).c_str( ) )
		<< endl;
	}

        buf << endl;
        if (IS_SET(flags, FCMD_ALIASES)) 
            buf << "����� ������ {y{lR�������{lEcommand{x, {y{lR������� �������{lEcommand hints{x � {y{lR������� �����{lEcommand show{x." << endl;
        else
            buf << "����� ������ {y{lR�������{lEcommad{x, {y{lR������� �������{lEcommand alias{x � {y{lR������� �����{lEcommand show{x." << endl;

    }
    else if (IS_SET(flags, FCMD_TABLE|FCMD_IMPORTANT)) {
	int i = 1;
	bool fRus = ch->getConfig( )->rucommands;
	const char *pattern = (fRus ? "%-17s" : "%-13s");
	const int columns = (fRus ? 4 : 6);
        
        if (IS_SET(flags, FCMD_TABLE))
            buf << "{c��� �������{x:" << endl;
        else
            buf << "{c������ �������{x:" << endl;

        for (c = commands.getCommands( ).begin( ); c != commands.getCommands( ).end( ); c++) {
	    Command::Pointer cmd = *c;

	    if (!cmd->visible( ch ))
		continue;
	    
	    if (cmd->getLevel( ) >= LEVEL_HERO)
		continue;

            if (IS_SET(flags, FCMD_IMPORTANT) && !cmd->getExtra( ).isSet( CMD_IMPORTANT ))
                continue;

	    buf << fmt( 0, pattern, 
	                   (fRus && !cmd->getRussianName( ).empty( ) ? 
				cmd->getRussianName( ).c_str( ) : cmd->getName( ).c_str( ) ));
	    
	    if (i++ % columns == 0)
		buf << endl;
	}

	buf << endl;
        if (IS_SET(flags, FCMD_TABLE))
            buf << "����� ������ {y{lR�������{lEcommand{x ��� �������� ������, {y{lR������� �������{lEcommand hints{x � {y{lR������� �����{lEcommand show{x." << endl;
        else
            buf << "��� ������� ������ ��������� {y{lR������� �������{lEcommand table{x, {y{lR������� �������{lEcommand hints{x." << endl;
    }
    else if (IS_SET(flags, FCMD_WIZARD)) {
	buf << fmt( 0, "%-12s | %-45s | %s", "��-���������", "�������", "��������" )
	    << endl
	    << "-------------+-----------------------------------------------+---------------" 
	    << endl;
        for (c = commands.getCommands( ).begin( ); c != commands.getCommands( ).end( ); c++) {
	    ostringstream aliases;
	    Command::Pointer cmd = *c;

	    if (!cmd->visible( ch ))
		continue;
	    
	    if (cmd->getLevel( ) <  LEVEL_HERO)
		continue;

	    show_aliases( cmd, aliases, FCMD_SHOW_FIRST_RUS );

	    buf << fmt( 0, "{c%-12s {x: %-45s : %s",
			   cmd->getName( ).c_str( ),
			   cmd->getHint( ).c_str( ),
			   aliases.str( ).c_str( ) )
		<< endl;
	}
    }

    page_to_char( buf.str( ).c_str( ), ch );
}

CMDRUN( commands )
{
    int flags = 0;
    DLString arg, args = constArguments; 
    
    arg = args.getOneArgument( );
    
    if (arg_oneof( arg, "show", "��������" )) {
	show_matched_commands( ch, args );
	return;
    }

    if (arg.empty( ))
	SET_BIT(flags, FCMD_IMPORTANT);
    
    if (arg_oneof( arg, "hints", "���������" ))
	SET_BIT(flags, FCMD_HINTS);

    if (arg_oneof( arg, "table", "�������" ))
	SET_BIT(flags, FCMD_TABLE);

    if (arg_oneof( arg, "aliases", "��������" ))
	SET_BIT(flags, FCMD_ALIASES);
    
    if (!flags) {
	ch->println( "�� ������ ������ ���� �� ����� ������� ������: aliases, hints, table,\r\n"
	             "���� ���������� ������ ��������� ������: commands show <��� �������>." );
	return;
    }
    
    show_commands( ch, flags );
}

/*
 * 'wizhelp'
 */
CMDRUN( wizhelp )
{
    show_commands( ch, FCMD_WIZARD );
}
