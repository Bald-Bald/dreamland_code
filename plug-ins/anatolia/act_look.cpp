/* $Id: act_look.cpp,v 1.1.2.12.6.37 2014-09-19 11:34:32 rufina Exp $
 *
 * ruffina, 2005
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
#include <map>
#include <list>
#include <sstream>

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "char.h"
#include "grammar_entities_impl.h"

#include "commandtemplate.h"
#include "command.h"
#include "commandmanager.h"
#include "mobilebehavior.h"
#include "behavior_utils.h"
#include "skill.h"
#include "affecthandler.h"

#include "affect.h"
#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcrace.h"
#include "liquid.h"
#include "room.h"
#include "desire.h"

#include "descriptor.h"
#include "webmanip.h"
#include "comm.h"
#include "gsn_plugin.h"
#include "directions.h"
#include "attract.h"
#include "occupations.h"
#include "move_utils.h"
#include "act_lock.h"
#include "handler.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#define MILD(ch)     (IS_SET((ch)->comm, COMM_MILDCOLOR))

#define CLR_MOB(ch)	(MILD(ch) ? "y" : "Y")
#define CLR_PLAYER(ch)	(MILD(ch) ? "W" : "W")
#define CLR_OBJ(ch)	(MILD(ch) ? "w" : "G")
#define CLR_OBJROOM(ch)	(MILD(ch) ? "g" : "G")
#define CLR_NOEQ(ch)	(MILD(ch) ? "D" : "w")
#define CLR_AEXIT(ch)	(MILD(ch) ? "w" : "C")
#define CLR_RNAME(ch)	(MILD(ch) ? "W" : "W")
#define CLR_RVNUM(ch)	(MILD(ch) ? "c" : "C")

DESIRE(bloodlust);
GSN(stardust);
GSN(rainbow_shield);
GSN(demonic_mantle);

/*
 * Extern functions needed
 */
DLString get_pocket_argument( char *arg );
bool can_see_objname_hint( Character *ch, Object *obj );
long long get_arg_id( const DLString &cArgument );
void lore_fmt_wear( int type, int wear, ostringstream &buf );
/*
 * Local functions.
 */
DLString format_obj_to_char( Object *obj, Character *ch, bool fShort );
void show_pockets_to_char( Object *container, Character *ch, ostringstream &buf );
void show_list_to_char( Object *list, Character *ch, bool fShort, 
			bool fShowNothing, DLString pocket = "", Object *container = NULL );
void show_char_to_char_0( Character *victim, Character *ch );
void show_char_to_char_1( Character *victim, Character *ch, bool fBrief );
void show_people_to_char( Character *list, Character *ch, bool fShowMount = true );
bool show_char_equip( Character *ch, Character *victim, ostringstream &buf, bool fShowEmpty );
static void show_exits_to_char( Character *ch, Room *targetRoom );


/*
 * "(english name)" for CONFIG_OBJNAME_HINT 
 */
static void get_obj_name_hint( Object *obj, std::ostringstream &buf )
{
    DLString name;
    unsigned int i;
    
    name = obj->getShortDescr( );
    name.colourstrip( );

    for (i = 0; i < name.size( ); i++)
	if (isalpha( name.at( i ) ))
	    return;
    
    name = obj->getName( );
    name.colourstrip( );

    while (!name.empty( )) {
	bool good = true;
	DLString hint = name.getOneArgument( );

	for (i = 0; i < hint.size( ); i++)
	    if (dl_isrusalpha( hint.at( i ) )) {
		good = false;
		break;
	    }
	
	if (good) {
	    buf << " {x(" << hint << "{x)";
	    return;
	}
    }

    warn( "(objhint) no hint found for [%d]", obj->pIndexData->vnum);
}

static bool is_empty_descr( const char *arg )
{
    DLString descr;

    if (arg == 0 || arg[0] == 0)
	return true;

    descr = arg;
    descr.colourstrip( );
    
    if (descr.empty( ))
	return true;

    return false;
}

/*
 * Show object on the floor or in inventory/equipment/container...
 */
DLString format_obj_to_char( Object *obj, Character *ch, bool fShort )
{
    std::ostringstream buf;
   
    // Hide items without short description inside object lists.
    if (fShort && is_empty_descr( obj->getShortDescr( ) ))
	    return "";
    
    // Hide items without long description on the floor.
    if (!fShort && is_empty_descr( obj->getDescription( ) ))
	return "";
    
#define FMT(cond, buf, ch, lng, color, letter)        \
    if (!(ch)->is_npc() && IS_SET((ch)->getPC()->config, CONFIG_SHORT_OBJFLAG))   \
	buf << color << ((cond) ? letter : ".");      \
    else if ((cond))                                  \
	buf << lng;                                   

    FMT( true, buf, ch, "", "{x", "[" );
    
    FMT( IS_OBJ_STAT(obj, ITEM_INVIS), buf, ch,
	 "({D��������{x) ", "{D", "�" );
	
    FMT( CAN_DETECT(ch, DETECT_EVIL) && IS_OBJ_STAT(obj, ITEM_EVIL), buf, ch,
	 "({R������� ����{x) ", "{R", "�" );
	
    FMT( CAN_DETECT(ch, DETECT_GOOD) && IS_OBJ_STAT(obj,ITEM_BLESS), buf, ch,
	"({C������� ����{x) ", "{C", "�" );
    	
    if (obj->item_type == ITEM_PORTAL) {
	FMT( CAN_DETECT(ch, DETECT_MAGIC) && IS_OBJ_STAT(obj, ITEM_MAGIC), buf, ch, 
	    "(����������) ", "{w", "�" );
    } else {
	FMT( CAN_DETECT(ch, DETECT_MAGIC) && IS_OBJ_STAT(obj, ITEM_MAGIC), buf, ch,
	    "(�����������) ", "{w", "�" );
    }

    FMT( IS_OBJ_STAT(obj, ITEM_GLOW), buf, ch,
	"({M������{x) ", "{M", "�" ); 

    FMT( IS_OBJ_STAT(obj, ITEM_HUM), buf, ch,   
	"({c���������{x) ", "{c", "�" );
   
    FMT( true, buf, ch, "", "{x", "] " );
#undef FMT
    
    if (fShort)
    {
	buf << "{" << CLR_OBJ(ch) << obj->getShortDescr( '1' ) << "{x";

	if (obj->pIndexData->vnum > 5)	/* money, gold, etc */
	    if (obj->condition <= 99 )
		buf << " [" << obj->get_cond_alias( ) << "]";

        if (!ch->is_npc() && IS_SET(ch->getPC()->config, CONFIG_OBJNAME_HINT)
            && can_see_objname_hint( ch, obj ))
	{
	    get_obj_name_hint( obj, buf );
	}
    }
    else
    {
	if (obj->in_room 
		&& IS_WATER( obj->in_room ) 
		&& !IS_SET(obj->extra_flags, ITEM_WATER_STAND)) 
	{
	    DLString msg;
	    DLString liq = obj->in_room->liquid->getShortDescr( );

	    msg << "%1$^O1 ";

	    switch(dice(1,3)) {
	    case 1: msg << "���� ����%1$n����|���� �� %2$N6.";break;
	    case 2: msg << "����%1$n��|�� �� %2$N3.";break;
	    case 3: msg << "������%1$n��|�� �� %2$N2.";break;
	    }

	    buf << fmt( ch, msg.c_str( ), obj, liq.c_str( ) );
	}
	else {
	    buf << "{" << CLR_OBJROOM(ch) << obj->getDescription( ) << "{x";
	}
    }

    return buf.str( );
}

