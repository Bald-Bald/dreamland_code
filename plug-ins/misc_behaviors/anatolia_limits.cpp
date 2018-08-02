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
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT		           *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Mandrake}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

#include "anatolia_limits.h"

#include "affect.h"
#include "room.h"
#include "pcharacter.h"
#include "character.h"
#include "object.h"

#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "magic.h"
#include "gsn_plugin.h"
#include "effects.h"
#include "handler.h"
#include "fight.h"
#include "damage_impl.h"
#include "def.h"

/*
 * Excalibur, Dwarven Catacombs
 */
void Excalibur::wear( Character *ch )
{
  act_p("$o1 ���������� ������������ ����� ������.", ch,obj,0,TO_CHAR,POS_RESTING);
  act_p("$o1 ���������� ������������ ����� ������.", ch,obj,0,TO_ROOM,POS_RESTING);
  
}

void Excalibur::equip( Character *ch )
{
  short level = ch->getModifyLevel();

  if ( level > 20 && level <= 30)	obj->value[2] = 4;
  else if ( level > 30 && level <= 40)   obj->value[2] = 5;
  else if ( level > 40 && level <= 50)   obj->value[2] = 6;
  else if ( level > 50 && level <= 60)   obj->value[2] = 8;
  else if ( level > 60 && level <= 70)   obj->value[2] = 10;
  else if ( level > 70 && level <= 80)   obj->value[2] = 11;
  else obj->value[2] = 12;
}

void Excalibur::remove( Character *ch )
{
  act_p("�������� ���� ������ $o2 ��������.",ch,obj,0,TO_CHAR,POS_RESTING);
  act_p("�������� ���� ������ $o2 ��������.",ch,obj,0,TO_ROOM,POS_RESTING);
}

bool Excalibur::death( Character *ch )
{
    if (obj->wear_loc != wear_wield && obj->wear_loc != wear_second_wield)
	return false;

    act_p("$o1 �������� ��������� ������� ��������.", ch,obj,0,TO_CHAR,POS_DEAD);
    act_p("$o1 �������� ��������� ������� ��������.", ch,obj,0,TO_ROOM,POS_RESTING);
    ch->hit = ch->max_hit;
    ch->send_to("�� ���������� ���� ������� �����.");
    act_p("$c1 �������� ������� �����.",ch,0,0,TO_ROOM,POS_RESTING);
    return true;
}

void Excalibur::speech( Character *ch, const char *speech )
{
    if (ch != obj->getCarrier( ))
	return;

  if ((!str_cmp(speech, "sword of acid") || !str_cmp(speech, "��� �������"))
      && (ch->fighting) && ((get_eq_char(ch,wear_wield) == obj) ||
			(get_eq_char(ch,wear_second_wield) == obj)  ) )
    {
      ch->send_to("������ ���������� ������� ��������.\n\r");
      act_p("������� ������� � ������ ����������.",
             ch,0,0,TO_ROOM,POS_RESTING);
      spell(gsn_acid_blast, ch->getModifyLevel(),ch,ch->fighting, FSPELL_BANE );
      ch->setWaitViolence( 2 );
    }
}

bool Excalibur::sac( Character *ch )
{
  act("{R���� � �����!{x",ch,0,0,TO_ALL);
  rawdamage( ch, ch, DAM_HOLY, (ch->hit - 1) > 1000 ? 1000 : (ch->hit - 1), true );
  ch->gold = 0;
  return true;
}

/*
 * Musical bracers, bracers of energy, glinting silver bracelet
 */
void HasteBracers::wear( Character *ch ) 
{
  if( !( ch->isAffected(gsn_haste ) || ch->isAffected(gsn_transform ) ) ) {
      ch->send_to("������ ��������� ����� ����� ���, ������ �������� � ����.\n\r");
      ch->send_to("���� ���� ����������� ������������ ���������.\n\r");
    }
}
void HasteBracers::equip( Character *ch ) 
{
  Affect af;
  short level = ch->getModifyLevel();

  if( !( ch->isAffected(gsn_haste ) || ch->isAffected(gsn_transform ) ) ) {
      af.where = TO_AFFECTS;
      af.type = gsn_haste;
      af.duration = -2;
      af.level = level;
      af.bitvector = AFF_HASTE;
      af.location = APPLY_DEX;
      af.modifier = 1 + ( level >= 18 ) + ( level >= 30) + ( level >= 45 );
      affect_to_char(ch, &af);
    }
}

