/* $Id: ruler.cpp,v 1.1.6.12.4.21 2010-09-01 21:20:44 rufina Exp $
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

#include "summoncreaturespell.h"
#include "affecthandlertemplate.h"
#include "spelltemplate.h"                                                 
#include "skillcommandtemplate.h"
#include "skill.h"
#include "skillmanager.h"

#include "pcharactermanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "dreamland.h"
#include "fight.h"
#include "magic.h"
#include "descriptor.h"
#include "damage.h"
#include "alignment.h"
#include "clanreference.h"
#include "merc.h"
#include "gsn_plugin.h"

#include "mercdb.h"
#include "vnum.h"
#include "handler.h"
#include "save.h"
#include "interp.h"
#include "act_move.h"
#include "act.h"

#include "ruler.h"
#include "cclantalk.h"


GSN(dispel_affects);
CLAN(chaos);
CLAN(ruler);


#define MOB_VNUM_STALKER           15
#define OBJ_VNUM_DEPUTY_BADGE      70
#define OBJ_VNUM_RULER_SHIELD1     71
#define OBJ_VNUM_RULER_SHIELD2     72
#define OBJ_VNUM_RULER_SHIELD3     73
#define OBJ_VNUM_RULER_SHIELD4     74

/*--------------------------------------------------------------------------
 * Ruler's Old Man
 *------------------------------------------------------------------------*/
void ClanGuardRulerPre::actGreet( PCharacter *wch )
{
    interpret_fmt( ch, "bow %s", wch->getNameP( ) );
}

void ClanGuardRulerPre::actInvited( PCharacter *wch, Object *obj )
{
    do_say( ch, "� ���� - ���� ����������?" );
}

void ClanGuardRulerPre::actPush( PCharacter *wch )
{
    act( "$C1 ���������� ����� ����� ���������... ������ ��������...\n\r... � �� ������...", wch, 0, ch, TO_CHAR );
    act( "$C1 �������� ��������� $c4 � $c1 � ������ �������...", wch, 0, ch, TO_ROOM );
}

void ClanGuardRulerPre::greet( Character *wch )
{
    Object *obj;
    PCharacter *pch = wch->getPC( );
    ClanArea::Pointer clanArea;

    clanArea = getClanArea( );

    if (!clanArea)
	return;
    
    if (wch->is_npc( ) || wch->is_immortal( ))
	return;

    if (pch->getClan( ) == clan) {
	actGreet( pch );
	return;
    }
    
    if (( obj = clanArea->findInvitation( pch ) )) {
	actInvited( pch, obj );
	return;
    }
    
    if (checkPush( pch ))
	return;
    
    if (checkGhost( pch ))
	return;

    do_say( ch, "�� ���� ���� ������!!! ������ ��� �����!" );
    do_say( ch, "��� ����� ��� ���� ������ ����������!" );
}

void ClanGuardRulerPre::speech( Character *, const char * )
{
}

bool ClanGuardRulerPre::specFight( )
{
    return false;
}

/*--------------------------------------------------------------------------
 * Ruler's Clan Guard 
 *------------------------------------------------------------------------*/
void ClanGuardRuler::greet( Character *wch )
{
    Object *obj;
    PCharacter *pch = wch->getPC( );
    ClanArea::Pointer clanArea;

    clanArea = getClanArea( );

    if (!clanArea)
	return;

    if (wch->is_npc( ) || wch->is_immortal( ))
	return;

    if (pch->getClan( ) == clan) 
	return;

    if (( obj = clanArea->findInvitation( pch ) )) {
	actInvited( pch, obj );
//	extract_obj(obj);
	return;
    }

    if (checkGhost( pch ))
	return;
    
    actIntruder( pch );
    doPetitionOutsider( pch );
    doAttack( pch );
}

int ClanGuardRuler::getCast( Character *victim )
{
    int sn = -1;

    switch ( dice(1,16) ) {
    case  0:
    case  1:
	    if (!victim->isAffected( gsn_spellbane ))
		sn = gsn_dispel_affects;
	    break;
    case  2:
    case  3:
	    sn = gsn_acid_arrow;
	    break;
    case  4:
    case  5:
	    sn = gsn_caustic_font;
	    break; 
    case  6:
    case  7:
    case  8:
    case  9:
    case 10:
	    sn = gsn_acid_blast;
	    break;
    default:
	    sn = -1;
	    break;
    }

    return sn;
}

/*--------------------------------------------------------------------------
 * Ruler's Jailer 
 *------------------------------------------------------------------------*/
void ClanGuardRulerJailer::actPush( PCharacter *wch )
{
    act( "$C1 ������� �� ���� ���������� ������.\n\r� ��� �� �� ����������, ��� ����� ���������� ���� ����������� ���� ���.", wch, 0, ch, TO_CHAR );
    act( "$C1 ������� �� $c4 ���������� ������ � $c1 ��������� ��������.", wch, 0, ch, TO_ROOM );
}

void ClanGuardRulerJailer::actIntruder( PCharacter *wch )
{
    interpret_raw( ch, "cb", "����������� � ������... ����������� ���� ��������!" );
} 

void ClanGuardRulerJailer::greet( Character *wch )
{
    PCharacter *pch = wch->getPC( );
    
    if (wch->is_npc( ) || wch->is_immortal( ))
	return;
    if (pch->getClan( ) == clan_ruler)
	return;
    if (pch->isAffected(gsn_manacles ))
	return;

    ch->setClan( clan_ruler );
    
    if (checkPush( pch ))
	return;

    if (checkGhost( pch ))
	return;
    
    actIntruder( pch );
    doAttack( pch );
}

void ClanGuardRulerJailer::speech( Character *, const char * )
{
}

bool ClanGuardRulerJailer::specFight( )
{
    char buf[MAX_STRING_LENGTH];
    Character *victim;
    Character *v_next;
    Character *ech;
    const char *crime;
    int max_evil;

    max_evil = 300;
    ech      = 0;
    crime    = "";

    for ( victim = ch->in_room->people; victim != 0; victim = v_next )
    {
	    v_next = victim->next_in_room;
    
	    if ( !victim->is_npc()
		    && ( IS_SET(victim->act, PLR_WANTED)
			    || ( victim->isAffected(gsn_jail )
				    && !victim->isAffected(gsn_manacles ) ) ) )
	    {
		    crime = "CRIMINAL";
		    break;
	    }
    
	    if ( victim->fighting != 0
		    && !victim ->is_npc()
		    && victim->getPC()->getClan() != clan_ruler 
		    && victim->fighting != ch )
	    {
		    ech = victim;
	    }
    }

    if ( victim != 0 )
    {
	    ch->setClan( clan_ruler );
	    interpret_raw(ch, "cb", "��������!!! %s ��������� %s � ������ %s",
			    victim->getNameP(), 
			    ch->in_room->name,
		            ch->in_room->area->name);


	    if ( ( ch->getModifyLevel() + 8 > victim->getModifyLevel() )
		    && !is_safe_nomessage ( ch, victim ) )
	    {
		    sprintf( buf, "%s %s! ������� ��������!! ������ ������������!!",
			    victim->getNameP( ), crime );
		    do_yell( ch, buf );
		    multi_hit( ch, victim );
	    }
	    else
	    {
		    sprintf( buf, "$c1 ������ '%s! �� ��� �������� �� ���� ������������!'",
			    victim->getNameP( ));
		    act_p( buf, ch, 0, 0, TO_ROOM,POS_RESTING );
	    }
	    return true;
    }

    if ( ech != 0 )
    {
	    act_p( "$c1 ������ ' ������� ��������!! ������ ������������!!",
		    ch, 0, 0, TO_ROOM,POS_RESTING );
	    multi_hit( ch, ech );
	    return true;
    }

    return false;
}


/*--------------------------------------------------------------------------
 * skills and spells 
 *------------------------------------------------------------------------*/