/*
 * Show list of all container pockets to a character
 */
void show_pockets_to_char( Object *container, Character *ch, ostringstream &buf )
{
    DLString name;
    std::map<DLString, int> pockets;
    Object *obj;
    
    if (container->item_type != ITEM_CONTAINER
	|| !IS_SET(container->value[1], CONT_WITH_POCKETS))
	return;

    if (IS_SET(container->value[1], CONT_PUT_ON|CONT_PUT_ON2))
	buf << "���������: " << endl;
    else if (!container->can_wear( ITEM_TAKE ))
	buf << "�����: " << endl;
    else
	buf << "�������: " << endl;
    
    for (obj = container->contains; obj; obj = obj->next_content) {
	if (obj->pocket.empty( ))
	    continue;
	
	pockets[obj->pocket]++;
    }

    if (pockets.empty( )) {
        buf << "      (������)" << endl;
    } 
    else {
       for (std::map<DLString, int>::iterator i = pockets.begin( ); i != pockets.end( ); i++) {
           buf << "     ";
           webManipManager->decoratePocket( buf, i->first, container, ch );
           buf << "{x" << endl;
       }
    }

    buf << endl
        << "�������� ���������:" << endl;
}


/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( Object *list, Character *ch, bool fShort, bool fShowNothing, DLString pocket, Object *container )
{
    char buf[MAX_STRING_LENGTH];
    ostringstream output;
    bool fCombine, fConfigCombine;
    map<DLString, int> dups;
    map<DLString, int>::iterator d;
    std::list<DLString> shortDescriptions;
    std::list<Object *> items;
    Object *obj;

    if ( ch->desc == 0 )
	    return;
    
    if (container && pocket.empty( ))
	show_pockets_to_char( container, ch, output );
    
    fConfigCombine = (ch->is_npc() || IS_SET(ch->comm, COMM_COMBINE));

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != 0; obj = obj->next_content )
    {
	DLString strShow;

	if (obj->wear_loc != wear_none)
	    continue;

	if (!ch->can_see( obj ))
	    continue;
	    
	if (pocket.empty( ) && !obj->pocket.empty( ))
	    continue;

	if (!pocket.empty( ) && obj->pocket != pocket)
	    continue;
	
	strShow = format_obj_to_char( obj, ch, fShort );

	if (strShow.empty( ))
	    continue;

	fCombine = false;

	if (fConfigCombine) {
	    /*
	     * Look for duplicates, case sensitive.
	     */
	    d = dups.find( strShow );

	    if (d != dups.end( )) {
		d->second++;
		fCombine = true;
	    }
	}

	/*
	 * Couldn't combine, or didn't want to.
	 */
	if (!fCombine) {
	    dups[strShow] = 1;
	    shortDescriptions.push_back( strShow );
            // Remember actual items, to later construct a list of possible manipulations.
            // For combined lists, only the first object with a given description is going to be
            // considered, which is not exactly accurate.
            items.push_back( obj );
	}
    }


    /*
     * Output the formatted list.
     */
    if (shortDescriptions.empty( ))
    {
	if (fShowNothing)
	    output << "     ������." << endl;
    }
    else {
	std::list<DLString>::iterator sd;
	std::list<Object *>::iterator item;
	int iShow;
	
	for (sd = shortDescriptions.begin( ), item = items.begin( ), iShow = 0; 
             sd != shortDescriptions.end( ); 
             sd++, item++, iShow++) {
	    if (iShow >= 100) {
		output << "{" << CLR_OBJ(ch)
			<< "     ... � ����� ���� ��� ...{x" << endl;
		break;
	    }

	    d = dups.find( *sd );

	    if (fConfigCombine && d->second != 1) {
		sprintf( buf, "(%2d) ", d->second );
		output << buf;
	    }
	    else
		output << "     ";

	    webManipManager->decorateItem( output, *sd, *item, ch, pocket, d->second );
            output << endl;
	}
    }

    page_to_char(output.str( ).c_str( ), ch);
}

/*
 * Display PK-flags
 */
void show_char_pk_flags( PCharacter *ch, ostringstream &buf )
{
    struct PKFlag {
	int bit;
	char color;
	const char *descr;
    };
    static const struct PKFlag pk_flag_table [] = {
	{ PK_VIOLENT, 'B', "VIOLENT" },
	{ PK_KILLER,  'R', "KILLER"  },
	{ PK_THIEF,   'R', "THIEF"   },
	{ PK_SLAIN,   'D', "SLAIN"   },
	{ PK_GHOST,   'D', "GHOST"   },
    };
    static const int size = sizeof(pk_flag_table) / sizeof(*pk_flag_table);

    for (int i = 0; i < size; i++)
	if (IS_SET(ch->PK_flag, pk_flag_table[i].bit))
	    buf << "[{" << pk_flag_table[i].color 
		<< pk_flag_table[i].descr << "{x]";
}

void show_char_blindness( Character *ch, Character *victim, ostringstream &buf )
{
    if (IS_AFFECTED(victim, AFF_BLIND))
	buf << fmt( ch, 
	            " ... %1$P1 �������� ����%1$G��|��|��, �������� ��� ������.{x",
		    victim )
	    << endl;
}

static DLString
oprog_show_where( Object *furniture, Character *ch, Character *looker )
{   
    FENIA_STR_CALL( furniture, "ShowWhere", "CC", ch, looker )
    FENIA_NDX_STR_CALL( furniture, "ShowWhere", "OCC", furniture, ch, looker )
    return DLString::emptyString;
}

GSN(curl);

