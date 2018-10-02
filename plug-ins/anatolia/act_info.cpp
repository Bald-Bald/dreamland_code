/* $Id$
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
 *    � ��� ���������, ��� ��������� � ����� � ���� MUD                    *
 ***************************************************************************/
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *	
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <map>
#include <sstream>

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "char.h"
#include "logstream.h"
#include "grammar_entities_impl.h"

#include "skill.h"
#include "skillmanager.h"
#include "spell.h"
#include "affecthandler.h"

#include "mobilebehavior.h"
#include "xmlattributeticker.h"
#include "commonattributes.h"
#include "commandtemplate.h"
#include "playerattributes.h"

#include "affect.h"
#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcrace.h"
#include "room.h"
#include "desire.h"
#include "helpmanager.h"

#include "dreamland.h"
#include "merc.h"
#include "descriptor.h"
#include "comm.h"
#include "colour.h"
#include "mudtags.h"
#include "bugtracker.h"
#include "act.h"
#include "alignment.h"
#include "interp.h"

#include "occupations.h"
#include "raceflags.h"
#include "gsn_plugin.h"
#include "def.h"
#include "act_move.h"
#include "act_lock.h"
#include "handler.h"
#include "stats_apply.h"
#include "vnum.h"
#include "mercdb.h"

using std::endl;
using std::min;
using std::max;

PROF(none);
PROF(universal);
PROF(samurai);
PROF(anti_paladin);
RELIG(none);
GSN(none);
GSN(gratitude);

/* command procedures needed */
bool omprog_give( Object *obj, Character *ch, Character *victim );
void password_set( PCMemoryInterface *pci, const DLString &plainText );
bool password_check( PCMemoryInterface *pci, const DLString &plainText );
DLString quality_percent( int ); /* XXX */

NPCharacter * find_mob_with_act( Room *room, bitstring_t act )
{    
    for (Character* rch = room->people; rch != 0; rch = rch->next_in_room )
       if (rch->is_npc() && IS_SET(rch->act, act))
          return rch->getNPC( );
    return NULL;
}

CMDRUNP( rules )
{
    do_help(ch,"worldrules");
}

#define MAX_PROMPT_SIZE 75

CMDRUNP( prompt )
{
    DLString old;

    if ( argument[0] == '\0' )
    {
	if (IS_SET(ch->comm,COMM_PROMPT))
	{
	    ch->send_to("����� ������ ��������� (prompt) ��������.\n\r");
	    REMOVE_BIT(ch->comm,COMM_PROMPT);
	}
	else
	{
	    ch->send_to("����� ������ ��������� (prompt) �������.\n\r");
	    SET_BIT(ch->comm,COMM_PROMPT);
	}
	return;
    }

    if (arg_is_all( argument )) {
	old = ch->prompt;
	ch->prompt = "<{r%h{x/{R%H{x�� {c%m{x/{C%M{x��� %v/%V�� {W%X{x�� ���:{g%d{x>%c";
    }
    else if (arg_is_show( argument )) {
	ch->println( "������� ������ ���������:" );
	ch->desc->send( ch->prompt.c_str( ) );
	ch->send_to( "\n\r" );
	return;
    }    
    else {
  	old = ch->prompt;
	ch->prompt = argument;
	ch->prompt.cutSize( MAX_PROMPT_SIZE );
    }
    
    if (!old.empty( )) {
	    ch->send_to( "���������� ������ ���������: " );
	    ch->desc->send(  old.c_str( ) );   
   	    ch->send_to( "\n\r" );
    }
    ch->printf("����� ������ ���������: %s\n\r",ch->prompt.c_str( ) );
}

CMDRUNP( battleprompt )
{
    DLString old;

   if ( argument[0] == '\0' )
   {
      ch->send_to("���������� ������� ��� ������ ���������.\n��� ��������� ����� ��������� ���������� ��������� 'help prompt'\n\r");
      return;
   }

    if (arg_is_all( argument )) {
	old = ch->batle_prompt;
	ch->batle_prompt = "<{r%h{x/{R%H{x�� {c%m{x/{C%M{x��� %v/%V�� %X�� ���:{g%d{x> [{r%y{x:{Y%o{x]%c";
    }
    else if (arg_is_show( argument )) {
	ch->println( "������� ������ ��������� � ���:" );
	ch->desc->send( ch->batle_prompt.c_str( ) );
	ch->send_to( "\n\r" );
	return;
    }    
    else {
	old = ch->batle_prompt;
	ch->batle_prompt = argument;
	ch->batle_prompt.cutSize( MAX_PROMPT_SIZE );
    }

    if (!old.empty( )) {
	    ch->send_to( "���������� ������ ��������� � ���: " );
	    ch->desc->send(  old.c_str( ) );   
   	    ch->send_to( "\n\r" );
    }
    ch->printf("����� ������ ��������� � ���: %s\n\r",ch->batle_prompt.c_str( ) );
}

CMDRUNP( clear )
{
    if (!ch->is_npc( ))
	ch->send_to("\033[0;0H\033[2J");
}

static DLString show_money( int g, int s )
{
    ostringstream buf;

    if (g > 0 || s > 0)
	buf << (g > 0 ? "%1$d �����%1$I��|��|��" : "")
	    << (g * s == 0 ? "" : " � ")
	    << (s > 0 ? "%2$d ��������%2$I��|��|��" : "")
	    << " ����%" << (s == 0 ? "1" : "2") << "$I��|��|�.";
    else
	buf << "��� �����.";
    
    return fmt( NULL, buf.str( ).c_str( ), g, s );
}

static DLString show_experience( PCharacter *ch )
{
    return fmt( NULL, "� ���� %1$d ���%1$I�|�|�� �����. "
	       "�� ���������� ������ �������� %2$d ���%2$I�|�|�� �� %3$d.",
	       ch->exp.getValue( ),
	       ch->getExpToLevel( ),
	       ch->getExpPerLevel( ch->getLevel( ) + 1 ) - ch->getExpPerLevel( ) );
}

CMDRUNP( money )
{
    ch->send_to( "� ���� � �������� " );
    ch->println( show_money( ch->gold, ch->silver ) );
}

CMDRUNP( experience )
{
    if (ch->is_npc( ))
	return;
    
    ch->println( show_experience( ch->getPC( ) ) );
}

CMDRUNP( worth )
{
    ch->send_to( "� ���� " );
    ch->println( show_money( ch->gold, ch->silver ) );

    if ( ch->is_npc() )
	    return;

    ch->println( show_experience( ch->getPC( ) ) );

    ch->pecho("�� ���%G��|�|�� %3d %s � %3d %s ����������.",
	    ch, 
	    ch->getPC( )->has_killed.getValue( ),
	    IS_GOOD(ch) ? "�� ������" : IS_EVIL(ch) ? "�� ����" : "�� �����������",
	    ch->getPC( )->anti_killed.getValue( ),
	    IS_GOOD(ch) ? "������" : IS_EVIL(ch) ? "����" : "�����������" );
}


#define MAX_MSGTABLE_SIZE 25
struct msgpair_t {
    int value;
    const char *msg;
};
typedef msgpair_t msgtable_t [MAX_MSGTABLE_SIZE];

const char * msgtable_lookup( const msgtable_t &table, int value )
{
    for (int i = 0; table[i].value != -1; i++)
	if (table[i].value > value)
	    return (i == 0 ? table[i].msg : table[i-1].msg);
	else if (table[i].value == value)
	    return table[i].msg;
	else if (table[i+1].value == -1)
	    return table[i].msg;


    return "";
}

msgtable_t msg_positions = {
    { POS_DEAD,     "�� ����!!!"                  },
    { POS_MORTAL,   "�� ���������."               },
    { POS_INCAP,    "�� � ����������� ���������." },
    { POS_STUNNED,  "���� ��������."              },
    { POS_SLEEPING, "�� �����."                   },
    { POS_RESTING,  "�� ���������."               },
    { POS_SITTING,  "�� ������."                  },
    { POS_FIGHTING, "�� ����������."              },
    { POS_STANDING, "�� ������."                  },
    { -1 }
};

msgtable_t msg_stat_cha = {        
    {  0, "Mongol     " },         
    { 10, "Poor       " },         
    { 14, "Average    " },         
    { 18, "Good       " },         
    { 20, "Familier   " },         
    { 22, "Charismatic" },         
    { -1 }                         
};                                 

msgtable_t msg_stat_con = {        
    {  0, "Fragile    " },         
    { 10, "Poor       " },         
    { 14, "Average    " },         
    { 18, "Healthy    " },         
    { 20, "Hearty     " },         
    { 22, "Iron       " },         
    { -1 }                         
};                                 

msgtable_t msg_stat_str = {        
    {  0, "Weak       " },         
    { 10, "Poor       " },         
    { 14, "Average    " },         
    { 18, "Strong     " },         
    { 20, "Herculian  " },         
    { 22, "Titantic   " },         
    { -1 }                         
};                                 

msgtable_t msg_stat_dex = {
    {  0, "Slow       " },
    { 10, "Clumsy     " },
    { 14, "Average    " },
    { 18, "Dextrous   " },
    { 20, "Quick      " },
    { 22, "Fast       " },
    { -1 }
};
                            
msgtable_t msg_stat_int = {
    {  0, "Hopeless   " },
    { 10, "Poor       " },
    { 14, "Average    " },
    { 18, "Good       " },
    { 20, "Clever     " },
    { 22, "Genious    " },
    { -1 }
};
                            
msgtable_t msg_stat_wis = {
    {  0, "Fool       " },
    { 10, "Dim        " },
    { 14, "Average    " },
    { 18, "Good       " },
    { 20, "Wise       " },
    { 22, "Excellent  " },
    { -1 }
};


msgtable_t msg_armor_oscore = {
    { -101, "����������� �������"   },
    { -100, "��������� �������"     },
    {  -80, "������������ �������"  },
    {  -60, "������� �������"       },
    {  -40, "����� ������ �������"  },
    {  -20, "������ �������"        },
    {    0, "�������"               },
    {   20, "���-��� �������"       },
    {   40, "����� �������"         },
    {   60, "���� �������"          },
    {   80, "�� �������"            },
    {  101, "���������� �� �������" },
    { -1 }
};

