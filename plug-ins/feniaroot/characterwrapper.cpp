/* $Id: characterwrapper.cpp,v 1.1.4.50.4.40 2009/11/08 17:46:27 rufina Exp $
 *
 * ruffina, 2004
 */

#include <iostream>

#include "json/json.h"

#include "logstream.h"
#include "mobilebehavior.h"
#include "mobilebehaviormanager.h"

#include "skill.h"
#include "skillmanager.h"
#include "clan.h"

#include "affect.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "desire.h"
#include "npcharacter.h"
#include "race.h"
#include "object.h"
#include "room.h"

#include "craftattribute.h"
#include "occupations.h"
#include "interp.h"
#include "comm.h"
#include "save.h"
#include "mercdb.h"
#include "fight.h"
#include "magic.h"
#include "movement.h"
#include "act_move.h"
#include "merc.h"
#include "handler.h"
#include "alignment.h"
#include "wiznet.h"
#include "xmlattributecoder.h"
#include "xmlattributerestring.h"
#include "pet.h"

#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "wrappermanager.h"
#include "mobindexwrapper.h"
#include "structwrappers.h"
#include "affectwrapper.h"
#include "xmleditorinputhandler.h"
#include "reglist.h"
#include "regcontainer.h"
#include "nativeext.h"

#include "wrap_utils.h"
#include "subr.h"
#include "def.h"

GSN(dark_shroud);
GSN(manacles);
GSN(charm_person);
DESIRE(hunger);
DESIRE(bloodlust);
DESIRE(thirst);
DESIRE(full);
DESIRE(drunk);

void password_set( PCMemoryInterface *pci, const DLString &plainText );
const char *ttype_name( int ttype );

using namespace std;
using namespace Scripting;
using Scripting::NativeTraits;

NMI_INIT(CharacterWrapper, "�������� (��� ��� �����)")

CharacterWrapper::CharacterWrapper( ) : target( NULL )
{
}

void CharacterWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );
    
    if (!self && target) {
	target->wrapper = 0;
	target = 0;
    }
}

void CharacterWrapper::extract( bool count )
{
    if (target) {
	target->wrapper = 0;
	target = 0;
    } else {
	if (Scripting::gc)
	    LogStream::sendError() << "Character wrapper: extract without target" << endl;
    }
    
    GutsContainer::extract( count );
}

void CharacterWrapper::setTarget( ::Character *target )
{
    this->target = target;
    id = target->getID( );
}

void CharacterWrapper::checkTarget( ) const throw( Scripting::Exception )
{
    if (zombie.getValue())
	throw Scripting::Exception( "Character is dead" );

    if (target == NULL) 
	throw Scripting::Exception( "Character is offline" );
}

Character * CharacterWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

/*
 * FIELDS
 */
NMI_GET( CharacterWrapper, id, "���������� ����� ���������" )
{
    return Register( DLString(id) );
}

NMI_GET( CharacterWrapper, online, "���������� true, ���� �������� � ����" )
{
    return Register( target != NULL );
}

NMI_GET( CharacterWrapper, dead, "���������� true, ���� �������� ��������� ��������� (suicide ��� pc, ������ ��� npc)" )
{
    return Register( zombie.getValue() );
}

#define CHK_PC \
    if (!target->is_npc()) \
	throw Scripting::Exception( "NPC field requested on PC" ); 
#define CHK_NPC \
    if (target->is_npc()) \
	throw Scripting::Exception( "PC field requested on NPC" ); 

#define GETWRAP(x, h) NMI_GET(CharacterWrapper, x, h) { \
    checkTarget(); \
    return wrap(target->x); \
}
#define GET_NPC_WRAP(x, h) NMI_GET(CharacterWrapper, x, h) { \
    checkTarget(); \
    CHK_PC \
    return wrap(target->getNPC()->x); \
}
#define GET_PC_WRAP(x, h) NMI_GET(CharacterWrapper, x, h) { \
    checkTarget(); \
    CHK_NPC \
    return wrap(target->getPC()->x); \
}

GET_NPC_WRAP( pIndexData, "��������� � ���������� ��� ���� ����� � ������ vnum"
                          "(mob index data, �.�. ��, ������������� � ������� OLC)")
GETWRAP( reply, "���, ������� ��������� ������� � ����. �� ������� reply ������� ���������� ������ ���" )
GETWRAP( next, "��������� ��� � ���������� ������ ���� �����, .char_list" )
GETWRAP( next_in_room, "��������� ��� � ���� �������, � ������ people � �������" )
GETWRAP( master, "���, �� ��� �������" )
GETWRAP( leader, "����� ������ ��� ���, ��� ��������" )
GETWRAP( fighting, "���, � ��� ���������" )
GETWRAP( last_fought, "���, � ������� ��������� ��������� ���" )
GET_PC_WRAP( pet, "���, �������� ��������" )
GET_PC_WRAP( switchedTo, "" )
GETWRAP( doppel, "�����, �������� ��������� � ������� doppelganger. "
                 "��� ������ - �����, ������� �� ������" )
GET_PC_WRAP( guarding, "�����, �������� �������� � ������� ������ guard" )
GET_PC_WRAP( guarded_by, "�����, ������� ��� ��������" )

GETWRAP( carrying, "������, ������ � ������ ������� � ���� (inventory � equipment). "
                   "�� ��������� ��������� ������ ����� ����������� ����� ���� ������� next_content")
GETWRAP( on, "������, ������, �� ������� �����" )

GETWRAP( in_room, "�������, � ������� ������ ���������" ) 
GETWRAP( was_in_room, "�������, � ������� ���������� ����� ���������� � �����(��� ������), \r\n"
                      "���� ����� ������������ � ������ (��� ��������)")
GETWRAP( mount, "�� ��� �� ������ ��� ��� ������ �� ���" )
    
NMI_SET( CharacterWrapper, leader, "����� ������ ��� ���, ��� ��������" )
{
    checkTarget( );

    if (arg.type == Register::NONE)
	target->leader = NULL;
    else
	target->leader = arg2character( arg );
}
NMI_SET( CharacterWrapper, last_fought, "���, � ������� ��������� ��������� ���" )
{
    checkTarget( );

    if (arg.type == Register::NONE)
	target->last_fought = NULL;
    else
	target->last_fought = arg2character( arg );
}