void show_char_position( Character *ch, Character *victim, 
			 const char *verb, int atFlag, int onFlag,
			 ostringstream &buf )
{
    if (!MOUNTED(victim)) {
	buf << verb << " ";

	if (victim->on != 0) {
	    DLString rc = oprog_show_where( victim->on, victim, ch );

	    if (!rc.empty( ))
		buf << rc;
	    else if (IS_SET(victim->on->value[2], atFlag))
		buf << "����� " << victim->on->getShortDescr( '2' );
	    else if (IS_SET(victim->on->value[2], onFlag))
		buf << "�� " << victim->on->getShortDescr( '6' );
	    else
		buf << "� " << victim->on->getShortDescr( '6' );
	}
	else
	    buf << "�����";

	if (victim->position == POS_SLEEPING && !IS_AFFECTED(victim, AFF_SLEEP)) 
	    if (gsn_curl->getEffective( victim ) > 1)
		buf << ", ����������� ���������";
    }
    else
	buf << "����� ����� ������ �� " 
	    << (ch == victim->mount ? "����" : ch->sees( MOUNTED(victim), '6' ));

    buf << "." << endl;
    show_char_blindness( ch, victim, buf );
}

static DLString oprog_show_end( Object *furniture, Character *ch, Character *looker )
{
    FENIA_STR_CALL( furniture, "ShowEnd", "CC", ch, looker ) 
    FENIA_NDX_STR_CALL( furniture, "ShowEnd", "OCC", furniture, ch, looker )
    return DLString::emptyString;
}

static DLString rprog_show_end( Room *room, Character *ch, Character *looker )
{
    FENIA_STR_CALL( room, "ShowEnd", "CC", ch, looker ) 
    return DLString::emptyString;
}

/*
 * Show a character in the room ('look' or 'look auto')
 */
void show_char_to_char_0( Character *victim, Character *ch )
{
    std::basic_ostringstream<char> buf;
    Character *origVict;
    NPCharacter *nVict;
    PCharacter *pVict;
    
    origVict = victim;
    victim = victim->getDoppel( );

    if (victim->is_npc( )) {
	nVict = victim->getNPC( );
	pVict = 0;
    }
    else {
	nVict = 0;
	pVict = victim->getPC( );
    }

    if (nVict && nVict->behavior)
	nVict->behavior->show( ch, buf );
    
    if (pVict) {
	show_char_pk_flags( pVict, buf );

	if (pVict->desc == 0 )
	    buf << "[{D��� �����{x]";

	if (IS_SET(pVict->comm, COMM_AFK ))
	    buf << "[{CAFK{x]";

	if (IS_SET(pVict->act, PLR_WANTED))
	    buf << "({R�������������{x)";

	if (pVict->isAffected(gsn_manacles ))
	    buf << "({D�������{x)";

	if (victim->isAffected(gsn_jail ))
	    buf << "({D������{x)";
	
	if (pVict->invis_level >= LEVEL_HERO)
	    buf << "(Wizi)";
    }

    if (RIDDEN( victim ))
	buf << "(��������)";

    if (IS_AFFECTED(victim, AFF_INVISIBLE))
	buf << "({D��������{x)";

    if (IS_AFFECTED(victim, AFF_IMP_INVIS))
	buf << "(Improved)";

    if (IS_AFFECTED(victim, AFF_HIDE))
	buf << "({D������{x)";

    if (IS_AFFECTED(victim, AFF_FADE))
	buf << "({D��������{x)";

    if (IS_AFFECTED(victim, AFF_CAMOUFLAGE))
	buf << "({D�������������{x)";

    if (IS_AFFECTED(victim, AFF_CHARM))
	buf << "(���������)";

    if (victim->is_npc()
	    && IS_SET(victim->act,ACT_UNDEAD)
	    && CAN_DETECT(ch, DETECT_UNDEAD))
	buf << "(������)";

    if (IS_AFFECTED(victim, AFF_PASS_DOOR))
	buf << "(���������)";

    if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
	buf << "({M������� ����{x)";

    if (IS_EVIL(victim) && CAN_DETECT(ch, DETECT_EVIL))
	buf << "({R������� ����{x)";

    if (IS_GOOD(victim) && CAN_DETECT(ch, DETECT_GOOD))
	buf << "({Y������� ����{x)";

    if (IS_AFFECTED(victim, AFF_SANCTUARY))
	buf << "({W����� ����{x)";

    if (victim->isAffected(gsn_rainbow_shield))
        buf << "({R�{Y�{G�{C�{B�{M�{x)";

    if (victim->isAffected(gsn_demonic_mantle))
        buf << "({R�{D�����{x)";

    if (victim->isAffected(gsn_dark_shroud))
	buf << "({D���� ����{x)";

    if (victim->isAffected(gsn_stardust))
	buf << "({W�{w��{W��{w��� {W�{w���{x)";

    if (nVict) 
	if (nVict->position == nVict->start_pos 
	    && nVict->getLongDescr( ) 
	    && nVict->getLongDescr( )[0])
	{
	    buf << "{" << CLR_MOB(ch) << nVict->getLongDescr( ) << "{x";
	    show_char_blindness( ch, victim, buf );
	    ch->send_to( buf);
	    return;
	}
    
    if (nVict) {
	buf << "{" << CLR_MOB(ch) << ch->sees( victim, '1' );
    }
    else {
	if (ch->getConfig( )->holy && origVict != victim)
	    buf << "{" << CLR_PLAYER(ch) << ch->sees( origVict, '1' ) << "{x "
		<< "(��� ������� " << ch->sees( victim, '2' ) << ") ";
	else {
	    buf << "{" << CLR_PLAYER(ch);
            webManipManager->decorateCharacter( buf, ch->sees( victim, '1' ), victim, ch );
            buf << "{x";
        }

	if (!IS_SET(ch->comm, COMM_BRIEF) 
	    && victim->position == POS_STANDING 
	    && ch->on == 0)
	{
	    buf << pVict->getParsedTitle( );
	}
    }

    buf << " {x";

    switch (victim->position.getValue( )) {
    case POS_DEAD:     
	buf << "��� {R����!!!{x" << endl;
	break;
	
    case POS_MORTAL:   
	buf << "���������." << endl;   
	break;
	
    case POS_INCAP:    
	buf << "� ����������� ���������." << endl;      
	break;
	
    case POS_STUNNED:  
	buf << "����� ��� ��������." << endl; 
	break;
	
    case POS_SLEEPING:
	show_char_position( ch, victim, "����", SLEEP_AT, SLEEP_ON, buf );
	break;

    case POS_RESTING:
	show_char_position( ch, victim, "��������", REST_AT, REST_ON, buf );
	break;

    case POS_SITTING:
	show_char_position( ch, victim, "�����", SIT_AT, SIT_ON, buf );
	break;
	    
    case POS_STANDING:
	show_char_position( ch, victim, "�����", STAND_AT, STAND_ON, buf );
	break;

    case POS_FIGHTING:
	buf << "�����, ��������� ";

	if (victim->fighting == 0)
	    buf << "���������� � ���...";
	else if (victim->fighting == ch)
	    buf << "� {R�����!!!{x";
	else if (victim->in_room == victim->fighting->in_room)
	    buf << "� " << ch->sees( victim->fighting, '5') << ".";
	else
	    buf << "���-�� ��� ����...";

	buf << endl;
	show_char_blindness( ch, victim, buf );
	break;
    }

    if (HAS_SHADOW(victim))
	buf << " ... ����������� �������� ����." << endl;
    
    if (victim->death_ground_delay > 0) {
	DLString rc = rprog_show_end( victim->in_room, victim, ch );

	if (!rc.empty( ))
	    buf << " ... " << rc << endl;
    }
    
    if (victim->on) {
	DLString rc = oprog_show_end( victim->on, victim, ch );

	if (!rc.empty( ))
	    buf << rc << endl;
    }

    ch->send_to( buf );
}

