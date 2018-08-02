/* $Id$
 *
 * ruffina, 2004
 */
#include "movetypes.h"

#include "pcharacter.h"

#include "mercdb.h"
#include "def.h"

const char * movedanger_names [] = {
    "moresafe", "safe", "normal", "dangerous", "death"
};

const struct movetype_t movetypes [] = {
 { MOVETYPE_SWIMMING,   MOVETYPE_NORMAL,    1, false, "swimming",  "������%1$G��|�|��",        "����%1$G��|�|��",
 },
 { MOVETYPE_WATER_WALK, MOVETYPE_NORMAL,    1, false, "waterwalk", "����%1$G��|��|�� �� ����", "��%1$G��|��|�� �� ����",
 },
 { MOVETYPE_SLINK,      MOVETYPE_MORESAFE,  3, true,  "slink",     "������%1$G���|�|���",      "����%1$G���|�|���",
 },
 { MOVETYPE_CRAWL,      MOVETYPE_SAFE,      2, true,  "crawl",     "�������%1$G���|��|���",    "�������%1$G���|��|���",
 },
 { MOVETYPE_WALK,       MOVETYPE_NORMAL,    1, true,  "normal",    "����%1$G��|��|��",         "��%1$G��|��|��",
 },
 { MOVETYPE_QUICKLY,    MOVETYPE_DANGEROUS, 1, true,  "quickly",   "��� ����",                 "��� ����",
 },
 { MOVETYPE_RUNNING,    MOVETYPE_DEATH,     1, true,  "running",   "�������%1$G��|�|��",       "�����%1$G��|�|��",
 },
 { MOVETYPE_FLEE,       MOVETYPE_DEATH,     1, true,  "flee",      "�������%1$G��|�|��",       "�����%1$G��|�|��",
 },
 { MOVETYPE_RIDING,     MOVETYPE_DANGEROUS, 1, true,  "riding",    "��������%1$G��|�|��",      "������%1$G��|�|��",
 },
 { MOVETYPE_FLYING,     MOVETYPE_NORMAL,    1, true,  "flying",    "�������%1$G��|�|��",       "�����%1$G��|�|��",
 },
 { 0, 0, 0, 0 },
};


int movetype_lookup( const char *argument )
{
    if (argument && argument[0])
	for (int i = 0; movetypes[i].name; i++)
	    if (!str_cmp( argument, movetypes[i].name ))
		return i;
    
    return MOVETYPE_WALK;
}


int movetype_resolve( Character *ch, const char *argument )
{
    int movetype;
    
    if (argument == NULL || argument[0] == 0)
	movetype = MOVETYPE_WALK;
    else if (!ch->is_npc( ) && ch->getPC( )->getAttributes( ).isAvailable( "speedwalk" ))
	movetype = MOVETYPE_RUNNING;
    else
	movetype = movetype_lookup( argument );
    
    return movetype;
}