msgtable_t msg_armor = {
    { -101, "�����������"   },
    { -100, "���������"     },
    {  -80, "������������"  },
    {  -60, "�������"       },
    {  -40, "����� ������"  },
    {  -20, "������"        },
    {    0, "�������"       },
    {   20, "���� ���-��"   },
    {   40, "������"        },
    {   60, "����"          },
    {   80, "����� �����"   },
    {  101, "�� �������"    },
    { -1 }
};

CMDRUNP( oscore )
{
    ostringstream buf;
    Room *room = 0;
    int i;
    PCharacter *pch = ch->getPC( );

    buf << fmt( 0, "�� %1$s%2$s{x, ������� %3$d",
		   ch->seeName( ch, '1' ).c_str( ),
		   ch->is_npc( ) ? "" : ch->getPC( )->getParsedTitle( ).c_str( ),
		   ch->getRealLevel( ));
    
    if (!ch->is_npc( ))
	buf << fmt( 0, ", ���� %1$d %1$I���|����|��� (%2$d ��%2$I�)|��)|���).",
			pch->age.getYears( ), pch->age.getHours( ) ); 
    
    buf << endl;

    if (ch->getRealLevel( ) != ch->get_trust( ))
	buf << "������� ������� � ���� ���������� " << ch->get_trust( ) << "." << endl;

    buf << "���� " << ch->getRace( )->getNameFor( ch, ch )
	<< "  ���: " << sex_table.message( ch->getSex( ) )
	<< "  �����: " << ch->getProfession( )->getNameFor( ch );
    
    if (!ch->is_npc( ))
	if (pch->getSubProfession( ) != prof_none)
	    buf << "(" << pch->getSubProfession( )->getNameFor( ch ) << ")";
    
    if (!ch->is_npc( ))
	room = get_room_index( ch->getPC()->getHometown( )->getAltar() );
    else
	room = get_room_index( ROOM_VNUM_TEMPLE );
    
    buf << "  ���: " << (room ? room->area->name : "�������" ) << endl
	<< dlprintf( "� ���� %d/%d �����, %d/%d ������� � %d/%d ��������.\n\r",
		    ch->hit.getValue( ), ch->max_hit.getValue( ), 
		    ch->mana.getValue( ), ch->max_mana.getValue( ), 
		    ch->move.getValue( ), ch->max_move.getValue( ));
    
    if (!ch->is_npc( )) 
	buf << fmt( 0, "� ���� %1$d ������%1$I��|��|� � %2$d �����������%2$I��|��|�� �����%2$I�|�|�.",
		       pch->practice.getValue( ), pch->train.getValue( ) )
	    << endl;
    
   buf << dlprintf( "�� ������ %d/%d ����� � ����� %d/%d ������.\n\r",
		ch->carry_number, ch->canCarryNumber( ),
		ch->getCarryWeight( )/10, ch->canCarryWeight( )/10 );

    buf << dlprintf( 
	    "���� ���������:   ����(Str): %d(%d) ���������(Int): %d(%d)\n\r"
	    "              ��������(Wis): %d(%d)  ��������(Dex): %d(%d)\n\r"
	    "              ��������(Con): %d(%d)   �������(Cha): %d(%d)\n\r",
	    ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR),
	    ch->perm_stat[STAT_INT], ch->getCurrStat(STAT_INT),
	    ch->perm_stat[STAT_WIS], ch->getCurrStat(STAT_WIS),
	    ch->perm_stat[STAT_DEX], ch->getCurrStat(STAT_DEX),
	    ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON),
	    ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA) );

    buf << dlprintf( "� ���� %d ����� �����, � %s\n\r",
                  ch->exp.getValue( ),
                  show_money( ch->gold, ch->silver ).c_str( ) );

    /* KIO shows exp to level */
    if (!ch->is_npc() && ch->getRealLevel( ) < LEVEL_HERO - 1)
	buf << dlprintf( "���� ����� ������� %d ����� ����� �� ���������� ������.\n\r",
		    ch->getPC()->getExpToLevel( ) );

    if (!ch->is_npc( )) {
	XMLAttributeTimer::Pointer qd = pch->getAttributes( ).findAttr<XMLAttributeTimer>( "questdata" );
	int qtime = qd ? qd->getTime( ) : 0;
	bool hasQuest = pch->getAttributes( ).isAvailable( "quest" );
	
	buf << fmt( 0, "� ���� %1$d �������%1$I��|��|�� �����%1$I��|��|�. ",
	               pch->questpoints.getValue( ) );
	if (qtime == 0)
	    buf << "� ���� ������ ��� �������.";
	else
	    buf << fmt( 0, "�� %1$s ������ �������� %2$d ��%2$I�|��|���.",
		       hasQuest ? "�����" : "����������",
		       qtime );

	buf << endl;

	if (ch->getProfession( ) != prof_samurai)
	    buf << dlprintf( "�� ����������� ������� ��� %d �����.  ", ch->wimpy.getValue( ) );
	else
	    buf << dlprintf( "���� ����� ��� %d ���.  ", ch->getPC( )->death.getValue( ));

	if (ch->getPC()->guarding != 0)
	    buf << dlprintf( "�� ���������: %s. ", ch->seeName( ch->getPC()->guarding, '4' ).c_str( ) );

	if (ch->getPC()->guarded_by != 0)
	    buf << dlprintf( "�� �����������: %s.", ch->seeName( ch->getPC()->guarded_by, '5' ).c_str( ) );
	
	buf << endl;
    }

    if (!ch->is_npc( )) {
	ostringstream dbuf;

	for (int i = 0; i < desireManager->size( ); i++) {
	    ostringstream b;
	    
	    desireManager->find( i )->report( ch->getPC( ), b );
	    
	    if (!b.str( ).empty( ))
		dbuf << b.str( ) << " ";
	}

	if (!dbuf.str( ).empty( ))
	    buf << dbuf.str( ); 
    }
    
    buf << msgtable_lookup( msg_positions, ch->position );

    if (ch->is_adrenalined( ) && ch->position > POS_INCAP)
	buf << " ���� ����� ����� ����������!";
    
    buf << endl;

    /* print AC values */
    if (ch->getRealLevel( ) >= 20) {	
	buf << dlprintf( "������ �� ����� %d, �� ����� %d, �� ���������� %d, �� �������� %d.\n\r",
		GET_AC(ch,AC_PIERCE),
		GET_AC(ch,AC_BASH),
		GET_AC(ch,AC_SLASH),
		GET_AC(ch,AC_EXOTIC));
	buf << dlprintf( "{lR��������{lEHitroll{lx: %d  {lR����{lEDamroll{lx: %d  {lR������ �� ����������{lESaves vs Spell{lx: %d\n\r",
		    ch->hitroll.getValue( ), ch->damroll.getValue( ), ch->saving_throw.getValue( ) );
    }
    else
	for (i = 0; i < ac_type.size; i++)
	    buf << dlprintf( "�� %s �� %s.\n\r",
	                msgtable_lookup( msg_armor_oscore, GET_AC(ch, i) ),
			ac_type.message( i, '2' ).c_str( ) );

    buf << dlprintf( "� ���� %s ��������.  ", align_name( ch ).ruscase( '1' ).c_str( ) );
    
    switch (ch->ethos.getValue( )) {
    case ETHOS_LAWFUL:
	    buf << "�� ��������� �������.\n\r";
	    break;
    case ETHOS_NEUTRAL:
	    buf << "� ���� ����������� ����.\n\r";
	    break;
    case ETHOS_CHAOTIC:
	    buf << "�� ������.\n\r";
	    break;
    default:
	    if (!ch->is_npc( ))
		buf << "� ���� ��� �����, ������ �� ���� �����!\n\r";
	    else
		buf << "\n\r";
	    break;
    }
    
    if (!ch->is_npc( )) {
	if (ch->getReligion( ) == god_none)
	    buf << "�� �� ������ � ����.  ";
	else
	    buf << dlprintf( "���� �������: %s.  ",
			ch->getReligion( )->getShortDescr( ).c_str( ) );
	
	buf << dlprintf("���� ������� ����� �������:  %d.\n\r", ch->getPC( )->loyalty.getValue( ));

	if (ch->getPC( )->curse != 100)
	    buf << dlprintf( "���������, ���������� �� ����, �������� ��� ���� ������ �� %d%%.\n\r",
			100 - ch->getPC( )->curse.getValue( ));

	if (ch->getPC( )->bless)
	    buf << dlprintf( "�������������� ����� �������� ��� ���� ������ �� %d%%.\n\r",
			ch->getPC( )->bless.getValue( ));
    }
#if 0
    if (ch->getProfession( ) == prof_universal)
	buf << dlprintf( "� ���� %d/%d {lR����� ������{lEskill points{lx.\n\r",
		    ch->getPC()->skill_points(),
		    ch->getPC()->max_skill_points.getValue( ));
#endif
    /* RT wizinvis and holy light */
    if (ch->is_immortal( )) 
	buf << dlprintf( "������������ ���� %s. ����������� %d ������, ��������� %d ������.",
	           (IS_SET(ch->act, PLR_HOLYLIGHT) ? "�������" : "��������"),
		   ch->getPC( )->invis_level.getValue( ),
		   ch->getPC( )->incog_level.getValue( ) )
	    << endl;

    // Collect information from various attributes, such as craft professions.    
    list<DLString> attrLines;
    if (ch->getPC()->getAttributes( ).handleEvent( ScoreArguments( ch->getPC(), attrLines ) ))
	for (list<DLString>::iterator l = attrLines.begin( ); l != attrLines.end( ); l++) {
	    buf << *l << endl;
	}

    ch->send_to( buf );

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
	interpret_raw( ch, "affects", "nocolor noempty" );
}