/*
 * Observation 
 */
void show_char_diagnose( Character *ch, Character *victim, ostringstream &buf )
{
    ostringstream str;

    if (!CAN_DETECT( ch, DETECT_OBSERVATION ))
	return;

    if (IS_AFFECTED( victim, AFF_BLIND ))
	str << "������ ������ �� �����." << endl;
    if (IS_AFFECTED( victim,  AFF_PLAGUE ))
	str << "������ ������� ���������." << endl;
    if (IS_AFFECTED( victim, AFF_POISON ))
	str << "��������." << endl;
    if (IS_AFFECTED( victim, AFF_SLOW ))
	str << "������������� � � � � � �  �   �." << endl;
    if (IS_AFFECTED( victim, AFF_HASTE ))
	str << "������������� ����� ������." << endl;
    if (IS_AFFECTED( victim, AFF_WEAKEN ))
	str << "�������� ���������� � �����." << endl;
    if (IS_AFFECTED( victim, AFF_CORRUPTION ))
	str << "����� ������." << endl;
    if (CAN_DETECT( victim, ADET_FEAR ))
	str << "������ �� ������." << endl;
    if (IS_AFFECTED( victim, AFF_CURSE ))
	str << "�������." << endl;
    if (IS_AFFECTED( victim, AFF_PROTECT_EVIL ))
	str << "������� �� ���" << endl;
    if (IS_AFFECTED( victim, AFF_PROTECT_GOOD ))
	str << "������� �� �����." << endl;

    if (!str.str( ).empty( )) 
	buf << endl << "�� ��������� ������ ������:" << endl
	    << str.str( ) << endl;
}

/*
 * Show character wounds
 */
void show_char_wounds( Character *ch, Character *victim, ostringstream &buf )
{
    int percent;

    if (victim->max_hit > 0)
	percent = HEALTH(victim);
    else
	percent = -1;

    if (percent >= 100)
	buf << "{C � ���������� ���������";
    else if (percent >= 90)
	buf << "{B ����� ��������� �������";
    else if (percent >= 75)
	buf << "{B ����� ��������� ��������� ��� � �������";
    else if (percent >= 50)
	buf << "{G ����� �������� ����� ���";
    else if (percent >=  30)
	buf << "{Y ����� ��������� �������, ������� ��� � �������";
    else if (percent >= 15)
	buf << "{M �������� ������ ������������";
    else if (percent >= 0 )
	buf << "{R � ������� ���������";
    else
	buf << "{R �������� ������";

    buf << ".{x" << endl;

    /* vampire ... */
    if (percent < 90 && !ch->is_npc( ))
	desire_bloodlust->gain( ch->getPC( ), -1 );
}

static void show_char_description( Character *ch, Character *vict )
{
    const char *dsc = vict->getDescription( );

    if ((vict->is_npc( ) && dsc) || (!vict->is_npc( ) && dsc[0])) {
	ch->send_to( dsc );
	return;
    }

    if (vict->getRace( )->isPC( )) {
	PCRace::Pointer pcRace = vict->getRace( )->getPC( );
	const char *rname = GET_SEX(vict, 
				    pcRace->getMaleName( ).c_str( ),
				    pcRace->getMaleName( ).c_str( ),
				    pcRace->getFemaleName( ).c_str( ));
	if (ch == vict)
	    act( "�� ��������� ��� �����$G��|��|�� $n1.", ch, rname, vict, TO_CHAR );
	else
	    act( "$E �������� ��� �����$G��|��|�� $n1.", ch, rname, vict, TO_CHAR );

	return;
    }
    
    act( "�� �� ������ ������ ���������� � $Z.", ch, 0, vict, TO_CHAR );
}

static void show_char_sexrace( Character *ch, Character *vict, ostringstream &buf )
{
    buf << "(";

    if (ch->getConfig( )->rucommands)
	buf << vict->getRace( )->getNameFor( ch, vict );
    else
	buf << GET_SEX(vict, "male", "sexless", "female")
	    << " "
	    << vict->getRace( )->getName( );

    buf << ") ";
}

/*
 *  Look at somebody 
 */
void show_char_to_char_1( Character *victim, Character *ch, bool fBrief )
{
    ostringstream buf;
    Character*	vict;
    bool naked;
    
    vict = victim->getDoppel( );
    
    if (!fBrief) 
	show_char_description( ch, vict );

    if (MOUNTED(vict))
	buf << ch->sees( vict, '1' ) << " ������ �� " 
	    << (ch == vict->mount ? "����" : ch->sees( MOUNTED( vict ), '6' ))
	    << endl;

    if (RIDDEN(vict)) {
	buf << ch->sees( vict, '1' );

	if (ch == vict->mount)
	    buf << " ��� ����� ������";
	else
	    buf << " ��� ������, � ������� ����� " 
		<< ch->sees( RIDDEN( vict ), '1' );
	buf << "." << endl;
    }
    
    show_char_diagnose( ch, victim, buf );
    show_char_sexrace( ch, vict, buf ); 
    buf << ch->sees( vict, '1' );
    show_char_wounds( ch, victim, buf );
    ch->send_to( buf);
    
    if (fBrief)
	return;
    
    buf.str( "" );
    naked = show_char_equip( ch, victim, buf, false );

    if (!naked) {
	act( "\r\n$C1 ����������: ", ch, 0, victim, TO_CHAR );
	ch->send_to( buf );
    }
	    
    if (victim->is_npc() && victim->getNPC()->behavior)
	if (victim->getNPC()->behavior->look_inv( ch ))
	    return;

    if (victim != ch 
	    && !ch->is_npc()
	    && (number_percent( ) < gsn_peek->getEffective( ch )
		|| ch->is_immortal()))

    {
	ch->println( "\n\r�� ������������ � ���������: " );
	gsn_peek->improve( ch, true );
	show_list_to_char( victim->is_mirror() ?
		vict->carrying : victim->carrying, ch, true, true );
    }
}

