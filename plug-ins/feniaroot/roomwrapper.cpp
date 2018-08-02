/* $Id: roomwrapper.cpp,v 1.1.4.21.6.22 2014-09-19 11:39:39 rufina Exp $
 *
 * ruffina, 2004
 */

#include <sys/time.h>
#include <iostream>

#include "logstream.h"
#include "skillmanager.h"
#include "affect.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "save.h"                                                               
#include "merc.h"
#include "profiler.h"

#include "structwrappers.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "wrap_utils.h"
#include "nativeext.h"
#include "subr.h"

#include "roomtraverse.h"
#include "def.h"

using namespace std;
using namespace Scripting;

NMI_INIT(RoomWrapper, "�������")

RoomWrapper::RoomWrapper( ) : target( NULL )
{
}

void RoomWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void RoomWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "Room wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void RoomWrapper::setTarget( ::Room *r )
{
    target = r;
    id = ROOM_VNUM2ID(r->vnum);
}

void RoomWrapper::checkTarget( ) const throw( Scripting::Exception )
{
    if (zombie.getValue())
	throw Scripting::Exception( "Room is dead" );
	
    if (target == NULL)
	throw Scripting::Exception( "Room is offline" );
}

Room * RoomWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}


#define GETWRAP(x) NMI_GET(RoomWrapper, x, "") { \
    checkTarget(); \
    return WrapperManager::getThis( )->getWrapper(target->x); \
}

GETWRAP( next )
GETWRAP( rnext )
GETWRAP( contents )
GETWRAP( people )

NMI_GET( RoomWrapper, vnum , "")
{
    checkTarget( );
    return Register( target->vnum );
}

NMI_GET( RoomWrapper, name , "")
{
    checkTarget( );
    return Register( target->name );
}

NMI_GET( RoomWrapper, areaname , "��� ����")
{
    checkTarget( );
    return Register( target->area->name );
}

NMI_GET( RoomWrapper, area, "��������� Area ��� ���� �������")
{
    checkTarget( );
    return AreaWrapper::wrap( target->area->area_file->file_name );
}

NMI_GET(RoomWrapper, ppl, "������ (List) ���� ����� � �������")
{
    checkTarget();
    RegList::Pointer rc(NEW);

    Character *rch;
    
    for(rch = target->people; rch; rch = rch->next_in_room)
	rc->push_back( WrapperManager::getThis( )->getWrapper( rch ) );
    
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_GET( RoomWrapper, items, "������ (List) ���� ��������� �� ����" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (::Object *obj = target->contents; obj; obj = obj->next_content)
	rc->push_back( WrapperManager::getThis( )->getWrapper( obj ) );
    
    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( rc );

    return Register( sobj );
}

NMI_GET( RoomWrapper, sector_type , "")
{
    checkTarget( );
    return Register( target->sector_type );
}

NMI_GET( RoomWrapper, affected_by, "" )
{
    checkTarget( );
    return (int)target->affected_by;
}

NMI_SET( RoomWrapper, affected_by, "" )
{
    checkTarget( );
    target->affected_by = arg.toNumber();
}

NMI_GET( RoomWrapper, room_flags, "" )
{
    checkTarget( );
    return (int)target->room_flags;
}

NMI_SET( RoomWrapper, room_flags, "" )
{
    checkTarget( );
    target->room_flags = arg.toNumber();
}

NMI_GET( RoomWrapper, light, "" )
{
    checkTarget( );
    return (int)target->light;
}

NMI_GET( RoomWrapper, description, "" )
{
    checkTarget( );
    return Register( target->description );
}

NMI_SET( RoomWrapper, light, "" )
{
    checkTarget( );
    target->light = arg.toNumber();
}

NMI_GET( RoomWrapper, clan, "������ ����� ����������� �������" )
{
    checkTarget();
    return Register( target->clan->getShortName( ) );
}

static Scripting::Register get_direction( Room *r, int dir )
{
    if (r->exit[dir])
	return WrapperManager::getThis( )->getWrapper(r->exit[dir]->u1.to_room);
    else 
	return Scripting::Register( );
}

static int get_door_argument( const RegisterList &args )
{
    int door;
    DLString doorName;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
    
    doorName = args.front( ).toString( );
    if (( door = direction_lookup( doorName.c_str( ) ) ) == -1)
	door = args.front( ).toNumber( );

    if (door < 0 || door >= DIR_SOMEWHERE)
	throw Scripting::IllegalArgumentException( );

    return door;
}

NMI_GET( RoomWrapper, north, "")
{
    checkTarget( );
    return get_direction( target, DIR_NORTH );
}
NMI_GET( RoomWrapper, south, "")
{
    checkTarget( );
    return get_direction( target, DIR_SOUTH );
}
NMI_GET( RoomWrapper, east, "")
{
    checkTarget( );
    return get_direction( target, DIR_EAST );
}
NMI_GET( RoomWrapper, west, "")
{
    checkTarget( );
    return get_direction( target, DIR_WEST );
}
NMI_GET( RoomWrapper, up, "")
{
    checkTarget( );
    return get_direction( target, DIR_UP );
}
NMI_GET( RoomWrapper, down, "")
{
    checkTarget( );
    return get_direction( target, DIR_DOWN );
}

/*
 * METHODS
 */

NMI_INVOKE(RoomWrapper, doorTo, "������ ����� �����, ������� �� ���� ������� � ���������" )
{
    Room *room;
    
    checkTarget( );
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );

    room = wrapper_cast<RoomWrapper>( args.front( ) )->getTarget( );

    for (int d = 0; d < DIR_SOMEWHERE; d++)
	if (target->exit[d] && target->exit[d]->u1.to_room == room)
	    return d;

    return -1;
}