CMDRUNP( count )
{
    int count, max_on;
    Descriptor *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for ( d = descriptor_list; d != 0; d = d->next )
        if (d->connected == CON_PLAYING 
	    && d->character
	    && ch->can_see( d->character ))
	    count++;

    max_on = Descriptor::getMaxOnline( );

    if (max_on == count)
        sprintf(buf,"����� %d %s. ��� �������� �� �������.\n\r",count,
                count == 1 ? "�����" : ((count > 1 && count < 5) ? "������" : "�������"));
    else
	sprintf(buf,"����� %d %s. �������� �� ������� ���: %d.\n\r",count,
                count == 1 ? "�����" : ((count > 1 && count < 5) ? "������" : "�������"),
                max_on);

    ch->send_to(buf);
}

CMDRUNP( compare )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    Object *obj1;
    Object *obj2;
    int value1;
    int value2;
    const char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	ch->send_to( "�������� ��� � � ���.?\n\r");
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == 0 )
    {
	ch->send_to( "� ���� ��� �����.\n\r");
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != 0; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != wear_none
	    &&  ch->can_see(obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == 0)
	{
	    ch->send_to("�� ���� ��� ������, � ��� ����� ���� �� ��������.\n\r");
	    return;
	}
    }

    else if ( (obj2 = get_obj_carry(ch,arg2) ) == 0 )
    {
	ch->send_to("� ���� ��� �����.\n\r");
	return;
    }

    msg		= 0;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "�� ����������� %1$O4 � ���%1$G��|��|��|��� �����. �������� ���������.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "�� �� ������ �������� %1$O4 � %2$O4.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "�� �� ������ �������� %1$O4 � %2$O4.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
	    	value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
	    	value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == 0 )
    {
	     if ( value1 == value2 ) msg = "%1$^O1 � %2$O1 �������� ���������.";
	else if ( value1  > value2 ) msg = "%1$^O1 ������%1$n��|�� ����� ��� %2$O1.";
	else                         msg = "%1$^O1 ������%1$n��|�� ���� ��� %2$O1.";
    }
    
    ch->pecho( msg, obj1, obj2 );
}



CMDRUNP( credits )
{
    do_help( ch, "credits" );
    return;
}

static void format_where( Character *ch, Character *victim )
{
    bool fPK, fAfk;
    
    fPK = (!victim->is_npc( ) 
	    && victim->getModifyLevel( ) >= PK_MIN_LEVEL 
	    && !is_safe_nomessage( ch, victim->getDoppel( ch ) ));
    fAfk = IS_SET(victim->comm, COMM_AFK);

    ch->pecho( "%-25C1 {x%s{x%s %-42s{x",
		victim,
		fPK  ? "({rPK{x)"  : "    ",
		fAfk ? "[{CAFK{x]" : "     ",
		victim->in_room->name );
}

static bool rprog_where( Character *ch, const char *arg )
{
    FENIA_CALL( ch->in_room, "Where", "Cs", ch, arg );
    return false;
}

CMDRUNP( where )
{
    Character *victim = 0;
    Descriptor *d;
    bool found;
    bool fPKonly = false;
    DLString arg( argument );

    ch->setWaitViolence( 1 );

    if (eyes_blinded( ch )) {
	ch->println( "�� �� ������ ������ ����!" );
	return;
    }
    
    if (eyes_darkened( ch )) {
	ch->println( "�� ������ �� ������! ������� �����!" );
	return;
    }

    arg.stripWhiteSpace( );
    arg.toLower( );
    
    if (arg_is_pk( arg ))
	fPKonly = true;
    
    if (rprog_where( ch, arg.c_str( ) ))
	return;

    if (arg.empty( ) || fPKonly)
    {
	ch->printf( "�� ���������� � ��������� {W%s{x. �������� �� ����:\r\n",
	             ch->in_room->area->name );
	found = false;

	for ( d = descriptor_list; d; d = d->next )
	{
	    if (d->connected != CON_PLAYING)
		continue;
	    if (( victim = d->character ) == 0)
		continue;
	    if (victim->is_npc( ))
		continue;
	    if (!victim->in_room || victim->in_room->area != ch->in_room->area)
		continue;
	    if (IS_SET(victim->in_room->room_flags, ROOM_NOWHERE))
		continue;
	    if (!ch->can_see( victim ))
		continue;
	    if (fPKonly && is_safe_nomessage( ch, victim ))
		continue;
	    
	    found = true;
	    format_where( ch, victim );
	}

	if (!found)
	    ch->send_to( "������.\n\r");
    }
    else
    {
	found = false;
	for ( victim = char_list; victim != 0; victim = victim->next )
	{
	    if ( victim->in_room != 0
		    && victim->in_room->area == ch->in_room->area
		    && ( !victim->is_npc()
		    || ( victim->is_npc() && !IS_SET(victim->act, ACT_NOWHERE) ) )
		    && ch->can_see( victim )
		    && is_name( arg.c_str(), victim->getNameP( '7' ).c_str() )
		    && !IS_SET(victim->in_room->room_flags, ROOM_NOWHERE))
	    {
		found = true;
		format_where( ch, victim );
	    }
	}

	if (!found)
	    act_p( "�� �� �������� $T.", ch, 0, arg.c_str(), TO_CHAR,POS_RESTING );
    }
}




CMDRUNP( consider )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    const char *msg;
    const char *align;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	ch->send_to( "�������� ���� ���� � ���?\n\r");
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
	ch->send_to( "��� ��� �����.\n\r");
	return;
    }

    if (is_safe(ch,victim))
    {
	ch->send_to("���� �� ����� �� ����.\n\r");
	return;
    }

    victim = victim->getDoppel( );

    diff = victim->getModifyLevel() - ch->getModifyLevel();

         if ( diff <= -10 ) msg = "�� ������ ����� $C4 ���� ��� ������.";
    else if ( diff <=  -5 ) msg = "$C1 �� �������� ����.";
    else if ( diff <=  -2 ) msg = "�� ������ ����� ������ $C4.";
    else if ( diff <=   1 ) msg = "���������� ��������!";
    else if ( diff <=   4 ) msg = "$C1 ������� '���������� �����, �����?'.";
    else if ( diff <=   9 ) msg = "$C1 ������� ��� ����� ��������������.";
    else                    msg = "�� ������� �������� �������� ������!";

    if (IS_EVIL(ch) && IS_EVIL(victim))
      align = "$C1 ������ ���������� ����.";
    else if (IS_GOOD(victim) && IS_GOOD(ch))
      align = "$C1 ������� ������������ ����.";
    else if (IS_GOOD(victim) && IS_EVIL(ch))
      align = "$C1 ��������� ����, ������� �������� ���� �� ���� �����.";
    else if (IS_EVIL(victim) && IS_GOOD(ch))
      align = "$C1 ������ ������� ��� �����.";
    else if (IS_NEUTRAL(ch) && IS_EVIL(victim))
      align = "$C1 ������ ����������.";
    else if (IS_NEUTRAL(ch) && IS_GOOD(victim))
      align = "$C1 ��������� ���������.";
    else if (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))
      align = "$C1 �������� �������� ����������������.";
    else
      align = "$C1 �������� ���������� ����������������.";

    act_p( msg, ch, 0, victim, TO_CHAR,POS_RESTING );
    act_p( align, ch, 0, victim, TO_CHAR,POS_RESTING);
    return;
}

static bool fix_title( PCharacter *ch, DLString &title )
{
    if (DLString( "{" ).strSuffix( title )
	&& !DLString( "{{" ).strSuffix( title ))
    {
	title.cutSize( title.length( ) - 1 );
    }

    title.replaces( "{/", "" );
    title.replaces( "{*", "" );
    title.replaces( "{+", "" );

    if (title.colorLength( ) > 50) {
	ch->println( "������� ������� �����." );
	return false;
    }
    
    return true;
}

CMDRUNP( title )
{
    DLString arg = argument;
    PCharacter *pch = ch->getPC( );

    if (ch->is_npc( ))
	return;

    if (IS_SET(ch->act, PLR_NO_TITLE)) {
	ch->println( "�� �� ������ ������� �����." );
	return;
    }
    
    if (arg.empty( ) || arg_is_show( arg )) {
	ostringstream buf;
	const DLString &title = pch->getTitle( );
	DLString parsed = pch->getParsedTitle( );
	parsed.stripWhiteSpace( );
	
	if (parsed.empty( )) {
	    buf << "� ���� ��� ������." << endl;
	}
	else {
	    buf << "�� ������ ����� " << parsed << "{x";
	    if (parsed != title)
		buf << " (" << title << "{x)";
	    buf << "." << endl;
	}

	pch->send_to( buf );
	return;
    }

    if (arg == "clear" || arg == "��������") {
	pch->setTitle( DLString::emptyString );
	pch->println( "����� ������." );
	return;
    }

    if (!fix_title( pch, arg ))
	return;

    pch->setTitle( arg );

    pch->printf( "������ �� {W%s{x%s{x\n\r", 
	         pch->getName( ).c_str( ), 
		 pch->getParsedTitle( ).c_str( ) );
}

static bool fix_pretitle( PCharacter *ch, DLString &title )
{
    ostringstream buf;
    mudtags_raw( title.c_str( ), buf );

    DLString stripped = buf.str( );
    DLString nospace = stripped;
    nospace.stripWhiteSpace( );
    
    if (stripped.size( ) > 25) {
	ch->println( "������� ������� ��������!" );
	return false;
    }
    
    if (nospace.size( ) != stripped.size( )) {
	ch->println( "� ������ ��� � ����� ��������� �� ������ ���� ��������." );
	return false;
    }
    
    for (unsigned int i = 0; i < stripped.size( ); i++)
	if (!dl_isalpha( stripped[i] ) 
		&& stripped[i] != ' ' 
		&& stripped[i] != '\'') 
	{
	    ch->println( "� ��������� ��������� ������������ ������ �����, ������� � ��������� �������." );
	    return false;
	}

    if (stripped.size( ) != title.size( )) {
	DLString buf;
	buf << "{1" << title << "{x{2";
	title = buf;
    }

    return true;
}