/*
 * display character equip list
 */
bool show_char_equip( Character *ch, Character *victim, ostringstream &buf, bool fShowEmpty )
{
    Wearlocation::DisplayList eq;
    Wearlocation::DisplayList::iterator e;
    bool naked = true;
    Character *	vict = (victim->is_mirror( ) ? victim->getDoppel( ) : victim);

    wearlocationManager->display( vict, eq );
    
    for (e = eq.begin( ); e != eq.end( ); e++) {
	DLString objName, wearName = e->first;
	Object *obj = e->second;

	if (!obj) {
	    if (fShowEmpty)
		objName << "{" << CLR_NOEQ(ch) << "������.{x";
	    else
		continue;
	}
	else if (!ch->can_see( obj )) {
	    if (fShowEmpty)
		objName = "�����.";
	    else
		continue;
	}
	else 
	    objName = format_obj_to_char( obj, ch, true );

        ostringstream mbuf;
        if (obj)
            webManipManager->decorateItem( mbuf, objName, obj, ch, DLString::emptyString, 1 );
        else
            mbuf << objName;

	buf << dlprintf( "<%-21s> %s\r\n", wearName.c_str( ), mbuf.str( ).c_str( ) );
	if (obj)
	    naked = false;
    }

    return naked;
}
	


/*
 * Show people in the room
 */
void show_people_to_char( Character *list, Character *ch, bool fShowMount )
{
    Character *rch;
    int life_count=0;

    for ( rch = list; rch != 0; rch = rch->next_in_room )
    {
	if (rch == ch)
	    continue;

	if (!fShowMount && rch == ch->mount)
	    continue;

	if (!rch->is_npc() && ch->get_trust() < rch->getPC()->invis_level)
	    continue;
	
	if (ch->can_see( rch ))
	    show_char_to_char_0( rch, ch );
	else if (!rch->is_immortal( )) {
	    if (rch->in_room->isDark( ) && IS_AFFECTED(rch, AFF_INFRARED ))
		ch->println( "{W�� ������ ������ {R�������� ������� ����{W, �������� �� �����!{x" );
		
	    life_count++;
	}
    }

    if (life_count && CAN_DETECT(ch, DETECT_LIFE))
	ch->printf( "�� ���������� ����������� %d �������%s ����%s � �������.\n\r",
		    life_count,
		    GET_COUNT(life_count, "��", "��", "��"),
		    GET_COUNT(life_count, "�", "", ""));
}

/*---------------------------------------------------------------------------
 * 'inventory' command 
 *--------------------------------------------------------------------------*/
CMDRUNP( inventory )
{
    ch->println( "�� ������:" );
    show_list_to_char( ch->carrying, ch, true, true );
}

/*---------------------------------------------------------------------------
 * 'equipment' command 
 *--------------------------------------------------------------------------*/
CMDRUNP( equipment )
{
    ostringstream buf;
    bool naked;
    
    naked = show_char_equip( ch, ch, buf, true );

    ch->println( "�� �����������:" );
    ch->send_to( buf );

    if (naked)
	ch->println( "�� ��... �� ������ �� �������!" );
}

/*---------------------------------------------------------------------------
 * extra-description looking (for 'look' and 'read') 
 *--------------------------------------------------------------------------*/
static bool
oprog_look( Object *obj, Character *ch, const char *keyword )
{
    FENIA_CALL( obj, "Look", "Cs", ch, keyword );
    FENIA_NDX_CALL( obj, "Look", "OCs", obj, ch, keyword );
    return false;
}

static bool
rprog_look( Room *room, Character *ch, const char *keyword )
{
    FENIA_CALL( room, "Look", "Cs", ch, keyword );
    return false;
}

static DLString
oprog_extra_descr(Object *obj, Character *ch, const char *arg)
{
    FENIA_STR_CALL( obj, "ExtraDescr", "Cs", ch, arg )
    FENIA_NDX_STR_CALL( obj, "ExtraDescr", "OCs", obj, ch, arg )

    if (obj->behavior)
	return obj->behavior->extraDescription( ch, arg );

    return DLString::emptyString;
}

struct EDInfo {
    EDInfo( DLString k, DLString d, Object *o, Room *r ) :
	keyword( k ), description( d ), source( o ), sourceRoom( r )
    {
    }

    DLString keyword;
    DLString description;
    Object *source;
    Room *sourceRoom;
};

struct ExtraDescList : public list<EDInfo> {
    ExtraDescList( Character *ch, const char *arg, int number = 1 ) 
         : ch( ch ), arg( arg ), number( number )
    {
    }
    
    void putObjects( Object *list ) 
    { 
	for (Object *obj = list; obj != 0; obj = obj->next_content) {
	    if (!ch->can_see( obj )) 
		continue;

	    size_type startSize = size( );

	    DLString desc = oprog_extra_descr( obj, ch, arg );
	    if (!desc.empty( ))
		push_back( EDInfo( arg, desc, obj, 0 ) );
		
	    size_type mySize = size( );
	    putDescriptions( obj->extra_descr, obj, 0 );

	    if (size( ) == mySize)
		putDescriptions( obj->pIndexData->extra_descr, obj, 0 );
	    
	    if (size( ) == startSize)
		putDefaultDescription( obj );
	}
    }
    
    void putDefaultDescription( Object *obj )
    {
	if (is_name( arg, obj->getName( ) )) {
	    const char *defaultDescr;
	    if (obj->in_room)
		defaultDescr = obj->getDescription( );
	    else
		defaultDescr = "�� �� ������ ����� ������ ����������.";
		
	    push_back( EDInfo( obj->getName( ), defaultDescr, obj, 0 ) );
	}
    }

    void putDescriptions( EXTRA_DESCR_DATA *ed, Object *obj, Room *room )
    {
	for (; ed; ed = ed->next)
	    if (is_name( arg, ed->keyword ))
		push_back( EDInfo( ed->keyword, ed->description, obj, room ) );
    }
    
    bool output( )
    {
	int count;
	iterator i;
	ostringstream buf;

	for (count = 1, i = begin( ); i != end( ); i++, count++)
	    if (count == number) {
                EXTRA_DESCR_DATA *sourceEdList = i->source ? i->source->pIndexData->extra_descr : i->sourceRoom->extra_descr;
		buf << "{x";
                webManipManager->decorateExtraDescr( buf, i->description.c_str( ), sourceEdList, ch );
                buf << endl;

		ch->send_to( buf );

		if (i->source)
		    oprog_look( i->source, ch, i->keyword.c_str( ) );
		if (i->sourceRoom)
		    rprog_look( i->sourceRoom, ch, i->keyword.c_str( ) );
		return true;
	    }

	return false;
    }

