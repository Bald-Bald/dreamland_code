/* $Id: material.cpp,v 1.1.2.6 2009/01/18 20:12:41 rufina Exp $
 *
 * ruffina, 2005
 */
#include "object.h"
#include "character.h"
#include "mercdb.h"
#include "merc.h"

#include "immunity.h"
#include "material.h"

static void 
material_parse( Object *obj, const material_t **argv, int size )
{
    const material_t **ap = argv;
    
    if (obj->getMaterial( )) {
	char *token, name[MAX_STRING_LENGTH];

	strcpy( name, obj->getMaterial( ) );
	
	token = strtok( name, ", " );
	
	for(token=strtok(name, ", "); token; token=strtok(NULL, ", "))
	    if (*token != '\0') {
		const material_t *mat;

		for (mat = &material_table[0]; mat->name; mat++)
		    if (!str_cmp( token, mat->name )) {
			*ap++ = mat;
			break;
		    }		

		if (ap >= argv + size) 
		    return;
	    }
    }

    if (ap < argv + size)
	*ap = NULL;
}

static const int msize = 10;

bool material_is_typed( Object *obj, int type )
{
    const material_t *argv[msize], **ap;

    material_parse( obj, argv, msize );
    
    for (ap = argv; ap < argv + msize && *ap != NULL; ap++) 
	if (IS_SET( (*ap)->type, type ))
	    return true;

    return false;
}

bool material_is_flagged( Object *obj, int flags )
{
    const material_t *argv[msize], **ap;

    material_parse( obj, argv, msize );
    
    for (ap = argv; ap < argv + msize && *ap != NULL; ap++) 
	if (IS_SET( (*ap)->flags, flags ))
	    return true;

    return false;
}

int material_swims( Object *obj )
{
    int swim;
    const material_t *argv[msize], **ap;

    material_parse( obj, argv, msize );
    
    for (swim = 0, ap = argv; ap < argv + msize && *ap != NULL; ap++) 
	swim += (*ap)->floats;
    
    if (swim > 0)
	return SWIM_ALWAYS;
    else if (swim < 0)
	return SWIM_NEVER;
    else
	return SWIM_UNDEF;
}

int material_burns( Object *obj )
{
    int max_burn;
    const material_t *argv[msize], **ap;

    material_parse( obj, argv, msize );
    
    for (max_burn = 0, ap = argv; ap < argv + msize && *ap != NULL; ap++) 
	if ((*ap)->burns < 0)
	    return (*ap)->burns;
	else if ((*ap)->burns > max_burn)
	    max_burn = (*ap)->burns;

    return max_burn;
}

int material_immune( Object *obj, Character *ch )
{
    const material_t *argv[msize], **ap;
    bitstring_t bits = 0;
    int res = RESIST_NORMAL;

    material_parse( obj, argv, msize );
    
    for (ap = argv; ap < argv + msize && *ap != NULL; ap++) 
	SET_BIT( bits, (*ap)->vuln );
    
    immune_from_flags( ch, bits, res );

    return immune_resolve( res );
}