CMDRUNP( pretitle )
{
    PCharacter *pch = ch->getPC( );
    DLString arg = argument;
    DLString rus;

    if (!pch)
        return;

    if (IS_SET(pch->act, PLR_NO_TITLE)) {
         pch->println( "�� �� ������ �������� ��������.");
         return;
    }
    
    if (arg.empty( ) || arg_is_show( arg )) {
	DLString eng = pch->getPretitle( );
	DLString rus = pch->getRussianPretitle( );

	pch->printf( "���� ��������: %s\r\n������� ��������: %s\r\n",
	             (eng.empty( ) ? "(���)" : eng.c_str( )),
		     (rus.empty( ) ? "(���)" : rus.c_str( )) );
        return;
    }
    
    if (arg == "clear" || arg == "��������") {
	pch->setPretitle( DLString::emptyString );
	pch->setRussianPretitle( DLString::emptyString );
	pch->println("������� � ���������� ��������� �������.");
	return;
    }
    
    rus = arg.getOneArgument( );

    if (rus == "rus" || rus == "���") {
	if (!fix_pretitle( pch, arg ))
	    return;

	pch->setRussianPretitle( arg );
	pch->printf( "������� ��������: %s\r\n", arg.c_str( ) );
    }
    else { 
	arg = argument;

	if (!fix_pretitle( pch, arg ))
	    return;

	pch->setPretitle( arg );
	pch->printf( "���� ��������: %s\r\n", arg.c_str( ) );
    }
}


CMDRUNP( report )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"� ���� %d/%d ����� (hp) %d/%d ������� (mana) %d/%d �������� (mv).",
	ch->hit.getValue( ),  ch->max_hit.getValue( ),
	ch->mana.getValue( ), ch->max_mana.getValue( ),
	ch->move.getValue( ), ch->max_move.getValue( ) );
    do_say( ch, buf );

    return;
}

/*
 * 'Wimpy' originally by Dionysos.
 */
CMDRUNP( wimpy )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ((ch->getProfession( ) == prof_samurai) && (ch->getRealLevel( ) >=10))
	{
	 sprintf(buf,"�������!!! ��� ����� ������� ������� ������� ��� �������.\n\r");
	 ch->send_to(buf);
	 if (ch->wimpy != 0) ch->wimpy = 0;
	 return;
	}

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else  wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	ch->send_to( "���� ������ ����������� ���� ��������.\n\r");
	return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
	ch->send_to( "��� ����� ������� ������� ������� ��� ����.\n\r");
	return;
    }

    ch->wimpy	= wimpy;

    sprintf( buf, "�� ����������� ������� ��� %d ����� (hit points).\n\r", wimpy );
    ch->send_to( buf);
    return;
}



CMDRUNP( password )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( ch->is_npc() )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( dl_isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( dl_isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';
    
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	ch->send_to( "���������: {lR������{lEpassword{lx <������> <�����>.\n\r");
	return;
    }
    
    if (!password_check( ch->getPC( ), arg1 ))
    {
	ch->setWait(40 );
	ch->send_to( "�������� ������. ��������� 10 ������.\n\r");
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	ch->send_to("����� ������ ������ ��������� ����� ���� ��������.\n\r");
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = arg2;
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    ch->send_to("����� ������ ����������, �������� ��� ���.\n\r");
	    return;
	}
    }
   
    password_set( ch->getPC( ), pwdnew );
    ch->getPC( )->save( );
    ch->send_to( "Ok.\n\r");
    return;
}

/* RT configure command */


CMDRUNP( request )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	Character *victim;
	Object  *obj;
	Affect af;

	if ( ch->isAffected(gsn_gratitude))
	{
		ch->send_to("������� �������.\n\r");
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if (ch->is_npc())
		return;

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		ch->send_to( "��� � � ���� �� ������ ���������?\n\r");
		return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
	{
		ch->send_to( "����� ����� ���.\n\r");
		return;
	}

	if (!victim->is_npc())
	{
		ch->send_to("����� ������ ��������: say <������ ��� ���>!\n\r");
		return;
	}

	if (victim->getNPC()->behavior 
	    && IS_SET(victim->getNPC()->behavior->getOccupation( ), (1 << OCC_SHOPPER)))
	{
		ch->send_to("������ -- ����!\n\r");
		return;
	}

	if (!IS_AWAKE(victim)) {
	    interpret_raw( victim, "snore" );
	    return;
	}

	if ((!IS_GOOD(ch) && !victim->getRace( )->getAttitude( *ch->getRace( ) ).isSet( RACE_DONATES ))
		|| IS_EVIL(ch))
	{
		do_say(victim, "� ���� �������� ����, � ������ ���� �� ���!");
		return;
	}

	if (ch->move < (50 + ch->getRealLevel( )))
	{
		do_say(victim, "�� ��������� ������, �����, ��������� �������?");
		return;
	}

	ch->setWaitViolence( 1 );
	ch->move -= 10;
	ch->move = max((int)ch->move, 0);

	if (victim->getRealLevel( ) >= ch->getRealLevel( ) + 10 || victim->getRealLevel( ) >= ch->getRealLevel( ) * 2)
	{
		do_say(victim, "����� ���� �����, �����.");
		return;
	}

	if ( ( ( obj = get_obj_carry(victim , arg1 ) ) == 0
			&& (obj = get_obj_wear(victim, arg1)) == 0)
		|| IS_SET(obj->extra_flags, ITEM_INVENTORY))
	{
		do_say(victim, "������, � ���� ��� �����.");
		return;
	}
	
	if (victim->getRace( )->getAttitude( *ch->getRace( ) ).isSet( RACE_DONATES ))
	{
	    if (IS_EVIL( victim )) {
		interpret( victim, "grin" );
		return;
	    }

	} else if (!IS_GOOD(victim))
	{
		do_say(victim, "� �� ��� ���� ������!!");
		interpret_raw( victim, "murder", ch->getNameP( ));
		return;
	}

	if ( obj->wear_loc != wear_none )
		unequip_char(victim, obj);

	if ( !can_drop_obj( ch, obj ) )
	{
		do_say(victim, "������, �� ��� ���� ��������, � �� ���� ���������� �� ���.");
		return;
	}

	if ( ch->carry_number + obj->getNumber( ) > ch->canCarryNumber( ) )
	{
		ch->send_to( "���� ���� �����.\n\r");
		return;
	}

	if ( ch->carry_weight + obj->getWeight( ) > ch->canCarryWeight( ) )
	{
		ch->send_to( "�� �� ������ ����� ����� ���.\n\r");
		return;
	}

	if ( !ch->can_see( obj ) )
	{
		ch->send_to( "�� �� ������ �����.\n\r");
		return;
	}

	if ( obj->pIndexData->vnum == 520 ) // Knight's key
	{
		ch->send_to("������, �� �� ������ ���� ���.\n\r");
		return;
	}

	obj_from_char( obj );
	obj_to_char( obj, ch );
	act_p( "$c1 ������ $o4 � $C2.", ch, obj, victim, TO_NOTVICT,POS_RESTING );
	act_p( "�� ������� $o4 � $C2.",   ch, obj, victim, TO_CHAR,POS_RESTING    );
	act_p( "$c1 ������ $o4 � ����.", ch, obj, victim, TO_VICT,POS_RESTING    );
	
	omprog_give( obj, victim, ch );

	ch->move -= ( 50 + ch->getModifyLevel() );
	ch->move = max( (int)ch->move, 0 );
	ch->hit -= 3 * ( ch->getModifyLevel() / 2 );
	ch->hit = max( (int)ch->hit, 0 );

	act_p("�� ���������� ������������� �� ������� $C2.", ch, 0, victim,TO_CHAR,POS_RESTING);

	af.type = gsn_gratitude;
	af.where = TO_AFFECTS;
	af.level = ch->getModifyLevel();
	af.duration = ch->getModifyLevel() / 10;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = 0;
	affect_to_char ( ch,&af );

	return;
}




CMDRUNP( identify )
{
    Object *obj;
    Character *rch;

    if ( ( obj = get_obj_carry( ch, argument ) ) == 0 )
    {
       ch->send_to( "� ���� ��� �����.\n\r");
       return;
    }

    rch = find_mob_with_act( ch->in_room, ACT_SAGE );

    if (!rch)
    {
       ch->send_to("��� ����� ������ ��������� �� ������ �� ���� ����.\n\r");
       return;
    }

    if (ch->is_immortal( )) {
	act_p( "$c1 ������� �� ����!\n\r", rch, obj, ch, TO_VICT,POS_RESTING );
    }
    else if (ch->gold < 20) {
	tell_dim( ch, rch, "� ���� ���� 20 ������� ����, ����� ��� ���������!" );
	return;
    }
    else {
       ch->gold -= 20;
       ch->send_to("���� ������� ���������� ����������� �����.\n\r");
    }

    act_p( "$c1 �������� ������� �� $o4.", rch, obj, 0, TO_ROOM,POS_RESTING );
    
    if (gsn_identify->getSpell( ))
	gsn_identify->getSpell( )->run( ch, obj, gsn_identify, 0 );
}

/* room affects */
CMDRUNP( raffects )
{
    ostringstream buf;
    
    for (Affect *paf = ch->in_room->affected; paf != 0; paf = paf->next)
	if (paf->type->getAffect( ))
	    paf->type->getAffect( )->toStream( buf, paf );
    
    if (buf.str( ).empty( )) 
	buf << "� ���� ����� ��� ������ ����������." << endl;

    ch->send_to( buf );
}