void HasteBracers::remove( Character *ch ) 
{
  if (ch->isAffected(gsn_haste))
    {
      affect_strip(ch, gsn_haste);
      ch->send_to("�� ����� �������� ������� � �����.\n\r");
    }
}

/*
 * Mudschool weapon
 */
void SubissueWeapon::fight( Character *ch )
{
    int hp;
    
    if (obj->wear_loc != wear_wield)
	return;

    if (chance( 70 ))
	return;
    
    hp = HEALTH(ch);
    
    if (hp > 90)
	ch->send_to("���� ������ �������, {Y'��� ���� �����������!'{x\n\r");
    else if (hp > 60)
	ch->send_to("���� ������ �������, {Y'��� �������! ������� ������!'{x\n\r");
    else if (hp > 40)
	ch->send_to("���� ������ �������, {Y'�� ������ ������� ���!'{x\n\r");
    else
	ch->send_to("���� ������ �������, {Y'����� �����! �������� ������!'{x\n\r");
}

/*
 * Two snake headed whip, Drow City
 */
void TwoSnakeWhip::wear( Character *ch )
{
  act_p("{G���� �� ������ �������� ���� ���.{x",
		ch,obj,0,TO_CHAR,POS_DEAD);
  act_p("{G���� �� ������ �������� ���� ���.{x",
		ch,obj,0,TO_ROOM,POS_DEAD);
}

void TwoSnakeWhip::equip( Character *ch )
{
  short level = ch->getModifyLevel();

  if (  level <= 10)			obj->value[2] = 3;
  else if ( level > 10 && level <= 20)   obj->value[2] = 4;
  else if ( level > 20 && level <= 30)   obj->value[2] = 5;
  else if ( level > 30 && level <= 40)   obj->value[2] = 6;
  else if ( level > 40 && level <= 50)   obj->value[2] = 7;
  else if ( level > 50 && level <= 60)   obj->value[2] = 8;
  else if ( level > 60 && level <= 70)   obj->value[2] = 9;
  else if ( level > 70 && level <= 80)   obj->value[2] = 10;
  else obj->value[2] = 11;
  return;
}

void TwoSnakeWhip::remove( Character *ch )
{
  act_p("{r���� �� ������ �������� ������������ ������ ����.{x",
		ch,obj,0,TO_CHAR,POS_DEAD);
  act_p("{r���� �� ������ �������� ������������ ������ ����.{x",
		ch,obj,0,TO_ROOM,POS_DEAD);
}

void TwoSnakeWhip::get( Character *ch )
{
  act_p("���� �������, ����� ���� �� ������ ������������.",ch,obj,0,TO_CHAR,POS_RESTING);
}

void TwoSnakeWhip::fight( Character *ch )
{
  if ( (get_eq_char(ch, wear_wield) == obj) ||
	( get_eq_char(ch,wear_second_wield) == obj) )
    {
      switch(number_bits(7)) {
      case 0:
  	act_p("���� �� ���� �� ����� ������ ����� $C4!", ch, 0,
		ch->fighting, TO_CHAR,POS_RESTING);
	act_p("���� � ������ $c2 �������� ����� ����!", ch, 0,
		ch->fighting, TO_VICT,POS_RESTING);
	act_p("���� � ������ $c2 ����� $C4!", ch, 0,
		ch->fighting, TO_NOTVICT,POS_RESTING);
	spell(gsn_poison, ch->getModifyLevel(), ch, ch->fighting, FSPELL_BANE );
	break;
      case 1:
  	act_p("���� �� ���� �� ����� ������ ����� $C4!", ch, 0,
		ch->fighting, TO_CHAR,POS_RESTING);
	act_p("���� � ������ $c2 �������� ����� ����!", ch, 0,
		ch->fighting, TO_VICT,POS_RESTING);
	act_p("���� � ������ $c2 ����� $C4!", ch, 0,
		ch->fighting, TO_NOTVICT,POS_RESTING);
	spell(gsn_weaken, ch->getModifyLevel(), ch, ch->fighting, FSPELL_BANE );
	break;
      }
    }
}

