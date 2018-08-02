/* $Id$
 *
 * ruffina, 2004
 */
#include "alignment.h"
#include "pcharacter.h"
#include "pcrace.h"
#include "mercdb.h"
#include "act.h"


const struct alignment_t alignment_table [] = {
  { -1000,  -950,  -900,   "���������|��|���|���|��|��|��" },
  {  -900,  -850,  -800,   "����������|��|���|���|��|��|��" },
  {  -800,  -750,  -700,   "����� ��|��|���|���|��|��|��" },
  {  -700,  -500,  -300,   "��|��|���|���|��|��|��" },
  {  -300,  -275,  -150,   "������-���������|��|���|���|��|��|��" },
  {  -150,  -100,   -50,   "����������-��|��|���|���|��|��|��" },
  {   -50,     0,    50,   "��������� ���������|��|���|���|��|��|��" },
  {    50,   100,   150,   "����������-����|��|���|���|��|��|��" },
  {   150,   275,   300,   "�����-���������|��|���|���|��|��|��" },
  {   300,   500,   700,   "����|��|���|���|��|��|��" },
  {   700,   750,   800,   "����� ����|��|���|���|��|��|��" },
  {   800,   850,   900,   "����|��|���|���|��|��|��" },
  {   900,   950,  1000,   "��������|��|���|���|��|��|��" },

  { 0, 0, 0, NULL }
};

DLString align_name_for_range( int min, int max )
{
    if (min <= -500 && max >= 500)
        return "�����";

    if (min <= -500)
        return "����";
    if (max >= 500)
        return "������";
    return "�����������";
}

int align_choose_range( int min, int max, int n )
{
    int cnt = 0;
    
    for (int i = 0; alignment_table[i].rname; i++)
	if (min <= alignment_table[i].minValue
	    && max >= alignment_table[i].maxValue)
	    if (++cnt == n)
		return alignment_table[i].aveValue;
    
    return ALIGN_ERROR;
}

int align_choose_range( int min, int max, const DLString &arg )
{
    if (arg.empty( ))
	return ALIGN_ERROR;

    for (int i = 0; alignment_table[i].rname; i++)
	if (min <= alignment_table[i].minValue
	    && max >= alignment_table[i].maxValue)
	    if (arg.strPrefix( russian_case( alignment_table[i].rname, '1' ) ))
		return alignment_table[i].aveValue;
    
    return ALIGN_ERROR;
}

void align_print_range( int min, int max, ostringstream &buf )
{
    int cnt = 0;
    
    for (int i = 0; alignment_table[i].rname; i++)
	if (min <= alignment_table[i].minValue
	    && max >= alignment_table[i].maxValue)
	{
	    buf << dlprintf( "%2d) %s\r\n", 
	                     ++cnt, 
			     russian_case( alignment_table[i].rname, '1' ).c_str( ) );
	}
}

void align_get_ranges( PCharacter *ch, int &a_min, int &a_max )
{
    int p_min = ch->getProfession( )->getMinAlign( ),
        p_max = ch->getProfession( )->getMaxAlign( );
    int r_min = ch->getRace( )->getPC( )->getMinAlign( ),
        r_max = ch->getRace( )->getPC( )->getMaxAlign( );

    if (r_min <= p_min && p_min <= r_max) {
	a_min = p_min;
	a_max = min( r_max, p_max );
	return;
    }

    if (p_min <= r_min && r_min <= p_max) {
	a_min = r_min;
	a_max = min( r_max, p_max );
	return;
    }

    a_min = r_min;
    a_max = r_max;
    return;
}


int align_choose_allowed( PCharacter *ch, const DLString &arg )
{
    int a_min, a_max;

    align_get_ranges( ch, a_min, a_max );
    return align_choose_range( a_min, a_max, arg );
}

int align_choose_allowed( PCharacter *ch, int n )
{
    int a_min, a_max;

    align_get_ranges( ch, a_min, a_max );
    return align_choose_range( a_min, a_max, n );
}

void align_print_allowed( PCharacter *ch, ostringstream &buf )
{
    int a_min, a_max;

    align_get_ranges( ch, a_min, a_max );
    align_print_range( a_min, a_max, buf );
}

DLString align_name( int a )
{
    for (int i = 0; alignment_table[i].rname; i++)
	if (a >= alignment_table[i].minValue	
	    && a <= alignment_table[i].maxValue)
	{
	    return alignment_table[i].rname;
	}

    return DLString::emptyString;
}

DLString align_name( Character *ch )
{
    return align_name( ch->alignment );
}

DLString align_min( PCharacter *ch )
{
    int a_min, a_max;
    align_get_ranges( ch, a_min, a_max );
    return align_name( a_min + 1 );
}

DLString align_max( PCharacter *ch )
{
    int a_min, a_max;
    align_get_ranges( ch, a_min, a_max );
    return align_name( a_max );
}