CMDRUNP( demand )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  Character *victim;
  Object  *obj;
  int chance;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (ch->is_npc())
	return;

  if (ch->getTrueProfession( ) != prof_anti_paladin)
    {
	ch->println( "�� ������ �� ��������� ����� �����." );
      return;
    }

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	ch->println( "����������� ��� � � ����?" );
      return;
    }

  if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
    {
	ch->println( "����� ��� ���." );
      return;
    }

    if (!victim->is_npc( )) {
	ch->println( "������ ���� � ������." );
	return;
    }

    if (IS_SET(victim->act, ACT_NODEMAND)) {
	act( "$C1 �� ���������� ������ ����������.", ch, 0, victim, TO_CHAR);
	return;
    }
  
    if (victim->getNPC()->behavior 
	&& IS_SET(victim->getNPC()->behavior->getOccupation( ), (1 << OCC_SHOPPER))) 
    {
	ch->send_to("������ -- ����!\n\r");
	return;
    }
  
  ch->setWaitViolence( 1 );

  chance = IS_EVIL(victim) ? 10 : IS_GOOD(victim) ? -5 : 0;
  chance += (ch->getCurrStat(STAT_CHA) - 15) * 10;
  chance += ch->getModifyLevel() - victim->getModifyLevel();

  if( victim->getModifyLevel() >= ch->getModifyLevel() + 10 || victim->getModifyLevel() >= ch->getModifyLevel() * 2)
	chance = 0;

    if (number_percent() > chance) {
	 do_say(victim, "� �� ��������� ������ �������� ����!");
	 interpret_raw( victim, "murder", ch->getNameP( ));
	 return;
    }

  if (( ( obj = get_obj_carry(victim , arg1 ) ) == 0
      && (obj = get_obj_wear(victim, arg1)) == 0)
      || IS_SET(obj->extra_flags, ITEM_INVENTORY))
    {
      do_say(victim, "������, � ���� ��� �����.");
      return;
    }


  if ( obj->wear_loc != wear_none )
    unequip_char(victim, obj);

  if ( !can_drop_obj( ch, obj ) )
    {
      do_say(victim,
	"��� ���� ��������, � � �� ���� ���������� �� ���. ������ ����, ����������.");
      return;
    }

  if ( ch->carry_number + obj->getNumber( ) > ch->canCarryNumber( ) )
    {
      ch->send_to( "���� ���� �����.\n\r");
      return;
    }

  if ( ch->carry_weight + obj->getWeight( ) > ch->canCarryWeight( ) )
    {
      ch->send_to( "�� �� ������� ����� ����� �������.\n\r");
      return;
    }

  if ( !ch->can_see( obj ) )
    {
      act_p( "�� �� ������ �����.", ch, 0, victim, TO_CHAR,POS_RESTING );
      return;
    }

    act( "$c1 ������� $o4 � $C2.", ch, obj, victim, TO_NOTVICT);
    act( "�� �������� $o4 � $C2.",   ch, obj, victim, TO_CHAR);
    act( "$c1 ������� � ���� $o4.", ch, obj, victim, TO_VICT);

    obj_from_char( obj );
    obj_to_char( obj, ch );

    omprog_give( obj, victim, ch );

    ch->println("���� ���������� ��������� ���� � ������.");
}



#define MILD(ch)     (IS_SET((ch)->comm, COMM_MILDCOLOR))

CMDRUNP( score )
{
    int ekle=0;
    PCharacter *pch = ch->getPC( );
    
    const char *CLR_FRAME = MILD(ch) ? "{Y" : "{G";
    const char *CLR_BAR   = MILD(ch) ? "{D" : "{C";
    const char *CLR_CAPT  = MILD(ch) ? "{g" : "{R";
    
    if (ch->is_npc( )) {
	interpret_raw( ch, "oscore" );
	return;
    }
    
    XMLAttributeTimer::Pointer qd = pch->getAttributes( ).findAttr<XMLAttributeTimer>( "questdata" );
    int age = pch->age.getYears( );
    Room *room = get_room_index( pch->getHometown( )->getAltar( ) );
    bool fRus = ch->getConfig( )->rucommands;
    DLString profName = ch->getProfession( )->getNameFor( ch );

    if (ch->getProfession( ) == prof_universal) 
	profName << "+"
		 << (pch->getSubProfession( ) != prof_none ? 
		        pch->getSubProfession( )->getWhoNameFor( ch ) : "   ");
	
    ostringstream name;
    DLString title = pch->getParsedTitle( );
    name << ch->seeName( ch, '1' ) << "{x ";
    vistags_convert( title.c_str( ), name, ch );

    ch->printf( 
"%s\n\r"
"      /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\\n\r", 
             CLR_FRAME);
    ch->println(
	fmt ( 0, "     %s|   %s%-50.50s {y%3d{x %4s   %s|____|",
	        CLR_FRAME,
		CLR_CAPT,
		name.str( ).c_str( ),
		age,
		GET_COUNT(age, "���", "����", "���"),
		CLR_FRAME ) );
		
	
    ch->printf(
"     |%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|\n\r" 
"     | %s�������:{x  %3d        %s|%s {lR����:{lE Str:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %s�������   :{x %-10s %s|\n\r"
"     | %s���� :{x  %-12s %s| %s{lR��  :{lE Int:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %s�������   :{x   %3d      %s|\n\r"
"     | %s���  :{x  %-11s  %s| %s{lR����:{lE Wis:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %s����������:{x   %3d      %s|\n\r"
"     | %s�����:{x  %-13s%s| %s{lR����:{lE Dex:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %s�����. ������:{x  %-5d%s  |\n\r"
"     | %s{lR������:{lEAlign: {lx{x %-11s  %s| %s{lR����:{lE Con:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %s�����. �����:{x   %-3d %s   |\n\r"
"     | %s{lR���� {lEEthos{lx:{x  %-12s %s| %s{lR����:{lE Cha:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %s%s :{x   %3d      %s|\n\r"
"     | %s���  :{x  %-30s %s| {Y%-22s %s|\n\r"		
"     |%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|\n\r",

	    CLR_BAR, CLR_FRAME,

	    CLR_CAPT,
	    ch->getRealLevel( ),
	    CLR_BAR,
	    CLR_CAPT,
	    ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR), pch->getMaxStat(STAT_STR),
	    CLR_BAR,
	    CLR_CAPT,
	    ch->getReligion( )->getNameFor( ch ).ruscase( '1' ).c_str( ),
	    CLR_FRAME,

	    CLR_CAPT,
	    pch->getRace( )->getPC( )->getScoreNameFor( ch, ch ).c_str( ), 
	    CLR_BAR,
	    CLR_CAPT,
	    ch->perm_stat[STAT_INT], ch->getCurrStat(STAT_INT), pch->getMaxStat(STAT_INT),
	    CLR_BAR, 
	    CLR_CAPT,
	    pch->practice.getValue( ),
	    CLR_FRAME,

	    CLR_CAPT,
	    ch->getSex( ) == 0 ? "�������" : ch->getSex( ) == SEX_MALE ? "�������" : "�������",
	    CLR_BAR,
	    CLR_CAPT,
	    ch->perm_stat[STAT_WIS], ch->getCurrStat(STAT_WIS), pch->getMaxStat(STAT_WIS),
	    CLR_BAR,
	    CLR_CAPT,
	    pch->train.getValue( ),
	    CLR_FRAME,

	    CLR_CAPT,
	    profName.c_str( ),
	    CLR_BAR,
	    CLR_CAPT,
	    ch->perm_stat[STAT_DEX], ch->getCurrStat(STAT_DEX), pch->getMaxStat(STAT_DEX),
	    CLR_BAR,
	    CLR_CAPT,
	    pch->questpoints.getValue( ),
	    CLR_FRAME,
    
	    CLR_CAPT,
	    fRus ? (IS_GOOD(ch) ? "������" : IS_EVIL(ch) ? "����" : "�����������")
		 : align_table.name( ALIGNMENT(ch) ).c_str( ),
	    CLR_BAR,
	    CLR_CAPT,
	    ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON), pch->getMaxStat(STAT_CON),
	    CLR_BAR,
	    CLR_CAPT,
	    qd ? qd->getTime( ) : 0,
	    CLR_FRAME,

	    CLR_CAPT,
	    fRus ?  ethos_table.message( ch->ethos ).cutSize( 12 ).c_str( )
		  : ethos_table.name( ch->ethos ).c_str( ),
	    CLR_BAR,
	    CLR_CAPT,
	    ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA), pch->getMaxStat(STAT_CHA),
	    CLR_BAR,
	    CLR_CAPT,
	    ch->getProfession( ) == prof_samurai 
		?  "{lR�������  {lEDeath    {lx" : "{lR�������� {lEWimpy    {lx" ,
	    ch->getProfession( ) == prof_samurai 
		? pch->death.getValue( ) : ch->wimpy.getValue( ),
	    CLR_FRAME,

	    CLR_CAPT,
	    room ? room->area->name : "�������",
	    CLR_BAR,
	    msgtable_lookup( msg_positions, ch->position ),
	    CLR_FRAME,

	    CLR_BAR, CLR_FRAME);

    if (pch->guarding != 0) {
	ekle = 1;
	ch->printf( 
"     | {w�� ���������    :{Y %-10s                                    %s|\n\r",
	    ch->seeName( pch->guarding, '4' ).c_str(),
	    CLR_FRAME);
    }

    if (pch->guarded_by != 0) {
	ekle = 1;
	ch->printf( 
"     | {w���� ��������     :{Y %-10s                                  %s|\n\r",
	ch->seeName( pch->guarded_by, '1' ).c_str(),
	CLR_FRAME);
    }

    for (int i = 0; i < desireManager->size( ); i++) {
	ostringstream buf;
	
	desireManager->find( i )->report( ch->getPC( ), buf );

	if (!buf.str( ).empty( )) {
	    ekle = 1;
	    ch->printf( "     | {w%-64s%s|\r\n", 
			buf.str( ).c_str( ),
			CLR_FRAME );
	}
    }

    if (ch->is_adrenalined()) {
	ekle = 1;
	ch->printf( 
"     | {y��������� ����� � ����� �����!                                  %s|\n\r",
                 CLR_FRAME );
    }

    if (ch->is_immortal()) {
	ekle = 1;
	ch->printf( 
"     | {w{lR�����������:{lEInvisible:  {lx {lR������{lElevel{lx %3d   "
         "{lR���������{lEIncognito  {lx: {lR������{lElevel{lx %3d                 %s|\n\r",
              pch->invis_level.getValue( ),
	      pch->incog_level.getValue( ),
	      CLR_FRAME);
    }

    list<DLString> attrLines;
    if (ch->getPC()->getAttributes( ).handleEvent( ScoreArguments( ch->getPC(), attrLines ) )) {
        ekle = 1;
	for (list<DLString>::iterator l = attrLines.begin( ); l != attrLines.end( ); l++) {
	    ch->printf("     | {w%-64s%s|\r\n", 
			l->c_str(),
			CLR_FRAME);
	}
    }

    if (ekle) {
	ch->printf( 
"     |%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|\n\r",
                CLR_BAR,
		CLR_FRAME);
    }


