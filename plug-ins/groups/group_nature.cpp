/* $Id$
 *
 * ruffina, 2004
 */

#include "skill.h"
#include "spelltarget.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "pcharactermanager.h"
#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "npcharacter.h"
#include "object.h"

#include "dreamland.h"
#include "gsn_plugin.h"
#include "magic.h"
#include "fight.h"
#include "stats_apply.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "save.h"
#include "act.h"
#include "vnum.h"
#include "interp.h"
#include "def.h"
#include "move_utils.h"

PROF(ranger);

/*
 * 'tame' skill command
 */
SKILL_RUNP( tame )
{
    Character *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument,arg);

    if ( ch->is_npc() || !gsn_tame->usable( ch ) )
    {
        ch->send_to("Это ж надо уметь!\n\r");
        return;
    }

    if (arg[0] == '\0')
    {
        ch->send_to("Ты не поддаешься укрощению.\n\r");
        act_p("$c1 пытается укротить са$gмо|м|ма себя, но эта попытка с треском проваливается.",
                ch,0,0,TO_ROOM,POS_RESTING);
        return;
    }

    if ( (victim = get_char_room(ch,arg)) == 0)
    {
        ch->send_to("Этого нет здесь.\n\r");
        return;
    }

    if (!victim->is_npc())
    {
        ch->pecho("%1$^C1 не подда%1$nется|ются укрощению.", victim);
        return;
    }
    
    /* 
     * ranger tame: remove aggression
     */
    if (ch->getProfession( ) == prof_ranger) {
        if (!IS_SET(victim->act,ACT_AGGRESSIVE))
        {
            ch->pecho("%1$^C1 обычно не агрессив%1$Gно|ен|на|ны.", victim);
            return;
        }

        ch->setWait( gsn_tame->getBeats( )  );

        if (number_percent() < gsn_tame->getEffective( ch ) + 15
                + 4 * ( ch->getModifyLevel() - victim->getModifyLevel() ) )
        {
            REMOVE_BIT(victim->act,ACT_AGGRESSIVE);
            SET_BIT(victim->affected_by,AFF_CALM);
            victim->pecho("Ты успокаиваешься.");
            act("Ты укрощаешь $C4.",ch,0,victim,TO_CHAR);
            act("$c1 укрощает $C4.",ch,0,victim,TO_NOTVICT);
            stop_fighting(victim,true);
            gsn_tame->improve( ch, true, victim );
        }
        else
        {
            ch->pecho("Попытка укрощения не удалась.");
            act("$c1 пытается укротить $C4, но безуспешно.",
                    ch,0,victim,TO_NOTVICT);
            act("$c1 пытается укротить тебя, но безуспешно.",
                    ch,0,victim,TO_VICT);
            gsn_tame->improve( ch, false, victim );
        }

        return;
    }
}

/*
 * 'hydroblast' spell
 */
SPELL_DECL(Hydroblast);
VOID_SPELL(Hydroblast)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int dam;

    if (!RoomUtils::hasWaterParticles(ch->in_room)) {
         ch->send_to("Здесь недостаточно водных молекул.\n\r");
         ch->wait = 0;
         return;
    }
    
    act("Молекулы воды вокруг $c2 собираются вместе, образуя кулак.", ch, 0, 0, TO_ROOM);
    act("Молекулы воды вокруг тебя собираются вместе, образуя кулак.", ch, 0, 0, TO_CHAR);
    dam = dice( level , 14 );
    damage_nocatch(ch,victim,dam,sn,DAM_BASH,true, DAMF_MAGIC|DAMF_WATER);
}

/*
 * 'entangle' spell
 */