NMI_INVOKE( RoomWrapper, getRoom, "" )
{
    int door;
    
    checkTarget( );
    door = get_door_argument( args );
    return get_direction( target, door );
}

NMI_INVOKE( RoomWrapper, getRevDoor, "" )
{
    checkTarget( );
    return dirs[get_door_argument( args )].rev;
}

NMI_INVOKE( RoomWrapper, doorNumber, "" )
{
    checkTarget( );
    return get_door_argument( args );
}

NMI_INVOKE( RoomWrapper, dirMsgLeave, "" )
{
    checkTarget( );
    return dirs[get_door_argument( args )].leave;
}

NMI_INVOKE( RoomWrapper, dirMsgEnter, "" )
{
    checkTarget( );
    return dirs[get_door_argument( args )].enter;
}

NMI_INVOKE( RoomWrapper, getExitFlags, "" )
{
    EXIT_DATA *pExit;
    
    checkTarget( );

    pExit = target->exit[get_door_argument( args )];
    if (pExit)
	return Register( pExit->exit_info );
    else
	return Register( 0 );
}

static void update_door_flags( Room *room, const RegisterList &args, int flags, bool fSet )
{
    EXIT_DATA *pExit = room->exit[get_door_argument( args )];

    if (pExit) {
	if (fSet)
	    SET_BIT(pExit->exit_info, flags );
	else
	    REMOVE_BIT(pExit->exit_info, flags );
    }
}

NMI_INVOKE(RoomWrapper, close, "������� ����� �� ���������� ����������� (0..5)")
{
    checkTarget( );
    update_door_flags( target, args, EX_CLOSED, true ); 
    return Register( ); 
}

NMI_INVOKE(RoomWrapper, open, "������� ����� �� ���������� ����������� (0..5)")
{
    checkTarget( );
    update_door_flags( target, args, EX_CLOSED|EX_LOCKED, false ); 
    return Register( ); 
}

NMI_INVOKE(RoomWrapper, lock, "�������� ����� �� ���������� ����������� (0..5)")
{
    checkTarget( );
    update_door_flags( target, args, EX_CLOSED|EX_LOCKED, true ); 
    return Register( ); 
}

NMI_INVOKE(RoomWrapper, unlock, "�������� ����� �� ���������� ����������� (0..5)")
{
    checkTarget( );
    update_door_flags( target, args, EX_LOCKED, false ); 
    return Register( ); 
}


NMI_INVOKE(RoomWrapper, isDark, "" )
{
    checkTarget( );
    return Register( target->isDark( ) );
}

NMI_INVOKE(RoomWrapper, isCommon, "" )
{
    checkTarget( );
    return Register( target->isCommon( ) );
}