/*
 * Hammer thunderbolt, Olympus
 */
void Thunderbolt::fight( Character *ch )
{
    if ( (get_eq_char(ch, wear_wield) == obj) ||
	(get_eq_char(ch,wear_second_wield) == obj) )
    {
	int dam;
	Character *victim = ch->fighting;
	int level = ch->getModifyLevel( );

	switch(number_bits(6)) {
	case 0:
	    act("������ ������ �������� �� ������ ���� � �������� $C4!", ch, 0, victim, TO_CHAR);
	    act("������ ������ ������������ ����� ������ ���� $c2 � ���������� � ���� �������!", ch, 0, victim, TO_VICT);
	    act("������ ������ �������� �� ���� $c2, ��������� � ������� $C2!", ch, 0, victim, TO_NOTVICT);

	    dam = dice(level,4) + 12;
	    if ( saves_spell( level, victim,DAM_LIGHTNING,ch, DAMF_SPELL) )
		dam /= 2;
	    damage( ch, victim, dam, gsn_lightning_bolt, DAM_LIGHTNING , false, DAMF_SPELL);
	    break;
	}
    }
}


/*
 * Firegauntlets, Olympus
 */
void FireGauntlets::fight( Character *ch )
{
int dam;

  if ( !(get_eq_char( ch, wear_wield ) == 0  &&
	get_eq_char( ch, wear_second_wield) == 0) )
	return;

  if ( get_eq_char( ch, wear_hands ) != obj )
	return;
  if ( ch->is_npc() )
	return;

    if ( number_percent() < 50 )  {
	dam = number_percent()/2 + 30 + 2 * ch->getModifyLevel();
	act_p( "���� �������� �������� ���� $C3!",
		ch, 0, ch->fighting, TO_CHAR,POS_RESTING);
	act_p( "�������� $c2 �������� ���� $C3!",
		ch, 0, ch->fighting, TO_NOTVICT,POS_RESTING);
	act_p( "�������� $C2 �������� ���� ����!",
		ch->fighting, 0, ch, TO_CHAR,POS_RESTING);
	damage( ch, ch->fighting, dam/2, gsn_burning_hands, DAM_FIRE, false);
	if ( ch == 0 || ch->fighting == 0 )
	  return;
	fire_effect( ch->fighting, obj->level/2, dam/2, TARGET_CHAR );
    }
}

void FireGauntlets::wear( Character *ch )
{
    ch->send_to("���� ���� ����������� �� ��������.\n\r");
}

void FireGauntlets::remove( Character *ch )
{
    ch->send_to("���� ���� ������������� ��������.\n\r");
}

/*
 * Armbands of volcanoe, Isles
 */
void VolcanoeArmbands::fight( Character *ch )
{
int dam;
  if ( get_eq_char( ch, wear_arms ) != obj )
	return;

  if ( ch->is_npc() )
	return;

  if ( number_percent() < 20 )  {
	dam = number_percent()/2 + 30 + 5 * ch->getModifyLevel();
	act_p( "���� ����������� �������� $C4!",
                ch, 0, ch->fighting, TO_CHAR,POS_RESTING);
	act_p( "����������� $c2 �������� $C4!",
                ch, 0, ch->fighting, TO_NOTVICT,POS_RESTING);
	act_p( "����������� $C2 �������� ����!",
                ch->fighting, 0, ch, TO_CHAR,POS_RESTING);
	damage( ch, ch->fighting, dam, gsn_burning_hands, DAM_FIRE, false);
	if ( ch == 0 || ch->fighting == 0 )
	  return;
	fire_effect( ch->fighting, obj->level/2, dam, TARGET_CHAR );
  }
  return;
}

void VolcanoeArmbands::wear( Character *ch )
{
    ch->send_to("���� ���� ��������� �����.\n\r");
}

void VolcanoeArmbands::remove( Character *ch )
{
    ch->send_to("���� ���� ����� ������������� ��������.\n\r");
}


/*
 * Demonfire shield, UnderDark
 */
