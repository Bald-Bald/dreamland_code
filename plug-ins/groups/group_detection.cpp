/* $Id: group_detection.cpp,v 1.1.2.22.6.20 2010-09-01 21:20:45 rufina Exp $
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

#include "spelltemplate.h"
#include "skillcommandtemplate.h"

#include "char.h"
#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "liquid.h"
#include "magic.h"
#include "fight.h"
#include "interp.h"
#include "gsn_plugin.h"
#include "handler.h"
#include "comm.h"
#include "act_move.h"
#include "act_lock.h"

#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

GSN(none);
DLString quality_percent( int ); /* XXX */

/* From act_info.cpp */
void lore_fmt_item( Character *ch, Object *obj, ostringstream &buf, bool showName );
void lore_fmt_wear( int type, int wear, ostringstream &buf );
void lore_fmt_affect( Affect *paf, ostringstream &buf );

SPELL_DECL(AcuteVision);
VOID_SPELL(AcuteVision)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( CAN_DETECT(victim, ACUTE_VISION) )
    {
        if (victim == ch)
          ch->send_to("���� ������ ��� ��������� �� �������. \n\r");
        else
          act_p("������ $C2 ��� ��������� �� �������.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
        return;
    }
    af.where            = TO_DETECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ACUTE_VISION;
    affect_to_char( victim, &af );
    victim->send_to("���� ������ �����������.\n\r");
    if ( ch != victim )
        ch->send_to("Ok.\n\r");
    return;
}



SPELL_DECL(DetectEvil);
VOID_SPELL(DetectEvil)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_EVIL) )
    {
	if (victim == ch)
	  ch->send_to("�� ��� ���������� ����������� ����������� ���.\n\r");
	else
	  act_p("$C1 ��� ��������� ����������� ����������� ���.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }
    af.where     = TO_DETECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = (5 + level / 3);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = DETECT_EVIL;
    affect_to_char( victim, &af );
    victim->send_to("������ �� ���������� {D���{x.\n\r");
    if ( ch != victim )
	ch->send_to("Ok.\n\r");
    return;

}



SPELL_DECL(DetectGood);
VOID_SPELL(DetectGood)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_GOOD) )
    {
	if (victim == ch)
	  ch->send_to("�� ��� ���������� ����������� ������ ���.\n\r");
	else
	  act_p("$C1 ��� ��������� ����������� ������ ���.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }
    af.where     = TO_DETECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (5 + level / 3);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = DETECT_GOOD;
    affect_to_char( victim, &af );
    victim->send_to("������ �� ���������� ����������� {W�����{x.\n\r");
    if ( ch != victim )
	ch->send_to("Ok.\n\r");
    return;

}


SPELL_DECL(DetectHidden);
VOID_SPELL(DetectHidden)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_HIDDEN) )
    {
	if (victim == ch)
	  ch->send_to("�� ��� ���������� ����������� ������� ���. \n\r");
	else
	  act_p("$C1 ��� ��������� ����������� ������� ���.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }
    af.where     = TO_DETECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (5 + level / 3);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = DETECT_HIDDEN;
    affect_to_char( victim, &af );
    victim->send_to("������ �� ���������� ����������� ������� ���.\n\r");
    if ( ch != victim )
	ch->send_to("Ok.\n\r");
    return;

}


SPELL_DECL(DetectInvis);
VOID_SPELL(DetectInvis)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_INVIS) )
    {
	if (victim == ch)
	  ch->send_to("�� ��� ���������� ����������� ��������� ���.\n\r");
	else
	  act_p("$C1 ��� ��������� ����������� ��������� ���.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    af.where     = TO_DETECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (5 + level / 3);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = DETECT_INVIS;
    affect_to_char( victim, &af );
    victim->send_to("������ �� ���������� ����������� ��������� ���.\n\r");
    if ( ch != victim )
	ch->send_to("Ok.\n\r");
    return;

}


SPELL_DECL(DetectMagic);
VOID_SPELL(DetectMagic)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_MAGIC) )
    {
	if (victim == ch)
	  ch->send_to("�� ��� ���������� ����������� ���������� ���.\n\r");
	else
	  act_p("$C1 ��� ��������� ����������� ���������� ���.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    af.where     = TO_DETECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = (5 + level / 3);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = DETECT_MAGIC;
    affect_to_char( victim, &af );
    victim->send_to("���� ����� ����������.\n\r");
    if ( ch != victim )
	ch->send_to("Ok.\n\r");
    return;

}


SPELL_DECL(DetectPoison);
VOID_SPELL(DetectPoison)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if (IS_SET(obj->value[3], DRINK_POISONED))
	    ch->send_to("�� ���������� ����� ���.\n\r");
	else
	    ch->send_to("��� �������� ������ ���������.\n\r");
    }
    else
    {
	ch->send_to("��� �������� �� �����������.\n\r");
    }

    return;

}


