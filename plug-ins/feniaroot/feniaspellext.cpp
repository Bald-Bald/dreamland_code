#include "feniaspellhelper.h"

#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"
#include "subr.h"
#include "closure.h"
#include "characterwrapper.h"
#include "objectwrapper.h"
#include "structwrappers.h"

#include "core/object.h"
#include "room.h"
#include "character.h"
#include "magic.h"
#include "damageflags.h"
#include "fight.h"
#include "fight_exception.h"
#include "effects.h"
#include "defaultspell.h"
#include "dl_math.h"
#include "def.h"

using Scripting::NativeTraits;
using namespace Scripting;

DLString regfmt(Character *to, const RegisterList &argv);

static RegisterList message_args(FeniaSpellContext *thiz, const RegisterList &args)
{
    RegisterList myArgs(args); 
    Register fmt = myArgs.front();
    myArgs.pop_front();

    if (thiz->vict.type != Register::NONE)
        myArgs.push_front(thiz->vict);
    else if (thiz->obj.type != Register::NONE)
        myArgs.push_front(thiz->obj);
    else if (thiz->room.type != Register::NONE)
        myArgs.push_front(thiz->room);
    else
        myArgs.push_front(thiz->arg);

    myArgs.push_front(thiz->ch);
    myArgs.push_front(fmt);
    return myArgs;
}

NMI_INVOKE(FeniaSpellContext, msgChar, "(fmt[,args]): выдать сообщение кастеру; кастер 1й аргумент, цель 2й аргумент")
{
    Character *caster = arg2character(ch);
    caster->println(regfmt(caster, message_args(this, args)));
    return Register();
}

NMI_INVOKE(FeniaSpellContext, msgVict, "(fmt[,args]): выдать сообщение жертве; кастер 1й аргумент, цель 2й аргумент")
{
    if (vict.type == Register::NONE)
        return Register();

    Character *victim = arg2character(vict);
    victim->println(regfmt(victim, message_args(this, args)));
    return Register();
}

NMI_INVOKE(FeniaSpellContext, msgNotVict, "(fmt[,args]): выдать сообщение всем, кроме кастера и жертвы; кастер 1й аргумент, цель 2й аргумент")
{
    if (vict.type == Register::NONE)
        return Register();

    Character *caster = arg2character(ch);
    Character *victim = arg2character(vict);
    RegisterList myArgs = message_args(this, args);

    for (Character *to = victim->in_room->people; to; to = to->next_in_room) {
        if (to == caster || to == victim)
            continue;            
        if (!to->can_sense(caster))
            continue;

        to->pecho(POS_RESTING, regfmt(to, myArgs).c_str());
    }

    return Register();
}

NMI_INVOKE(FeniaSpellContext, msgRoom, "(fmt[,args]): выдать сообщение всем, кроме кастера; кастер 1й аргумент, цель 2й аргумент")
{
    Character *caster = arg2character(ch);
    RegisterList myArgs = message_args(this, args); 

    for (Character *to = caster->in_room->people; to; to = to->next_in_room) {
        if (to == caster)
            continue;
        if (!to->can_sense(caster))
            continue;

        to->pecho(POS_RESTING, regfmt(to, myArgs).c_str());
    }
    return Register();
}

NMI_INVOKE(FeniaSpellContext, msgAll, "(fmt[,args]): выдать сообщение всем в комнате; кастер 1й аргумент, цель 2й аргумент")
{
    Character *caster = arg2character(ch);
    RegisterList myArgs = message_args(this, args); 
    caster->in_room->echo(POS_RESTING, regfmt(NULL, myArgs).c_str());
    return Register();
}

NMI_INVOKE(FeniaSpellContext, msgArea, "(fmt[,args]): выдать сообщение всем в той же зоне, кроме кастера")
{
    Character *caster = arg2character(ch);
    RegisterList myArgs = message_args(this, args); 
    DLString message = regfmt(NULL, myArgs);
    area_message(caster, message, true);
    return Register();
}