/* Thanx zihni@karmi.emu.edu.tr for the code of do_judge */
/*
 * 'judge' skill command
 */

SKILL_RUNP( judge )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int  value;
	Character *victim;

	if ( !gsn_judge->available( ch ))
	{
	    ch->send_to("���?\n\r");
	    return;
	}

	if (!gsn_judge->usable( ch ))
  		return;

	if ( ch->isAffected(gsn_dismiss ) )
	{
		ch->send_to( "� ���� �������� ���������� ������!\n\r" );
		return;
	}

	argument =  one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->send_to( "���� ����� ��������?\n\r");
		return;
	}

	/* judge thru world */
	if ( ( victim = get_char_world( ch, arg ) ) == 0 )
	{
		ch->send_to( "��� ����� ���.\n\r");
		return;
	}

	if (victim->is_npc())
	{
		ch->send_to("���� ������� ������ ��� ������������ ��������.\n\r");
		return;
	}

	if (victim->is_immortal() && !ch->is_immortal())
	{
		ch->send_to( "�� �� � ����� ��������� ������� ������ � �����������.\n\r");
		return;
	}

	// Well let's look for numbers
	argument =  one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		ch->pecho("� %1$#C2 %2$s ���� � %3$s ��������.\n\r"
		          "%1$#^P2 ������� ����� �������: %4$d.",
			  victim, 
			  ethos_table.message( victim->ethos ).c_str( ),
			  align_name( victim ).ruscase( '1' ).c_str( ),
			  victim->getPC( )->loyalty.getValue( ));

		return;
	}
	else
	{
		if ( victim->getClan() == clan_chaos )
		{
			ch->send_to("{x���, � {M������ {x���� �������� �������� ������� ����������!\n\r");
			return;
		}

		if( is_number( arg ) )
			value = atoi( arg );
		else
		{
			ch->send_to("������������ �����.\n\r");
			return;
		}

		value=victim->getPC( )->loyalty+value;
		if ( value>1000 )
			value=1000;
		if ( value<-1000 )
			value=-1000;

		victim->getPC( )->loyalty=value;

		sprintf(buf,"����������� � %d.\n\r",value);
		ch->send_to(buf);

		return;
	}
}

/*
 * 'manacles' skill command
 */

SKILL_RUNP( manacles )
{
	char			arg[MAX_INPUT_LENGTH];
	Character	*victim;

	if ( !gsn_manacles->available( ch ))
	{
	    ch->send_to("���?\n\r");
	    return;
	}

	if (!gsn_manacles->usable( ch )) return;

	if ( ch->isAffected(gsn_dismiss ) )
	{
		ch->send_to( "� ���� �������� ���������� ������!\n\r" );
		return;
	}

	argument =  one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->send_to( "� ���� �� ����� �������� � �������?\n\r");
		return;
	}

	/* manacles works only in current room */
	if ( ( victim = get_char_room( ch, arg ) ) == 0 )
	{
		ch->send_to( "��� ����� ���.\n\r" );
		return;
	}

	if ( victim == ch )
	{
		ch->send_to( "���� �������, �� �� ����� ����� ������.\n\r" );
		return;
	}

	if (victim->is_npc())
	{
		ch->send_to("��� �� ������� ��� �����, �� ����� ���������.\n\r" );
		return;
	}

	if (victim->is_immortal() && !ch->is_immortal())
	{
		ch->send_to( "�� �� ������ ���������� � ������� �����������.\n\r" );
		return;
	}

	if ( victim->getClan() == clan_chaos )
	{
		ch->send_to("{x���, � {M������ {x���� �������� �������� ������� ����������!\n\r");
		return;
	}

	// Well let's look for action
	argument =  one_argument( argument, arg );

	if ( arg[0] == '\0' || arg_oneof_strict( arg, "check", "��������" ))
	{

		act_p("$c1 ������� ������� ������ �� ���� ����.",
					ch,0,victim,TO_VICT,POS_RESTING);
		act_p("$c1 ������� ������� ������ �� ���� $C4.",
					ch,0,victim,TO_NOTVICT,POS_RESTING);

		if ( victim->isAffected(gsn_manacles) )
		{
			char buf[MAX_STRING_LENGTH];
			Affect *paf = victim->affected->affect_find (gsn_manacles);

			if ( paf->duration >= 0 )
			{
				sprintf (buf,"$C1 ������� �� %d ���%s."
					,paf->duration
					,GET_COUNT(paf->duration, "","�","��"));
			}
			else
			{
				sprintf (buf,"$C1 ������� ��������.");
			}

			act_p("���� $C4 �������� � �������!",
						ch,0,victim,TO_CHAR,POS_RESTING);

			act_p (buf, ch, 0, victim, TO_CHAR, POS_RESTING);
		}
		else
		{
			act_p("$C1 �������� �� ����.",
						ch,0,victim,TO_CHAR,POS_RESTING);
		}
	
	} else
	if (arg_oneof_strict( arg, "remove", "�����" ))
	{
		if ( victim->isAffected(gsn_manacles) )
		{
			affect_strip ( victim, gsn_manacles );

			act_p("�� ������������ ���� $C4 �� ����.",
						ch,0,victim,TO_CHAR,POS_RESTING);
			act_p("$c1 ������� ������� � ����� ���.",
						ch,0,victim,TO_VICT,POS_RESTING);
			act_p("$c1 ������� ������� � ��� $C4.",
						ch,0,victim,TO_NOTVICT,POS_RESTING);
		}
		else
		{
			act_p("�� ��������� ���������� ���� $C4 �� ����, �� �������� �� ���������.",
						ch,0,victim,TO_CHAR,POS_RESTING);
			act_p("$c1 ������ ���, ��� ������� ������� � ����� ���. � ���� �� ���...",
						ch,0,victim,TO_VICT,POS_RESTING);
			act_p("$c1 ������� ������ ��� $C4... ��������, ����� ����-��..",
						ch,0,victim,TO_NOTVICT,POS_RESTING);
		}
	}
	else if (arg_oneof_strict( arg, "place", "��������" ))
	{
		int success = 0;
		int duration;

		argument =  one_argument( argument, arg );

		if ( arg == 0
			|| arg[0] == '\0' )
		{
			duration = -1;
		}
		else if( is_number( arg ) )
			duration = atoi( arg );
		else
		{
			ch->send_to("������������ �����.\n\r");
			return;
		}

		if ( duration == 0 || duration <= -2 )
		{
			ch->send_to ("�� ����� ����� ������ ?\n\r");
			return;
		}

		if ( victim->isAffected(gsn_manacles) )
		{
			affect_strip ( victim, gsn_manacles );
		}

		if( victim->position > POS_DEAD &&
				victim->position < POS_FIGHTING )
		{
			success = !success;
		}
		else if ( victim->position <= POS_DEAD )
		{
			ch->send_to( "��������� ��������� ������� - ����� �������.\n\r" );
		}
		else if ( victim->position >= POS_FIGHTING )
		{
			success = (ch->getCurrStat(STAT_DEX) > victim->getCurrStat(STAT_DEX)) ||
								(number_percent() > (gsn_dodge->getEffective( victim ) / 2));

			if (!success)
			{
				act_p("�� ��������� �������� $C4 � �������, �� ���-�� ���� �� ���...",
							ch,0,victim,TO_CHAR,POS_RESTING);
				act_p("$c1 �������� ������� ���� ����! �� �� ����!",
							ch,0,victim,TO_VICT,POS_RESTING);
				act_p("$c1 �������� �������� $C4 � �������, �� ������ �������.",
							ch,0,victim,TO_NOTVICT,POS_RESTING);

			}

		}

		if (success)
		{
			Affect af;

			af.where			= TO_AFFECTS;
			af.type				= gsn_manacles;
			af.level			= ch->getPC()->getClanLevel();
			af.duration		= duration;
			af.bitvector	= 0;
			af.modifier		= 0;
			af.location		= APPLY_NONE;
			affect_to_char ( victim, &af);

			act_p("�� ������� ����������� $C4 � �������!",
						ch,0,victim,TO_CHAR,POS_RESTING);
			act_p("$c1 ���������� ���� � �������!",
						ch,0,victim,TO_VICT,POS_RESTING);
			act_p("$c1 ���������� $C4 � �������!",
						ch,0,victim,TO_NOTVICT,POS_RESTING);
		};
	
	} else
	{
		ch->send_to( "�� �� ������ ����� �������.\n\r" );
	};

	return;
}