SPELL_DECL(DetectUndead);
VOID_SPELL(DetectUndead)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(ch, DETECT_UNDEAD) )
    {
		ch->send_to("�� ��� ���������� ������.\n\r");

		return;
    }

    af.where     = TO_DETECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = (5 + level / 3);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = DETECT_UNDEAD;
    affect_to_char( victim, &af );
    ch->send_to("������ �� ���������� ������.\n\r");

    return;

}


Room * check_place( Character *ch, char *argument )
{
     char arg[MAX_INPUT_LENGTH];
     EXIT_DATA *pExit;
     Room *dest_room;
     int number, door;
     int range = ( ch->getModifyLevel() / 10) + 1;

     number = number_argument(argument,arg);
     if ((door = direction_lookup( arg )) < 0) 
	 return 0;

     dest_room = ch->in_room;

     while (number > 0) {
	number--;
	
	if (--range < 1) 
	    return 0;
	    
	if ( (pExit = dest_room->exit[door]) == 0
	  || !ch->can_see( pExit )
	  || IS_SET(pExit->exit_info,EX_CLOSED) )
	{    
	    break;
	}
	
	dest_room = pExit->u1.to_room;
	if (number < 1)    
	    return dest_room;
     }

     return 0;
}

SPELL_DECL(Farsight);
VOID_SPELL(Farsight)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    Room *room;

    if ( (room = check_place(ch,target_name)) == 0)
      {
	ch->send_to("��� ������ ���� �� �����.\n\r");
	return;
      }
    
    do_look_auto( ch, room );
}


SPELL_DECL(ImprovedDetect);
VOID_SPELL(ImprovedDetect)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_IMP_INVIS) )
    {
	if (victim == ch)
	  ch->send_to("�� ��� ���������� ����������� ����� ��������� ���.\n\r");
	else
	  act_p("$C1 ��� ��������� ����������� ����� ��������� ���.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
	return;
    }

    af.where     = TO_DETECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 3;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = DETECT_IMP_INVIS;
    affect_to_char( victim, &af );
    victim->send_to("������ �� ���������� ����������� ����� ��������� ���.\n\r");
    if ( ch != victim )
	ch->send_to("Ok.\n\r");
    return;

}

SPELL_DECL(KnowAlignment);
VOID_SPELL(KnowAlignment)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
	
	const char *msg;

	if ( IS_GOOD(victim) )
		msg = "$C1 ����� ������� � ������ ����.";
	else if ( IS_NEUTRAL(victim) )
		msg = "$C1 ����� ����� ����.";
	else
		msg = "$C1 - ���������� {D���{x!.";

	act_p( msg, ch, 0, victim, TO_CHAR,POS_RESTING);

	if (!victim->is_npc())
	{
		switch (victim->ethos.getValue( )) {
		case ETHOS_LAWFUL:
			msg = "$C1 ���� �����.";
			break;
		case ETHOS_NEUTRAL:
			msg = "$C1 ��������� � ������ ����������.";
			break;
		case ETHOS_CHAOTIC:
			msg = "$C1 ����� ����������� ��������.";
			break;
		default:
			msg = "$C1 ������� �� �����, ��� ���������� � �������.";
			break;
		}
		act_p( msg, ch, 0, victim, TO_CHAR,POS_RESTING);
	}
	return;

}