NMI_INVOKE(FeniaSpellContext, calcDamage, "([tier]): рассчитать повреждения для tier в параметрах или из конфигурации")
{
    int tier = args.empty() ? arg2spell(spell)->tier.getValue() : args2number(args);

    if (tier == 2) {
        if (level <= 20)
            dam = dice(level, 8);
        else if (level <= 40)
            dam = dice(level, 12);
        else if (level <= 70)
            dam = dice(level, 15);
        else
            dam = dice(level, 18);

        return Register(dam);
    } else if (tier == 3) {
        if (level <= 20)
            dam = dice(level, 7);
        else if (level <= 40)
            dam = dice(level, 10);
        else if (level <= 70)
            dam = dice(level, 13);
        else
            dam = dice(level, 16);
    }

    return Register(dam);
}

NMI_INVOKE(FeniaSpellContext, savesSpell, "([damtype,damflags]): уменьшить повреждения вдвое, если прошел спассбросок у жертвы")
{
    if (vict.type == Register::NONE)
        return Register();

    DefaultSpell *mySpell = arg2spell(spell);
    bitnumber_t damtype = args.empty() ? mySpell->damtype.getValue() : argnum2flag(args, 1, damage_table);
    bitstring_t damflags = args.size() <= 1 ? mySpell->damflags : argnum2flag(args, 2, damage_flags);
    
    Character *myCh = arg2character(ch);
    Character *myVict = arg2character(vict);

    if (saves_spell(level, myVict, damtype, myCh, damflags)) {
        dam /= 2;
        return Register(true);
    }
        
    return Register(false);
}

NMI_INVOKE(FeniaSpellContext, damage, "([damtype,damflags]): нанести повреждения жертве")
{
    if (vict.type == Register::NONE)
        return Register();
 
    DefaultSpell *mySpell = arg2spell(spell);
    bitnumber_t damtype = args.empty() ? mySpell->damtype.getValue() : argnum2flag(args, 1, damage_table);
    bitstring_t damflags = args.size() <= 1 ? mySpell->damflags : argnum2flag(args, 2, damage_flags);
   
    Character *myCh = arg2character(ch);
    Character *myVict = arg2character(vict);
    int sn = mySpell->getSkill()->getIndex();

    try {
        damage_nocatch(myCh, myVict, dam, sn, damtype, true, damflags | DAMF_SPELL);    

    } catch (const VictimDeathException &e) {
        throw Scripting::CustomException("victim is dead");
    }

    return Register(false);
}

NMI_INVOKE(FeniaSpellContext, damageRoom, "(func): вызвать ф-ию для всех в комнате, кто не защищен от заклинания")
{
    Character *caster = arg2character(ch);
    RegisterList::const_iterator ai = args.begin();
    Register rfun = *ai++;
    Closure *fun = rfun.toFunction( );
    RegisterList funArgs;
    funArgs.assign(ai, args.end( ));

    for (auto &vch: caster->in_room->getPeople()) {
        if (vch->isDead())
            continue;
        if (vch->in_room != caster->in_room)
            continue;
        if (vch == caster)
            continue;
        if (is_safe_spell(caster, vch, true))
            continue;
        if (vch->is_mirror() && number_percent() < 50)
            continue;

        vict = wrap(vch);

        try {
            fun->invoke(thiz, funArgs);
        } catch (const CustomException &ce) {

        } catch (const VictimDeathException &vde) {
            
        }
    }

    return Register();
}

NMI_INVOKE(FeniaSpellContext, effectCold, "(): применить холодный эффект на жертву")
{
    if (vict.type == Register::NONE)
        return Register();

    Character *myVict = arg2character(vict);
    cold_effect(myVict, level, dam, TARGET_CHAR, DAMF_SPELL);
    return Register();    
}

NMI_INVOKE(FeniaSpellContext, effectFire, "(): применить огненный эффект на жертву")
{
    if (vict.type == Register::NONE)
        return Register();

    Character *myVict = arg2character(vict);
    fire_effect(myVict, level, dam, TARGET_CHAR, DAMF_SPELL);
    return Register();
}

NMI_GET(FeniaSpellContext, skill, "прототип умеиния для этого заклинания (.Skill())")
{
    return Register::handler<SkillWrapper>(name);    
}