if ( ch->getRealLevel( ) >= 20 )
{
    ch->printf( 
"     | %s����          :{x     %3d/%-4d        %s������ �� ������:{x   %-5d   %s|\n\r"
"     | %s���           :{x  %6d/%-8d    %s������ �� ������:{x   %-5d   %s|\n\r"
"     | %s������        :{Y %-10d          %s������ �� ��������:{x %-5d   %s|\n\r"
"     | %s�������       :{W %-10d          %s������ �� ��������:{x %-5d   %s|\n\r"
"     | %s������ �����  :{x %-6d              %s{lR������ �� ����������{lESaves vs Spell      {lx:{x %4d  %s|\n\r",
	CLR_CAPT,
	ch->carry_number, ch->canCarryNumber( ),
	CLR_CAPT,
	GET_AC(ch,AC_PIERCE),
	CLR_FRAME,

	CLR_CAPT,
	ch->getCarryWeight( )/10, ch->canCarryWeight( )/10,
	CLR_CAPT,
	GET_AC(ch,AC_BASH),
	CLR_FRAME,

        CLR_CAPT,
	ch->gold.getValue( ),
	CLR_CAPT,
	GET_AC(ch,AC_SLASH),
	CLR_FRAME,

	CLR_CAPT,
	ch->silver.getValue( ),
	CLR_CAPT,
	GET_AC(ch,AC_EXOTIC),
	CLR_FRAME,

	CLR_CAPT,
	ch->exp.getValue( ),
	CLR_CAPT,
	ch->saving_throw.getValue( ),
	CLR_FRAME);
}
else
{
    ch->printf( 
"     | %s����   :{x %3d/%-4d            %s������ �� ������:{x   %-12s   %s|\n\r"
"     | %s���    :{x %6d/%-8d     %s������ �� ������:{x   %-12s   %s|\n\r"
"     | %s������ :{Y %-10d          %s������ �� ��������:{x %-12s   %s|\n\r"
"     | %s�������:{W %-10d          %s������ �� ��������:{x %-12s   %s|\n\r"
"     | %s������ �����:{x   %-6d                                          %s|\n\r",
	CLR_CAPT,
	ch->carry_number, ch->canCarryNumber( ),
	CLR_CAPT,
	msgtable_lookup( msg_armor, GET_AC(ch, AC_PIERCE) ),
	CLR_FRAME,

	CLR_CAPT,
	ch->getCarryWeight( )/10, ch->canCarryWeight( )/10,
	CLR_CAPT,
	msgtable_lookup( msg_armor, GET_AC(ch, AC_BASH) ),
	CLR_FRAME,

	CLR_CAPT,
	ch->gold.getValue( ),
	CLR_CAPT,
	msgtable_lookup( msg_armor, GET_AC(ch, AC_SLASH) ),
	CLR_FRAME,

	CLR_CAPT,
	ch->silver.getValue( ),
	CLR_CAPT,
	msgtable_lookup( msg_armor, GET_AC(ch, AC_EXOTIC) ),
	CLR_FRAME,

	CLR_CAPT,
	ch->exp.getValue( ),
	CLR_FRAME);
}

    ch->printf( 
"     | %s����� �� ������:{x %-6d                                         %s|\n\r"
"     |                                    %s�����:{x %5d / %5d         %s|\n\r",
	CLR_CAPT,
	pch->getExpToLevel( ),
	CLR_FRAME,

	CLR_CAPT,
	ch->hit.getValue( ), ch->max_hit.getValue( ),
	CLR_FRAME);

if ( ch->getRealLevel( ) >= 20 )
{
    ch->printf( 
"     | %s{lR��������{lEHitroll {lx      :{x   %-3d            %s�������:{x %5d / %5d         %s|\n\r"
"     | %s{lR����   {lEDamroll{lx       :{x   %-3d           %s��������:{x %5d / %5d         %s|\n\r",
	CLR_CAPT,
	ch->hitroll.getValue( ),
	CLR_CAPT,
	ch->mana.getValue( ), ch->max_mana.getValue( ),
	CLR_FRAME,

	CLR_CAPT,
	ch->damroll.getValue( ),
	CLR_CAPT,
	ch->move.getValue( ), ch->max_move.getValue( ),
	CLR_FRAME);
}
else {
    ch->printf( 
"     |                                  %s�������:{x %5d / %5d         %s|\n\r"
"     |                                 %s��������:{x %5d / %5d         %s|\n\r",
	CLR_CAPT,
	ch->mana.getValue( ), ch->max_mana.getValue( ),
	CLR_FRAME,

	CLR_CAPT,
	ch->move.getValue( ), ch->max_move.getValue( ),
	CLR_FRAME);
}


    ch->printf( 
"  /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r"
"  \\________________________________________________________________\\__/{x\n\r");

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
	interpret_raw( ch, "affects", "noempty");
}

CMDRUNP( areas )
{
    ostringstream buf;
    AREA_DATA *pArea;
    int minLevel, maxLevel, level;
    DLString arguments( argument ), args, arg1, arg2;
    
    arguments.stripWhiteSpace( );
    level = minLevel = maxLevel = -1;
    args = arguments;
    arg1 = arguments.getOneArgument( );
    arg2 = arguments.getOneArgument( );
    
    if (!arg1.empty( ) && !arg2.empty( ) && arg1.isNumber( ) != arg2.isNumber( )) {
	ch->println( "�������������: \r\n"
	             "{lEareas [<level> | <min level> <max level> | <string>]"
		     "{lR���� [<�������> | <���.�������> <����.�������> | <��������>]{lx" );
	return;
    }
    
    try {
	if (arg1.isNumber( )) {
	    level = minLevel = arg1.toInt( );
	    args.clear( );
	}

	if (arg2.isNumber( )) {
	    level = -1;
	    maxLevel = arg2.toInt( );
	    args.clear( );
	}
    } catch (const ExceptionBadType & ) {
	ch->send_to( "������� ���� ������ �������.\r\n" );
	return;
    }
    
    if (level != -1) 
	buf << "���� ���� Dream Land ��� ������ " << level << ":" << endl;
    else if (!args.empty( ))
	buf << "������� ����: " << endl;
    else if (minLevel != -1 && maxLevel != -1)
	buf << "���� ���� Dream Land, ��� ������� " 
	    << minLevel << " - " << maxLevel << ":" << endl;
    else
	buf << "��� ���� ���� Dream Land: " << endl;
    
    buf << "������    ��������                                 ������  {W({x�����������{W){x" << endl
        << "------------------------------------------------------------------------" << endl;

    for (pArea = area_first; pArea; pArea = pArea->next) {
	if (IS_SET(pArea->area_flag, AREA_HIDDEN)) 
	    continue;
	
	if (level != -1) {
	    if (pArea->low_range > level || pArea->high_range < level)
		continue;
	}
	else if (minLevel != -1 && maxLevel != -1) {
	    if (pArea->low_range < minLevel || pArea->high_range > maxLevel)
		continue;
	}

	if (!args.empty( )) {
            DLString aname = DLString( pArea->name ).colourStrip( );
            DLString altname = DLString( pArea->altname ).colourStrip( );
            DLString acredits = DLString( pArea->credits ).colourStrip( );
            if (!is_name( args.c_str( ), aname.c_str( ) ) 
                    && !is_name( args.c_str( ), acredits.c_str( ) )
                    && !is_name( args.c_str( ), altname.c_str( ) ))
	        continue;
        }
	
	buf << fmt( ch, "(%3d %3d) %-40s %s",
	                pArea->low_range, pArea->high_range, 
			pArea->name, 
			pArea->authors );

	if (str_cmp( pArea->translator, "" ))
	    buf << " {W({x" << pArea->translator << "{W){x";

	buf << endl;
    }
    
    buf << endl;
    page_to_char( buf.str( ).c_str( ), ch );	
}

/*-----------------------------------------------------------------
 * 'affect' command
 *----------------------------------------------------------------*/
enum {
    FSHOW_LINES = (A),
    FSHOW_TIME = (B),
    FSHOW_COLOR = (C),
    FSHOW_EMPTY = (D),
    FSHOW_RUSSIAN = (E),
};

struct AffectOutput {
    AffectOutput( ) { }
    AffectOutput( Affect *, int flags );
    
    void format_affect( Affect * );
    DLString format_affect_location( Affect * );
    DLString format_affect_bitvector( Affect * );
    DLString format_affect_global( Affect * );
    void show_affect( ostringstream &, int );

    int type;
    int duration;
    DLString name;
    list<DLString> lines;
    bool unitMinutes;
};

struct ShadowAffectOutput : public AffectOutput {
    ShadowAffectOutput( int, int flags );
};

ShadowAffectOutput::ShadowAffectOutput( int shadow, int flags )
{
    duration = shadow * 4; 
    name = "������ ����";
    type = -1;
    unitMinutes = false;
}

AffectOutput::AffectOutput( Affect *paf, int flags )
{
    type = paf->type;
    name = (IS_SET(flags, FSHOW_RUSSIAN) ? 
             paf->type->getRussianName( ) : paf->type->getName( ));
    duration = paf->duration;
    unitMinutes = true;
}

