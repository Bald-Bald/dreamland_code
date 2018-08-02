/* $Id$
 *
 * ruffina, 2004
 */
#include "walkment.h"
#include "movetypes.h"
#include "move_utils.h"
#include "terrains.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skillreference.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"

#include "stats_apply.h"
#include "act.h"
#include "loadsave.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(mount_drive);
GSN(riding);
GSN(web);
GSN(camouflage_move);

Walkment::Walkment( Character *ch )
            : Movement( ch )
{
    silence = false;
    boat_type = boat_get_type( horse ? horse : ch );
    boat = boat_object_find( horse ? horse : ch );
}

bool Walkment::moveAtomic( )
{
    if (horse) {
	if (!canControlHorse( ))
	    return false;

	visualize( horse );
	visualize( ch );

	if (!canMove( horse ) || !canMove( ch ))
	    return false;
	
	if (!tryMove( horse ) || !tryMove( ch ))
	    return false;

	place( horse );
	place( ch );
    }
    else if (rider) {
	visualize( rider );
	visualize( ch );

	if (!canMove( ch ) || !canMove( rider ))
	    return false;

	if (!tryMove( ch ) || !tryMove( rider ))
	    return false;

	place( ch );
	place( rider );
    }
    else {
	visualize( ch );

	if (!canMove( ch ))
	    return false;

	if (!tryMove( ch ))
	    return false;
	
	place( ch );
    }
    
    setWaitstate( );
    return true;
}

bool Walkment::canLeaveMaster( Character *wch )
{
    if (!IS_AFFECTED(wch, AFF_CHARM))
	return true;
    
    if (wch->master == 0)
	return true;
    
    if (from_room != wch->master->in_room)
	return true;
    
    if (wch->master == wch->mount)
	return true;
    
    msgSelfParty( wch, 
	          "���? � �������� ������ �������?",
		  "%^C1 �� ����� �������� ������ �������." );
    return false;
}

void Walkment::setWaitstate( )
{
}

static bool rprog_cant_move( Character *ch, Room *to_room, const char *movetype )
{
    FENIA_CALL(ch->in_room, "CantMove", "CRs", ch, to_room, movetype );
    return false;
}

static bool mprog_cant_move( Character *ch, Room *to_room, const char *movetype )
{
    FENIA_CALL( ch, "CantMove", "Rs", to_room, movetype );
    FENIA_NDX_CALL( ch->getNPC( ), "CantMove", "CRs", ch, to_room, movetype );
    return false;
}

static bool mprog_cant_leave( Character *ch )
{
    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
	if (rch != ch) {
	    FENIA_CALL( rch, "CantLeave", "C", ch );
	    FENIA_NDX_CALL( rch->getNPC( ), "CantLeave", "CC", rch, ch );
	}
    
    return false;
}

bool Walkment::autoDismount( Character *wch ) 
{
    bool rc;
    
    if (wch == ch)
	return false;
    if (!MOUNTED(wch))
	return false;
    if (wch->mount == wch->master)
	return false;

    silence = true;
    rc = canLeaveMaster( wch );
    silence = false;

    if (rc)
	return false;

    interpret_raw( wch, "dismount" );
    horse = MOUNTED(ch);
    rider = RIDDEN(ch);
    return true;
}

bool Walkment::canLeave( Character *wch )
{
    if (autoDismount( wch ))
	return true;

    return canLeaveMaster( wch )
	    && checkPosition( wch )
	    && !mprog_cant_leave( wch )
	    && checkTrap( wch );
}

bool Walkment::canMove( Character *wch )
{
    return checkVisibility( wch )
	    && checkCyclicRooms( wch )
	    && !mprog_cant_move( wch, to_room, movetypes[movetype].name )
	    && !rprog_cant_move( wch, to_room, movetypes[movetype].name )
            && checkLawzone( wch )
	    && checkClosedDoor( wch )
	    && checkRoomCapacity( wch )
	    && checkGuild( wch )
	    && checkSafe( wch )
	    && checkAir( wch )
	    && checkWater( wch );
}

bool Walkment::checkPosition( Character *wch )
{
    if (horse) {
	if (wch == horse)
	    return checkPositionHorse( );
    }
    else if (rider) {
	if (wch == rider)
	    return checkPositionRider( );
    }
    else {
	return checkPositionWalkman( );
    }

    return true;
}


bool Walkment::checkPositionHorse( )
{
    if (horse->fighting) {
	msgSelf( ch, "�� ����%1$G��|��|�� c����� ���������." ); 
	return false;
    }

    if (horse->position <= POS_RESTING) {
	msgSelfParty( horse,
		      "�������� ��������� ��� ������ - ����!",
		      "%2$^C1 ����%2$G��|��|�� ������� ������." );
	return false;
    }

    return true;
}