    Character *ch;
    const char *arg;
    int number;
};

static bool do_look_extradescr( Character *ch, const char *arg, int number )
{
    ExtraDescList edlist( ch, arg, number );
    
    edlist.putObjects( ch->carrying );
    edlist.putObjects( ch->in_room->contents );
    edlist.putDescriptions( ch->in_room->extra_descr, 0, ch->in_room );

    return edlist.output( );
}

/*---------------------------------------------------------------------------
 * 'look' subroutines 
 *--------------------------------------------------------------------------*/
static DLString
rprog_descr( Room *room, Character *ch, const DLString &descr )
{   
    FENIA_STR_CALL( room, "Descr", "Cs", ch, descr.c_str( ) )
    return descr;
}

/* NOTCOMMAND */ void do_look_auto( Character *ch, Room *room, bool fBrief, bool fShowMount )
{
    ostringstream buf;

    if (eyes_darkened( ch )) {
	ch->println( "����� ������� ����� ... " );
	show_people_to_char( room->people, ch, fShowMount );
	return;
    }
    
    buf << "{" << CLR_RNAME(ch) << room->name << "{x";

    if (ch->getConfig( )->holy) 
	buf << " {" << CLR_RVNUM(ch) << "[Room " << room->vnum
	    << "][" << room->area->name << "]{x";
    
    buf << endl;

    if (!fBrief)
    {
	ostringstream rbuf;
	const char *dsc = room->description;

	if (*dsc == '.')
	    ++dsc;
	else
	    rbuf << " ";
        
        webManipManager->decorateExtraDescr( rbuf, dsc, room->extra_descr, ch );

	for (EXTRA_EXIT_DATA *peexit = room->extra_exit;
				peexit;
				peexit = peexit->next)
	    if (ch->can_see( peexit ))
		rbuf << peexit->room_description;

	buf << rprog_descr( room, ch, rbuf.str( ) );
    }

    if (ch->getPC( ) && IS_SET(ch->getPC( )->act, PLR_AUTOEXIT))
    {
	buf << endl;
	ch->send_to( buf );
	show_exits_to_char( ch, room );
    }
    else
	ch->send_to( buf );

    show_list_to_char( room->contents, ch, false, false );
    show_people_to_char( room->people, ch, fShowMount );
}

static void do_look_move( Character *ch, bool fBrief )
{
    if (!ch->is_npc( ) && ch->getPC( )->getAttributes( ).isAvailable( "speedwalk" )) {
	if (eyes_darkened( ch ))
	    ch->println( "����� ������� ����� ... " );
	else
	    ch->printf( "{W%s{x", ch->in_room->name );
	return;
    }

    do_look_auto( ch, ch->in_room, fBrief, false );
}

static void do_look_into( Character *ch, char *arg2 )
{
    DLString pocket;
    Object *obj;
    
    if (arg2[0] == '\0')
    {
	ch->println( "���������� �� ���?" );
	return;
    }
    
    pocket = get_pocket_argument( arg2 );
    obj = get_obj_here( ch, arg2 );

    if (!obj) {
	ch->println( "�� �� ������ ����� ���." );
	return;
    }
    
    oprog_examine( obj, ch, pocket );
}

static void afprog_look( Character *ch, Character *victim )
{
    Affect *paf, *paf_next;
    
    for (paf = ch->affected; paf; paf = paf_next) {
	paf_next = paf->next;

	if (paf->type->getAffect( ))
	    paf->type->getAffect( )->look( ch, victim, paf );
    }
}

static void do_look_character( Character *ch, Character *victim )
{
    if (victim->can_see( ch )) {
	if (ch == victim)
	    act_p( "$c1 ������� �� ����.",ch,0,0,TO_ROOM,POS_RESTING);
	else
	{
	    act_p( "$c1 ������� �� ����.", ch, 0, victim, TO_VICT,POS_RESTING);
	    act_p( "$c1 ������� �� $C4.",  ch, 0, victim, TO_NOTVICT,POS_RESTING);
	}
    }

    show_char_to_char_1( victim, ch, false );
    afprog_look( ch, victim );    
}

static bool do_look_direction( Character *ch, const char *arg1 )
{
    EXIT_DATA *pexit;
    int door;
    
    door = find_exit( ch, arg1, FEX_NONE );

    if (door < 0)
	return false;

    if ( ( pexit = ch->in_room->exit[door] ) == 0 )
    {
	    ch->println( "������ ���������� ���." );
	    return true;
    }

    if ( pexit->description != 0 && pexit->description[0] != '\0' ) {
	    ch->send_to( pexit->description);
            ch->send_to( "\r\n" );
    }
    else
	    ch->println( "����� ��� ������ ����������." );

    if ( pexit->keyword    != 0
	    && pexit->keyword[0] != '\0'
	    && pexit->keyword[0] != ' ' )
    {
	    if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    {
		    act_p( "$d: ��� �������.", ch, 0, pexit->keyword, TO_CHAR,POS_RESTING );
	    }
	    else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	    {
		    act_p( "$d: ��� �������.",   ch, 0, pexit->keyword, TO_CHAR,POS_RESTING );
	    }
    }
    
    DoorKeyhole( ch, ch->in_room, door ).doExamine( );
    return true;
}

// TODO show act msg, item type and wear.
static void do_look_object( Character *ch, Object *obj )
{
        ostringstream buf;
            
        buf << "�� �������� �� {c" << obj->getShortDescr( '4' ) << "{x."
            << " ��� {W" << item_table.message(obj->item_type) << "{x";


        for (int i = 0; i < wearlocationManager->size( ); i++) {
            Wearlocation *loc = wearlocationManager->find( i );
            if (loc->matches( obj )) {
                buf << ", " << loc->getPurpose( ).toLower( );
                break;
            }
        }

        buf << "." << endl;
        ch->send_to( buf.str( ) );

        DLString desc = oprog_extra_descr( obj, ch, obj->getName( ) );
        if (desc.empty( )) { 
            for (EXTRA_DESCR_DATA *ed = obj->extra_descr; ed; ed = ed->next) 
                if (arg_contains_someof( ed->keyword, obj->getName( ) )) {
                    desc = ed->description;
                    break;
                }
        }

        if (desc.empty( )) { 
            for (EXTRA_DESCR_DATA *ed = obj->pIndexData->extra_descr; ed; ed = ed->next) 
                if (arg_contains_someof( ed->keyword, obj->getName( ) )) {
                    desc = ed->description;
                    break;
                }
        }

        if (desc.empty( )) {
	    if (obj->in_room)
		desc = obj->getDescription( );
	    else
		desc = "�� �� ������ ����� ������ ����������.";
        }            

        ostringstream descBuf;
        webManipManager->decorateExtraDescr( descBuf, desc.c_str( ), obj->pIndexData->extra_descr, ch );
        ch->send_to( descBuf );

	oprog_look( obj, ch, obj->getName( ) );
}