/*
 * 'wanted' skill command
 */

SKILL_RUNP( wanted )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	Character *victim;

	if ( !gsn_wanted->available( ch ))
	{
	    ch->send_to("���?\n\r");
	    return;
	}

	if ( !gsn_wanted->usable( ch ) )
		return;

	if ( ch->isAffected(gsn_dismiss ) )
	{
		ch->send_to( "� ���� �������� ���������� ������!\n\r" );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		ch->send_to("���������: wanted <player> <Y|N>\n\r");
		return;
	}

	victim = get_char_world(ch, arg1);

	if ( (victim == 0) ||	!(ch->can_see(victim)) )
	{
		ch->send_to("����� ��� �����.\n\r");
		return;
	}

	if ( victim->getRealLevel( ) >= LEVEL_IMMORTAL
		&& (ch->getRealLevel( ) < victim->getRealLevel( )) )
	{
		act_p( "� ���� �� ������� ��� �������� $C4 � ������.",
				ch, 0, victim, TO_CHAR,POS_RESTING);
		return;
	}

	if (victim == ch)
	{
		ch->send_to("������ ������ � ����� �����!\n\r");
		return;
	}

	if ( victim->getClan() == clan_chaos )
	{
		ch->send_to("{x���, � {M������ {x���� �������� �������� ������� ����������!\n\r");
		return;
	}

	switch(arg2[0])
	{
		case 'Y':
		case 'y':
		case '�':
		case '�':
			if ( victim->isAffected(gsn_suspect ) )
			{
				affect_strip ( victim, gsn_suspect );
				victim->send_to ("���� �������� � ��� ����� ����� ��������!\n\r");
			}

			if ( IS_SET(victim->act ,PLR_WANTED) )
			{
				act_p( "$C1 ��� �������������.", ch, 0, victim, TO_CHAR,POS_RESTING);
			}
			else
			{
				SET_BIT(victim->act, PLR_WANTED);
				act_p( "$c1 ������ � �������!!!",victim, 0, ch, TO_NOTVICT,POS_RESTING);
				victim->send_to("�� ������ � �������!!!\n\r");
				if ( !victim->is_npc() )
					victim->getPC( )->loyalty = max ( victim->getPC( )->loyalty - 50, -1000);
				ch->send_to("Ok.\n\r");
			}
			break;
		case 'N':
		case 'n':
		case '�':
		case '�':
			if ( !IS_SET(victim->act,PLR_WANTED) )
				act_p( "$C1 �� �������������.", ch, 0, victim, TO_CHAR,POS_RESTING);
			else
			{
				REMOVE_BIT(victim->act, PLR_WANTED);
				act_p( "$c1 ������ �� �������������.",victim, 0, ch, TO_NOTVICT,POS_RESTING);
				victim->send_to("���� ������ �� �����������.\n\r");
				ch->send_to("Ok.\n\r");
			}
			break;
		default:
			ch->send_to("���������: wanted <player> <Y|N>\n\r");
			break;
	}
}

/*
 * 'fine' skill command
 */

SKILL_RUNP( fine )
{
	char			arg[MAX_INPUT_LENGTH];
	Character	*victim;
	Character *recepient = 0;
	int				value = 0;
	int				value2;
	bool			inroom = 0;

	if ( !gsn_fine->available( ch ))
	{
	    ch->send_to("���?\n\r");
	    return;
	}

	if (!gsn_fine->usable( ch ))
		return;

	if ( ch->isAffected(gsn_dismiss ) )
	{
		ch->send_to( "� ���� �������� ���������� ������!\n\r" );
		return;
	}

	argument =  one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->send_to( "���������: fine <����������> <�����> [<����������>]\n\r");
		return;
	}

	if ( !( inroom = ( (victim = get_char_room (ch, arg) ) != 0 ) ) )
	{
		victim = get_char_world(ch, arg);

		if ( (victim == 0) ||	!(ch->can_see(victim)) )
		{
			ch->send_to("� ���� �� ���������� ����� ����������� � ����� ������.\n\r");
			return;
		}

		if ( victim->getClan() == clan_chaos )
		{
			ch->send_to("{x���, � {M������ {x���� �������� �������� ������� ����������!\n\r");
			return;
		}

	}

	argument =  one_argument( argument, arg );

	if( is_number( arg ) )
		value = atoi( arg );
	else
	{
		ch->send_to("������������ �����\n\r");
		return;
	}

	if ( value <= 0 )
	{
		ch->send_to ("�������� ����� �� � ���� ������.");
		return;
	}

	argument =  one_argument( argument, arg );

	if ( arg[0] != '\0' )
	{
		recepient = get_char_room (ch, arg);

		if ( recepient == 0 || !(ch->can_see(recepient)) )
		{
			ch->send_to("�� �� �������� ����, ���� ��������� ������ ��������� ����.\n\r");
			return;
		}
	}

	if ( victim->is_npc()
		|| ( ( recepient != 0 ) && recepient->is_npc() ) 
		|| victim == ch )
	{
		ch->send_to ("���� �� �������, ��� �� ��������� ������� ������?\n\r");
		return;
	}

	if ( ( ( (inroom && victim->isAffected(gsn_manacles) ) ? (int)victim->gold : 0 ) + victim->getPC()->bank_g ) < value )
	{
		act_p ("�� �� ������ ������� ������� ������� ����� � $C4."
			, ch, 0, victim, TO_CHAR, POS_RESTING );
		return;
	}

	value2 = value;

	if ( inroom
		&& victim->isAffected(gsn_manacles)
		&& ( victim->gold > 0 ) )
	{
		int amount =  (victim->gold > value) ? value : (int)victim->gold;

		victim->gold -= amount;
		value -= amount;

		act_p ("�� ��������� � $C4 ��������� ������� ����� � �������� ������."
			, ch, 0, victim, TO_CHAR, POS_RESTING );
		act_p ("$c1 �������� � ���� ��������� ������� ����� � �������� ������."
			, ch, 0, victim, TO_VICT, POS_RESTING );
		act_p ("$c1 �������� � $C4 ��������� ������� ����� � �������� ������."
			, ch, 0, victim, TO_NOTVICT, POS_RESTING );
	}

	if ( value > 0 )
	{
		victim->getPC()->bank_g -= value;

		act_p ("�� �������� � $C4 �� ����� ��������� ������� ����� � �������� ������."
			, ch, 0, victim, TO_CHAR, POS_RESTING );
		act_p ("$c1 ������� � $C4 �� ����� ��������� ������� ����� � �������� ������."
			, ch, 0, victim, TO_NOTVICT, POS_RESTING );
		if ( inroom )
		{
			act_p ("$c1 �������� � ���� �� ����� ��������� ������� ����� � �������� ������."
				, ch, 0, victim, TO_VICT, POS_RESTING );
		}
		else
		{
			victim->send_to ("������, ��� ���� ������� ������ � ����� �����������.\n\r");
		}
	}

  if ( recepient == 0 )
	{
		ch->getPC()->bank_g += value2;

		act_p ("�� ���������� �� ���� ���� ��������� ������� �����."
			, ch, 0, 0, TO_CHAR, POS_RESTING );
		act_p ("$c1 ��������� �� ���� ���� ��������� ������� �����."
			, ch, 0, 0, TO_ROOM, POS_RESTING );
	}
	else
	{
		recepient->getPC()->bank_g += value2;
		
		act_p ("�� ���������� �� ���� $C4 ��������� ������� �����."
			, ch, 0, recepient, TO_CHAR, POS_RESTING );
		act_p ("$c1 ��������� �� ���� ���� ��������� ������� �����."
			, ch, 0, recepient, TO_VICT, POS_RESTING );
		act_p ("$c1 ��������� �� ���� $C4 ��������� ������� �����."
			, ch, 0, recepient, TO_NOTVICT, POS_RESTING );
	}
}