void AffectOutput::show_affect( ostringstream &buf, int flags )
{
    ostringstream f;
    DLString fmtResult;
    
    if (IS_SET(flags, FSHOW_RUSSIAN))
	f << "{Y%1$-23s{x";
    else
	f << "{r������{y: {Y%1$-15s{x";
    
    if (IS_SET(flags, FSHOW_LINES|FSHOW_TIME)) 
	f << "{y:";
    
    if (IS_SET(flags, FSHOW_LINES))
	for (list<DLString>::iterator l = lines.begin( ); l != lines.end( ); l++) {
	    if (l != lines.begin( ))
		f << "," << endl << "                        ";

	    f << "{y " << *l;
	}
    
    if (IS_SET(flags, FSHOW_TIME)) {
	if (duration < 0)
	    f << " {c���������";
	else
	    f << " {y� ������� {m%2$d{y "
	      << (unitMinutes ? "���%2$I�|��|��" : "�����%2$I��|�|�");
    }
    
    fmtResult = fmt( 0, f.str( ).c_str( ), name.c_str( ), duration );

    buf << fmtResult << "{x" << endl;
}

void AffectOutput::format_affect( Affect *paf )
{
    DLString l;

    if (!( l = format_affect_location( paf ) ).empty( ))
	lines.push_back( l );

    if (!( l = format_affect_bitvector( paf ) ).empty( ))
	lines.push_back( l );

    if (!( l = format_affect_global( paf ) ).empty( ))
	lines.push_back( l );
}

DLString AffectOutput::format_affect_location( Affect *paf )
{
    DLString buf;
    
    if (paf->location != APPLY_NONE) 
	switch (paf->location) {
	case APPLY_HEAL_GAIN:
	case APPLY_MANA_GAIN:
	    if (paf->modifier > 100)
		buf << fmt( 0, "�������� {m%1$s{y �� {m%2$d%%{y",
		               apply_flags.message( paf->location ).c_str( ),
			       paf->modifier - 100 );
	    else if (paf->modifier < 100 && paf->modifier > 0)
		buf << fmt( 0, "�������� {m%1$s{y �� {m%2$d%%{y",
		               apply_flags.message( paf->location ).c_str( ),
			       100 - paf->modifier );
	    break;
	    
	default:
	    buf << "�������� {m" << apply_flags.message( paf->location ) << "{y "
		<< "�� {m" << paf->modifier << "{y";
	    break;
	}

    return buf;
}

DLString AffectOutput::format_affect_bitvector( Affect *paf )
{
    DLString buf;

    if (paf->bitvector != 0) {
	bitstring_t b = paf->bitvector;
	const char *word = 0;
	char gcase = '1';
	const FlagTable *table = 0;

	switch(paf->where) {
	case TO_AFFECTS: 
	    table = &affect_flags;
	    word = "���������";
	    gcase = '4';
	    break;
	case TO_IMMUNE:	
	    table = &imm_flags;
	    word = "��������� �";
	    break;
	case TO_RESIST:	
	    table = &imm_flags;
	    word = "���������������� �";
	    break;
	case TO_VULN:	
	    table = &imm_flags;
	    word = "���������� �";
	    break;
	case TO_DETECTS: 
	    table = &detect_flags;
	    word = (IS_SET(b, ADET_WEB|ADET_FEAR) ?  "���������" : "�����������");
	    gcase = (IS_SET(b, ADET_WEB|ADET_FEAR) ? '4': '2');
	    break;
	}

	if (word && table)
	    buf << word << " {m" << table->messages( b, true, gcase ) << "{y";
    }

    return buf;
}

DLString AffectOutput::format_affect_global( Affect *paf )
{
    DLString buf;

    if (!paf->global.empty( )) {
	ostringstream s;
	vector<int> bits = paf->global.toArray( );
	
	switch (paf->where) {
	case TO_LIQUIDS:
	    for (unsigned int i = 0; i < bits.size( ); i++) {
		Liquid *liq = liquidManager->find( bits[i] );

		if (!s.str( ).empty( ))
		    s << ", ";

		s << "{m" <<  liq->getShortDescr( ).ruscase( '2' ).colourStrip( ) << "{x";
	    }

	    buf << "����� " << s.str( );
	    break;

	case TO_LOCATIONS:
	    if (paf->global.isSet( wear_wrist_r ))
		buf << "���������� ������ ����";
	    else if (paf->global.isSet( wear_wrist_l ))
		buf << "���������� ����� ����";
	    else
		buf << "���������� ����������";
	    break;
	}
    }

    return buf;
}

static bool __aff_sort_time__( const AffectOutput &a, const AffectOutput &b )
{
    if (a.unitMinutes == b.unitMinutes)
	return a.duration < b.duration;
    else
	return a.unitMinutes ? b.duration : a.duration;
}

static bool __aff_sort_name__( const AffectOutput &a, const AffectOutput &b )
{
    return a.name < b.name;
}

CMDRUNP( affects )
{
    ostringstream buf;
    list<AffectOutput> output;
    list<AffectOutput>::iterator o;
    int flags = FSHOW_LINES|FSHOW_TIME|FSHOW_COLOR|FSHOW_EMPTY;

    if (ch->getConfig( )->ruskills)
	SET_BIT(flags, FSHOW_RUSSIAN);
    
    for (Affect* paf = ch->affected; paf != 0; paf = paf->next ) {
	if (output.empty( ) || output.back( ).type != paf->type) 
	    output.push_back( AffectOutput( paf, flags ) );
	
	output.back( ).format_affect( paf );
    }
    
    if (HAS_SHADOW(ch))
	output.push_front( ShadowAffectOutput( ch->getPC( )->shadow, flags ) );
    
    if (arg_has_oneof( argument, "time", "�����" ))
	output.sort( __aff_sort_time__ );
    
    if (arg_has_oneof( argument, "name", "���" ))
	output.sort( __aff_sort_name__ );

    if (arg_has_oneof( argument, "desc", "����" ))
	output.reverse( );

    if (arg_has_oneof( argument, "short", "brief", "������" ))
	REMOVE_BIT(flags, FSHOW_LINES);

    if (arg_has_oneof( argument, "nocolor", "��������" ))
	REMOVE_BIT(flags, FSHOW_COLOR);

    if (arg_has_oneof( argument, "noempty" ))
	REMOVE_BIT(flags, FSHOW_EMPTY);

    if (ch->getLevel( ) < 20)
	REMOVE_BIT(flags, FSHOW_TIME|FSHOW_LINES);
    
    for (o = output.begin( ); o != output.end( ); o++) 
	o->show_affect( buf, flags );

    if (buf.str( ).empty( )) {
	if (IS_SET(flags, FSHOW_EMPTY))
	    ch->println( "�� �� ���������� ��� ��������� �����-���� ��������." );
    } 
    else {
	ch->println( "�� ���������� ��� ��������� ��������� ��������:" );
	buf << "{x";

	if (!IS_SET(flags, FSHOW_COLOR)) {
	    ostringstream showbuf;
	    mudtags_convert_nocolor( buf.str( ).c_str( ), showbuf, ch );	
	    ch->send_to( showbuf );
	}
	else
	    ch->send_to( buf );
    }
}

CMDRUNP( nohelp )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->println("�� ���������� ������ ������� ������� �� ������ ��������?");
        return;
    }

    bugTracker->reportNohelp( ch, txt );
    ch->println("��������.");
}

CMDRUNP( bug )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->println("� ����� ������ ������ �� ������ ��������?");
        return;
    }

    bugTracker->reportBug( ch, txt );
    ch->println( "������ ��������.");
}

CMDRUNP( typo )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->println("� ����� ������ �������� �� ������ ��������?");
        return;
    }

    bugTracker->reportTypo( ch, txt );
    ch->println( "�������� ��������.");
}

CMDRUNP( iidea )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->println("� ����� ������ ���� �� ������ ��������?");
        return;
    }

    bugTracker->reportIdea( ch, txt );
    ch->println( "���� ��������.");
}


CMDRUNP( help )
{
    std::basic_ostringstream<char> buf;
    HelpArticle::Pointer findHelp;
    HelpArticles::const_iterator a;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
    int count=0,number=0;
    DLString origArgument( argument );

    if ( argument[0] == '\0' ) {
	if (ch->getConfig( )->rucommands)
	    strcpy(argument, "summary_ru");
	else
	    strcpy(argument, "summary_en");
    }

    argall[0] = '\0';
    // ������� 2.create?
    if (strchr( argument , '.')){
	number = number_argument(argument, argall);
	strcpy(argument,argall);	
	if ( number < 1){
	    ch->send_to( "��� ��������� �� ������� �����.\n\r");
	    bugTracker->reportNohelp( ch, origArgument.c_str( ) );
    	    return;
	}
	/* this parts handles help a b so that it returns help 'a b' */
        while (argument[0] != '\0' )
	{
	    argument = one_argument(argument,argone);
	    if (argall[0] != '\0')
		strcat(argall," ");
	    strcat(argall,argone);
	}
	for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
	    if (!(*a)->visible( ch ))
		continue;

	    if (is_name( argall, (*a)->getKeyword( ).c_str( ) ) ){
		count++;
		if (count == number){
		    findHelp = *a;
		    break;
		}
	    }
	}
	count=(findHelp.isEmpty( )?0:1);
    }
    else{
        /* this parts handles help a b so that it returns help 'a b' */
	while (argument[0] != '\0' )
	{
	    argument = one_argument(argument,argone);
	    if (argall[0] != '\0')
		strcat(argall," ");
	    strcat(argall,argone);
	}

        buf << "�� ������� '{C" << origArgument << "{x' ������� ��������� �������� �������:" << endl << endl;

	count=0;
	for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
	    if (!(*a)->visible( ch ))
		continue;

	    if (is_name( argall, (*a)->getKeyword( ).c_str( ) ) ){
		count++;
                buf << "    {C{hh" << count << "." << origArgument << "{x : " << (*a)->getKeyword( ) << endl;
		findHelp = *a;
            }
	}

        buf << endl;
    }
    /*
     * Strip leading '.' to allow initial blanks.
     */
    if (findHelp && count==1 ){
	page_to_char( findHelp->getText( ch ).c_str( ), ch );
    }
    else if (count==0) {
	ch->send_to( "��� ��������� �� ������� �����.\n\r");
	bugTracker->reportNohelp( ch, origArgument.c_str( ) );
    }
    else if (count > 1) {
        buf << "��� ������ ������������ ������� ��������� {C? 1." << origArgument << "{x, {C? 2." << origArgument << "{x � ��� �����." << endl;
        ch->send_to(buf.str().c_str());
    }
}                  