SPELL_DECL(Entangle);
VOID_SPELL(Entangle)::run( Character *ch, Object *grave, int sn, int level ) 
{
    int dam;
    PCharacter *victim;

    if (!RoomUtils::isNature(ch->in_room))
    {
        ch->send_to("Терновник растет только в лесу, поле, горах или на холмах.\n\r");
        return;
    }

    if (grave->pIndexData->vnum != OBJ_VNUM_GRAVE) {
        ch->send_to("Это не вампирская могила.\r\n");
        return;
    }

    victim = PCharacterManager::findPlayer( grave->getOwner( ) );

    if (!victim || !DIGGED(victim)) {
        ch->send_to("Колючий терновник опутывает могилу... но в ней никого не оказывается!\r\n");
        LogStream::sendError( ) << "Unexistent grave owner: " << grave->getOwner( ) << endl;
        return;
    }

    if (is_safe(ch, victim)) 
        return;
    
    if (number_percent( ) > ch->getSkill( sn ) ) {
        act("Могила покрывается цветочками и вьющимся барвинком.", ch, 0, 0, TO_ALL);
        return;
    }

    act("Корни терновника проникают в могилу, тревожа твой покой.", victim, 0, 0, TO_CHAR);
    act("Колючий терновник опутывает могилу, проникая корнями глубоко под землю!", ch, 0, 0, TO_ALL);
    act("Из-под земли раздается недовольное ворчание.", ch, 0, 0, TO_ALL);

    undig( victim );

    dam = number_range(level, 4 * level);
    if ( saves_spell( level, victim, DAM_PIERCE, ch, DAMF_MAGIC ) )
        dam /= 2;

    damage_nocatch(ch,victim, level,gsn_entangle,DAM_PIERCE, true, DAMF_MAGIC);
}

VOID_SPELL(Entangle)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int dam;
    Affect todex;

   if (victim == ch)
   {
        ch->send_to("Ты задумчиво колешь себя шипом терновника в пятку. Ай!\n\r");
        return;
   }
    
   if (!RoomUtils::isNature(ch->in_room))
   {
        ch->send_to("Терновник растет только в лесу, поле, горах или на холмах.\n\r");
        return;
   }

   if (IS_SET(victim->imm_flags, IMM_PIERCE))
   {
        act_p("$C1 обладает иммунитетом к шипам терновника.", ch, 0,
                victim, TO_CHAR,POS_RESTING);
        return;
   }
    
   if (is_flying( victim ))
   {
        ch->send_to("Побеги терновника не смогут навредить летучему противнику.\n\r");
        return;
   }

    dam = number_range(level, 4 * level);
    victim->move -= victim->max_move / 3;
    victim->move = max( 0, (int)victim->move );
    
    if ( !victim->isAffected(gsn_entangle) )
    {
        if ( !saves_spell(level, victim, DAM_PIERCE, ch, DAMF_MAGIC) ){
            act("Колючий терновник прорастает сквозь землю, обвивая ноги $c2!",
                victim, 0, 0, TO_ROOM);
            act("Колючий терновник прорастает сквозь землю, обвивая твои ноги!",
                victim, 0, 0, TO_CHAR);

            todex.type = sn;
            todex.level = level;
            todex.duration = level / (5 * victim->size) + 1;
            todex.location = APPLY_DEX;
            todex.modifier = -1 * (level / 20 + 1);
            affect_join( victim, &todex); 
            
            dam = dam * 2;   
        }
        else {
            act("Колючий терновник прорастает сквозь землю, но $c1 с трудом разрывает его путы!",
                victim, 0, 0, TO_ROOM);
            act("Колючий терновник прорастает сквозь землю, но ты с трудом разрываешь его путы!",
                victim, 0, 0, TO_CHAR);
        }
    }
    else {
            act("Колючий терновник прорастает сквозь землю, больно раня ноги $c2!",
                victim, 0, 0, TO_ROOM);
            act("Колючий терновник прорастает сквозь землю, больно раня твои ноги!",
                victim, 0, 0, TO_CHAR);        
    }
   
    damage_nocatch(ch, victim, level, gsn_entangle, DAM_PIERCE, true, DAMF_MAGIC);
}