const material_t material_table [] = {
 { "unknown",   0,  0, MAT_NONE,   0, 0, "����������� ��������" },
 { "none",      0,  0, MAT_NONE,   0, 0, "����������� ��������" },
 { "oldstyle",  0,  0, MAT_NONE,   0, 0, "����������� ��������" },

 // metals
 { "adamantite",0, -1, MAT_METAL, 0, 0, "���������" }, // "dull, heavy and dense" (c) adnd
 { "aluminum",  0, -1, MAT_METAL, 0, 0, "��������" },
 { "brass",     0, -1, MAT_METAL, 0, 0, "������" },
 { "bronze",    0, -1, MAT_METAL, 0, 0, "������" },
 { "copper",    0, -1, MAT_METAL, 0, 0, "����" },
 { "gold",      0, -1, MAT_METAL, 0, 0, "������" },
 { "iron",      0, -1, MAT_METAL, 0, VULN_IRON, "������" },
 { "lead",      0, -1, MAT_METAL, 0, 0, "������" },
 { "metal",     0, -1, MAT_METAL, 0, 0, "������" },
 { "mithril",   0, -1, MAT_METAL, 0, 0, "������" }, // "reflective, light and pure" (c) adnd
 { "platinum",  0, -1, MAT_METAL, MAT_INDESTR, 0, "�������" }, 
 { "silver",    0, -1, MAT_METAL, 0, VULN_SILVER, "�������" },
 { "steel",     0, -1, MAT_METAL, 0, VULN_IRON, "�����" },
 { "tin",       0, -1, MAT_METAL, 0, 0, "�����" },
 { "titanium",  0, -1, MAT_METAL, MAT_TOUGH, 0, "�����" },

 // gems
 { "amethyst",  0, -1, MAT_GEM,   0, 0, "�������" },
 { "diamond",   0, -1, MAT_GEM,   0, 0, "�����" },
 { "emerald",   0, -1, MAT_GEM,   0, 0, "�������" },
 { "gem",       0, -1, MAT_GEM,   0, 0, "����������� ������" },
 { "jade",      0, -1, MAT_GEM,   0, 0, "������" },
 { "lapis",     0, -1, MAT_GEM,   0, 0, "�������" },
 { "malachite", 0, -1, MAT_GEM,   0, 0, "�������" },
 { "moonstone", 0, -1, MAT_GEM,   0, 0, "������ ������" },
 { "onyx",      0, -1, MAT_GEM,   0, 0, "�����" },
 { "opal",      0, -1, MAT_GEM,   0, 0, "����" },
 { "ruby",      0, -1, MAT_GEM,   0, 0, "�����" },
 { "sandstone", 0, -1, MAT_GEM,   0, 0, "��������" },
 { "sapphire",  0, -1, MAT_GEM,   0, 0, "������" },
 { "topaz",     0, -1, MAT_GEM,   0, 0, "�����" },

 // wood
 { "ash",       7,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "�����" },
 { "aspen",     5,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "�����" },
 { "bamboo",    6,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "������" },
 { "ebony",     7,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "������ ������" },
 { "hardwood",  7,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "������� ���������" },
 { "oak",       7,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "���" },
 { "redwood",   7,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "������� ������" },
 { "softwood",  5,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "������ ������" },
 { "wood",      5,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "���������" },
 { "yew",       5,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "���" },

 // cloth
 { "canvas",    5,  1, MAT_CLOTH, 0, 0, "�������||�|�||��|�" },
 { "cashmere",  5,  1, MAT_CLOTH, 0, 0, "�������||�|�||��|�" },
 { "cloth",     3,  1, MAT_CLOTH, 0, 0, "����|�|�|�|�|��|�" },
 { "cotton",    3,  1, MAT_CLOTH, 0, 0, "����|��|��|��|��|���|��" },
 { "felt",      3,  1, MAT_CLOTH, 0, 0, "����|��|��|��|��|���|��" },
 { "fur",       4,  1, MAT_CLOTH, 0, 0, "���||�|�||��|�" },
 { "hard leather",5,1, MAT_CLOTH, 0, 0, "������|��|��|��|��|��|�� ���|�|�|�|�|��|�" },
 { "leather",   4,  1, MAT_CLOTH, 0, 0, "���|�|�|�|�|��|�" },
 { "linen",     3,  1, MAT_CLOTH, 0, 0, "������|�|�|�|�|��|�" },
 { "satin",     3,  1, MAT_CLOTH, 0, 0, "�����||�|�||��|�" },
 { "silk",      3,  1, MAT_CLOTH, 0, 0, "����||�|�||��|�" },
 { "soft leather",4,1, MAT_CLOTH, 0, 0, "����|��|��|��|��|��|�� ���|�|�|�|�|��|�" },
 { "tweed",     4,  1, MAT_CLOTH, 0, 0, "����||�|�||��|�" },
 { "velvet",    3,  1, MAT_CLOTH, 0, 0, "������||�|�||��|�" },
 { "wool",      4,  1, MAT_CLOTH, 0, 0, "�����|�|�|�|�|��|�" },
                   
 // elements       
 { "air",       0,  1, MAT_ELEMENT,   0, 0, "������" },
 { "wind",      0,  1, MAT_ELEMENT,   0, 0, "�����" },
 { "fire",      0,  1, MAT_ELEMENT,   0, VULN_FIRE, "�����" },
 { "flame",     0,  1, MAT_ELEMENT,   0, VULN_FIRE, "�����" },
 { "ice",      -1,  1, MAT_ELEMENT,   MAT_MELTING, 0, "���" },
 { "light",     0,  1, MAT_ELEMENT,   0, VULN_LIGHT, "����" },
 { "water",    -1,  1, MAT_ELEMENT,   0, 0, "����" },
 { "snow",     -1,  1, MAT_ELEMENT,   0, 0, "����" },
                   
 // minerals       
 { "brick",     0, -1, MAT_MINERAL,   0, 0, "������" },
 { "ceramics",  0,  0, MAT_MINERAL,   0, 0, "��������" },
 { "clay",      0,  0, MAT_MINERAL,   0, 0, "�����" },       
 { "corund",    0,  0, MAT_MINERAL,   0, 0, "������" },
 { "crystal",   0,  0, MAT_MINERAL,   0, 0, "��������" },
 { "glass",     0,  0, MAT_MINERAL,   MAT_FRAGILE, 0, "������" },
 { "granite",   0, -1, MAT_MINERAL,   0, 0, "������" },
 { "ground",   -1,  0, MAT_MINERAL,   0, 0, "�����" },
 { "marble",    0, -1, MAT_MINERAL,   0, 0, "������" },
 { "obsidian",  0, -1, MAT_MINERAL,   0, 0, "��������, ����. ������" },
 { "parafin",   0,  1, MAT_MINERAL,   0, 0, "�������" },
 { "plastic",   2,  0, MAT_MINERAL,   0, 0, "�������" },
 { "porcelain", 0,  0, MAT_MINERAL,   0, 0, "������" },
 { "quartz",    0,  0, MAT_MINERAL,   0, 0, "�����" },
 { "stone",     0,  0, MAT_MINERAL,   0, 0, "������" },
 { "sulfur",    2,  1, MAT_MINERAL,   0, 0, "����" },
                   
 // organics       
 { "amber",     0,  0, MAT_ORGANIC,   0, 0, "������" },
 { "chalk",     0,  0, MAT_ORGANIC,   0, 0, "���" },
 { "coal",      8,  0, MAT_ORGANIC,   0, 0, "�����" },
 { "coral",     0,  0, MAT_ORGANIC,   0, 0, "������" },
 { "fiber",     4,  0, MAT_ORGANIC,   0, 0, "�������" },
 { "pearl",     0, -1, MAT_ORGANIC,   0, 0, "������" },
 { "rubber",    3,  0, MAT_ORGANIC,   0, 0, "������" },
 { "shell",     0,  0, MAT_ORGANIC,   0, 0, "��������" },
 { "wax",       0,  0, MAT_ORGANIC,   MAT_MELTING, 0, "����" },
                   
 // papers
 { "paper",     1,  1, MAT_ORGANIC,   0, 0, "������" },
 { "parchment", 1,  1, MAT_ORGANIC,   0, 0, "���������" },
 { "vellum",    1,  1, MAT_ORGANIC,   0, 0, "������ ���������" },

 // bones          
 { "bone",      0,  0, MAT_ORGANIC,   0, 0, "�����" },
 { "human femur",0, 0, MAT_ORGANIC,   0, 0, "������������ �����" },
 { "ivory",     0, -1, MAT_ORGANIC,   0, 0, "�������� �����" },
                   
 // flesh          
 { "bearskin",  6,  1, MAT_ORGANIC,   0, 0, "�������� �����" },
 { "blood",    -1,  1, MAT_ORGANIC,   0, 0, "�����" },
 { "dragonskin",8,  1, MAT_ORGANIC,   0, 0, "�������� �����" },
 { "feathers",  1,  1, MAT_ORGANIC,   0, 0, "�����" },
 { "fish",      1,  1, MAT_ORGANIC,   0, 0, "����" },
 { "flesh",     1,  1, MAT_ORGANIC,   0, 0, "�����" },
 { "gut",       1,  1, MAT_ORGANIC,   0, 0, "�����" },
 { "hair",      1,  1, MAT_ORGANIC,   0, 0, "������" },
 { "human flesh",1, 1, MAT_ORGANIC,   0, 0, "������������ �����" },
 { "lion-fell", 4,  1, MAT_ORGANIC,   0, 0, "������� �����" },
 { "skin",      2,  1, MAT_ORGANIC,   0, 0, "����, �����" },
 { "snakeskin", 2,  1, MAT_ORGANIC,   0, 0, "������� ����" },
                   
 // plant          
 { "flower",    1,  1, MAT_ORGANIC,   0, 0, "������" },
 { "gourd",     1,  1, MAT_ORGANIC,   0, 0, "�����" },
 { "grain",     1,  1, MAT_ORGANIC,   0, 0, "�����" },
 { "grass",     1,  1, MAT_ORGANIC,   0, 0, "�����" },
 { "hay",       1,  1, MAT_ORGANIC,   0, 0, "����" },
 { "hemp",      1,  1, MAT_ORGANIC,   0, 0, "�������" },
 { "moss",      1,  1, MAT_ORGANIC,   0, 0, "���" },
 { "plant",     1,  1, MAT_ORGANIC,   0, 0, "��������" },
 { "plant_organism",1,1, MAT_ORGANIC,   0, 0, "������������ ��������" },
 { "pollen",    1,  1, MAT_ORGANIC,   0, 0, "������" },
 { "root",      1,  1, MAT_ORGANIC,   0, 0, "������" },
 { "straw",     1,  1, MAT_ORGANIC,   0, 0, "������" },
                   
 // meal           
 { "bread",     1,  0, MAT_ORGANIC,   0, 0, "����" },
 { "cake",      1,  0, MAT_ORGANIC,   0, 0, "������� �����" },
 { "pastry",    1,  0, MAT_ORGANIC,   0, 0, "������� �����" },
 { "drink",    -1,  0, MAT_ORGANIC,   0, 0, "�����" },
 { "flour",     1,  0, MAT_ORGANIC,   0, 0, "����" },
 { "food",      2,  0, MAT_ORGANIC,   0, 0, "����" },
 { "meat",      1,  0, MAT_ORGANIC,   0, 0, "����" },
                   
 // abstract       
 { "darkness",   0, 1, MAT_ABSTRACT,   0, 0, "����" },
 { "energy",     0, 1, MAT_ABSTRACT,   0, VULN_ENERGY, "�������" },
 { "entropia",   0, 1, MAT_ABSTRACT,   0, 0, "��������" },
 { "ethereal",   0, 1, MAT_ABSTRACT,   0, 0,   },
 { "etherealness",0,1, MAT_ABSTRACT,   0, 0,   },
 { "evil",       0, 1, MAT_ABSTRACT,   0, 0, "���" },
 { "magic",      0, 1, MAT_ABSTRACT,   0, 0, "�����" },
 { "nothingness",0, 1, MAT_ABSTRACT,   0, 0, "������" },
 { "shadow",     0, 1, MAT_ABSTRACT,   0, 0, "����" },
 { "vacuum",     0, 1, MAT_ABSTRACT,   0, 0, "������" },

 { NULL,         0, 0, MAT_NONE,   0, 0, }, 
};