void DemonfireShield::fight( Character *ch )
{
int dam;

  if ( get_eq_char( ch, wear_shield ) != obj )
	return;
  if ( ch->is_npc() )
	return;

  if ( number_percent() < 15 )  {
	dam = number_percent()/2 + 5 * ch->getModifyLevel();

	act_p( "���� ��� �������� ���� $C3!", ch, 0, ch->fighting, TO_CHAR,POS_RESTING);
	act_p( "��� $c2 �������� ���� $C3!", ch, 0, ch->fighting, TO_NOTVICT,POS_RESTING);
	act_p( "��� $C2 �������� ���� ����!", ch->fighting, 0, ch, TO_CHAR,POS_RESTING);

	damage( ch, ch->fighting, dam, gsn_demonfire, DAM_FIRE, false);
	if ( ch == 0 || ch->fighting == 0 )
	  return;
	fire_effect( ch->fighting, obj->level,dam, TARGET_CHAR );
  }
  return;
}

void DemonfireShield::wear( Character *ch )
{
    ch->send_to("���� ���� ��������� ��� {R��������� ����{x.\n\r");
}

void DemonfireShield::remove( Character *ch )
{
    ch->send_to("���� ���� ����� ������������� ��������.\n\r");
}

/*
 * Sword of Sun, Olympus
 */
void SwordOfSun::wear( Character *ch )
{
  act_p("$o1 ���������� ������������ ����� ������.", ch,obj,0,TO_CHAR,POS_RESTING);
  act_p("$o1 ���������� ������������ ����� ������.", ch,obj,0,TO_ROOM,POS_RESTING);
  
}
void SwordOfSun::equip( Character *ch )
{
  short level = ch->getModifyLevel();

  if ( level > 20 && level <= 30)	obj->value[2] = 4;
  else if ( level > 30 && level <= 40)   obj->value[2] = 5;
  else if ( level > 40 && level <= 50)   obj->value[2] = 6;
  else if ( level > 50 && level <= 60)   obj->value[2] = 8;
  else if ( level > 60 && level <= 70)   obj->value[2] = 10;
  else if ( level > 70 && level <= 80)   obj->value[2] = 11;
  else obj->value[2] = 12;
}

void SwordOfSun::fight( Character *ch )
{
    Character *victim;

    if ( ch->is_npc() )
	    return;

    if ( ( get_eq_char( ch, wear_wield ) != obj )
	    && ( get_eq_char(ch, wear_second_wield) !=obj ) )
    {
	    return;
    }

    victim = ch->fighting;

    if ( !victim->is_immortal()
	    && number_percent() < 5 )
    {
	ch->send_to("���� ������ ��������������� ������� � ��� ������ ����������!\n\r");

	if ( number_percent() < 40 )
	{
	    act_p( "������ ���������� ����, $o1 �������� ������ $C3!",
		    ch, obj, victim, TO_CHAR,POS_RESTING);
	    act_p( "������ $c2 �� ������� �������� ���� ������!",
		    ch, 0, victim, TO_VICT,POS_RESTING);
	    act_p( "������ $c2 �� ������� �������� ������ $C3!",
		    ch, 0, victim, TO_NOTVICT,POS_RESTING);

	    act_p( "$c1 ��� ����!!", victim, 0, 0, TO_ROOM,POS_RESTING);

	    raw_kill( victim, 3, 0, FKILL_CRY|FKILL_GHOST|FKILL_CORPSE );
	    victim->send_to("���� �����!!\n\r");
	    return;
	}
    }
    return;
}

/*
 * Wind boots, Elemental Canyon
 * Boots of Flying, UnderDark
 */
void FlyingBoots::wear( Character *ch )
{
    if (!ch->isAffected(gsn_fly)) {
	act( "�� �������� $o4, � ���� ���� �������� ���������� �� �����.", ch, obj, 0, TO_CHAR );
	act( "�� ������������ � ������.", ch, 0, 0, TO_CHAR );
	act( "$c1 ����������� � ������.", ch, 0, 0, TO_ROOM );
    }
}
void FlyingBoots::equip( Character *ch )
{
    Affect af;

    if (ch->isAffected(gsn_fly))
	return;

    af.where = TO_AFFECTS;
    af.type = gsn_fly;
    af.duration = -2;
    af.level = ch->getModifyLevel();
    af.bitvector = AFF_FLYING;
    af.location = 0;
    af.modifier = 0;
    affect_to_char(ch, &af);
}