#define ARMOR(x) \
NMI_GET( CharacterWrapper, armor##x, "�����-�����" ) \
{ \
    checkTarget(); \
    return target->armor[x]; \
} \
NMI_SET( CharacterWrapper, armor##x, "�����-�����" ) \
{ \
    checkTarget(); \
    target->armor[x] = arg.toNumber(); \
}

ARMOR(0)
ARMOR(1)
ARMOR(2)
ARMOR(3)
#undef ARMOR

NMI_GET( CharacterWrapper, pc, "" )
{
    checkTarget( );
    return wrap(target->getPC( ));
}

NMI_GET( CharacterWrapper, logon, "" )
{
    checkTarget( );
    CHK_NPC
    return (int)target->getPC( )->age.getLogon( ).getTime( );
}
NMI_SET( CharacterWrapper, logon, "" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->age.setLogon( arg.toNumber( ) );
}
NMI_GET( CharacterWrapper, terminal_type, "��� ��������� � mud-�������" )
{
    checkTarget( );
    CHK_NPC
    if (!target->desc)
	return "";
    return ttype_name( target->desc->telnet.ttype );
}

NMI_SET( CharacterWrapper, damage_number, "" )
{
    checkTarget( );
    CHK_PC
    target->getNPC( )->damage[DICE_NUMBER] = arg.toNumber();        
}
NMI_SET( CharacterWrapper, damage_type, "" )
{
    checkTarget( );
    CHK_PC
    target->getNPC( )->damage[DICE_TYPE] = arg.toNumber();        
}
NMI_GET( CharacterWrapper, damage_number, "" )
{
    checkTarget( );
    CHK_PC
    return target->getNPC( )->damage[DICE_NUMBER];
}
NMI_GET( CharacterWrapper, damage_type, "" )
{
    checkTarget( );
    CHK_PC
    return target->getNPC( )->damage[DICE_TYPE];
}

NMI_INVOKE( CharacterWrapper, setLevel, "���������� ������� ����" )
{
    checkTarget();
    CHK_PC

    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );

    target->setLevel( args.front( ).toNumber( ) );
    return Register( );
}

NMI_GET( CharacterWrapper, short_descr, "�������� �������� ����" )
{
    checkTarget( );
    CHK_PC
    return Register( target->getNPC()->getShortDescr( ) );
}

NMI_SET( CharacterWrapper, short_descr, "�������� �������� ����" )
{
    checkTarget( );
    CHK_PC
    target->getNPC()->setShortDescr( arg.toString( ) );
}

NMI_GET( CharacterWrapper, long_descr, "������� �������� ����" )
{
    checkTarget( );
    CHK_PC
    return Register( target->getNPC()->getLongDescr( ) );
}

NMI_SET( CharacterWrapper, long_descr, "������� �������� ����" )
{
    checkTarget( );
    CHK_PC
    target->getNPC()->setLongDescr( arg.toString( ) );
}

NMI_GET( CharacterWrapper, description, "�� ��� ����� �� look mob" )
{
    checkTarget( );
    return Register( target->getDescription( ) );
}

NMI_SET( CharacterWrapper, description, "�� ��� ����� �� look mob" )
{
    checkTarget( );
    target->setDescription( arg.toString( ) );
}

NMI_GET( CharacterWrapper, trust, "" )
{
    PCMemoryInterface *pci;
    
    checkTarget( );
    
    if (!target->is_npc( ) && target->getLevel( ) == 0) { // may be not loaded yet
	if (( pci = PCharacterManager::find( target->getName( ) ) ))
	    return pci->get_trust( );
	else
	    return 0;
    }

    return target->get_trust( );
}

NMI_GET( CharacterWrapper, pretitle, "��������" )
{
    checkTarget( );
    CHK_NPC
    return Register( target->getPC( )->getPretitle( ) );
}

NMI_SET( CharacterWrapper, pretitle, "��������" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setPretitle( arg.toString( ) );
}

NMI_GET( CharacterWrapper, title, "�����" )
{
    checkTarget( );
    CHK_NPC
    return Register( target->getPC( )->getTitle( ) );
}

NMI_SET( CharacterWrapper, title, "�����" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setTitle( arg.toString( ) );
}

NMI_GET( CharacterWrapper, password, "������: deprecated" )
{
    PCMemoryInterface *pci;

    checkTarget( );
    CHK_NPC

    if (( pci = PCharacterManager::find( target->getName( ) ) ))
	return pci->getPassword( );
    else
	return target->getPC( )->getPassword( );
}

NMI_SET( CharacterWrapper, password, "������" )
{
    checkTarget( );
    CHK_NPC
    password_set( target->getPC( ), arg.toString( ) );
}

NMI_GET( CharacterWrapper, remort_count, "���-�� ��������" )
{
    checkTarget( );
    CHK_NPC
    return Register( (int)target->getPC( )->getRemorts( ).size( ) );
}

NMI_GET( CharacterWrapper, altar, "vnum �������-������ � ��������� ����" )
{
    checkTarget( );
    CHK_NPC
    return Register( (int)target->getPC( )->getHometown( )->getAltar( ) );
}


NMI_GET( CharacterWrapper, craftProfessions, "map �� ��������->������� ���������� ��� ��������� ���������" )
{
    ::Pointer<RegContainer> rc(NEW);
    checkTarget( );
    CHK_NPC
    XMLAttributeCraft::Pointer attr = target->getPC( )->getAttributes( ).findAttr<XMLAttributeCraft>("craft");
    XMLAttributeCraft::Proficiency::const_iterator p;
    
    if (attr) 
        for (p = attr->getProficiency().begin(); p != attr->getProficiency().end(); p++)
            (*rc)->map[p->first] = Register(p->second.level);

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

#define CONDITION(type, api) \
NMI_GET( CharacterWrapper, cond_##type, api ) \
{ \
    checkTarget( ); \
    CHK_NPC \
    return Register( target->getPC( )->desires[desire_##type] ); \
} \
NMI_SET( CharacterWrapper, cond_##type, "�������� '" api "' �� ��������� ����� ������" ) \
{ \
    checkTarget( ); \
    CHK_NPC \
    desire_##type->gain( target->getPC( ), arg.toNumber( ) ); \
}

CONDITION(hunger,    "�����");
CONDITION(thirst,    "�����");
CONDITION(full,      "������������� �������");
CONDITION(bloodlust, "����� �����");
CONDITION(drunk,     "���������");
#undef CONDITION


NMI_SET( CharacterWrapper, sex, "")
{
    checkTarget( );
    target->setSex( arg.toNumber( ) );
}

NMI_GET( CharacterWrapper, sex, "")
{
    checkTarget( );
    return target->getSex( );
}

NMI_GET( CharacterWrapper, wait, "")
{
    checkTarget( );
    return target->wait;
}

NMI_SET( CharacterWrapper, wait, "wait state (� �������, 1 ����� = �������� �������)")
{
    checkTarget( );
    target->setWait( arg.toNumber( ) );
}

NMI_GET( CharacterWrapper, boat, "������ �����" )
{
    checkTarget( );
    return wrap( boat_object_find( target ) );
}

NMI_GET( CharacterWrapper, flying, "true ���� �� GHOST, ������ ��� ������ �� �������� �������" )
{
    checkTarget( );
    
    if (IS_GHOST(target))
	return true;
	
    if (is_flying( target ))
	return true;

    if (MOUNTED(target) && is_flying(MOUNTED(target)))
	return true;

    return false;
}

NMI_GET( CharacterWrapper, alignMin, "" )
{
    checkTarget( );
    CHK_NPC
    return align_min( target->getPC( ) );
}

NMI_GET( CharacterWrapper, alignMax, "" )
{
    checkTarget( );
    CHK_NPC
    return align_max( target->getPC( ) );
}

NMI_GET( CharacterWrapper, alignName, "" )
{
    checkTarget( );
    return align_name( target );
}


#define DEF_STAT(x, stat, help) \
NMI_GET( CharacterWrapper, cur_##x, "������� ��������: " help ) \
{ \
    checkTarget( ); \
    return Register( target->getCurrStat(stat) ); \
} \
NMI_GET( CharacterWrapper, max_train_##x, "�������� ���������� ��� ���������: " help ) \
{ \
    checkTarget( ); \
    CHK_NPC \
    return Register( target->getPC( )->getMaxTrain(stat) ); \
} \
NMI_GET( CharacterWrapper, perm_##x, "������������ ��������: " help ) \
{ \
    checkTarget( ); \
    return Register( target->perm_stat[stat] ); \
} \
NMI_SET( CharacterWrapper, perm_##x, "������������ ��������: " help ) \
{ \
    checkTarget( ); \
    int max_value = (target->is_npc( ) ? MAX_STAT : target->getPC( )->getMaxTrain(stat)); \
    target->perm_stat[stat] = URANGE(1, arg.toNumber( ), max_value); \
}

DEF_STAT(str, STAT_STR, "����")
DEF_STAT(int, STAT_INT, "��")
DEF_STAT(wis, STAT_WIS, "��������")
DEF_STAT(dex, STAT_DEX, "��������")
DEF_STAT(con, STAT_CON, "������������")
DEF_STAT(cha, STAT_CHA, "�������")

#define STR_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    return Register( target->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    target->x = arg.toString(); \
}

STR_FIELD(prompt, "������ ���������")
STR_FIELD(batle_prompt, "������ ��������� � ���")

#define INT_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    return Register( (int) target->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    target->x = arg.toNumber(); \
}

#define FLAG_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    return Register( (int) target->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    target->x.setValue( arg.toNumber() ); \
}

INT_FIELD(ethos, "�����������������")
INT_FIELD(timer, "������� ������ ������ � ��������� �������")
INT_FIELD(daze, "dase state (� �������, 1 ����� = �������� �������)")
INT_FIELD(hit, "������� �������� (hit points)")
INT_FIELD(max_hit, "������������ ��������")
INT_FIELD(mana, "������� mana")
INT_FIELD(max_mana, "������������ mana")
INT_FIELD(move, "������� moves")
INT_FIELD(max_move, "������������ moves")
INT_FIELD(gold, "������")
INT_FIELD(silver, "�������")
INT_FIELD(exp, "��������� ����")
INT_FIELD(invis_level, "������� ��� wisinvis")
INT_FIELD(incog_level, "������� ��� incognito")
INT_FIELD(lines, "���-�� ����� � ������ ������")
INT_FIELD(act, "act ����� ��� ����� � plr ��� �������")
INT_FIELD(comm, "comm �����")
INT_FIELD(add_comm, "���������� ���� comm")
INT_FIELD(imm_flags, "����� ����������")
INT_FIELD(res_flags, "����� ����������������")
INT_FIELD(vuln_flags, "����� ����������")
INT_FIELD(affected_by, "����� ��������")
INT_FIELD(add_affected_by, "���������� ������ ��������")
INT_FIELD(detection, "����� ��������")
INT_FIELD(position, "�������")
INT_FIELD(carry_weight, "��� ������� ����� ���")
INT_FIELD(carry_number, "���������� ����� ������� ����� ���")
INT_FIELD(saving_throw, "������")
INT_FIELD(alignment, "��������")
INT_FIELD(hitroll, "hr")
INT_FIELD(damroll, "dr")
INT_FIELD(wimpy, "��������. ��� ������� hp ��� ����� ������� �������������")
INT_FIELD(dam_type, "��� �����������")
INT_FIELD(form, "�����")
INT_FIELD(parts, "����� ����")
INT_FIELD(size, "������")
INT_FIELD(death_ground_delay, "������� �������")
FLAG_FIELD(trap, "����� �������")
INT_FIELD(riding, "���� mount!=null: true - �� ������, false - �� ��������")

#undef INT_FIELD
#define INT_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    CHK_NPC \
    checkTarget( ); \
    return Register( (int) target->getPC()->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    CHK_NPC \
    checkTarget( ); \
    target->getPC()->x = arg.toNumber(); \
}

INT_FIELD(last_level, "����� ��� played, ����� ������ ��������� �����")
INT_FIELD(last_death_time, "����� ��������� ��� ��� ����")
INT_FIELD(ghost_time, "������� ����� ghost")
INT_FIELD(PK_time_v, "������� ����� violent")
INT_FIELD(PK_time_sk, "������� ����� slain � killer")
INT_FIELD(PK_time_t, "������� ����� thief")
INT_FIELD(PK_flag, "KILLER, SLAIN, VIOLENT, GHOST, THIEF")
INT_FIELD(death, "������� ��� ������")
INT_FIELD(anti_killed, "������� ����� �� ����� align �����")
INT_FIELD(has_killed, "������� ����� ����� �����")
INT_FIELD(perm_hit, "max hp ��� �����")
INT_FIELD(perm_mana, "max mana ��� �����")
INT_FIELD(perm_move, "max move ��� �����")
INT_FIELD(max_skill_points, "���-�� ����������� � ����")
INT_FIELD(practice, "������� �������")
INT_FIELD(train, "������� ����������")
INT_FIELD(loyalty, "���������� �� ��������� � ������ (������������)")
INT_FIELD(curse, "��������� �����")
INT_FIELD(bless, "������������� �����")
INT_FIELD(bank_s, "������� � �����")
INT_FIELD(bank_g, "������ � �����")
INT_FIELD(questpoints, "qp")
INT_FIELD(config, "��������� ����")
INT_FIELD(shadow, "������� ������ ���� (shadowlife) � ��������")
INT_FIELD(start_room, "")
    
#undef INT_FIELD

#define INT_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    CHK_PC \
    checkTarget( ); \
    return Register( (int) target->getNPC()->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    CHK_PC \
    checkTarget( ); \
    target->getNPC()->x = arg.toNumber(); \
}
INT_FIELD(off_flags, "")

NMI_SET( CharacterWrapper, wearloc, "")
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->wearloc.fromString( arg.toString( ) );
}

NMI_GET( CharacterWrapper, expToLevel, "")
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getExpToLevel( );
}

NMI_GET( CharacterWrapper, hostname, "")
{
    checkTarget( );

    if (!target->desc)
	return "";
    else
	return target->desc->getRealHost( );
}

NMI_GET( CharacterWrapper, level, "��������� �������" )
{
    checkTarget( );
    return target->getRealLevel( );
}

NMI_SET( CharacterWrapper, level, "��������� �������" )
{
    checkTarget( );
    return target->setLevel( arg.toNumber( ) );
}

NMI_GET( CharacterWrapper, lastAccessTime, "" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getLastAccessTime( ).getTimeAsString( );
}

NMI_GET( CharacterWrapper, profession, "" )
{
    checkTarget( );
    CHK_NPC
    return ProfessionWrapper::wrap( target->getPC( )->getProfession( )->getName( ) );
}

NMI_SET( CharacterWrapper, profession, "" )
{
    checkTarget( );
    CHK_NPC
    if (arg.type == Register::NONE)
	target->getPC( )->setProfession( "none" );
    else
	target->getPC( )->setProfession( wrapper_cast<ProfessionWrapper>(arg)->name );
}

NMI_GET( CharacterWrapper, uniclass, "���-��������� ����������" )
{
    checkTarget( );
    CHK_NPC
    return ProfessionWrapper::wrap( target->getPC( )->getSubProfession( )->getName( ) );
}

NMI_SET( CharacterWrapper, uniclass, "���-��������� ����������" )
{
    checkTarget( );
    CHK_NPC
    if (arg.type == Register::NONE)
	target->getPC( )->setSubProfession( "none" );
    else
	target->getPC( )->setSubProfession( wrapper_cast<ProfessionWrapper>(arg)->name );
}

NMI_GET( CharacterWrapper, hometown, "" )
{
    checkTarget( );
    CHK_NPC
    return HometownWrapper::wrap( target->getPC( )->getHometown( )->getName( ) );
}

NMI_SET( CharacterWrapper, hometown, "" )
{
    checkTarget( );
    CHK_NPC
    if (arg.type == Register::NONE)
	target->getPC( )->setHometown( "none" );
    else
	target->getPC( )->setHometown( wrapper_cast<HometownWrapper>(arg)->name );
}

NMI_SET( CharacterWrapper, russianName, "" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setRussianName( arg.toString( ) );
}

NMI_GET( CharacterWrapper, russianName, "" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getRussianName( ).getFullForm( );
}

NMI_SET( CharacterWrapper, name, "" )
{
    checkTarget( );
    target->setName( arg.toString( ) );
}

NMI_GET( CharacterWrapper, name, "" )
{
    checkTarget( );
    return target->getName( );
}

NMI_GET( CharacterWrapper, race, "" )
{
    checkTarget( );
    return RaceWrapper::wrap( target->getRace( )->getName( ) );
}

NMI_SET( CharacterWrapper, race, "" )
{
    checkTarget( );
    if (arg.type == Register::NONE)
	target->setRace( "none" );
    else
	target->setRace( wrapper_cast<RaceWrapper>(arg)->name );
}

NMI_GET( CharacterWrapper, connected, "" )
{
    Character *ch;
    
    checkTarget( );
    
    if (!target->is_npc( ) && target->getPC( )->switchedTo)
	ch = target->getPC( )->switchedTo;
    else
	ch = target;

    return (ch->desc != NULL);
}

NMI_GET( CharacterWrapper, isInInterpret, "true ���� ����� � ��������� ����� ������ (�� ed, �� olc, �� pager, etc)" )
{
    checkTarget();
    CHK_NPC
    return Register(target->desc && target->desc->handle_input.front( )->getType() == "InterpretHandler");
}

/*
 * METHODS
 */

NMI_INVOKE( CharacterWrapper, ptc, "print to char, �������� ������" )
{
    checkTarget( );
    DLString d = args.front().toString();
    page_to_char(d.c_str(), target);
    return Register();
}

NMI_INVOKE( CharacterWrapper, interpret, "�������������� ������, ��� ����� ��� �� ������ ���" )
{
    checkTarget( );

    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );

    DLString d = args.front().toString();
    return ::interpret( target, d.c_str() );
}

NMI_INVOKE( CharacterWrapper, interpret_raw, "��������� ������� � ����������� �� ����� ����, ��� ��������������� ��������" )
{
    DLString cmdName, cmdArgs;
    RegisterList::const_iterator i;
    checkTarget( );

    if (args.size( ) < 1)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    cmdName = i->toString( );

    if (++i != args.end( ))
        cmdArgs = i->toString( );

    ::interpret_raw( target, cmdName.c_str( ), cmdArgs.c_str( ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, interpret_cmd, "��������� ������� � ����������� �� ����� ����" )
{
    DLString cmdName, cmdArgs;
    RegisterList::const_iterator i;
    checkTarget( );

    if (args.size( ) < 1)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    cmdName = i->toString( );

    if (++i != args.end( ))
        cmdArgs = i->toString( );

    ::interpret_cmd( target, cmdName.c_str( ), cmdArgs.c_str( ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, get_char_world, "���������: ������ � ������ ����. ������ �������� ��� ��� ���� � ����� ������" )
{
    checkTarget( );
    return wrap( ::get_char_world( target, args2string( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_here, "���������: ������ � ������ �������. ������ ������� ��� ������ � �������, ��������� ��� equipment" )
{
    checkTarget( );
    return wrap( ::get_obj_here( target, args2string( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_room, "���������: ������ � ������ �������. ������ ������� ��� ������ � �������" )
{
    checkTarget( );
    return wrap( ::get_obj_room( target, args2string( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_wear, "���������: ������ � ������ �������. ������ ������� ���� ������ � ����������" )
{
    checkTarget( );
    return wrap( ::get_obj_wear( target, args2string( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_wear_vnum, "" )
{
    checkTarget( );

    int vnum = args2number( args );

    for (::Object *obj = target->carrying; obj; obj = obj->next_content)
	if (obj->pIndexData->vnum == vnum && obj->wear_loc != wear_none)
	    return wrap( obj );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, get_char_room, "���������: ������ � ������ ����. ������ �������� ��� ���� � ���� �������" )
{
    checkTarget( );
    
    Room *room;
    DLString name = args2string( args );

    if (args.size( ) == 2)
	room = arg2room( args.back( ) );
    else
	room = target->in_room;
    
    return wrap( ::get_char_room( target, room, name ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_carry, "���������: ������ � ������ �������. ������ ������� ��� ������ �� inventory ��� equipment" )
{
    checkTarget( );
    return wrap( ::get_obj_carry( target, args2string( args ) ) );
}


NMI_INVOKE( CharacterWrapper, transfer, "" )
{
    Room *room;
    Character *actor;
    RegisterList::const_iterator i = args.begin( );
    DLString m1, m2, m3, m4;
    
    checkTarget( );

    if (args.size( ) != 6)
	throw Scripting::NotEnoughArgumentsException( );
    
    room = arg2room( *i );
    actor = arg2character( *++i );
    m1 = (++i)->toString();
    m2 = (++i)->toString();
    m3 = (++i)->toString();
    m4 = (++i)->toString();
    transfer_char( target, actor, room, m1.c_str(), m2.c_str(), m3.c_str(), m4.c_str() );
    
    return Register( );
}

NMI_INVOKE( CharacterWrapper, char_to_room, "��������: �������. ���������� ���� � ��� �������" )
{
    checkTarget( );
    Room *room = arg2room( get_unique_arg( args ) ); 
    
    if (target->in_room) {
	undig( target );
	target->dismount( );
	::char_from_room( target );
    }

    ::char_to_room( target, room );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, is_npc, "���������� true, ���� ��� ���" )
{
    checkTarget( );
    return Register( (int)target->is_npc( ) );
}

NMI_INVOKE( CharacterWrapper, getName, "���������� ��� ������ ��� ������ ���� ����" )
{
    checkTarget( );
    return Register( target->getName() );
}

NMI_INVOKE( CharacterWrapper, setName, "������������� ����� ����" )
{
    checkTarget( );
    CHK_PC
    target->setName( args2string( args ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, seeName, 
	"���������: ��� ch, ����� ������ (�� ��������� ������������)."
	"������ ��, ��� this ����� ��� � �������� (�������� ��������) ���� ch. "
	"���������, ��� ch ������� ��� this." )
{
    checkTarget( );
    int cse = 1;
    
    RegisterList::const_iterator i = args.begin( );

    if(i == args.end())
	throw Scripting::NotEnoughArgumentsException( );

    Character *ch = arg2character( *i );

    i++;
    
    if(i != args.end())
	cse = i->toNumber();
	
    return Register( target->seeName(ch, '0' + cse ) );
}

NMI_INVOKE( CharacterWrapper, can_see_mob, "���������: ���. ������ true, ���� this ����� ��� ������ " )
{
    checkTarget( );
    return target->can_see( arg2character( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, can_see_obj, "���������: ������. ������ true, ���� ������ ������� ��� this" )
{
    checkTarget( );
    return target->can_see( arg2item( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, can_see_room, "���������: �������. ������ true, ���� ������� ������ ��� this" )
{
    checkTarget( );
    return target->can_see( arg2room( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, can_see_exit, "" )
{
    int door;
    EXIT_DATA *pExit;

    checkTarget( );
    door = args2number( args );
    if (door < 0 || door >= DIR_SOMEWHERE)
	throw Scripting::IllegalArgumentException( );

    if (!( pExit = target->in_room->exit[door] ))
	return false;

    return target->can_see( pExit );
}

DLString regfmt(Character *to, const RegisterList &argv);

NMI_INVOKE( CharacterWrapper, print, "���������: ������-������, ���������. ���������� ����������������� ������ (������ sprintf)" )
{
    checkTarget();
    
    return Register( regfmt(target, args) );
}

NMI_INVOKE( CharacterWrapper, act, "���������: ������-������, ���������. �������� ��� ����������������� ������ (� �������� ����� ������). " )
{
    checkTarget();
    
    target->send_to( regfmt(target, args) + "\r\n");
    
    return Register( );
}

NMI_INVOKE( CharacterWrapper, recho, "���������: c�����-������, ���������. ������� ����������������� ������ ���� � �������, ����� ���" )
{
    checkTarget( );
    target->recho( regfmt( target, args ).c_str( ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, getModifyLevel, "������ �������, � ������ ������ �� ��������" )
{
    checkTarget();
    
    return target->getModifyLevel();
}

NMI_INVOKE( CharacterWrapper, getRealLevel, "������ ��������� �������" )
{
    checkTarget();
    
    return target->getRealLevel();
}

NMI_INVOKE( CharacterWrapper, getSex, "������ ����� ���� (0 neutral, 1 male, 2 female, 3 random - ������ � ����������)" )
{
    checkTarget();
    
    return target->getSex();
}

NMI_INVOKE( CharacterWrapper, is_immortal, "������ true, ���� this ����������� ��� �����" )
{
    checkTarget();
    
    return target->is_immortal();
}

NMI_INVOKE( CharacterWrapper, edit, "��������� this � ����� ��������������" )
{
    checkTarget();
    
    PCharacter *pch = target->getPC();
    
    if(!pch)
	throw Scripting::Exception( "only for PCs" );
    
    DLString str;
    
    XMLEditorInputHandler::Pointer eih( NEW );
    
    if(!args.empty()) {
	eih->clear( );
	eih->setBuffer(args.front().toString());
    }

    eih->attach(pch);

    return Register( );
}

NMI_INVOKE( CharacterWrapper, edReg, "([ndx[, txt]]) -- ����������/������������� ���������� ��������� ���������" )
{
    RegisterList::const_iterator i = args.begin( );

    checkTarget();
    
    PCharacter *pch = target->getPC();
    
    if(!pch)
	throw Scripting::Exception( "only for PCs" );
    
    unsigned char ndx = 0;

    if(i != args.end()) {
	ndx = i->toNumber();
	i++;
    }

    Editor::reg_t &reg = pch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[ndx];

    DLString str;

    if(i == args.end())
	for(Editor::reg_t::const_iterator j = reg.begin(); j != reg.end(); j++)
	    str.append(*j).append("\n");
    else 
	reg.split(str = i->toString());

    return Register(str);
}


NMI_INVOKE( CharacterWrapper, gainExp, "��������: �����. ��������� ��������� ���������� ����� �����" )
{
    checkTarget( );
    RegisterList::const_iterator i = args.begin( );

    if(i == args.end())
	throw Scripting::NotEnoughArgumentsException( );
    
    CHK_NPC
    target->getPC()->gainExp(i->toNumber());

    return Register();
}

NMI_INVOKE( CharacterWrapper, getClass, "���������� ������ � ��������� ���������" )
{
    checkTarget();
    return Register( target->getProfession( )->getName( ).c_str( ) );
}
NMI_INVOKE( CharacterWrapper, getClan, "���������� ������ � ��������� �����" )
{
    checkTarget();
    return Register( target->getClan( )->getShortName( ) );
}
NMI_INVOKE( CharacterWrapper, setClan, "������������� ���� �� ������ � ������" )
{
    Clan *clan;
    
    checkTarget();

    if (args.empty())
	throw Scripting::NotEnoughArgumentsException( );
    
    clan = ClanManager::getThis( )->findUnstrict( args.front( ).toString( ) );

    if (!clan)
	throw Scripting::IllegalArgumentException( );
    else
	target->setClan( clan->getName( ) );
    
    return Register( );
}
NMI_INVOKE( CharacterWrapper, getClanLevel, "���������� �������� �������: ����� �� 0 �� 8" )
{
    checkTarget();
    CHK_NPC
    return Register( target->getPC()->getClanLevel() );
}
NMI_INVOKE( CharacterWrapper, getRace, "���������� ������ � ��������� ����" )
{
    checkTarget();
    return Register( target->getRace( )->getName( ) );
}

NMI_INVOKE( CharacterWrapper, extract, "" )
{
    checkTarget( );
    RegisterList::const_iterator i = args.begin( );

    if(i == args.end())
	throw Scripting::NotEnoughArgumentsException( );
    
    extract_char(target, i->toNumber());
    return Register();
}

NMI_INVOKE( CharacterWrapper, add_follower, "��������: master, ������ ����� ���� �������������� master-�." )
{
    checkTarget( );
    target->add_follower( arg2character( get_unique_arg( args ) ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, stop_follower, "���������� ����������, ������� � ���� ����������." )
{
    checkTarget( );
    target->stop_follower();
    return Register();
}

NMI_INVOKE( CharacterWrapper, clearBehavior, "" )
{
    checkTarget( );
    CHK_PC
    MobileBehaviorManager::assignBasic( target->getNPC( ) );
    return Register();
}


NMI_INVOKE( CharacterWrapper, get_random_room, "��������� ����, ���� ����� ������� ���� ���" )
{
    checkTarget( );
    
    std::vector<Room *> rooms;
    Room *r;
    
    for (r = room_list; r; r = r->rnext)
	if (target->canEnter(r) && !r->isPrivate())
	    rooms.push_back(r);
    
    if (rooms.empty())
	return Register( );
    else {
	r = rooms[::number_range(0, rooms.size() - 1)];
	return WrapperManager::getThis( )->getWrapper(r); 
    }
}

NMI_INVOKE( CharacterWrapper, is_safe, "" )
{
    checkTarget( );
    return ::is_safe_nomessage( target, 
                                arg2character( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, rawdamage, "���������: victim, ������ �����������, ��� ����������� �� damage_table" )
{
    RegisterList::const_iterator i;
    Character *victim;
    int dam;
    int dam_type = DAM_NONE;

    checkTarget( );

    if (args.size() < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    victim = arg2character( *i );
    dam = (++i)->toNumber( );

    if (args.size() > 2) {
        DLString d = (++i)->toString();
	dam_type = damage_table.value( d.c_str(), true );
	if (dam_type == NO_FLAG)
	    throw Scripting::CustomException( "Invalid damage type");
    }

    ::rawdamage(target, victim, dam_type, dam, true);

    return Register( );
}

NMI_INVOKE( CharacterWrapper, damage, "���������: victim, ������ �����������, �������� ����� ���������� �����������, ��� ����������� �� damage_table" )
{
    RegisterList::const_iterator i;
    Character *victim;
    int dam;
    int dam_type = DAM_NONE;
    Skill *skill; 
    DLString skillName;

    checkTarget( );

    if (args.size() < 3)
       throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    victim = arg2character( *i );
    dam = (++i)->toNumber( );

    skillName = (++i)->toString( );
    skill = skillManager->findExisting( skillName );
    if (!skill)
	throw Scripting::CustomException( skillName + ": invalid skill name");
    
    if (args.size() > 2) {
        DLString d = (++i)->toString();
	dam_type = damage_table.value( d.c_str(), true );
	if (dam_type == NO_FLAG)
	    throw Scripting::CustomException( "Invalid damage type");
    }

    ::damage(target, victim, dam, skill->getIndex( ), dam_type, true);

    return Register( );
}


NMI_INVOKE( CharacterWrapper, setSkillLearned, "" )
{
    Skill *skill;
    int value;
    
    checkTarget( );
    CHK_NPC

    if (args.size( ) < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    DLString d = args.front().toString();
    skill = SkillManager::getThis( )->findExisting( d.c_str( ) );
    value = args.back( ).toNumber( );
    
    if (!skill || value < 0)
	throw Scripting::IllegalArgumentException( );
    
    target->getPC( )->getSkillData( skill->getIndex( ) ).learned = value;
    return Register( );
}

NMI_INVOKE( CharacterWrapper, getSkill, "������ ������� �������� ����� � ������ ������" )
{
    Skill *skill;
    
    checkTarget( );

    if (args.size() < 1)
	throw Scripting::NotEnoughArgumentsException( );
	
    DLString d = args.front().toString();
    skill = SkillManager::getThis( )->findExisting( d.c_str( ) );

    if (!skill)
	throw Scripting::IllegalArgumentException( );

    return Register( skill->getEffective( target ) );
}

NMI_INVOKE( CharacterWrapper, improveSkill, "���������� �������� ������ ���������� �����, �� ������/������� (true/false). �������������� �������� - ������" )
{
    RegisterList::const_iterator i;
    Skill *skill;
    Character *victim = NULL;
    int success, weight;
    
    checkTarget( );

    if (args.size() < 3)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    DLString d = i->toString();
    skill = SkillManager::getThis( )->findExisting( d.c_str( ) );
    i++;
    success = i->toNumber( );
    i++;
    weight = i->toNumber( );
    i++;

    if (i != args.end( ))
	victim = arg2character( *i );

    if (!skill || weight <= 0)
	throw Scripting::IllegalArgumentException( );
    
    skill->improve( target, success, victim );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, spell, "���������� ���������� ( ��������, �������, ������, ����������? )")
{
    RegisterList::const_iterator i;
    Skill *skill;
    Character *victim;
    int level;
    bool fBane;
    
    checkTarget( );

    if (args.size() < 4)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    DLString d = i->toString();
    skill = SkillManager::getThis( )->findExisting( d.c_str( ) );
    
    i++;
    level = i->toNumber( );

    i++;
    victim = arg2character( *i );

    i++;
    fBane = i->toNumber( );

    if (!skill || !victim)
	throw Scripting::IllegalArgumentException( );
    
    spell( skill->getIndex( ), level, target, victim, fBane );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, multi_hit, "" )
{
    checkTarget( );
    ::multi_hit( target, arg2character( get_unique_arg( args ) ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, raw_kill, "�����. �������������� ���������: ����� ����� ���� (-1 ���������) � ������" )
{
    RegisterList::const_iterator i;
    Character *killer = NULL;
    int part = -1;

    checkTarget();
    
    i = args.begin( );

    if (i != args.end( )) {
	part = i->toNumber( );

	if (++i != args.end( ))
	    killer = arg2character( *i );
    }
    
    raw_kill( target, part, killer, FKILL_CRY|FKILL_GHOST|FKILL_CORPSE );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectAdd, "" )
{
    checkTarget( );
    AffectWrapper *aw;
    Affect af;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    aw->toAffect( af );
    affect_to_char( target, &af );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectJoin, "" )
{
    checkTarget( );
    AffectWrapper *aw;
    Affect af;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    aw->toAffect( af );
    affect_join( target, &af );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectBitStrip, "����� � ���� ��� �������, ��������������� ����� ���. ������ �������� - �������� �� ������� affwhere_flags, ������ - ���")
{
    int where, bits;
    
    checkTarget( );

    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    where = args.front( ).toNumber( );
    bits = args.back( ).toNumber( );
    affect_bit_strip( target, where, bits );
    return Register( ); 
}

NMI_INVOKE( CharacterWrapper, isAffected, "��������� �� ��� ��� ������������ ������� � ������ ������" )
{
    Skill *skill;
    
    checkTarget( );

    if (args.size( ) != 1)
        throw Scripting::NotEnoughArgumentsException( );

    skill = skillManager->findExisting( args.front( ).toString( ) );

    if (skill)
	return target->isAffected( skill->getIndex( ) );
    else
	return false;
}

NMI_INVOKE( CharacterWrapper, affectStrip, "" )
{
    checkTarget( );
    Skill *skill;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );

    skill = skillManager->findExisting( args.front( ).toString( ) );
    
    if (!skill)
	throw Scripting::IllegalArgumentException( );
    
    affect_strip( target, skill->getIndex( ) );
    return Register( );
}


NMI_INVOKE( CharacterWrapper, stop_fighting, "" )
{
    checkTarget( );
    stop_fighting(target, get_unique_arg( args ).toBoolean( ));
    return Register( );
}

NMI_INVOKE( CharacterWrapper, move_char, "���������� ����, ������ �������� - ����� �����, ������ �������������� �������� - ��� ����������� ('running', 'normal', 'crawl' � ��). ������ true ���� ����������� �����������")
{
    int door, rc;
    DLString movetypeName;
    
    checkTarget( );

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    if (args.size( ) > 2)
	throw Scripting::TooManyArgumentsException( );
    
    door = args.front( ).toNumber( );
    if (door < 0 || door >= DIR_SOMEWHERE)
	return false;
    
    if (args.size( ) > 1)
	movetypeName = args.back( ).toString( );
    else 
	movetypeName = "normal";

    rc = ::move_char( target, door, movetypeName.c_str( ) );
    return Register( rc == RC_MOVE_OK );
}

NMI_INVOKE( CharacterWrapper, addDarkShroud, "")
{
    Affect af;
    
    checkTarget( );

    af.where     = TO_AFFECTS;
    af.type      = gsn_dark_shroud;
    af.level     = target->getRealLevel( );
    af.duration  = -1;
    affect_to_char( target, &af );

    return Register( );
}    

NMI_INVOKE( CharacterWrapper, isLawProtected, "���������� �� ��� �������" )
{
    NPCharacter *mob;
    
    checkTarget();
    CHK_PC
    mob = target->getNPC( );

    if (IS_SET(mob->pIndexData->area->area_flag, AREA_HOMETOWN))
	return true;

    return false;
}

NMI_INVOKE( CharacterWrapper, can_get_obj, "����� �� ������� �������" )
{
    checkTarget( );

    ::Object *obj = arg2item( get_unique_arg( args ) );

    if (!obj->can_wear( ITEM_TAKE )) 
	return false;
    if (obj->getOwner( ))
	return false;
    if (obj->behavior)
	return false;
    if (!target->can_see( obj ))
	return false;
    if (obj->isAntiAligned( target ))
	return false;

    return true;
}



NMI_INVOKE(CharacterWrapper, get_obj_carry_vnum, "����� ������� � inv ��� eq �� ��� �����" )
{
    checkTarget( );

    int vnum = args2number( args );

    for (::Object *obj = target->carrying; obj; obj = obj->next_content)
	if (obj->pIndexData->vnum == vnum)
	    return wrap( obj );

    return Register( );
}

NMI_INVOKE(CharacterWrapper, can_drop_obj, "����� �� ��� ���������� �� �������� � ���������" )
{
    checkTarget( );
    ::Object *obj = arg2item( get_unique_arg( args ) );
    return ::can_drop_obj(target, obj, false);
}

NMI_INVOKE( CharacterWrapper, mortality, "" )
{
    checkTarget( );

    if (target->is_npc( ) || !target->getPC( )->getAttributes( ).isAvailable( "mortality" ))
	throw Scripting::Exception( "Attribute not found" );
    
    if (target->getPC( )->getAttributes( ).isAvailable( "coder" )) {
	target->getPC( )->getAttributes( ).eraseAttribute( "coder" );
	target->getPC( )->setSecurity( 0 );
	target->println("Now you are mortal.");
	return 1;
    }
    else {
	target->getPC( )->getAttributes( ).getAttr<XMLAttributeCoder>( "coder" );
	target->getPC( )->setSecurity( 999 );
	target->println("Now you are immortal.");
	return 0;
    }
}

NMI_INVOKE( CharacterWrapper, echoOn, "" )
{
    checkTarget( );
    
    if (target->desc)
	target->desc->echoOn( );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, echoOff, "" )
{
    checkTarget( );
    
    if (target->desc)
	target->desc->echoOff( );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, save, "" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->save( );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, updateSkills, "" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->updateSkills( );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, hasAttribute, "" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getAttributes( ).isAvailable( args2string( args ) );
}

NMI_INVOKE( CharacterWrapper, eraseAttribute, "" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->getAttributes( ).eraseAttribute( args2string( args ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, canRecall, "" )
{
    checkTarget( );

    if (IS_SET(target->in_room->room_flags, ROOM_NO_RECALL))
	return false; 
    if (IS_RAFFECTED(target->in_room, AFF_ROOM_CURSE))
	return false;
    if (IS_AFFECTED(target, AFF_CURSE))
	return false;
    if (target->isAffected(gsn_manacles))
	return false;
    if (target->position <= POS_SLEEPING || target->fighting)
	return false;

    return true;
}

NMI_INVOKE( CharacterWrapper, get_eq_char, "" )
{
    checkTarget( );
    return wrap( arg2wearloc( get_unique_arg( args ) )->find( target ) );
}

NMI_INVOKE( CharacterWrapper, hasWearloc, "")
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getWearloc( ).isSet( 
                 arg2wearloc( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, add_charmed, "" )
{
    Character *victim;
    int duration;
    Affect af;
    RegisterList::const_iterator i;
    
    checkTarget( );
    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );

    i = args.begin( );
    victim = arg2character( *i++ );
    duration = (i == args.end( ) ? -1 : i->toNumber( ));

    if (victim->master)
	victim->stop_follower( );

    victim->add_follower( target );
    victim->leader = target;

    af.where     = TO_AFFECTS;
    af.type      = gsn_charm_person;
    af.level     = target->getRealLevel( );
    af.duration  = duration;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, add_pet, "" )
{
    Character *pet;

    checkTarget( );
    CHK_NPC

    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );
    
    pet = arg2character( args.front( ) );
    if (!pet->is_npc( ))
	throw Scripting::Exception( "NPC field requested on PC" ); 
    
    if (pet->getNPC( )->behavior) {
	Pet::Pointer bhv = pet->getNPC( )->behavior.getDynamicPointer<Pet>( );
	if (bhv)
	    bhv->config( target->getPC( ), pet->getNPC( ) );
    }

    SET_BIT( pet->affected_by, AFF_CHARM );
    pet->add_follower( target );
    pet->leader = target;
    target->getPC( )->pet = pet->getNPC( );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, look_auto, "" )
{
    checkTarget( );
    do_look_auto( target, arg2room( get_unique_arg( args ) ) );
    return Register( );
}

NMI_GET( CharacterWrapper, affected, "" )
{
    checkTarget();
    RegList::Pointer rc(NEW);
    Affect *paf;

    for (paf = target->affected; paf != 0; paf = paf->next) 
	rc->push_back( AffectWrapper::wrap( *paf ) );
	
    Scripting::Object *sobj = &Scripting::Object::manager->allocate();
    sobj->setHandler(rc);

    return Register( sobj );
}

NMI_GET( CharacterWrapper, hasDestiny, "" )
{
    checkTarget( );
    CHK_PC
    
    if (target->getNPC( )->behavior)
	return target->getNPC( )->behavior->hasDestiny( );
    else
	return Register( false );
}

NMI_GET( CharacterWrapper, clan, "" )
{
    checkTarget( );
    CHK_NPC
    return ClanWrapper::wrap( target->getPC( )->getClan( )->getName( ) );
}

NMI_INVOKE( CharacterWrapper, hasOccupation, "" )
{
    checkTarget( );
    CHK_PC

    DLString occName = args2string( args );
    return mob_has_occupation( target->getNPC( ), occName.c_str( ) );
}

NMI_INVOKE( CharacterWrapper, switchTo, "" )
{
    Character *victim;
	
    checkTarget();
    CHK_NPC

    victim = arg2character( args.front( ) );
    if (!victim->is_npc( ))
	throw Scripting::Exception( "Impossible to switch to PC" ); 
    
    if (target->desc == 0)
	throw Scripting::Exception( "Zero descriptor for switch" ); 

    if (target->getPC( )->switchedTo) 
	throw Scripting::Exception( "Character already switched" ); 

    if (victim->desc != 0)
	throw Scripting::Exception( "Switch victim is already in use" ); 

    wiznet( WIZ_SWITCHES, WIZ_SECURE, target->get_trust( ), "%C1 switches into %C4.", target, victim );

    victim->getNPC( )->switchedFrom = target->getPC( );
    target->getPC( )->switchedTo = victim->getNPC( );
    
    target->desc->associate( victim );
    target->desc = 0;

    return Register( );
}


NMI_INVOKE( CharacterWrapper, switchFrom, "" )
{
    checkTarget( );
    CHK_PC
    
    if (target->desc == 0)
	throw Scripting::Exception( "Switched mobile has no descriptor" ); 
    
    if (!target->getNPC( )->switchedFrom) 
	throw Scripting::Exception( "Try to return from non-switched mobile" );

    wiznet( WIZ_SWITCHES, WIZ_SECURE, target->get_trust( ), "%C1 returns from %C2.", target->getNPC( )->switchedFrom, target );
    
    target->desc->associate( target->getNPC( )->switchedFrom );
    target->getNPC( )->switchedFrom->switchedTo = 0;
    target->getNPC( )->switchedFrom = 0;
    target->desc = 0;

    return Register( );
}

NMI_INVOKE( CharacterWrapper, setDead, "" )
{
    checkTarget( );
    CHK_PC
    target->setDead( );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, isDead, "" )
{
    checkTarget( );
    CHK_PC
    return target->isDead( );
}

NMI_INVOKE( CharacterWrapper, writeWSCommand, "" )
{
    checkTarget( );
    CHK_NPC

    Json::Value val;
    RegisterList::const_iterator i = args.begin( );

    val["command"] = (i++)->toString();

    for(int j=0;i != args.end();i++, j++) {
        switch(i->type) {
            case Register::NONE:
                val["args"][j] = Json::Value::null;
                break;
            case Register::NUMBER:
                val["args"][j] = i->toNumber();
                break;
            case Register::STRING:
                val["args"][j] = i->toString();
                break;
            default:
                throw Scripting::Exception( "Unsupported type exception" );
        }
    }

    return target->desc->writeWSCommand(val);
}

NMI_INVOKE( CharacterWrapper, eat, "(item) ��������� ������� ���, ����� item ��� ������" )
{
    checkTarget( );
    ::Object *obj = arg2item( args.front( ) );

    if (obj->item_type == ITEM_FOOD) {
	desire_hunger->eat( target->getPC( ), obj->value[0] * 2 );
	desire_full->eat( target->getPC( ), obj->value[1] * 2 );
    }

    return Register( );
}

NMI_INVOKE( CharacterWrapper, drink, "(item, amount) ��������� ������� ���, ����� �� item ���������� amount �������" )
{
    checkTarget( );
    ::Object *obj;
    int amount;

    if (args.size( ) != 2)
	throw Scripting::NotEnoughArgumentsException( );

    obj = arg2item( args.front( ) );
    amount = args.back( ).toNumber( );

    if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOUNTAIN) {
	Liquid *liq = liquidManager->find( obj->value[2] );

	desire_full->drink( target->getPC( ), amount, liq );
	desire_thirst->drink( target->getPC( ), amount, liq );
	desire_drunk->drink( target->getPC( ), amount, liq );
    }

    return Register( );
}

NMI_INVOKE(CharacterWrapper, restring, "(skill, key, names, short, long) ���������� �������� ��� ��������� ����������� ����������")
{
    checkTarget( );
    CHK_NPC
    if (args.size( ) != 5)
	throw Scripting::NotEnoughArgumentsException( );

    RegisterList::const_iterator i = args.begin( );
    DLString skillName = (i++)->toString();
    DLString key = (i++)->toString();
    DLString objName = (i++)->toString();
    DLString objShort = (i++)->toString();
    DLString objLong = (i)->toString();

    Skill *skill = skillManager->findExisting( skillName );
    if (!skill)
	throw Scripting::Exception( "Skill name not found" );

    XMLAttributeRestring::Pointer attr = target->getPC( )->getAttributes( ).getAttr<XMLAttributeRestring>( skillName );
    XMLAttributeRestring::iterator r = attr->find( key );
    if (r != attr->end( )) {
	r->second.name = objName;
        r->second.shortDescr = objShort;
	r->second.longDescr = objLong;
    } else {
	(**attr)[key].name = objName;
	(**attr)[key].shortDescr = objShort;
	(**attr)[key].longDescr = objLong;
    }

    target->getPC( )->save( );
    return Register( );
}
 
NMI_INVOKE( CharacterWrapper, api, "�������� ���� API" )
{
    ostringstream buf;
    Scripting::traitsAPI<CharacterWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( CharacterWrapper, rtapi, "�������� ��� ���� � ������, ������������� � runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( CharacterWrapper, clear, "������� ���� runtime �����" )
{
    guts.clear( );
    self->changed();
    return Register( );
}