static bool do_look_extraexit( Character *ch, const char *arg3 )
{
    EXTRA_EXIT_DATA * peexit = get_extra_exit( arg3, ch->in_room->extra_exit );

    if (!peexit)
	return false;

    if ( peexit->description != 0
	    && peexit->description[0] != '\0'
	    && ch->can_see( peexit ) )
	    ch->send_to( peexit->description);
    else
	    ch->println( "����� ��� ������ ����������." );
    
    if (peexit->short_desc_from != 0
	&& peexit->short_desc_from[0] != '\0'
	&& ch->can_see( peexit ) )
    {
	    if ( IS_SET(peexit->exit_info, EX_CLOSED) )
	    {
		ch->pecho( "%1$N1: ��� �������.", peexit->short_desc_from );
	    }
	    else if ( IS_SET(peexit->exit_info, EX_ISDOOR) )
	    {
		ch->pecho( "%1$N1: ��� �������.", peexit->short_desc_from );
	    }
    }
    
    ExtraExitKeyhole( ch, ch->in_room, peexit ).doExamine( );
    return true;
}

/*-------------------------------------------------------------------------
 * 'look' command 
 *-------------------------------------------------------------------------*/
CMDRUNP( look )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    Character *victim;
    int number;
    bool fAuto, fMove, fBrief;

    if (ch->desc == 0)
	return;

    if (ch->position < POS_SLEEPING) {
	ch->println( "�� �� ������ ������, ����� �����!" );
	return;
    }

    if (ch->position == POS_SLEEPING) {
	ch->println( "�� ������ �� ������, �� �����!" );
	return;
    }
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    fAuto = !str_cmp( arg1, "auto" );
    fMove = !str_cmp( arg1, "move" );
    fBrief = !ch->is_npc( ) && IS_SET(ch->comm, COMM_BRIEF);

    if (eyes_blinded( ch )) {
	if (fAuto || fMove)
	    ch->println( "�� �� ������ ������ ����!" );
	else
	    eyes_blinded_msg( ch );

	return;
    }

    if (arg1[0] == '\0') {
	do_look_auto( ch, ch->in_room, false );
	return;
    }
    
    if (fAuto) {
	do_look_auto( ch, ch->in_room, fBrief );
	return;
    }

    if (fMove) {
	do_look_move( ch, fBrief );
	return;
    }

    if (eyes_darkened( ch )) {
	ch->println( "���� �� ������� ������ ���������� � ��������� �������." );
	return;
    }
    
    if (arg_is_in( arg1 ) || arg_is_on( arg1 )) {
	do_look_into( ch, arg2 );
	return;
    }

    if (( victim = get_char_room( ch, arg1 ) ) != 0) {
	do_look_character( ch, victim );
	return;
    }

    Object *obj;
    if (get_arg_id( arg1 ) && (obj = get_obj_here( ch, arg1 ))) {
        do_look_object( ch, obj );
        return;
    }
    
    if (do_look_extradescr( ch, arg3, number ))
	return;

    if (do_look_extraexit( ch, arg3 ))
	return;

    if (do_look_direction( ch, arg1 ))
	return;

    ch->println( "�� �� ������ ����� ���." );
}

/*-------------------------------------------------------------------------
 * 'read' command 
 *-------------------------------------------------------------------------*/
CMDRUNP( read )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    int number;

    if (eyes_blinded( ch )) {
	eyes_blinded_msg( ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    number = number_argument( arg1, arg2 );

    if (!do_look_extradescr( ch, arg2, number ))
	ch->println( "�� �� ������ ����� ���." );
}

/*-------------------------------------------------------------------------
 * 'examine' command 
 *-------------------------------------------------------------------------*/
CMDRUNP( examine )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    Character *victim;

    if (eyes_blinded( ch )) {
	eyes_blinded_msg( ch );
	return;
    }

    argument = one_argument( argument, arg );

    if (arg[0] == '\0') {
	ch->println( "������� ���?" );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) != 0 )
    {
	if ( victim->can_see( ch ) )
	{
	    if (ch == victim)
		act_p( "$c1 ����������� ����.",ch,0,0,TO_ROOM,POS_RESTING);
	    else
	    {
		act_p( "$c1 ������� ������ �� ����.", ch, 0, victim, TO_VICT,POS_RESTING);
		act_p( "$c1 ������� ������ �� $C4.",  ch, 0, victim, TO_NOTVICT,POS_RESTING);
	    }
	}

	show_char_to_char_1( victim, ch, true );
	return;
    }
    
    if ( ( obj = get_obj_here( ch, arg ) ) != 0 ) {
	oprog_examine( obj, ch, argument );
	return;
    }

    ch->println( "�� �� ������ ����� ���." );
}



/*---------------------------------------------------------------------------
 * examine object trigger
 *--------------------------------------------------------------------------*/
static bool oprog_examine_money( Object *obj, Character *ch, const DLString& )
{
    if (obj->value[0] == 0)
    {
	if (obj->value[1] == 0)
		ch->printf("����... �� ����� ��� ������.\n\r");
	else if (obj->value[1] == 1)
		ch->printf("��-��. ���� ������� �������!\n\r");
	else
		ch->printf("����� %d �����%s.\n\r",
			obj->value[1],GET_COUNT(obj->value[1], "�� ������","�� ������","�� �����"));
    }
    else if (obj->value[1] == 0)
    {
	if (obj->value[0] == 1)
		ch->printf("��-��. ���� ���������� �������.\n\r");
	else
		ch->printf("����� %d ��������%s.\n\r",
			obj->value[0],GET_COUNT(obj->value[0], "�� ������","�� ������","�� �����"));
    }
    else
	ch->printf("����� %d �����%s � %d ��������%s.\n\r",
		obj->value[1],GET_COUNT(obj->value[1], "��","��","��"),
		obj->value[0],GET_COUNT(obj->value[0], "�� ������","�� ������","�� �����"));
    return true;
}

static bool oprog_examine_drink_container( Object *obj, Character *ch, const DLString& )
{
    if (IS_SET(obj->value[3], DRINK_CLOSED)) {
	ch->println("��� ������� �������.");
	return true;
    }

    if (obj->value[1] <= 0) {
	ch->println( "��� �����." );
	return true;
    }

    ch->printf( "%s �������� ��������� %s �����.\n\r",
		obj->value[1] < obj->value[0] / 4 ? 
		    "������, ��� �� ��������" :
		    obj->value[1] < 3 * obj->value[0] / 4 ? 
			"�� ��������"  : 
			"������, ��� �� ��������",
		liquidManager->find( obj->value[2] )->getColor( ).ruscase( '2' ).c_str( )
	      );
    return true;
}