/*
 * 'confiscate' skill command
 */

SKILL_RUNP( confiscate )
{
	char			arg[MAX_INPUT_LENGTH];
	Character	*victim;
	int				percent = 0;
	Object		*obj, *obj_next;

	if ( !gsn_confiscate->available( ch ))
	{
	    ch->send_to("���?\n\r");
	    return;
	}

	if (!gsn_confiscate->usable( ch ))
		return;

	if ( ch->isAffected(gsn_dismiss ) )
	{
		ch->send_to( "� ���� �������� ���������� ������!\n\r" );
		return;
	}

	argument =  one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->send_to( "���������: confiscate <����������> <% �� ���-�� �����>\n\r");
		return;
	}

	victim = get_char_room (ch, arg);

	if ( (victim == 0) ||	!(ch->can_see(victim)) )
	{
		ch->send_to("� ���� �� ���������� ����� ����������� � ����� ������.\n\r");
		return;
	}

	if ( victim->is_npc()
		|| victim == ch )
	{
		ch->send_to ("���� �� �������, ��� �� ��������� ������� ������?\n\r");
		return;
	}

	if ( !victim->isAffected(gsn_manacles) )
	{
		act_p ("� �� �������, ��� $C1 ��� ������ ������ ���� ���� ����?",
			ch, 0, victim, TO_CHAR, POS_RESTING);
		return;
	}

	if ( victim->getClan() == clan_chaos )
	{
		ch->send_to("{x���, � {M������ {x���� �������� �������� ������� ����������!\n\r");
		return;
	}

	argument =  one_argument( argument, arg );

	if( is_number( arg ) )
		percent = atoi( arg );
	else
	{
		ch->send_to("������������ �������.\n\r");
		return;
	}

	if ( ( percent <= 0 ) || ( percent > 100 ) )
	{
		ch->send_to ("������������ �������� ��������.\n\r");
		return;
	}

	dreamland->removeOption( DL_SAVE_OBJS );

	for ( obj = victim->carrying ; obj != 0; obj = obj_next )
	{
		obj_next = obj->next_content;

		if (!ch->can_see( obj ) || !victim->can_see( obj ))
		    continue;

		if (IS_SET(obj->extra_flags, ITEM_NODROP))
		    continue;
		
		if (obj->item_type == ITEM_TATTOO)
		    continue;

		if (obj->wear_loc != wear_none && IS_SET(obj->extra_flags, ITEM_NOREMOVE))
		    continue;

		if (obj->behavior && !obj->behavior->canConfiscate( ))
		    continue;

		if (number_percent () <= percent )
		{
			if ( obj->carried_by == 0 )
			{
				bug( "Confiscate: null victim in confiscated item.", 0 );
				return;
			}

			obj_from_char( obj );
			obj->wear_loc = wear_none ;

			act_p( "�� ����������� $o4 � $C4.", ch, obj, victim, TO_CHAR,POS_RESTING);
			act_p( "$c1 ���������� � ���� $o4.", ch, obj, victim, TO_VICT,POS_RESTING);
			act_p( "$c1 ���������� $o4 � $C4.", ch, obj, victim, TO_NOTVICT,POS_RESTING);

			obj_to_room( obj, ch->in_room );
			act_p( "�� ��������� ������� $o4 �� ���.", ch, obj, 0, TO_CHAR,POS_RESTING);
			act_p( "$c1 ��������� ������ $o4 �� ���.", ch, obj, 0, TO_ROOM,POS_RESTING);

		}
	}  

	dreamland->resetOption( DL_SAVE_OBJS );

	save_items( ch->in_room );

	return;
}

/*
 * 'suspect' skill command
 */

SKILL_RUNP( suspect )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int  value;
	Character *victim;

	if ( !gsn_suspect->available( ch ))
	{
	    ch->send_to("���?\n\r");
	    return;
	}

  if (!gsn_suspect->usable( ch )) return;

	if ( ch->isAffected(gsn_dismiss ) )
	{
		ch->send_to( "� ���� �������� ���������� ������!\n\r" );
		return;
	}

	argument =  one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->send_to( "���� ������ �������� � ���?\n\r");
		return;
	}

	if ( ( victim = get_char_world( ch, arg ) ) == 0 )
	{
		ch->send_to( "��� ����� ���.\n\r");
		return;
	}

	if (victim->is_npc())
	{
		ch->send_to("��� ������������ �������� - �� �������� �� ��������.\n\r");
		return;
	}

	if ( ch == victim )
	{
		ch->send_to ("� ���� �� �������, ����� ������� ��� � �����������?\n\r");
		return;
	}

	if (victim->is_immortal() && !ch->is_immortal())
	{
		ch->send_to( "������ ��������� ����-�� �� �����������.\n\r" );
		return;
	}

	if ( victim->getClan() == clan_chaos )
	{
		ch->send_to("{x���, � {M������ {x���� �������� �������� ������� ����������!\n\r");
		return;
	}

	argument =  one_argument( argument, arg );
	if ( arg == 0
		|| arg[0] == '\0' )
	{
		Affect *paf = victim->affected->affect_find (gsn_suspect);

		if ( paf != 0 )   
		{
			sprintf (buf,"�������� $C2 ������������� � ������� %d ���%s."
				,paf->duration
				,GET_COUNT(paf->duration, "�","��","��"));

			victim->send_to ("�� ���������� - ���� ���� � ����.\n\r");
		}
		else
			sprintf (buf,"$C1 �� ���������� �������� � ���.");

		act_p (buf, ch, 0, victim, TO_CHAR, POS_RESTING);

		return;
	}
	else if (arg_oneof_strict( arg, "off", "��������" ))
	{
		if ( !victim->isAffected(gsn_suspect) )
		{
			act_p ("�� ���� $C3 �� ���������� �������� � ���!", ch, 0, victim,
				TO_CHAR, POS_RESTING );
		}
		else
		{
			act_p ("�� ����������� �������� $C4.", ch, 0, victim,
				TO_CHAR, POS_RESTING );
			act_p ("$c1 ���������� ���� �������� � ���.", ch, 0, victim,
				TO_VICT, POS_RESTING );
			act_p ("$c1 ���������� �������� $C3 � ���.", ch, 0, victim,
				TO_NOTVICT, POS_RESTING );
			act_p ("$C1 ���������� �������� $c3 � ���.", victim, 0, ch,
				TO_NOTVICT, POS_RESTING );

			affect_strip ( victim, gsn_suspect );
		}

		return;
	}
	else if( is_number( arg ) )
		value = atoi( arg );
	else
	{
		ch->send_to("������������ �����.\n\r");
		return;
	}

	if ( value <= 0 )
	{
		ch->send_to("�������: ������ �������� �������� ������ ������.\n\r");
		return;
	}

	if ( !victim->isAffected(gsn_suspect) )
	{
		Affect af;

		af.where			= TO_AFFECTS;
		af.type				= gsn_suspect;
		af.level			= ch->getPC()->getClanLevel();
		af.duration		= value;
		af.bitvector	= 0;
		af.modifier		= 0;
		af.location		= APPLY_NONE;

		affect_to_char ( victim, &af);

		act_p ("�� ��������� �������� $C4.", ch, 0, victim,
			TO_CHAR, POS_RESTING );
		act_p ("$c1 �������� ���� �������� � ���.", ch, 0, victim,
			TO_VICT, POS_RESTING );
		act_p ("$c1 �������� $C3 �������� � ���.", ch, 0, victim,
			TO_NOTVICT, POS_RESTING );
		act_p ("$C1 �������� $c3 �������� � ���.", victim, 0, ch,
			TO_NOTVICT, POS_RESTING );
	}
	else
	{
		ch->send_to ("������� ���������� �������� ���������� ��������.");
	}

	return;

}

