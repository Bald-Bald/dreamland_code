/* $Id$
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"
#include "repairman.h"
#include "attract.h"
#include "occupations.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "act.h"
#include "interp.h"
#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

/*
 * Repairman
 */
Repairman::Repairman( )
            : repairs( 0, &item_table )
{
}

int Repairman::getOccupation( )
{
    int occ = BasicMobileDestiny::getOccupation( );
    
    if (repairs.getValue( ) != 0)
	occ |= (1 << OCC_REPAIRMAN);

    return occ;
}

bool Repairman::specIdle( )
{
    if (BasicMobileDestiny::specIdle( ))
	return true;
	
    if (!IS_AWAKE( ch ))
	return false;

    if (repairs.getValue( ) == 0)
	return false;

    if (chance( 1 )) {
	interpret_raw(ch, "say", "������� ����� ��� ������� ����� ��������.");
	return true;
    }

    return false;
}

void Repairman::doRepair( Character *client, const DLString &cArgs )
{
    Object *obj;
    int cost;
    DLString args = cArgs, arg;
    
    arg = args.getOneArgument( );
    
    if (arg.empty( )) {
	say_act( client, ch, "� ������������ ���� ���-������ �� ������.");
	client->println("��������� repair <item>, ���� ������������ ������������ �������.");
	client->println("��������� estimate <item>, ����� ������, ������� ����� ������ �������.");
	return;
    }

    if (( obj = get_obj_carry(client, arg.c_str( ))) == 0) {
	say_act( client, ch, "� ���� ��� �����");
	return;
    }
    
    if (!canRepair( obj, client ))
	return;
    
    cost = getRepairCost( obj );

    if (cost > client->gold) {
	say_act( client, ch, "� ���� �� ������� �����, ���� �������� ��� ������.");
	return;
    }

    client->setWaitViolence( 1 );

    client->gold -= cost;
    ch->gold += cost;

    act("$C1 ����� $o4 y $c2, ��������������� � ���������� $c3.",client,obj,ch,TO_ROOM);
    act("$C1 ����� $o4, ��������������� � ���������� ����.",client,obj,ch,TO_CHAR);

    if (cost) 
	client->pecho( "���� ������� ���� ����� �� %1$d �����%1$I��|��|�� ����%1$I��|��|�.", cost );
    else
	client->println( "� ����� ��� ������ �������� ������� �������� ���� ���������." );

    obj->condition = 100;
}

void Repairman::doEstimate( Character *client, const DLString &cArgs )
{
    Object *obj;
    DLString args = cArgs, arg;
    
    arg = args.getOneArgument( );
    
    if (arg.empty( )) {
	say_act( client, ch, "�������� ������������ estimate <item>.");
   	return;
    }

    if ((obj = get_obj_carry(client, arg.c_str( ))) == 0) {
	say_act( client, ch, "� ���� ��� �����");
	return;
    }
    
    if (!canRepair( obj, client ))
	return;
    
    tell_fmt( "�������������� ����� ������ %3$d.", 
               client, ch, getRepairCost( obj ) );
}

int Repairman::getRepairCost( Object *obj )
{
    int cost;
    
    cost = obj->level * 10 + (obj->cost * (100 - obj->condition)) / 100;
    cost /= 100;
    return cost;
}

bool Repairman::canRepair( Object *obj, Character *client )
{
    if (!repairs.isSetBitNumber( obj->item_type )) {
	say_act( client, ch, 
	         "� �� ����� ��������������� $t.", 
		 item_table.message( obj->item_type, '4' ).c_str( ) );
	return false;
    }
    
    if (obj->pIndexData->vnum == OBJ_VNUM_HAMMER) {
	say_act( client, ch, "� �� ���������� ������ ������.");
	return false;
    }

    if (obj->condition >= 100) {
	say_fmt( "%2$^O1 �� �����%2$n����|���� � �������.", ch, obj );
        return false;
    }

    if (obj->cost == 0) {
	say_fmt( "%2$^O1 �� ������%2$n��|�� �������.", ch, obj );
   	return false;
    }

    return true;
}


/*
 * 'repair' command
 */
CMDRUN( repair )
{
    Repairman::Pointer man;
    
    if (!( man = find_attracted_mob_behavior<Repairman>( ch, OCC_REPAIRMAN ) )) {
        ch->println( "����� ��� ����������.");
	return;
    }

    if (!ch->is_npc( )) 
	man->doRepair( ch, constArguments );
}

/*
 * 'estimate' command
 */
CMDRUN( estimate )
{
    Repairman::Pointer man;
    
    if (!( man = find_attracted_mob_behavior<Repairman>( ch, OCC_REPAIRMAN ) )) {
        ch->println( "����� ��� ����������.");
	return;
    }

    if (!ch->is_npc( )) 
	man->doEstimate( ch, constArguments );
}

