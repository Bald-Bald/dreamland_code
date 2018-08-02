/* $Id$
 *
 * ruffina, 2004
 */
#include "move_utils.h"
#include "terrains.h"

#include "pcharacter.h"
#include "object.h"

#include "merc.h"
#include "def.h"

WEARLOC(none);

const struct terrain_t terrains [SECT_MAX] = {
/* type           move  wait   hit           fall         where*/
 { SECT_INSIDE,       1,  0, "�� ���",      "�� ���",   "�� ����"   },  // inside
 { SECT_CITY,         2,  0, "�� ��������", "�� ���",   "�� ����"   },  // city
 { SECT_FIELD,        2,  0, "�� �����",    "�� �����", "�� �����"  },  // field
 { SECT_FOREST,       3,  0, "�� �����",    "�� �����", "�� �����"  },  // forest
 { SECT_HILLS,        4,  0, "�� �����",    "�� �����", "�� �����"  },  // hills
 { SECT_MOUNTAIN,     6,  0, "�� �����",    "�� �����", "�� �����"  },  // mountain
 { SECT_WATER_SWIM,   4,  1, "�� ���",      "� ����",   "�� ����"   },  // water_swim
 { SECT_WATER_NOSWIM, 1,  1, "�� ����",     "� ����",   "�� ����"   },  // water_noswim
 { SECT_UNUSED,       6,  1, "�� �����",    "�� �����", "�� �����"  },  // <gap> 
 { SECT_AIR,          10, 0, "�� ������",   "� ������", "� �������" },  // air
 { SECT_DESERT,       6,  1, "�� �����",    "�� �����", "�� �����"  },  // desert
 { SECT_UNUSED,       6,  1, "�� �����",    "�� �����", "�� ���"    },  // underwater
};

/*-----------------------------------------------------------------------------
 * boats
 *----------------------------------------------------------------------------*/
static bool obj_is_wearable( Object *obj )
{
    int wear = obj->wear_flags;

    REMOVE_BIT(wear, ITEM_TAKE);
    REMOVE_BIT(wear, ITEM_NO_SAC);
    return wear;
}

Object * boat_object_find( Character *ch )
{
    Object *obj;

    for (obj = ch->carrying; obj; obj = obj->next_content)
	if (obj->wear_loc != wear_none && obj->item_type == ITEM_BOAT)
	    return obj;
		
    for (obj = ch->carrying; obj; obj = obj->next_content)
	if (obj->wear_loc == wear_none && obj->item_type == ITEM_BOAT)
	    if (!obj_is_wearable( obj ))
		return obj;

    return NULL;
}

int boat_get_type( Character *ch )
{
    Object *boat;
    
    if (ch->is_immortal( ) || ch->is_mirror( ))
	return BOAT_FLY;

    if (is_flying( ch ) || IS_GHOST(ch))
	return BOAT_FLY;

    if (IS_AFFECTED(ch, AFF_SWIM))
	return BOAT_SWIM;

    boat = boat_object_find( ch );

    if (!boat)
	return BOAT_NONE;

    if (boat->wear_loc == wear_none)
	return BOAT_INV;
    else
	return BOAT_EQ;
}