static bool oprog_examine_portal( Object *obj, Character *ch, const DLString &pocket )
{
    return PortalKeyhole( ch, obj ).doExamine( );
}

static bool oprog_examine_container( Object *obj, Character *ch, const DLString &pocket )
{
    ContainerKeyhole( ch, obj ).doExamine( );

    if (IS_SET(obj->value[1], CONT_CLOSED)) {
	ch->println( "��� �������." );
	return true;
    }
    
    if (!pocket.empty( )) {
	if (!IS_SET(obj->value[1], CONT_WITH_POCKETS)) {
	    ch->println( "�� �� ������ ����� �� ������ �������." );
	    return true;
	}
    }
    
    const char *p = pocket.c_str( );

    if (IS_SET(obj->value[1],CONT_PUT_ON|CONT_PUT_ON2)) {
	if (!pocket.empty( ))
	    ch->pecho( "��������� '%2$s' %1$O2 ��������:", obj, p );
	else
	    ch->pecho( "�� %1$O6 �� ������:", obj );
    }
    else {
	if (!pocket.empty( )) {
	    if (!obj->can_wear( ITEM_TAKE ))
		ch->pecho( "�� ����� %1$O2 � �������� '%2$s' �� ������:", obj, p );
	    else
		ch->pecho( "� ������� %1$O2 � �������� '%2$s' �� ������:", obj, p );
	}
	else {
	    ch->pecho( "%1$^O1 ������%1$n��|��:", obj );
	}
    }

    show_list_to_char( obj->contains, ch, true, true, pocket, obj );
    return true;
}    

static bool oprog_examine_corpse( Object *obj, Character *ch, const DLString & )
{
    act( "$o1 ��������:", ch, obj, 0, TO_CHAR );
    show_list_to_char( obj->contains, ch, true, true );
    return true;
}	

static bool oprog_examine_keyring( Object *obj, Character *ch, const DLString & )
{
    act( "�� $o4 ��������:", ch, obj, 0, TO_CHAR );
    show_list_to_char( obj->contains, ch, true, true );
    return true;
}	

bool oprog_examine( Object *obj, Character *ch, const DLString &arg )
{
    FENIA_CALL( obj, "Examine", "Cs", ch, arg.c_str( ) );
    FENIA_NDX_CALL( obj, "Examine", "OCs", obj, ch, arg.c_str( ) );
    BEHAVIOR_CALL( obj, examine, ch );
   
    bool rc = false;
    switch (obj->item_type) {
    case ITEM_MONEY:
	rc = oprog_examine_money( obj, ch, arg );
        break;

    case ITEM_DRINK_CON:
	rc = oprog_examine_drink_container( obj, ch, arg );
        break;
	
    case ITEM_CONTAINER:
	rc = oprog_examine_container( obj, ch, arg );
        break;

    case ITEM_KEYRING:
	rc = oprog_examine_keyring( obj, ch, arg );
        break;
	
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
	rc = oprog_examine_corpse( obj, ch, arg );
        break;

    case ITEM_PORTAL:
	rc = oprog_examine_portal( obj, ch, arg );
        break;
    }

    if (!rc)
        ch->pecho( "������ %O2 ���������� ���������.", obj );

    return rc;
}

/*---------------------------------------------------------------------------
 * 'exits' command 
 *--------------------------------------------------------------------------*/
static void show_exits_to_char( Character *ch, Room *targetRoom )
{
    ostringstream buf;
    EXIT_DATA *pexit;
    const char *ename;
    PlayerConfig::Pointer cfg;
    Room *room;
    bool found;

    if (eyes_blinded( ch )) 
	return;

    buf << "{" << CLR_AEXIT(ch) << "[������:";
    found = false;
    cfg = ch->getConfig( );

    for (int door = 0; door < DIR_SOMEWHERE; door++) {
	if (!( pexit = targetRoom->exit[door] ))
	    continue;
	if (!( room = pexit->u1.to_room ))
	    continue;
	if (!ch->can_see( room ))
	    continue;

	ename = (cfg->ruexits ? dirs[door].rname : dirs[door].name);

	if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
	    found = true;
	    buf << " " << ename;
	}
	else if (number_percent() < gsn_perception->getEffective( ch )) {
	    found = true;
	    gsn_perception->improve( ch, true );
	    buf << " {x" << ename << "*{" << CLR_AEXIT(ch);
	}
    }
    
    if (!found)
	buf << (cfg->ruexits ? " ���" : " none");
	    
    buf << "]{x" << endl;
    ch->send_to( buf );
}

CMDRUNP( exits )
{
    ostringstream buf;
    EXIT_DATA *pexit;
    const char *ename;
    Room *room;
    PlayerConfig::Pointer cfg;
    bool found;
    int door;

    if (eyes_blinded( ch )) {
	act( "�� �������$g��|�|�� � �� ������ ������ ������ ����!", ch, 0, 0, TO_CHAR );
	return;
    }

    if (ch->is_immortal())
	buf << "������� ������ �� ������� " << ch->in_room->vnum << ":" << endl;
    else
	buf << "������� ������:" << endl;

    found = false;
    cfg = ch->getConfig( );

    for (door = 0; door <= 5; door++)
    {
	if (!( pexit = ch->in_room->exit[door] ))
	    continue;
	if (!( room = pexit->u1.to_room ))
	    continue;
	if (!ch->can_see( room ))
	    continue;

	ename = (cfg->ruexits ? dirs[door].rname : dirs[door].name);

	if (!IS_SET(pexit->exit_info, EX_CLOSED))
	{
	    found = true;

	    buf << fmt( ch, "%-6^s", ename ) << " - ";

	    if (room->isDark( ) && !cfg->holy)
		buf << "������ ����� � ������� � �������������...";
	    else
		buf << room->name;

	    if (ch->is_immortal())
		buf << " (room " << room->vnum << ")";
	    
	    buf << endl;
	}
	else {
	    if (number_percent() < gsn_perception->getEffective( ch )) {
		gsn_perception->improve( ch, true );
		found = true;

		buf << fmt( ch, "%-6^s", ename ) 
		    << " * (" << pexit->keyword << ")";

		if (ch->is_immortal())
		    buf << " (room " << room->vnum << ")";
		
		buf << endl;
	    }
	}
    }
    
    if (!found)
	buf << (cfg->ruexits ? "���." : "None.") << endl;
	    
    ch->send_to( buf );
}