/*
 * 'jail' skill command
 */

SKILL_RUNP( jail )
{
	char			arg[MAX_INPUT_LENGTH];
	Character	*victim;

	if ( !gsn_jail->available( ch ))
	{
	    ch->send_to("���?\n\r");
	    return;
	}

	if (!gsn_jail->usable( ch )) return;

	if ( ch->isAffected(gsn_dismiss ) )
	{
		ch->send_to( "� ���� �������� ���������� ������!\n\r" );
		return;
	}

	argument =  one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->send_to( "� ���� �� ����� �������� � �������?\n\r");
		return;
	}

	/* Jail works only in current room */
	if ( ( victim = get_char_room( ch, arg ) ) == 0 )
	{
		ch->send_to( "��� ����� ���.\n\r" );
		return;
	}

	if ( victim == ch )
	{
		ch->send_to( "���� �������, �� �� ����� ����� ������.\n\r" );
		return;
	}

	if (victim->is_npc())
	{
		ch->send_to("��� �� ������� ��� �����, �� ����� ���������.\n\r" );
		return;
	}

	if (victim->is_immortal() && !ch->is_immortal())
	{
		ch->send_to( "�� �� ������ ����� �� ������� �����������.\n\r" );
		return;
	}

	if ( victim->getClan() == clan_chaos )
	{
		ch->send_to("{x���, � {M������ {x���� �������� �������� ������� ����������!\n\r");
		return;
	}

	// Well let's look for action
	argument =  one_argument( argument, arg );

	if ( arg[0] == '\0' || arg_oneof_strict( arg, "check", "��������" ))
	{

		act_p("$c1 ���������� ������� �� ����.",
					ch,0,victim,TO_VICT,POS_RESTING);
		act_p("$c1 ���������� ������� �� $C4.",
					ch,0,victim,TO_NOTVICT,POS_RESTING);

		if ( victim->isAffected(gsn_jail) )
		{
			char buf[MAX_STRING_LENGTH];
			Affect *paf = victim->affected->affect_find (gsn_jail);

			if ( paf->duration >= 0 )
			{
				sprintf (buf,"$C1 � ������ �� %d ���%s."
					,paf->duration
					,GET_COUNT(paf->duration, "","�","��"));
			}
			else
			{
				sprintf (buf,"$C1 � ������ ����������.");
			}

			if ( victim->isAffected(gsn_manacles) )
				act_p("���� $C4 �������� � �������!",ch,0,victim,TO_CHAR,POS_RESTING);

			act_p (buf, ch, 0, victim, TO_CHAR, POS_RESTING);
		}
		else
		{
			act_p("$C1 �� �������� ���������.",
						ch,0,victim,TO_CHAR,POS_RESTING);
		}
	
	} 
	else if (arg_oneof_strict( arg, "remove", "����������" ))
	{
		if ( victim->isAffected(gsn_jail) )
		{
			affect_strip ( victim, gsn_jail );

			act_p("�� ������������ $C4 �� ���������.",
						ch,0,victim,TO_CHAR,POS_RESTING);
			act_p("$c1 ����������� ���� �� ���������.",
						ch,0,victim,TO_VICT,POS_RESTING);
			act_p("$c1 ����������� $C4 �� ���������.",
						ch,0,victim,TO_NOTVICT,POS_RESTING);
		}
		else
		{
			act_p("�� ��������� ���������� $C4 �� ������, �� ���� �$g��|�|�� �� �����.",
						ch,0,victim,TO_CHAR,POS_RESTING);
			act_p("$c1 �������� ���� ���������� �� ������. � ���� �� ���...",
						ch,0,victim,TO_VICT,POS_RESTING);
			act_p("$c1 ��������� ��� �� ������� $C4... $C1 ��������...",
						ch,0,victim,TO_NOTVICT,POS_RESTING);
		}
	}
	else if (arg_oneof_strict( arg, "place", "�����������" ))
	{
		int duration;

		argument =  one_argument( argument, arg );

		if ( arg == 0
			|| arg[0] == '\0' )
		{
			duration = -1;
		}
		else if( is_number( arg ) )
			duration = atoi( arg );
		else
		{
			ch->send_to("������������ �����.\n\r");
			return;
		}

		if ( duration == 0 || duration <= -2 )
		{
			ch->send_to ("�� ����� ����� ������ ?\n\r");
			return;
		}

		if ( victim->isAffected(gsn_jail) )
		{
			affect_strip ( victim, gsn_jail );
		}

		Affect af;

		af.where			= TO_AFFECTS;
		af.type				= gsn_jail;
		af.level			= ch->getPC()->getClanLevel();
		af.duration		= duration;
		af.bitvector	= 0;
		af.modifier		= 0;
		af.location		= APPLY_NONE;
		affect_to_char ( victim, &af);

		act_p("�� �������������� $C4 � ��������� ����������!",
					ch,0,victim,TO_CHAR,POS_RESTING);
		act_p("$c1 ������������� ���� � ��������� ����������.",
					ch,0,victim,TO_VICT,POS_RESTING);
		act_p("$c1 ������������� $C4 � ��������� ����������.",
					ch,0,victim,TO_NOTVICT,POS_RESTING);
	} else
	{
		ch->send_to( "�� �� ������ ����� �������.\n\r" );
	};

	return;
}


/*
 * 'dismiss' skill command
 */