SPELL_DECL(LocateObject);
VOID_SPELL(LocateObject)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    char buf[MAX_INPUT_LENGTH];
    ostringstream buffer;
    Object *obj;
    Object *in_obj;
    bool found;
    int number = 0, max_found;

    found = false;
    number = 0;
    max_found = ch->is_immortal() ? 200 : 2 * level;

    for ( obj = object_list; obj != 0; obj = obj->next )
    {
	if ( !ch->can_see( obj ) || !is_name( target_name, obj->getName( ) )
	||   IS_OBJ_STAT(obj,ITEM_NOLOCATE) || number_percent() > 2 * level
	||   ch->getModifyLevel() < obj->level)
	    continue;

	found = true;
	number++;

	for ( in_obj = obj; in_obj->in_obj != 0; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != 0 && ch->can_see(in_obj->carried_by))
	{
	    sprintf( buf, "������� � %s\n\r",
		ch->sees(in_obj->carried_by, '2').c_str() );
	}
	else
	{
	    if (ch->is_immortal() && in_obj->in_room != 0)
		sprintf( buf, "��������� � %s [������� %d]\n\r",
		    in_obj->in_room->name, in_obj->in_room->vnum);
	    else
		sprintf( buf, "��������� � %s\n\r",
		    in_obj->in_room == 0
			? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = Char::upper(buf[0]);
	buffer << buf;

	if (number >= max_found)
	    break;
    }

    if ( !found )
	ch->send_to("� Dream Land ��� ������ �������� �� ���.\n\r");
    else
	page_to_char( buffer.str( ).c_str( ), ch );
}


SPELL_DECL(Observation);
VOID_SPELL(Observation)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  Affect	af;

  if( CAN_DETECT( victim, DETECT_OBSERVATION ) ) {
    ch->send_to("�� ��� ��������� ��������� ������.\n\r");
    return;
  }

  af.where	= TO_DETECTS;
  af.type	= sn;
  af.level	= level;
  af.duration	= ( 10 + level / 5 );
  af.location	= APPLY_NONE;
  af.modifier	= 0;
  af.bitvector	= DETECT_OBSERVATION;
  affect_to_char( victim, &af );
  ch->send_to("������ �� ��������� ��������� ������.\n\r");
  return;

}


/*
 * 'detect hide' skill command
 */

SKILL_RUNP( detect )
{
	Affect af;

	if (!gsn_detect_hide->usable( ch ))
	{
		ch->send_to( "����?\n\r");
		return;
	}

	if ( CAN_DETECT(ch, DETECT_HIDDEN) )
	{
		ch->send_to("�� ��� ���������� ����������� ������� ���. \n\r");
		return;
	}

	ch->setWait( gsn_detect_hide->getBeats( ) );
	
	if ( number_percent( ) > gsn_detect_hide->getEffective( ch ) )
	{
		ch->send_to("�� ��������� ������� ������� � ����, �� � ���� ������ �� �������.\n\r");
		gsn_detect_hide->improve( ch, false );
		return;
	}

	af.where     = TO_DETECTS;
	af.type      = gsn_detect_hide;
	af.level     = ch->getModifyLevel();
	af.duration  = ch->getModifyLevel();
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = DETECT_HIDDEN;
	affect_to_char( ch, &af );
	ch->send_to( "���� ����������������� ����������.\n\r");
	gsn_detect_hide->improve( ch, true );
	return;
}


SPELL_DECL(Identify);
VOID_SPELL(Identify)::run( Character *ch, Object *obj, int sn, int level ) 
{
    ostringstream buf;

    lore_fmt_item( ch, obj, buf, true );
    ch->send_to( buf );
}

#if 0
void lore_fmt_obj( Object *obj, ostringstream &buf, 
		   int weight,
		   int cost,
		   const char *material,
		   int level,
		   int type,
		   int extra,
		   int limit )
{
    DLString itype;

    buf << "������, ��� ��� ���� ����������� �� ����� {W" << obj->getName( ) << "{x." << endl;

    if (weight != -1) {
	weight /= 10;

	if (weight == 0)
	    buf << "������, ������ ������ �� �����." << endl;
	else
	    buf << "�� ��� ��� ����� {W" << weight << "{x ����" << GET_COUNT(weight, "�", "��", "��") << "." << endl;
    }

    if (cost != -1)
	if (cost == 0)
	    buf << "�������, ��� ���� ������ �� �����." << endl;
	else
	    buf << "��������� ��������� - {W" << cost << "{x �������." << endl;

    if (material && strcmp( material, "none" ) && strcmp( material, "oldstyle" ))
	buf << "�������� ���������� {W" << material << "{x." << endl;

    if (level != -1)
	buf << "��������������� ������� - {W" << level << "{x." << endl;
    
    itype = item_table.message(type );
    if (!itype.empty( ))
	buf << "�� ��������� ��������� �� �������, ��� ��� {W" << itype << "{x." << endl;

    if (extra > 0)
	buf << "���������� ������ ��������� ��������� ��������: {W" << extra_flags.messages(extra, true ) << "{x." << endl;

    if (limit != -1 && limit < 100)
	buf << "����� �����, ��� ��� ����� ������ ���� - ���������� �� �������� � ���� �� ��������� {W" << limit << "{x!" << endl;
}