NMI_INVOKE(RoomWrapper, zecho, "��������� ��� ���� � ���� ����" )
{
    Character *wch;
    const char *msg;
    
    checkTarget( );

    if (args.size( ) != 1)
	throw Scripting::NotEnoughArgumentsException( );

    msg = args.front( ).toString( ).c_str( );
    
    for (wch = char_list; wch; wch = wch->next) 
	if (wch->in_room->area == target->area) 
	    wch->println( msg );

    return Register( );
}

NMI_INVOKE(RoomWrapper, get_obj_vnum, "����� ������� � ������� �� ��� �����" )
{
    int vnum;
    ::Object *obj;

    checkTarget( );

    if (args.size( ) != 1)
	throw Scripting::NotEnoughArgumentsException( );
    
    vnum = args.front( ).toNumber( );

    for (obj = target->contents; obj; obj = obj->next_content)
	if (obj->pIndexData->vnum == vnum)
	    return WrapperManager::getThis( )->getWrapper(obj); 

    return Register( );
}

NMI_INVOKE( RoomWrapper, list_obj_vnum, "����� ������ �������� � ������� �� �����" )
{
    checkTarget( );
    RegList::Pointer rc(NEW);

    int vnum = args2number( args );

    for (::Object *obj = target->contents; obj; obj = obj->next_content)
	if (obj->pIndexData->vnum == vnum)
	    rc->push_back( WrapperManager::getThis( )->getWrapper( obj ) );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( rc );

    return Register( sobj );
}

NMI_INVOKE(RoomWrapper, get_mob_vnum, "����� ���� � ������� �� ��� �����" )
{
    int vnum;
    Character *rch;

    checkTarget( );

    if (args.size( ) != 1)
	throw Scripting::NotEnoughArgumentsException( );
    
    vnum = args.front( ).toNumber( );

    for (rch = target->people; rch; rch = rch->next_in_room)
	if (rch->is_npc( ) && rch->getNPC( )->pIndexData->vnum == vnum)
	    return WrapperManager::getThis( )->getWrapper( rch ); 

    return Register( );
}

NMI_INVOKE( RoomWrapper, list_mob_vnum, "����� ������ ����� � ������� �� �����" )
{
    checkTarget( );
    RegList::Pointer rc(NEW);

    int vnum = args2number( args );

    for (Character *rch = target->people; rch; rch = rch->next_in_room)
	if (rch->is_npc( ) && rch->getNPC( )->pIndexData->vnum == vnum)
	    rc->push_back( WrapperManager::getThis( )->getWrapper( rch ) );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( rc );

    return Register( sobj );
}

/*---------------------------------------------------------
 * fenia traverse
 *--------------------------------------------------------*/
struct FeniaDoorFunc {
    FeniaDoorFunc( Character *w = 0, bitstring_t sa = 0, bitstring_t sd = 0 )
                  : walker( w ), sectorsAllow( sa ), sectorsDeny( sd )
    {
    }
    bool operator () ( Room *const room, EXIT_DATA *exit ) const
    {
	if (IS_SET(exit->exit_info, EX_LOCKED))
	    return false;
	
	Room *toRoom = exit->u1.to_room;
	bitstring_t mysector = (1 << toRoom->sector_type);

	if (sectorsAllow != 0 && !IS_SET(sectorsAllow, mysector))
	    return false;

	if (sectorsDeny != 0 && IS_SET(sectorsDeny, mysector))
	    return false;

	if (!toRoom->isCommon( ))
	    return false;
	    
	if (walker && !walker->canEnter( toRoom ))
	    return false;
	
	return true;
    }
     
    Character *walker;
    bitstring_t sectorsAllow, sectorsDeny;
};

struct FeniaExtraExitFunc {
    bool operator () ( Room *const room, EXTRA_EXIT_DATA *eexit ) const
    {
	return false;
    }
};

struct FeniaPortalFunc {
    bool operator () ( Room *const room, ::Object *portal ) const
    {
	return false;
    }
};

typedef RoomRoadsIterator<FeniaDoorFunc, FeniaExtraExitFunc, FeniaPortalFunc> 
                      FeniaHookIterator;

struct PathWithDepthComplete {
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    PathWithDepthComplete( int d, RegList::Pointer r ) : depth( d ), rooms( r ) 
    { 
    }

    inline bool operator () ( const MyNodesEntry *const head, bool last ) 
    {
	if (head->generation < depth && !last)
	    return false;

	for (const MyNodesEntry *i = head; i->prev; i = i->prev) 
	    rooms->push_front( WrapperManager::getThis( )->getWrapper( i->node ) );

	return true;
    }

