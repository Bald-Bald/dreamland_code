/* $Id: mobindexwrapper.cpp,v 1.1.2.15.6.10 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "npcharacter.h"
#include "room.h"
#include "merc.h"
#include "mercdb.h"
#include "loadsave.h"

#include "mobindexwrapper.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "nativeext.h"
#include "register-impl.h"

#include "def.h"


using Scripting::NativeTraits;

NMI_INIT(MobIndexWrapper, "�������� ��� ����� (mob index data)")

MobIndexWrapper::MobIndexWrapper( ) : target( NULL )
{
}

void 
MobIndexWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "MobIndex wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void MobIndexWrapper::setSelf( Scripting::Object *s )
{
#if 0
    if(s)
	LogStream::sendNotice() << "set self" << endl;
    else
	LogStream::sendNotice() << "unset self" << endl;
#endif

    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void
MobIndexWrapper::setTarget( mob_index_data * pIndex )
{
    target = pIndex;
    id = MOB_VNUM2ID(pIndex->vnum);
}

void 
MobIndexWrapper::checkTarget( ) const throw( Scripting::Exception )
{
    if (zombie.getValue())
	throw Scripting::Exception( "MOB_INDEX_DATA is dead" );

    if (!target)
	throw Scripting::Exception( "MOB_INDEX_DATA is offline?!");
}

MOB_INDEX_DATA *
MobIndexWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

NMI_GET( MobIndexWrapper, name, "") 
{ 
    checkTarget( ); 
    return target->player_name;
}
NMI_GET( MobIndexWrapper, short_descr, "") 
{ 
    checkTarget( ); 
    return target->short_descr;
}
NMI_GET( MobIndexWrapper, long_descr, "") 
{ 
    checkTarget( ); 
    return target->long_descr;
}
NMI_GET( MobIndexWrapper, description, "") 
{ 
    checkTarget( ); 
    return target->description;
}
NMI_GET( MobIndexWrapper, count, "") 
{ 
    checkTarget( ); 
    return target->count;
}
NMI_GET( MobIndexWrapper, vnum , "") 
{ 
    checkTarget( ); 
    return target->vnum;
}
NMI_GET( MobIndexWrapper, imm_flags , "") 
{ 
    checkTarget( ); 
    return (int)target->imm_flags;
}
NMI_GET( MobIndexWrapper, group, "") 
{ 
    checkTarget( ); 
    return target->group;
}
NMI_SET( MobIndexWrapper, group, "") 
{ 
    checkTarget( ); 
    target->group = arg.toNumber( );
}

NMI_GET( MobIndexWrapper, spec_fun, "") 
{
    checkTarget( ); 
    if (target->spec_fun.func)
	return Register( spec_name(target->spec_fun.func) );
    else
	return Register( );
}

NMI_GET( MobIndexWrapper, practicer, "") 
{
    checkTarget( );
    return target->practicer.toString( );
}

NMI_GET( MobIndexWrapper, repopPlaces, "������ ������ ������, � ������� ��������� ���") 
{
    Room *room;
    RESET_DATA *pReset;
    RegList::Pointer rc(NEW);
    
    checkTarget( );
    
    for (room = room_list; room; room = room->rnext)
	for (pReset = room->reset_first; pReset; pReset = pReset->next)
	    if (pReset->command == 'M' && pReset->arg1 == target->vnum)
		rc->push_back( Register( room->vnum ) );

    Scripting::Object *obj = &Scripting::Object::manager->allocate( );
    obj->setHandler( rc );

    return Register( obj );
}    

NMI_GET( MobIndexWrapper, instances, "������ ���� ����������� ����� � ���� pIndexData" )
{
    checkTarget();
    RegList::Pointer rc(NEW);
    Character *ch;

    for (ch = char_list; ch; ch = ch->next)
	if (ch->is_npc( ) && ch->getNPC( )->pIndexData == target)
	    rc->push_back( WrapperManager::getThis( )->getWrapper( ch ) );

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_INVOKE(MobIndexWrapper, create, "")
{
    NPCharacter *mob;

    checkTarget( );
    mob = create_mobile( target );
    char_to_room( mob, get_room_index( ROOM_VNUM_FENIA_STORAGE ) );
    return WrapperManager::getThis( )->getWrapper( mob ); 
}


NMI_INVOKE( MobIndexWrapper, api, "�������� ���� API" )
{
    ostringstream buf;
    Scripting::traitsAPI<MobIndexWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( MobIndexWrapper, rtapi, "�������� ��� ���� � ������, ������������� � runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( MobIndexWrapper, clear, "������� ���� runtime �����" )
{
    guts.clear( );
    self->changed();
    return Register( );
}