int lore_rnd_value( int x, int learned )
{
    learned = 100 - learned;
    x = number_range( x - x * learned / 100, x + x * learned / 100 );
    return x;
}

/*
 * 'lore' skill command
 */

SKILL_RUNP( lore )
{
    Object *obj;
    ostringstream buf;
    int learned;
    int weight, cost, level, type, extra, wear, limit;
    const char *material;
    int value[5], i;
    
    if (( obj = get_obj_carry( ch, argument ) ) == 0) {
	ch->send_to("� ���� ��� �����.\n\r");
	return;
    }

    learned = gsn_lore->getEffective( ch );
    weight = cost = level = type = extra = wear = limit = -1;
    material = NULL;
    
    for (i = 0; i < 5; i++)
	value[i] = -1;

    if (learned < 10) {
	act("�� ������������ ����������� �����$g�|��|�� ��������� ������, "
	    "� ������ ������� ���-�� ������ � $o6.", ch, obj, 0, TO_CHAR);
	return;
    }

    if (learned > 20) {
	weight   = lore_rnd_value( obj->weight, learned );
	cost     = lore_rnd_value( obj->cost, learned );
	material = obj->getMaterial( );
    }

    if (learned > 40) {
	weight   = obj->weight;
	level    = lore_rnd_value( obj->level, learned );
    } 

    if (learned > 60) {
	type     = lore_rnd_value( obj->item_type, learned );
    }
    
    if (learned > 80) {
	type     = obj->item_type;
	extra    = lore_rnd_value( obj->extra_flags, learned );
	wear     = lore_rnd_value( obj->wear_flags, learned );
    }

    if (learned > 90) {
	extra    = obj->extra_flags;
	wear     = obj->wear_flags;
	limit    = obj->pIndexData->limit;
    }

    if (learned >= 100) {
	cost     = obj->cost;
	level    = obj->level;
    }
    
    act( "�� ����������� �������������� $o4, ���������� ���, ��� �����-���� "
         "�����$g��|�|�� � �������� �����.", ch, obj, 0, TO_CHAR );

    lore_fmt_obj( obj, buf, weight, cost, material, level, type, extra, limit );
    lore_fmt_wear( type, wear, buf );

    ch->send_to( buf );
}
#endif

/*
 * 'lore' skill command
 */