SKILL_RUNP( dismiss )
{
	char			arg[MAX_INPUT_LENGTH];
	Character	*victim;

	if ( !gsn_dismiss->available( ch ))
	{
	    ch->send_to("���?\n\r");
	    return;
	}

	if (!gsn_dismiss->usable( ch )) return;

	if ( ch->isAffected(gsn_dismiss ) )
	{
		ch->send_to( "� ���� �������� ���������� ������!\n\r" );
		return;
	}

	argument =  one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		ch->send_to( "� ���� �� ����� ������ ����?\n\r");
		return;
	}

	/* Dismiss works in whole room */
	if ( ( victim = get_char_world( ch, arg ) ) == 0 )
	{
		ch->send_to( "��� ����� ���.\n\r" );
		return;
	}

	if ( victim == ch )
	{
		ch->send_to( "���� �������, �� �� ����� ����� ������.\n\r" );
		return;
	}

	if (victim->is_npc())
	{
		ch->send_to("��� �� ������� ��� �����, �� ����� ���������.\n\r" );
		return;
	}

	if (victim->is_immortal() && !ch->is_immortal())
	{
		ch->send_to( "�� �� ������ ������ ����������� �� ����.\n\r" );
		return;
	}

	if ( victim->getPC()->getClan() != clan_ruler )
	{
		ch->send_to( "�� ���� ��� ���� �� ����� - ���� ��� ������ ��?\n\r" );
		return;
	}

	if ( victim->getPC()->getClanLevel() >= ch->getPC()->getClanLevel() )
	{
		ch->send_to( "���... ����� ���������� ��� ���� ������������!\n\r" );
		return;
	}

	argument =  one_argument( argument, arg );

	if ( arg[0] == '\0' || arg_oneof_strict( arg, "check", "��������" ))
	{

		act_p("$c1 ������ � ����� ������ ����.",
					ch,0,victim,TO_VICT,POS_RESTING);
		act_p("$c1 ������ � ������ ���� $C4.",
					ch,0,victim,TO_NOTVICT,POS_RESTING);

		if ( victim->isAffected(gsn_dismiss) )
		{
			char buf[MAX_STRING_LENGTH];
			Affect *paf = victim->affected->affect_find (gsn_dismiss);

			if ( paf->duration >= 0 )
			{
				sprintf (buf,"$C1 ����$G��|�|�� ����� ��������� ���� �� %d ���%s."
					,paf->duration
					,GET_COUNT(paf->duration, "","�","��"));
			}
			else
			{
				sprintf (buf,"$C1 ����$G��|�|�� ����� ��������� ���� ��������.");
			}

			act_p (buf, ch, 0, victim, TO_CHAR, POS_RESTING);
		}
		else
		{
			act_p("$C1 ��������� �����.",
						ch,0,victim,TO_CHAR,POS_RESTING);
		}
	
	} 
	else if (arg_oneof_strict( arg, "remove", "�������" ))
	{
		if ( victim->isAffected(gsn_dismiss) )
		{
			affect_strip ( victim, gsn_dismiss );

			act_p("�� ����������� $C3 ����� ������� ���.",
						ch,0,victim,TO_CHAR,POS_RESTING);
			act_p("$c1 ���������� ���� ����� ������� ���.",
						ch,0,victim,TO_VICT,POS_RESTING);
			act_p("$c1 ���������� $C3 ����� ������� ���.",
						ch,0,victim,TO_NOTVICT,POS_RESTING);
		}
		else
		{
			ch->send_to( "����������. ��� �� ����� ������ � ����� ��� ����!\n\r" );
		}
	}
	else if (arg_oneof_strict( arg, "place", "������" ))
	{
		int duration;

		argument =  one_argument( argument, arg );

		if ( arg == 0
			|| arg[0] == '\0' )
		{
			duration = -1;
		}
		else if( is_number( arg ) )
			duration = atoi( arg );
		else
		{
			ch->send_to("������������ �����.\n\r");
			return;
		}

		if ( duration == 0 || duration <= -2 )
		{
			ch->send_to ("�� ����� ����� ������ ?\n\r");
			return;
		}

		if ( victim->isAffected(gsn_dismiss) )
		{
			affect_strip ( victim, gsn_dismiss );
		}

		Affect af;

		af.where			= TO_AFFECTS;
		af.type				= gsn_dismiss;
		af.level			= ch->getPC()->getClanLevel();
		af.duration		= duration;
		af.bitvector	= 0;
		af.modifier		= 0;
		af.location		= APPLY_NONE;
		affect_to_char ( victim, &af);

		act_p("�� ������� $C4 ����� ������� ���!",
					ch,0,victim,TO_CHAR,POS_RESTING);
		act_p("$c1 ������ ���� ����� ������� ���.",
					ch,0,victim,TO_VICT,POS_RESTING);
		act_p("$c1 ������ $C4 ����� ������� ���.",
					ch,0,victim,TO_NOTVICT,POS_RESTING);
	} else
	{
		ch->send_to( "�� �� ������ ����� �������.\n\r" );
	};

	return;
}


SPELL_DECL(OpticResonance);
VOID_SPELL(OpticResonance)::run( Character *ch, Character *victim, int sn, int level )
{
    Character *rch;
    Character *target;
    int dam;
     
    if (victim->is_mirror())
	target = victim->doppel;
    else
	target = victim;
    
    if (target->in_room != victim->in_room) {
	ch->send_to( "����� ������ ��� ���������.\r\n" );
	return;
    }

    dam = dice( level, 5 );
    if (saves_spell( level, target, DAM_LIGHT, ch, DAMF_SPELL ))
	dam /= 2;

    damage( ch, target, ch->applyCurse( dam ), sn, DAM_LIGHT, true, DAMF_SPELL );
    
    if (target->is_npc( ))
	return;

    for (rch = victim->in_room->people; rch; rch = rch->next_in_room) {
	if (rch->is_mirror() && rch->doppel == target) {
	    act( "��� �����, ��������� $c5, ����������� �� �������, �������� ����!",
	         ch, 0, target, TO_VICT );
	    act( "��� �����, ��������� $c5, ����������� �� �������, �������� $C4!",
	         ch, 0, target, TO_NOTVICT );
	    act( "��� �����, ��������� �����, ����������� �� �������, �������� $C4!",
	         ch, 0, target, TO_CHAR );

	    dam = dice( level, 5 );

	    if (saves_spell( level, target, DAM_LIGHT, ch, DAMF_SPELL ))
		dam /= 2;
	    
	    try {
		damage_nocatch( ch, target, ch->applyCurse( dam ), sn, DAM_LIGHT, true, DAMF_SPELL );
	    } catch (const VictimDeathException &) {
		return;
	    }
	}
    }
}


/*
 * special guard behavior
 */
bool RulerSpecialGuard::specFight( )
{
    char buf[MAX_STRING_LENGTH];
    Character *victim;

    for (victim = ch->in_room->people; victim != 0; victim = victim->next_in_room)
	if ( !victim->is_npc()
		&& ( IS_SET(victim->act, PLR_WANTED)
			|| victim->isAffected(gsn_jail ) ) )
	{
	    break;
	}

    if (victim == 0)
	return false;

    ch->setClan( clan_ruler );
    interpret_raw(ch, "cb", "��������!!! %s ��������� %s � ������ %s",
		    victim->getNameP(), 
		    ch->in_room->name, 
		    ch->in_room->area->name);

    if ( ( ch->getModifyLevel() + 20 > victim->getModifyLevel() )
	    && !is_safe_nomessage ( ch, victim ) )
    {
	sprintf( buf, "%s CRIMINAL! ������� ��������!! ������ ������������!!",
		victim->getNameP( ) );
	do_yell( ch, buf );
	multi_hit( ch, victim );
    }
    else
    {
	sprintf( buf, "$c1 ������ '%s! �� ��� �������� �� ���� ������������!'",
		 victim->getNameP( ));
	act_p( buf, ch, 0, 0, TO_ROOM,POS_RESTING );
    }

    return true;
}

SPELL_DECL_T(GuardCall, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, GuardCall)::createMobile( Character *ch, int level ) const 
{
    return createMobileAux( ch, ch->getModifyLevel( ), 
				     2 * ch->max_hit, ch->max_mana,
				     number_range(level/18, level/14),
				     number_range(level/4, level/3),
				     number_range(level/10, level/8) );
} 

VOID_SPELL(GuardCall)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    if ( ch->isAffected(gsn_dismiss ) ) {
	ch->send_to( "� ���� �������� ���������� ������!\n\r" );
	return;
    }
    
    SummonCreatureSpell::run( ch, target_name, sn, level );
}

SPELL_DECL(KnowPersone);
VOID_SPELL(KnowPersone)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( ch->isAffected(gsn_dismiss ) )
    {
	ch->send_to( "� ���� �������� ���������� ������!\n\r" );
	return;
    }

    if (victim->is_npc( )) {
	NPCharacter *mob = victim->getNPC( );
	
	if (IS_SET(mob->pIndexData->area->area_flag, AREA_HOMETOWN)) {
	    ostringstream buf;
	    list<Room *> repops;

	    for (Room *room = room_list; room; room = room->rnext)
		for (RESET_DATA *pReset = room->reset_first; pReset; pReset = pReset->next)
		    if (pReset->command == 'M' && pReset->arg1 == mob->pIndexData->vnum) 
			repops.push_back( room );
	    
	    if (repops.size( ) == 1) {
		ch->printf( "%s ������� � ��������� ��� ��������� %s (%s).\r\n",
			    mob->getNameP( '1' ).c_str( ), 
			    repops.front( )->name, repops.front( )->area->name );
	    }
	    else if (repops.size( ) > 0) {
		act( "$C1 ����� ������� � ����� �� ��������� ����:", ch, 0, mob, TO_CHAR );

		for (list<Room *>::iterator r = repops.begin( ); r != repops.end( ); r++)
		    ch->printf( "    %s  (%s)\r\n", (*r)->name, (*r)->area->name );
	    }
	}
    }

    if (!victim->isAffected(gsn_doppelganger)) {
	act("�� �� ��������� �� ��������� $C2 ������ ����������.", ch, 0, victim, TO_CHAR);
    }
    else if (saves_spell( level, victim, DAM_MENTAL, ch, DAMF_SPELL)) {
	act("���� �� ������� ��������� ��� ������ $C2.", ch, 0, victim, TO_CHAR);
    }
    else {
	if (victim->is_mirror( )) 
	    ch->pecho( "%^C1 - ��� ����� ���� �������, ��������� %#^C5!",
	               victim, victim->doppel );
	else 
	    ch->pecho( "�� ���������, ��� ��� ������� %1$^C2 ���������� %1$#^C1!",
	               victim );
    }
}