    int depth;
    RegList::Pointer rooms;
};

struct PathToTargetComplete {
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    
    PathToTargetComplete( Room *t, RegList::Pointer r ) : target( t ), rooms( r ) 
    { 
    }

    inline bool operator () ( const MyNodesEntry *const head, bool last ) 
    {
	if (head->node != target)
	    return false;
	
	for (const MyNodesEntry *i = head; i->prev; i = i->prev) 
	    rooms->push_front( WrapperManager::getThis( )->getWrapper( i->node ) );

	return true;
    }
    
    Room *target;
    RegList::Pointer rooms;
};

NMI_INVOKE( RoomWrapper, traverse, "depth, walker, sectorsAllow, sectorsDeny" )
{
    bitstring_t sectorsAllow, sectorsDeny;
    int depth;
    Character *walker;
    Scripting::RegisterList::const_iterator i = args.begin( );

    checkTarget( );
    
    depth = (i == args.end( ) ? 4000 : (i++)->toNumber( ));
    walker = (i == args.end( ) || i->type == Register::NUMBER) ? 0 : wrapper_cast<CharacterWrapper>( *i++ )->getTarget( );
    sectorsAllow = (i == args.end( ) ? 0 : (i++)->toNumber( ));
    sectorsDeny= (i == args.end( ) ? 0 : (i++)->toNumber( ));
    
    FeniaDoorFunc df( walker, sectorsAllow, sectorsDeny );
    FeniaExtraExitFunc eef;
    FeniaPortalFunc pf;
    FeniaHookIterator iter( df, eef, pf, 5 );
    
    RegList::Pointer rooms( NEW );
    PathWithDepthComplete complete( depth, rooms );

    room_traverse( target, iter, complete, 10000 );

    Scripting::Object *obj = &Scripting::Object::manager->allocate( );
    obj->setHandler( rooms );

    return Scripting::Register( obj );
}

NMI_INVOKE( RoomWrapper, traverseTo, "target, walker, sectorsAllow, sectorsDeny" )
{
    bitstring_t sectorsAllow, sectorsDeny;
    Room *targetRoom;
    Character *walker;
    Scripting::RegisterList::const_iterator i = args.begin( );

    checkTarget( );
    
    targetRoom = (i == args.end( ) ? target : wrapper_cast<RoomWrapper>( *i++ )->getTarget( ));
    walker = (i == args.end( ) || i->type == Register::NUMBER) ? 0 : wrapper_cast<CharacterWrapper>( *i++ )->getTarget( );
    sectorsAllow = (i == args.end( ) ? 0 : (i++)->toNumber( ));
    sectorsDeny= (i == args.end( ) ? 0 : (i++)->toNumber( ));
    
    FeniaDoorFunc df( walker, sectorsAllow, sectorsDeny );
    FeniaExtraExitFunc eef;
    FeniaPortalFunc pf;
    FeniaHookIterator iter( df, eef, pf, 5 );
    
    RegList::Pointer rooms( NEW );
    PathToTargetComplete complete( targetRoom, rooms );

    room_traverse( target, iter, complete, 10000 );

    Scripting::Object *obj = &Scripting::Object::manager->allocate( );
    obj->setHandler( rooms );

    return Scripting::Register( obj );
}

NMI_GET( RoomWrapper, resetMobiles, "������ ������ �����, ������� ��������� � ���� �������") 
{
    RESET_DATA *pReset;
    RegList::Pointer rc(NEW);
    
    checkTarget( );
    
    for (pReset = target->reset_first; pReset; pReset = pReset->next)
	if (pReset->command == 'M')
	    rc->push_back( Register( pReset->arg1 ) );

    Scripting::Object *obj = &Scripting::Object::manager->allocate( );
    obj->setHandler( rc );

    return Register( obj );
}    

NMI_INVOKE( RoomWrapper, api, "�������� ���� API" )
{
    ostringstream buf;
    Scripting::traitsAPI<RoomWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( RoomWrapper, rtapi, "�������� ��� ���� � ������, ������������� � runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( RoomWrapper, clear, "������� ���� runtime �����" )
{
    guts.clear( );
    self->changed();
    return Register( );
}