SKILL_RUNP( lore )
{
  char arg1[MAX_INPUT_LENGTH];
  Object *obj;
  char buf[MAX_STRING_LENGTH];
  Affect *paf;
  int chance;
  int value0, value1, value2, value3;
  int mana, learned;
  Keyhole::Pointer keyhole;

  argument = one_argument( argument, arg1 );

  if ( ( obj = get_obj_carry( ch, arg1 ) ) == 0 )
    {
      ch->send_to("� ���� ��� �����.\n\r");
      return;
    }

    mana = gsn_lore->getMana( );
    learned = gsn_lore->getEffective( ch );
    
  if (ch->mana < mana)
    {
      ch->send_to("� ���� ������������ ������� ��� �����.\n\r");
      return;
    }

  if (!gsn_lore->usable( ch ) || learned < 10)
    {
      ch->send_to("�� ������������ ������ ������ �������.\n\r");
      return;
    }

	if ( IS_SET(obj->extra_flags, ITEM_NOIDENT) )
	{
		sprintf( buf,"������ '{G%s{x', ��� %s\n\r, ��� %d, ������� %d.\n\r",
			obj->getName( ),
			item_table.message(obj->item_type).c_str( ),
			obj->weight / 10,
			obj->level);
		ch->send_to(buf);
		ch->send_to("\n\r����� ��� ��� ���� ���������� ������ �������.\n\r");
		return;
	}

  /* a random lore */
  chance = number_percent();

    bitstring_t extra = obj->extra_flags; /* TODO different flags on diff lore levels */
    REMOVE_BIT(extra, ITEM_WATER_STAND|ITEM_INVENTORY|ITEM_HAD_TIMER|ITEM_DELETED);

  if (learned < 20)
    {
      sprintf( buf, "������: '%s'.\n\r", obj->getName( ));
      ch->send_to(buf);
      ch->mana -= mana;
      gsn_lore->improve( ch, true );
      return;
    }

  else if (learned < 40)
    {
      sprintf( buf,
	  "������: '%s'.  ���: %d.  ���������: %d.\n\r",
	      obj->getName( ),
	      chance < 60 ? obj->weight : number_range(1, 2 * obj->weight),
	      chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost
	      );
      ch->send_to(buf);
      if ( str_cmp( obj->getMaterial( ), "oldstyle" ) )  {
        sprintf( buf, "��������: %s.\n\r", obj->getMaterial( ));
        ch->send_to(buf);
      }
      ch->mana -= mana;
      gsn_lore->improve( ch, true );
      return;
    }

  else if (learned < 60)
    {
      sprintf( buf,
	      "������: '%s'.  ���: %d.\n\r���������: %d.  �������: %d.\n\r��������: %s.\n\r",
	      obj->getName( ),
	      obj->weight,
	      chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost,
	      chance < 60 ? obj->level : number_range(1, 2 * obj->level),
	  str_cmp(obj->getMaterial( ),"oldstyle")?obj->getMaterial( ):"unknown"
	      );
      ch->send_to(buf);
      ch->mana -= mana;
      gsn_lore->improve( ch, true );
      return;
    }

  else if (learned < 80)
    {
      sprintf( buf,
	      "������: '%s'.  ���: %s.\n\r�����������: %s.\n\r���: %d.  ���������: %d.  �������: %d.\n\r��������: %s.\n\r",
	      obj->getName( ),
	      item_table.message(obj->item_type).c_str( ),
	      extra_flags.messages( extra, true ).c_str( ),
	      obj->weight,
	      chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost,
	      chance < 60 ? obj->level : number_range(1, 2 * obj->level),
	  str_cmp(obj->getMaterial( ),"oldstyle")?obj->getMaterial( ):"unknown"
	      );
      ch->send_to(buf);
      ch->mana -= mana;
      gsn_lore->improve( ch, true );
      return;
    }

  else if (learned < 85)
    {
      sprintf( buf,
	      "������: '%s'.  ���: %s.\r\n�����������: %s.\n\r���: %d.  ���������: %d.  �������: %d.\n\r��������: %s.\n\r",
	      obj->getName( ),
	      item_table.message(obj->item_type).c_str( ),
	      extra_flags.messages( extra, true ).c_str( ),
	      obj->weight,
	      obj->cost,
	      obj->level,
	  str_cmp(obj->getMaterial( ),"oldstyle")?obj->getMaterial( ):"unknown"
	      );
      ch->send_to(buf);
    }
  else
    {
      sprintf( buf,
	      "������: '%s'.  ���: %s.\r\n�����������: %s.\n\r���: %d.  ���������: %d.  �������: %d.\n\r��������: %s.\n\r",
	      obj->getName( ),
	      item_table.message(obj->item_type).c_str( ),
	      extra_flags.messages( extra, true ).c_str( ),
	      obj->weight,
	      obj->cost,
	      obj->level,
	  str_cmp(obj->getMaterial( ),"oldstyle")?obj->getMaterial( ):"unknown"
	      );
      ch->send_to(buf);
    }

  ch->mana -= mana;

  value0 = obj->value[0];
  value1 = obj->value[1];
  value2 = obj->value[2];
  value3 = obj->value[3];

  switch ( obj->item_type )
    {
    case ITEM_KEY:
	if (( keyhole = Keyhole::locate( ch, obj ) )) {
	    ostringstream buf;
	    
	    if (keyhole->doLore( buf ) )
		ch->send_to( buf );
	}
	break;
    case ITEM_KEYRING:
	if (learned < 85) 
	    value0 = number_fuzzy( obj->value[0] );
	else 
	    value0 = obj->value[0];
	
	ch->pecho( "����� �������� %1$d ���%1$I�|��|���.", value0 );
	break;
    case ITEM_LOCKPICK:
	if (learned < 85) {
	    value0 = number_fuzzy( obj->value[0] );   
	    value1 = number_fuzzy( obj->value[1] );
	}
	else {
	    value0 = obj->value[0];
	    value1 = obj->value[1];
	}
	
	if (value0 == Keyhole::LOCK_VALUE_BLANK) {
	    ch->println( "��� ��������� ��� ����� ��� �������." );
	}
	else {
	    if (value0 == Keyhole::LOCK_VALUE_MULTI)
		ch->send_to( "��������� ����� �����. " );
	    else
		ch->send_to( "��������� ���� �� ����� ������. " );
	    
	    ch->printf( "������� %s ��������.\r\n", 
			quality_percent( value1 ).colourStrip( ).ruscase( '2' ).c_str( ) );
	}
	break;
	
    case ITEM_SPELLBOOK:
	if (learned < 85) {
	    value0 = number_fuzzy( obj->value[0] );
	    value1 = number_fuzzy( obj->value[1] );
	    value2 = number_range( 1, 100 );
	}
	else {
	    value0 = obj->value[0];
	    value1 = obj->value[1];
	    value2 = number_fuzzy( obj->value[2] );
	}
	
	ch->printf( "�������: %d �� %d. ������������ �������� ������ %d%%.\r\n",
	             value1, value0, value2 ); 
	break;

    case ITEM_TEXTBOOK:
	if (learned < 85) {
	    value0 = number_fuzzy( obj->value[0] );
	    value1 = number_fuzzy( obj->value[1] );
	    value2 = number_range( 1, 100 );
	}
	else {
	    value0 = obj->value[0];
	    value1 = obj->value[1];
	    value2 = number_fuzzy( obj->value[2] );
	}
	ch->printf( "�������: %d �� %d. ������������ �������� ������� %d%%.\r\n",
	             value1, value0, value2 ); 
        break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
      if (learned < 85)
	{
	  value0 = number_range(1, 60);
	  if (chance > 40) {
	    value1 = number_range(1, (SkillManager::getThis( )->size() - 1));
	    if (chance > 60) {
	      value2 = number_range(1, (SkillManager::getThis( )->size() - 1));
	      if (chance > 80)
		value3 = number_range(1, (SkillManager::getThis( )->size() - 1));
	    }
	  }
	}
      else
	{
	  if (chance > 60) {
	    value1 = number_range(1, (SkillManager::getThis( )->size() - 1));
	    if (chance > 80) {
	      value2 = number_range(1, (SkillManager::getThis( )->size() - 1));
	      if (chance > 95)
		value3 = number_range(1, (SkillManager::getThis( )->size() - 1));
	    }
	  }
	}

      sprintf( buf, "������� %d ����������:", obj->value[0] );
      ch->send_to(buf);

      if (value1 >= 0 && value1 < SkillManager::getThis( )->size() && value1 != gsn_none)
	{
	  ch->send_to(" '");
	  ch->send_to(SkillManager::getThis( )->find(value1)->getNameFor( ch ));
	  ch->send_to("'");
	}

      if (value2 >= 0 && value2 < SkillManager::getThis( )->size() && value2 != gsn_none)
	{
	  ch->send_to(" '");
	  ch->send_to(SkillManager::getThis( )->find(value2)->getNameFor( ch ));
	  ch->send_to("'");
	}

      if (value3 >= 0 && value3 < SkillManager::getThis( )->size() && value3 != gsn_none)
	{
	  ch->send_to(" '");
	  ch->send_to(SkillManager::getThis( )->find(value3)->getNameFor( ch ));
	  ch->send_to("'");
	}

      ch->send_to(".\n\r");
      break;

    case ITEM_WAND:
    case ITEM_STAFF:
      if (learned < 85)
	{
	  value0 = number_range(1, 60);
	  if (chance > 40) {
	    value3 = number_range(1, (SkillManager::getThis( )->size() - 1));
	    if (chance > 60) {
	      value2 = number_range(0, 2 * obj->value[2]);
	      if (chance > 80)
		value1 = number_range(0, value2);
	    }
	  }
	}
      else
	{
	  if (chance > 60) {
	    value3 = number_range(1, (SkillManager::getThis( )->size() - 1));
	    if (chance > 80) {
	      value2 = number_range(0, 2 * obj->value[2]);
	      if (chance > 95)
		value1 = number_range(0, value2);
	    }
	  }
	}

      sprintf( buf, " %d(%d) ���������� %d-�� ������",
	      value1, value2, value0 );
      ch->send_to(buf);

      if (value3 >= 0 && value3 < SkillManager::getThis( )->size() && value3 != gsn_none)
	  {
	    ch->send_to(" '");
	    ch->send_to(SkillManager::getThis( )->find(value3)->getNameFor( ch ));
	    ch->send_to("'");
	  }

      ch->send_to(".\n\r");
      break;

    case ITEM_WEAPON:
      ch->send_to("��� ������: ");
      if (learned < 85)
	{
	  value0 = number_range(0, 8);
	  if (chance > 33) {
	    value1 = number_range(1, 2 * obj->value[1]);
	    if (chance > 66)
	      value2 = number_range(1, 2 * obj->value[2]);
	  }
	}
      else
	{
	  if (chance > 50) {
	    value1 = number_range(1, 2 * obj->value[1]);
	    if (chance > 75)
	      value2 = number_range(1, 2 * obj->value[2]);
	  }
	}

	ch->printf("%s (%s)\r\n",
		   weapon_class.message(value0 ).c_str( ),
		   weapon_class.name( value0 ).c_str( )
		  );

	sprintf(buf,"����������� %dd%d (������� %d).\n\r",
		value1,value2,
		(1 + value2) * value1 / 2);
      ch->send_to(buf);
      if (learned > 85)
        if (obj->value[4])  /* weapon flags */
        {
          sprintf(buf,"����� ������:%s.\n\r",weapon_type2.messages(obj->value[4]).c_str( ));
          ch->send_to(buf);
        }

      break;

    case ITEM_ARMOR:
      if (learned < 85)
	{
	  if (chance > 25) {
	    value2 = number_range(0, 2 * obj->value[2]);
	      if (chance > 45) {
		value0 = number_range(0, 2 * obj->value[0]);
		  if (chance > 65) {
		    value3 = number_range(0, 2 * obj->value[3]);
		      if (chance > 85)
			value1 = number_range(0, 2 * obj->value[1]);
		  }
	      }
	  }
	}
      else
	{
	  if (chance > 45) {
	    value2 = number_range(0, 2 * obj->value[2]);
	      if (chance > 65) {
		value0 = number_range(0, 2 * obj->value[0]);
		  if (chance > 85) {
		    value3 = number_range(0, 2 * obj->value[3]);
		      if (chance > 95)
			value1 = number_range(0, 2 * obj->value[1]);
		  }
	      }
	  }
	}

      sprintf( buf,
	      "����� ������: %d ����  %d ����  %d ����������  %d vs. �����.\n\r",
	      value0, value1, value2, value3 );
      ch->send_to(buf);
      break;
    }

  /* wear location */
    ostringstream ostr;
    lore_fmt_wear( obj->item_type, obj->wear_flags, ostr );
    ch->send_to( ostr );
/* */

  if (learned < 87)
  {
    gsn_lore->improve( ch, true );
    return;
  }

  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != 0; paf = paf->next )
      {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	  {
	    sprintf( buf, "��������: %s �� %d.\n\r",
		    apply_flags.message( paf->location ).c_str( ), paf->modifier );
	    ch->send_to(buf);
	  }
      }

  for ( paf = obj->affected; paf != 0; paf = paf->next )
    {
      if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	  sprintf( buf, "��������: %s �� %d",
		  apply_flags.message( paf->location ).c_str( ), paf->modifier );
	  ch->send_to(buf);
          if ( paf->duration > -1)
              sprintf(buf,", � ������� %d �����.\n\r",paf->duration);
          else
              sprintf(buf,".\n\r");
          ch->send_to(buf);
	}
    }
  // check for limited
    if ( obj->pIndexData->limit != -1 )
    {
        sprintf(buf,
        "{R�������� � ���� �� �����: %d.\n\r{x",
         obj->pIndexData->limit);
         ch->send_to(buf);
    }

  gsn_lore->improve( ch, true );
  return;
}