SPELL_DECL(RemoveBadge);
VOID_SPELL(RemoveBadge)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  Object *badge;
  Object *obj_next;

  badge = 0;

  for ( badge = victim->carrying; badge != 0;
       badge = obj_next )
    {
      obj_next = badge->next_content;
      if (badge->pIndexData->vnum == OBJ_VNUM_DEPUTY_BADGE)
	{
	  act_p("���� $o1 ��������.",ch, badge, 0, TO_CHAR,POS_RESTING);
	  act_p("� $c2 �������� $o1.", ch, badge, 0, TO_ROOM,POS_RESTING);
	
	  obj_from_char(badge);
	  extract_obj(badge);
	  continue;
	}
    }
  return;

}




SPELL_DECL(RulerAura);
VOID_SPELL(RulerAura)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;

    if (!ch->isAffected(sn))
    {
      ch->send_to("������ �� ���������� ���� ����� ���������������, ����� �����.\n\r");

      af.where = TO_IMMUNE;
      af.type = sn;
      af.duration = level / 4;
      af.level = ch->getModifyLevel();
      af.bitvector = IMM_CHARM;
      af.location = 0;
      af.modifier = 0;
      affect_to_char(ch, &af);


      af.where		= TO_DETECTS;
      af.modifier = ch->applyCurse( level / 8 );
      af.location = APPLY_NONE;
      af.bitvector = DETECT_FADE;
      affect_to_char(ch, &af);

      af.where     = TO_DETECTS;
      af.location  = APPLY_NONE;
      af.modifier  = 0;
      af.bitvector = ACUTE_VISION;
      affect_to_char( ch, &af );

    }
  else
      ch->send_to("�� � ��� ��� ������ ������ � ���� ����, ������������� ������.\n\r");
 return;

}




SPELL_DECL(RulerBadge);
VOID_SPELL(RulerBadge)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Object *badge;
  Object *obj_next;
  Affect af;

  if ( (get_eq_char(ch, wear_neck_1)  != 0 ) &&
	(get_eq_char(ch, wear_neck_2)  != 0 ) )
  {
    ch->send_to("�� � ���� ��� ���-�� �����!\n\r");
    return;
  }

  for ( badge = ch->carrying; badge != 0;
       badge = obj_next )
    {
      obj_next = badge->next_content;
      if (badge->pIndexData->vnum == OBJ_VNUM_DEPUTY_BADGE)
	{
	  act_p("���� $o1 ��������.",ch, badge, 0, TO_CHAR,POS_RESTING);
	  obj_from_char(badge);
	  extract_obj(badge);
	  continue;
	}
    }


  badge = create_object( get_obj_index(OBJ_VNUM_DEPUTY_BADGE),level);
  badge->level = ch->getRealLevel( );

  af.where        = TO_OBJECT;
  af.type         = sn;
  af.level        = level;
  af.duration     = -1;
  af.modifier     = ch->applyCurse( level );
  af.bitvector    = 0;

  af.location     = APPLY_HIT;
  affect_to_obj( badge, &af);

  af.location     = APPLY_MANA;
  affect_to_obj( badge, &af);

  af.modifier     = ch->applyCurse( level / 8 );

  af.location     = APPLY_HITROLL;
  affect_to_obj( badge, &af);

  af.location     = APPLY_DAMROLL;
  affect_to_obj( badge, &af);


  badge->timer = 200;
  act_p("�� �������� ������ ��������� ������!",ch, 0, 0, TO_CHAR,POS_RESTING);
  act_p("$c1 ������� ������ ��������� ������!", ch, 0, 0, TO_ROOM,POS_RESTING);

  obj_to_char(badge,ch);
  wear_obj( ch, badge, 0 );
}




SPELL_DECL(ShieldOfRuler);
VOID_SPELL(ShieldOfRuler)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
  int shield_vnum;
  Object *shield;
  Affect af;

  if (level >= 71)
    shield_vnum = OBJ_VNUM_RULER_SHIELD4;
  else if (level >= 51)
    shield_vnum = OBJ_VNUM_RULER_SHIELD3;
  else if (level >= 31)
    shield_vnum = OBJ_VNUM_RULER_SHIELD2;
  else shield_vnum = OBJ_VNUM_RULER_SHIELD1;

  shield = create_object( get_obj_index(shield_vnum), level );
  shield->timer = level;
  shield->cost  = 0;
  obj_to_char(shield, ch);

  af.where        = TO_OBJECT;
  af.type         = sn;
  af.level        = level;
  af.duration     = -1;
  af.modifier     = ch->applyCurse( level / 8 );
  af.bitvector    = 0;

  af.location     = APPLY_HITROLL;
  affect_to_obj( shield, &af);

  af.location     = APPLY_DAMROLL;
  affect_to_obj( shield, &af);


  af.where        = TO_OBJECT;
  af.type         = sn;
  af.level        = level;
  af.duration     = -1;
  af.modifier     = ch->applyCurse( -level * 2 );
  af.bitvector    = 0;
  af.location     = APPLY_AC;
  affect_to_obj( shield, &af);

  af.where        = TO_OBJECT;
  af.type         = sn;
  af.level        = level;
  af.duration     = -1;
  af.modifier     = ch->applyCurse( max( 1, level /  30 ) );
  af.bitvector    = 0;
  af.location     = APPLY_CHA;
  affect_to_obj( shield, &af);

  act_p("�� ����������� ������ � �������� $o4!",
         ch,shield,0,TO_CHAR,POS_RESTING);
  act_p("$c1 ���������� ������ � ������� $o4!",
         ch,shield,0,TO_ROOM,POS_RESTING);

}