bool Walkment::checkPositionRider( )
{
    if (rider->fighting) {
	msgSelfMaster( ch, 
		       "���� ����� ���������! ������ ��� ��� ������.",
		       "����� %1$C2 ���������, ������� ��� ��������!" );
	return false;
    }

    return true;
}

bool Walkment::checkPositionWalkman( )
{
    if (ch->fighting) {
	rc = RC_MOVE_FIGHTING;
	msgSelf( ch, "����? �� �� ����������!" );
	return false;
    }
    
    if (ch->position < POS_STANDING) {
	rc = RC_MOVE_RESTING;
	msgSelf( ch, "�������� ��������� ��� ������ - ����!" );
	return false;
    }

    return true;
}

void Walkment::visualize( Character *wch )
{
    if (IS_AFFECTED( wch, AFF_HIDE|AFF_FADE ) && !IS_AFFECTED(wch, AFF_SNEAK)) {
	REMOVE_BIT(wch->affected_by, AFF_HIDE|AFF_FADE);
	wch->println( "�� �������� �� ����." );
	act( "$c1 ������� �� ����.", wch, 0, 0, TO_ROOM );
    }

    if (IS_AFFECTED( wch, AFF_CAMOUFLAGE )) {
	if (number_percent( ) < gsn_camouflage_move->getEffective( wch )) {
	    gsn_camouflage_move->improve( wch, true );
	}
	else {
	    strip_camouflage( wch );
	    gsn_camouflage_move->improve( wch, false );
	}
    }
}

bool Walkment::canControlHorse( )
{
    if (!canOrderHorse( )) {
	act( "�� �� ������ ��������� $C5.", ch, 0, horse, TO_CHAR );
	return false;
    }
    
    /* horrible XXX until riding skills are available for all */
    if (horse->is_npc( ) 
	    && (   (horse->getNPC( )->pIndexData->vnum >= 50000
	           && horse->getNPC( )->pIndexData->vnum <= 51000)
		|| (horse->getNPC( )->pIndexData->vnum >= 550
		   && horse->getNPC( )->pIndexData->vnum <= 560)))
	return true;

    if (number_percent( ) > gsn_riding->getEffective( ch ) && !ch->isCoder( )) {
	act( "���� �� ������� ���������� ��������� $C5.", ch, 0, horse, TO_CHAR );
	gsn_riding->improve( ch, false );
	return false; 
    }

    /* XXX more checks */
    gsn_riding->improve( ch, true );
    return true;
}

bool Walkment::checkCyclicRooms( Character *wch )
{
    if (from_room == to_room) {
	msgSelfParty( wch, 
		      "� ����������� ����������� �� ��������� �� �����... � ��� ������?",
	              "%2$^C1 � ����������� ����������� �������� �� ����� �����." );
	return false;
    }

    return true;
}
    
bool Walkment::checkTrap( Character *wch )
{
    if (wch->death_ground_delay > 0 && wch->trap.isSet( TF_NO_MOVE )) {
	msgSelfParty( wch, 
		     "�� �� ������ �������� ��� ����� - ��� ����������� ������!",
	             "%2$^C1 �� ����� �������� ��� ����� ��� ����������� ������!" );
	return false;
    }

    return true;
}


bool Walkment::checkVisibility( Character *wch )
{
    if (!wch->can_see( to_room )) {
	msgSelfParty( wch,
		      "����, �� �� �� ������ ���� ����.",
	              "����, �� %2$C1 �� ����� ���� ����." );
	return false;
    }

    return true;
}

bool Walkment::checkSafe( Character *wch )
{
    if (!IS_SET( to_room->room_flags, ROOM_SAFE ))
	return true;
	
    if (IS_BLOODY(wch)) {
	msgSelfRoom( wch, 
		     "������������ ���� �� ��������� ���� ����� ���� � ����� ���������.",
		     "%2$^C1 ������������� �����������... � ����������� ������� ����� � �������." );
	return false;
    }

    return true;
}

bool Walkment::checkGuild( Character *wch )
{
    if (wch->is_immortal( )) 
	return true;
    
    if (wch->is_npc( ))
	return true;
    
    if (to_room->guilds.empty( ))
	return true;

    if (!to_room->guilds.isSet( wch->getProfession( ) )) {
	msgSelfParty( wch, 
		      "�� �� ������ ����� � ����� �������.", 
		      "%2$^C1 �� ����� ����� � ����� �������." );
	return false;
    }

    if (IS_BLOODY(wch)) {
	msgSelfParty( wch, 
		      "���� ������� �� ����� ������ ������� ���� ��������.",
		      "������� �� ����� ������ ������� �������� ��� %2$C2." );
	return false;
    }
    
    return true;
}

bool Walkment::checkAir( Character *wch )
{
    if (from_room->sector_type != SECT_AIR && to_room->sector_type != SECT_AIR)
	return true;

    if (MOUNTED(wch))
	return true;

    if (is_flying( wch ))
	return true;

    if (wch->is_immortal( ) || wch->is_mirror( ))
	return true;

    if (IS_GHOST(wch))
	return true;
    
    rc = RC_MOVE_AIR;
    msgSelfParty( wch, 
	          "�� �� ������ ������.", 
	          "%2$^C1 �� ����� ������." );
    return false;
}