/*-----------------------------------------------------------------
 * cast 'identify', shop item properties
 *----------------------------------------------------------------*/

void lore_fmt_affect( Affect *paf, ostringstream &buf )
{
    int b = paf->bitvector,
        d = paf->duration;

    if (paf->location == APPLY_NONE || paf->modifier == 0)
	return;

    buf << "�������� " << apply_flags.message(paf->location ) 
	<< " �� " << paf->modifier;

    if (d > -1)
	buf << ", � ������� " << d << " ���" << GET_COUNT(d, "�", "��", "��");
    
    buf << endl;

    if (!b)
	return;
    
    switch(paf->where) {
	case TO_AFFECTS:
	    buf << "��������� ������ " << affect_flags.messages(b ) << endl;
	    break;
	case TO_IMMUNE:
	    buf << "��������� ��������� � " << imm_flags.messages(b ) << endl;
	    break;
	case TO_RESIST:
	    buf << "��������� ���������������� � " << res_flags.messages(b ) << endl;
	    break;
	case TO_VULN:
	    buf << "��������� ���������� � " << vuln_flags.messages(b ) << endl;
	    break;
	case TO_DETECTS:
	    buf << "��������� ����������� " << detect_flags.messages(b ) << endl;
	    break;
    }
}

void lore_fmt_wear( int type, int wear, ostringstream &buf )
{
    if (type == ITEM_LIGHT) {
	buf << "������������ ��� ���������" << endl;
	return;
    }
    
    if (wear == -1)
	return;

    if (IS_SET( wear, ITEM_WEAR_FINGER ))
       buf << "���������� �� �����" << endl;	
    if (IS_SET( wear, ITEM_WEAR_NECK ))
       buf << "���������� �� ���" << endl;	
    if (IS_SET( wear, ITEM_WEAR_BODY ))
       buf << "���������� �� ����" << endl;	
    if (IS_SET( wear, ITEM_WEAR_HEAD ))
       buf << "���������� �� ������" << endl;	
    if (IS_SET( wear, ITEM_WEAR_EARS ))
       buf << "���������� � ���" << endl;	
    if (IS_SET( wear, ITEM_WEAR_FACE ))
       buf << "���������� �� ����" << endl;	
    if (IS_SET( wear, ITEM_WEAR_FEET ))
       buf << "���������� �� ������" << endl;	
    if (IS_SET( wear, ITEM_WEAR_LEGS ))
       buf << "���������� �� �����" << endl;	
    if (IS_SET( wear, ITEM_WEAR_HANDS ))
       buf << "���������� �� ����" << endl;	
    if (IS_SET( wear, ITEM_WEAR_ARMS ))
       buf << "���������� �� �����" << endl;	
    if (IS_SET( wear, ITEM_WEAR_ABOUT ))
       buf << "������������ ������ ����" << endl;	
    if (IS_SET( wear, ITEM_WEAR_WAIST ))
       buf << "���������� �� �����" << endl;	
    if (IS_SET( wear, ITEM_WEAR_WRIST ))
       buf << "���������� �� ��������" << endl;
    if (IS_SET( wear, ITEM_WEAR_SHIELD ))
       buf << "������������ ��� ���" << endl;
    if (IS_SET( wear, ITEM_WEAR_HORSE ))
	buf << "���������� �� ��������� �����" << endl;
    if (IS_SET( wear, ITEM_WEAR_HOOVES ))
	buf << "���������� �� ������" << endl;
}

void lore_fmt_item( Character *ch, Object *obj, ostringstream &buf, bool showName )
{
    int lim;
    Skill *skill;
    Liquid *liquid;
    const char *mat;
    Affect *paf;
    Keyhole::Pointer keyhole;

    buf << "{W" << obj->getShortDescr( '1' ) << "{x";
    
    if (showName)
        buf << ", ����������� �� ����� '{W" << obj->getName( ) << "{x'";

    buf << endl
	<< "{W" << item_table.message(obj->item_type ) << "{x, "
	<< "������ {W" << obj->level << "{x" << endl;

    if (obj->weight > 10)
	buf << "����� {W" << obj->weight / 10 << "{x ���" << GET_COUNT(obj->weight/10, "�", "��", "���"); 
    else
	buf << "������ �� �����";

    if (IS_SET(obj->extra_flags, ITEM_NOIDENT)) {
	buf << endl << "����� ��� ��� ���� ���������� ������ �������." << endl;
	return;
    }
    
    buf << ", ";
    
    if (obj->cost)
	buf << "����� {W" << obj->cost << "{x �������";
    else
	buf << "������ �� �����";
   
    // XXX '����������� ��' + ������
    mat = obj->getMaterial( );
    if (mat && strcmp( mat, "none" ) && strcmp( mat, "oldstyle" ))
	buf << ", �������� {W" << mat << "{x";
    
    buf << endl;
    
    bitstring_t extra = obj->extra_flags;
    REMOVE_BIT(extra, ITEM_WATER_STAND|ITEM_INVENTORY|ITEM_HAD_TIMER|ITEM_DELETED);
    if (extra)
	buf << "������ ��������: " << extra_flags.messages(extra, true ) << endl;

    lim = obj->pIndexData->limit;
    if (lim != -1 && lim < 100)
	buf << "����� ����� � ���� ����� ���� �� ����� {W" << lim << "{x!" << endl;

    switch (obj->item_type) {
    case ITEM_KEY:
	if (( keyhole = Keyhole::locate( ch, obj ) ))
	    keyhole->doLore( buf );
	break;
    case ITEM_KEYRING:
	buf << "�������� " << obj->value[1] << " ������ �� ��������� " << obj->value[0] << "." << endl;
	break;
    case ITEM_LOCKPICK:
	if (obj->value[0] == Keyhole::LOCK_VALUE_BLANK) {
	    buf << "��� ��������� ��� ����� ��� �������." << endl;
	}
	else {
	    if (obj->value[0] == Keyhole::LOCK_VALUE_MULTI)
		buf << "��������� ����� �����. ";
	    else
		buf << "��������� ���� �� ����� ������. ";
	    
	    buf << "������� " 
		<< quality_percent( obj->value[1] ).colourStrip( ).ruscase( '2' ) 
		<< " ��������." << endl;
	}
	break;
    case ITEM_SPELLBOOK:
	buf << "����� �������: " << obj->value[0] << ", �� ��� ������������: " << obj->value[1] << "." << endl
            << "������������ �������� ���������� � �����: " << obj->value[2] << "." << endl;
	break;

    case ITEM_TEXTBOOK:
	buf << "����� �������: " << obj->value[0] << ", �� ��� ������������: " << obj->value[1] << "." << endl
            << "������������ �������� ������� � ��������: " << obj->value[2] << "." << endl;
	break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
	buf << "���������� " << obj->value[0] << " ������:";

	for (int i = 1; i <= 4; i++) 
	    if (( skill = SkillManager::getThis( )->find( obj->value[i] ) ))
		if (skill->getIndex( ) != gsn_none)
		    buf << " '" << skill->getNameFor( ch ) << "'";
	
	buf << endl;
	break;

    case ITEM_WAND:
    case ITEM_STAFF:
	buf << "����� " << obj->value[2] << " ���������" << GET_COUNT(obj->value[2], "�", "�", "�") << " " 
	    << obj->value[0] << " ������:";
	
	if (( skill = SkillManager::getThis( )->find( obj->value[3] ) ))
	    if (skill->getIndex( ) != gsn_none)
		buf << " '" << skill->getNameFor( ch ) << "'";

	buf << endl;
	break;

    case ITEM_DRINK_CON:
	liquid = liquidManager->find( obj->value[2] );
	buf << "�������� " 
	    << liquid->getShortDescr( ).ruscase( '4' ) << " "
	    << liquid->getColor( ).ruscase( '2' ) 
	    << " �����." << endl;
        break;

    case ITEM_CONTAINER:
	buf << "���������������: " << obj->value[0] << "  "
	    << "������. ���: " << obj->value[3] << " ���" << GET_COUNT(obj->value[3], "�", "��", "���") << " ";
	
	if (obj->value[4] != 100)
	    buf << " ����. �������� ����: " << obj->value[4] << "%";
	    
	if (obj->value[1])
	    buf << endl << "�����������: " << container_flags.messages(obj->value[1], true );
	
	buf << endl;
	break;

    case ITEM_WEAPON:
	buf << "��� ������: " 
	    << weapon_class.message(obj->value[0] ) << " "
	    << "(" << weapon_class.name( obj->value[0] ) << "), ";
	
	buf << "����������� " << obj->value[1] << "d" << obj->value[2] << " "
	    << "(������� " << (1 + obj->value[2]) * obj->value[1] / 2 << ")" << endl;
    
        if (obj->value[4])  /* weapon flags */
            buf << "����������� ������: " << weapon_type2.messages(obj->value[4], true ) << endl;

	break;

    case ITEM_ARMOR:
	buf << "����� �����: ";

	for (int i = 0; i <= 3; i++)
	    buf << obj->value[i] << " " << ac_type.message(i )
		<< (i == 3 ? "" : ", ");

	buf << endl;
	break;
    }
    
    lore_fmt_wear( obj->item_type, obj->wear_flags, buf );

    if (!obj->enchanted)
	for (paf = obj->pIndexData->affected; paf != 0; paf = paf->next)
	    lore_fmt_affect( paf, buf );

    for (paf = obj->affected; paf != 0; paf = paf->next)
	lore_fmt_affect( paf, buf );
}