SPELL_DECL(Stalker);
VOID_SPELL(Stalker)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  NPCharacter *stalker;
  Affect af;
  int i;

	if ( ch->isAffected(gsn_dismiss ) )
	{
		ch->send_to( "� ���� �������� ���������� ������!\n\r" );
		return;
	}

  if (victim == ch || victim->in_room == 0
      || victim->is_npc() || !IS_SET(victim->act, PLR_WANTED))
    {
      ch->send_to("���� ������� ����������� ��������.\n\r");
      return;
    }

  if (ch->isAffected(sn))
    {
      ch->send_to("��� ���������� �������������� ������ �������.\n\r");
      return;
    }

  if( !is_safe_nomessage(ch,victim) )
    {
      ch->send_to("���� ����� ������������ ���������� ��� ���� ����.\n\r");
      return;
    }

	if ( victim->getClan() == clan_chaos )
	{
		ch->send_to("{x���, � {M������ {x���� �������� �������� ������� ����������!\n\r");
		return;
	}

  ch->send_to("�� ��������� �������� ���� � ������ �������� �� ��������.\n\r");
  act_p("$c1 �������� �������� ���� �� ������ �������� �� ��������.",ch,0,0,TO_ROOM,POS_RESTING);

  stalker = create_mobile( get_mob_index(MOB_VNUM_STALKER) );

  af.where		= TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 6;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  for (i=0;i < stat_table.size; i++)
    {
      stalker->perm_stat[i] = victim->perm_stat[i] * max( 2, ( level / 20 ) );
    }

  stalker->max_hit = ch->applyCurse( min( 30000, 5 * victim->max_hit ) );
  stalker->hit = stalker->max_hit;
  stalker->max_mana = ch->applyCurse( victim->max_mana );
  stalker->mana = stalker->max_mana;
  stalker->setLevel( victim->getModifyLevel() );

  stalker->damage[DICE_NUMBER] =
		ch->applyCurse( number_range( victim->getModifyLevel(), victim->getModifyLevel() / 8 ) );
  stalker->damage[DICE_TYPE] =
		ch->applyCurse( number_range( victim->getModifyLevel(), victim->getModifyLevel() / 2 ) );
  stalker->damage[DICE_BONUS] =
		ch->applyCurse( number_range( victim->getModifyLevel(), victim->getModifyLevel() / 6 ) );

  for (i=0; i < 3; i++)
    stalker->armor[i] = ch->applyCurse( interpolate( stalker->getRealLevel( ), 100, -400 ) );
  stalker->armor[3] = ch->applyCurse( interpolate( stalker->getRealLevel( ), 100, -200 ) );
  stalker->gold = 0;
  stalker->invis_level = LEVEL_HERO;
  stalker->detection = (A|B|C|D|E|F|G|H|ee);

  char_to_room(stalker,victim->in_room);

    if (!stalker->behavior || !stalker->behavior.getDynamicPointer<Stalker>( )) {
	extract_char( stalker );
	return;
    }

    stalker->behavior.getDynamicPointer<Stalker>( )->setVictimName( victim->getName( ) );

	if ( victim->isAffected(gsn_suspect ) )
	{
		affect_strip ( victim, gsn_suspect );
		victim->send_to ("���� �������� � ��� ����� ����� ��������!\n\r");
	}
  victim->send_to("������� �� �������� ������ �� �����!\n\r");
  act_p("������� �� �������� ���������, ����� ������ $c4!",
         victim,0,0,TO_ROOM,POS_RESTING);
  ch->send_to("������� �� �������� ������ �� �������.\n\r");
}

AFFECT_DECL(Jail);
VOID_AFFECT(Jail)::remove( Character *victim ) 
{
    Room *location;

    DefaultAffectHandler::remove( victim );

    clantalk( *clan_ruler, "� ����� ������� %s ����� �� �������!", victim->getNameP( '1' ).c_str() );

    if (victim->isAffected(gsn_manacles ))
	affect_strip( victim, gsn_manacles );

    act_p("$c1 ������$g��|�|�� ���� ����������� � ������������� �� ��� ������.", victim, 0, 0, TO_ROOM, POS_RESTING);
    act_p("�� ����� �� �������!", victim, 0, 0, TO_CHAR, POS_RESTING);

    if (victim->in_room
	&& victim->in_room->vnum >= 4343
	&& victim->in_room->vnum <= 4369)
    {
	if ( ( location = get_room_index( 4283 ) ) == 0 )
	{
	    victim->send_to("���... �����������-�� ���� - ������.\n\r");
	    return;
	}
	
	transfer_char( victim, 0, location,
		    NULL, NULL, "%1$^C1 ����������, ������ ������ �������." );
    }
}

VOID_AFFECT(Jail)::update( Character *ch, Affect *paf ) 
{
    DefaultAffectHandler::update( ch, paf );

    if ( paf->duration < 3) {
	clantalk(*clan_ruler, "%s ������ �� ������� ����� %d ���%s",
		ch->getNameP( '1' ).c_str(),
		paf->duration,
		GET_COUNT(paf->duration, "","�","��"));
    }
}

AFFECT_DECL(Manacles);
VOID_AFFECT(Manacles)::remove( Character *ch ) 
{
    DefaultAffectHandler::remove( ch );

    clantalk(*clan_ruler, "� %s ����� �������!", ch->getNameP( '2' ).c_str() );
}

VOID_AFFECT(Manacles)::update( Character *ch, Affect *paf ) 
{ 
    DefaultAffectHandler::update( ch, paf );

    if ( paf->duration < 3) {
	clantalk(*clan_ruler, "� %s ������ ������� ����� %d ���%s",
		ch->getNameP( '2' ).c_str(),
		paf->duration,
		GET_COUNT(paf->duration, "","�","��"));
    }
}


AFFECT_DECL(Suspect);
VOID_AFFECT(Suspect)::remove( Character *ch ) 
{ 
    DefaultAffectHandler::remove( ch );
    clantalk(*clan_ruler, "�������� � ��� ��� %s �������!", ch->getNameP( '2' ).c_str() );
}

VOID_AFFECT(Suspect)::update( Character *ch, Affect *paf ) 
{ 
    DefaultAffectHandler::update( ch, paf );

    if ( paf->duration < 3) {
	clantalk(*clan_ruler, "�������� � ��� ��� %s �������� ����� %d ���%s",
		ch->getNameP( '2' ).c_str(),
		paf->duration,
		GET_COUNT(paf->duration, "","�","��"));
    }
}

/*--------------------------------------------------------------------------
 * Stalker behavior 
 *-------------------------------------------------------------------------*/
Stalker::Stalker( ) 
{
}

void Stalker::clantalk( const char *msg )
{
    ch->setClan( clan_ruler );
    interpret_raw( ch, "cb", msg, victimName.getValue( ).c_str( ) );
}

bool Stalker::ourVictim( Character *vch )
{
    return !vch->is_npc( ) && vch->getName( ) == victimName.getValue( );
}

void Stalker::attackVictim( Character *vch )
{
    interpret_raw( ch, "yell", "%s, you criminal! Now you die!", victimName.getValue( ).c_str( ) );
    multi_hit_nocatch( ch, vch );
}

Character * Stalker::findVictimHere( )
{
    Character *rch;

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
	if (ourVictim( rch )) 
	    return rch;

    return rch;
}

Character * Stalker::findVictimWorld( )
{
    return PCharacterManager::findPlayer( victimName );
}

bool Stalker::canEnter( Room *const room )
{
    return !room->isPrivate( );
}

void Stalker::entry( ) 
{
    Character *vch = findVictimHere( );

    if (vch)
	attackVictim( vch );
}

void Stalker::greet( Character *vch ) 
{
    if (ourVictim( vch ))
	attackVictim( vch );
}

bool Stalker::specAdrenaline( ) 
{
    return specIdle( );
}

bool Stalker::specIdle( ) 
{ 
    Character *vch;
    Room *start_room = ch->in_room;
    
    memoryFought.clear( );
    clearLastFought( );

    vch = findVictimHere( );

    if (vch) {
	try {
	    attackVictim( vch );
	}
	catch (const VictimDeathException &) {
	}

	return false;
    }

    vch = findVictimWorld( );

    if (!vch) {
	clantalk( "To his shame, my victim has cowarldy left the realms. I must leave also." );
	extract_char( ch );
	return true;
    }
    
    path.clear( );
    pathToTarget( start_room, vch->in_room, 10000 );

    if (path.empty( )) {
	clantalk( "To my shame I have lost track of %s. I must leave." );
	extract_char( ch );
	return true; 
    }
    
    makeOneStep( );
    return false;
}

bool Stalker::death( Character *killer ) 
{
    clantalk( "� ������ ������� ����� %s! ��� ���� �� ������." );
    return false;
}

bool Stalker::kill( Character *vch ) 
{
    if (ourVictim( vch )) {
	clantalk( "I have killed my victim, now I can leave the realms." );
	ch->setDead( );
	return true;
    }

    return BasicMobileBehavior::kill( vch );
}