bool Walkment::checkWater( Character *wch )
{
    if (from_room->sector_type != SECT_WATER_NOSWIM
	 && to_room->sector_type != SECT_WATER_NOSWIM)
	return true;
    
    if (MOUNTED(wch))
	return true;

    if (boat_type != BOAT_NONE) 
	return true;
    
    rc = RC_MOVE_WATER;
    msgSelfParty( wch, 
	          "���� ���� ������ ���� ����� �����.",
	          "%2$^C1 �� ����� ������ �� ����." );
    return false;
}

bool Walkment::checkRoomCapacity( Character *wch )
{
    int capacity;
    
    if (wch->is_immortal( ))
	return true;
    
    if (MOUNTED(wch))
	return true;
    
    capacity = to_room->getCapacity( );
    
    if (capacity < 0)
	return true;

    if (capacity < (RIDDEN(wch) ? 2 : 1)) {
	msgSelfParty( wch, 
		      "��� ���� ��� ��� ������ �����.",
		      "��� ��� � %2$^C5 ��� ��� ������ �����." );
	return false;
    }

    return true;
}

bool Walkment::checkLawzone( Character *wch )    
{
    if (!IS_SET(to_room->room_flags, ROOM_LAW))
	return true;
    
    if (!wch->is_npc( ) || !IS_SET(wch->act, ACT_AGGRESSIVE))
	return true;
    
    if (!(wch->master && IS_AFFECTED( wch, AFF_CHARM )))
	return true;
    
    rc = RC_MOVE_LAWZONE;
    msgSelfMaster( wch, 
	           "�� �� ������ ����������� � �����.", 
                   "�� �� ������ ����� � ����� %1$C4 � �����." );
    return false; 
}

bool Walkment::tryMove( Character *wch )
{
    return applyMovepoints( wch )
	   && applyWeb( wch )
	   && applyCamouflage( wch );
}

bool Walkment::applyMovepoints( Character *wch )
{
    int move;
    
    if (MOUNTED( wch ))
	return true;

    if (IS_GHOST( wch ))
	return true;

    if (wch->is_npc( ))
	return true;
    
    if (( move = getMoveCost( wch ) ) == 0)
	return true;

    if (wch->move < move) {
	msgSelfParty( wch,
		      "�� ������� ����%1$G��|�|��.",
		      "%2$^C1 ������� ����%2$G��|�|��." );
	return false;
    }

    ch->move -= move;
    return true;
}

bool Walkment::checkMovepoints( Character *wch )
{
    if (wch->move < getMoveCost( wch )) {
	msgSelfParty( wch,
	              "� ���� �� ������� ��� �������� ������.",
		      "� %2$C2 �� ������� ��� �������� ������." );
	return false;
    }
    return true;
}


bool Walkment::applyCamouflage( Character *wch )
{
    if (IS_AFFECTED(wch, AFF_CAMOUFLAGE)
	    && to_room->sector_type != SECT_FIELD
	    && to_room->sector_type != SECT_FOREST
	    && to_room->sector_type != SECT_MOUNTAIN
	    && to_room->sector_type != SECT_HILLS)
    {
	strip_camouflage( wch );
    }	

    return true;
}

bool Walkment::applyWeb( Character *wch )
{
    if (MOUNTED(wch))
	return true;

    if (!CAN_DETECT(wch, ADET_WEB))
	return true;

    wch->setWaitViolence( 1 );
    
    if (number_percent( ) < get_str_app(wch).web) {
	affect_strip(wch, gsn_web);
	msgSelfRoom( wch,
		     "�� ���������� ����, ������� ������ ���� �������� ��� �����.",
	             "%2$^C1 ��������� ����, ������� ������ %2$P3 �������� ��� �����." );
	return true;
    }
    else {
	rc = RC_MOVE_WEB;
	msgSelfRoom( wch,
		     "�� ��������� ��������� ����, ������� ������ ������, �� ������� �������.",
	             "%2$^C1 �������� ��������� ����, ������������� %2$P3 ������, �� ������ �������." );
	return false;
    }
}

void Walkment::moveFollowers( Character *wch )
{
    list<Character *> followers;
    list<Character *>::iterator f;
    Character *fch;

    if (!wch)
	return;

    for (fch = from_room->people; fch != 0; fch = fch->next_in_room) 
	if (fch->master == wch && fch->position == POS_STANDING)
	    followers.push_back( fch );

    for (f = followers.begin( ); f != followers.end( ); f++) 
	if ((*f)->in_room == from_room)
	    moveOneFollower( wch, *f );
}

bool Walkment::canHear( Character *victim, Character *wch )
{
    return !silence && Movement::canHear( victim, wch );
}