void FlyingBoots::remove( Character *ch )
{
    if (!ch->isAffected(gsn_fly))
	return;
    
    affect_strip(ch, gsn_fly);
    act( "�� ������� �� �����. \r\n�� ��!...", ch, 0, 0, TO_CHAR );
    act( "$c1 ������ �� �����.", ch, 0, 0, TO_ROOM );
}


/*
 * Hercules armbands, Galaxy
 * Girdle of giant strength, Ryzen Caverns
 * Breastplate of strength, UnderDark
 */
void GiantStrengthArmor::wear( Character *ch )
{
    act( "�� ����������, ��� ����������� ������� �������!\r\n"
         "���� ����� ��������� �� ������������ ��������.", ch, obj, 0, TO_CHAR );
    act( "����� $c2 ������ �� ������� ����!", ch, 0, 0, TO_ROOM );
}
void GiantStrengthArmor::equip( Character *ch )
{
    Affect af;
    short level = ch->getModifyLevel();
    
    af.where = TO_AFFECTS;
    af.type = gsn_giant_strength;
    af.duration = -2;
    af.level = ch->getModifyLevel();
    af.bitvector = 0;
    af.location = APPLY_STR;
    af.modifier = 1 + ( level >= 18) + ( level >= 30) + ( level >= 45);
    affect_join(ch, &af);
}

void GiantStrengthArmor::remove( Character *ch )
{
    if (ch->isAffected(gsn_giant_strength))
    {
	affect_strip(ch, gsn_giant_strength);
	act( "���� ����� ����������� �� �������� ���������.", ch, 0, 0, TO_CHAR );
	act( "����� $c2 ����������� �� �������� ���������.", ch, 0, 0, TO_ROOM );
    }
}


/*
 * A shield of rose, Sewer, carried by Grand Knight of Paladins
 */
void RoseShield::fight( Character *ch )
{
  if (!( (ch->in_room->sector_type != SECT_FIELD) ||
       (ch->in_room->sector_type != SECT_FOREST) ||
       (ch->in_room->sector_type != SECT_MOUNTAIN) ||
       (ch->in_room->sector_type != SECT_HILLS) ) )
  return;
  if (get_eq_char(ch,wear_shield) != obj )
  return;

  if ( number_percent() < 90 )  return;

  ch->send_to("������ �� ����� ���� �������� ������������.\n\r");
  ch->fighting->send_to("������ �� ���� �������� ����!\n\r");
  act_p("��� ���� $c2 �������� ������������.",ch,0,0,TO_ROOM,POS_RESTING);
  spell(gsn_slow, ch->getModifyLevel(),ch,ch->fighting);
  return;
}

/*
 * Lion claw, Pyramids
 */
void LionClaw::fight( Character *ch )
{
  bool secondary = false;
  
  if ( number_percent() < 90 )  return;

  if ( ( obj == get_eq_char(ch,wear_wield)) ||
	(secondary = (obj == get_eq_char(ch,wear_second_wield))) )
   {
     ch->send_to("{W����� �� ��������� ���������� �� ������� ����.{x\n\r");
     act_p("{W����� �� ��������� ���������� �� ������� ���� $c2.{x",
		ch,0,0,TO_ROOM,POS_DEAD);
     
     one_hit(ch, ch->fighting, secondary);
     one_hit(ch, ch->fighting, secondary);
     one_hit(ch, ch->fighting, secondary);
     one_hit(ch, ch->fighting, secondary);
    
     return;
   }
 return;
}

/*
 * Ring of Ra, Pyramids
 */
void RingOfRa::speech( Character *ch, const char *speech )
{
    if (ch != obj->getCarrier( ))
	return;
    
  if ((!str_cmp(speech, "punish") || !str_cmp(speech, "��������") || !str_cmp(speech, "��������"))
      && (ch->fighting) &&
((get_eq_char(ch,wear_finger_l) == obj) || (get_eq_char(ch,wear_finger_r))) )
    {
      ch->send_to("������������� ������ ������������ �� ������.\n\r");
      act_p("������������� ������ ������������ �� ������.",
             ch,0,0,TO_ROOM,POS_RESTING);
      spell(gsn_lightning_breath, ch->getModifyLevel(),ch,ch->fighting, FSPELL_BANE );
      ch->setWaitViolence( 2 );
    }
}


