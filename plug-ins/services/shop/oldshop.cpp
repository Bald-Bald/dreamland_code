/* $Id: oldshop.cpp,v 1.1.2.9 2010-09-01 21:20:46 rufina Exp $
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
#include <math.h>

#include "commandtemplate.h"
#include "shoptrader.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "class.h"

#include "attract.h"
#include "occupations.h"
#include "webmanip.h"

#include "skillreference.h"
#include "skill.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "handler.h"
#include "interp.h"
#include "def.h"
    
GSN(haggle);

using std::min;
using std::max;

bool obj_has_name( Object *obj, const DLString &arg, Character *ch );
long long get_arg_id( const DLString &cArgument );
short get_wear_level( Character *ch, Object *obj );

/*
 * Local functions
 */
int get_cost( NPCharacter *keeper, Object *obj, bool fBuy, ShopTrader::Pointer trader );
ShopTrader::Pointer find_keeper( Character *ch );
void obj_to_keeper( Object *obj, NPCharacter *ch );
Object *get_obj_keeper( Character *ch, ShopTrader::Pointer, const DLString &constArguments );
void deduct_cost(Character *ch, int cost);

static bool  
mprog_sell( Character *ch, Character *buyer, Object *obj, int cost, int number )
{
    FENIA_CALL( ch, "Sell", "COii", buyer, obj, cost, number );
    FENIA_NDX_CALL( ch->getNPC( ), "Sell", "CCOii", ch, buyer, obj, cost, number );
    return false;
}

struct StockInfo {
    StockInfo( Object *obj, int cost, int count ) {
        this->obj = obj;
        this->cost = cost;
        this->count = count;
    }
    Object *obj;
    int cost;
    int count;
};

typedef vector<StockInfo> ShopStock;

ShopStock get_stock_keeper( ShopTrader::Pointer trader, Character *client, const DLString &arg )
{
    Object *obj;
    ShopStock stock;
    NPCharacter *keeper = trader->getChar( );

    for( obj = keeper->carrying; obj; obj = obj->next_content) {
	if (obj->wear_loc != wear_none)
            continue;
        
        if (client && !client->can_see( obj ))
            continue;

	int cost = get_cost( keeper, obj, true, trader );
        if (cost <= 0)
            continue;

        if (!arg.empty( ) && !obj_has_name( obj, arg, client ))
            continue;
        
	int count = 1;

        if (!IS_OBJ_STAT( obj, ITEM_INVENTORY )) {
            while (obj->next_content
                    &&  obj->pIndexData == obj->next_content->pIndexData
                    &&  !str_cmp( obj->getShortDescr( ), obj->next_content->getShortDescr( ) ))
            {
                obj = obj->next_content;
                count++;
            }
        }
        
        stock.push_back( StockInfo( obj, cost, count ) );
    }

    return stock;
}

void describe_goods( NPCharacter *keeper, Character *client, const DLString &arg, bool fStrict )
{
}

/*----------------------------------------------------------------------------
 * 'buy' command
 *---------------------------------------------------------------------------*/
CMDRUN( buy )
{
    if (constArguments.empty( ))
    {
	ch->send_to("������ ���?\n\r");
	return;
    }

    char buf[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];
    char argument[MAX_INPUT_LENGTH];
    int	cost, roll;
    NPCharacter*  keeper;
    ShopTrader::Pointer trader;
    Object*	    obj,*t_obj;
    int	    number, count = 1;

    if ( !( trader = find_keeper( ch ) ) )
	return;
    
    keeper = trader->getChar( );
    strcpy( argument, constArguments.c_str( ) );
    number = mult_argument( argument, arg );
    obj  = get_obj_keeper( ch, trader, arg );
    cost = get_cost( keeper, obj, true, trader );

    if ( cost <= 0 || !ch->can_see( obj ) )
    {
	act_p( "$c1 ������� ���� '{g� �� ������ ����� - ��������� 'list'{x'.", keeper, 0, ch, TO_VICT, POS_RESTING );
	ch->reply = keeper;
	return;
    }


    if ( !IS_OBJ_STAT( obj, ITEM_INVENTORY ) ) {
	for ( t_obj = obj->next_content; count < number && t_obj; t_obj = t_obj->next_content )
	{
	    if ( t_obj->pIndexData == obj->pIndexData
		&& !str_cmp( t_obj->getShortDescr( ), obj->getShortDescr( ) ) )
	    {
		count++;
	    }
	    else
		break;
	}

	if ( count < number ) {
	    act_p( "$c1 ������� ���� '{g� ���� ��� �������.{x'",
	    keeper, 0, ch, TO_VICT, POS_RESTING );
	    ch->reply = keeper;
	    return;
	}
    }

    if (obj->pIndexData->limit > 0 && obj->pIndexData->limit < obj->pIndexData->count - 1 + number) {
	act_p("$c1 ������� ���� '{g������ ���������� �������� � ���� ����.{x'",
	keeper, 0, ch, TO_VICT, POS_RESTING );
	ch->reply = keeper;
	return;
    }

    if ( ( ch->silver + ch->gold * 100) < cost * number )
    {
	if ( number > 1 )
	    act_p( "$c1 ������� ���� '{g�� �� ������ ��������� �� �������.{x'",
		    keeper, obj, ch, TO_VICT, POS_RESTING );
	else
	    act_p( "$c1 ������� ���� '{g� ���� ��� ������ �����, ���� ������ $o4.{x'",
		    keeper, obj, ch, TO_VICT,POS_RESTING );
	
	ch->reply = keeper;
	return;
    }

    if ( ch->getRealLevel( ) < get_wear_level( ch, obj ) )
    {
	act_p( "$c1 ������� ���� '{g�� �� ������� ������������ $o4{x'.",
		keeper, obj, ch, TO_VICT,POS_RESTING );

	ch->reply = keeper;
	return;
    }

    if ( ch->carry_number + number * obj->getNumber( ) > ch->canCarryNumber( ) )
    {
	ch->send_to("�� �� ������ ����� ��� ����� �����.\n\r");
	return;
    }

    if ( ch->carry_weight + number * obj->getWeight( ) > ch->canCarryWeight( ) )
    {
	ch->send_to("�� �� ������ ����� ����� �������.\n\r");
	return;
    }

    /* haggle */
    roll = number_percent( );
    if ( !IS_OBJ_STAT( obj, ITEM_SELL_EXTRACT ) && roll < gsn_haggle->getEffective( ch ) )
    {
	cost -= obj->cost / 2 * roll / 100;
	act_p( "�� ���������� � $C5.", ch, 0, keeper, TO_CHAR, POS_RESTING );
	gsn_haggle->improve( ch, true );
    }

    if ( number > 1 )
    {
	sprintf( buf, "$c1 �������� $o4[%d].", number );
	act_p( buf, ch, obj, 0, TO_ROOM, POS_RESTING );
	sprintf( buf, "�� ��������� $o4[%d] �� %d ��������%s.",
			number, cost * number,
			GET_COUNT( cost * number, "�� ������", "�� ������", "�� �����" ) );
	act_p( buf, ch, obj, 0, TO_CHAR, POS_RESTING );
    }
    else
    {
	act_p( "$c1 �������� $o4.", ch, obj, 0, TO_ROOM,POS_RESTING );
	sprintf( buf, "�� ��������� $o4 �� %d ��������%s.",
			cost, GET_COUNT( cost, "�� ������", "�� ������", "�� �����" ) );
	act_p( buf, ch, obj, 0, TO_CHAR,POS_RESTING );
    }

    deduct_cost( ch, cost * number );
    mprog_sell( keeper, ch, obj, cost, number );

    cost += keeper->silver;
    /* 'number' ��������� �� ���� � ����� - � ���� */
    dreamland->putToMerchantBank( cost * number / 100 );
    /* �������� ����� � ����� � ������� ��, ��� ���� � ���� */
    keeper->silver = cost * number - ( cost * number / 100 ) * 100;

    for ( count = 0; count < number; count++ )
    {
	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) 
		&& (obj->pIndexData->limit < 0 
	    || obj->pIndexData->limit > obj->pIndexData->count))
	{
	    t_obj = create_object( obj->pIndexData, obj->level );
	}
	else
	{
	    t_obj = obj;
	    obj = obj->next_content;
	    obj_from_char( t_obj );
	}

	if ( t_obj->timer > 0 && !IS_OBJ_STAT( t_obj, ITEM_HAD_TIMER ) )
	    t_obj->timer = 0;
	
	REMOVE_BIT( t_obj->extra_flags, ITEM_HAD_TIMER );
	obj_to_char( t_obj, ch );

	if ( cost < t_obj->cost )
	    t_obj->cost = cost;
    }
}

/*----------------------------------------------------------------------------
 * 'sell' command
 *---------------------------------------------------------------------------*/
CMDRUN( sell )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    NPCharacter *keeper;
    ShopTrader::Pointer trader;
    Object *obj;
    int cost;
    int roll;
    int gold, silver;
    DLString arg = constArguments;

    if (arg.empty( ))
    {
	ch->send_to("������� ���?\n\r");
	return;
    }

    if ( !( trader = find_keeper( ch ) ) )
	return;
    
    keeper = trader->getChar( );

    ShopStock stock = get_stock_keeper( trader, NULL, "" );
    if (stock.size( ) > 25) {
        tell_dim( ch, keeper, "� ������ �� �������! ��� ������ ������� �����!");
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg.c_str( ) ) ) == 0 )
    {
	act_p( "$c1 ������� ���� '{g� ���� ��� �����.{x'",
	keeper, 0, ch, TO_VICT,POS_RESTING );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	ch->send_to("�� �� ������ ���������� �� �����.\n\r");
	return;
    }

    if (!keeper->can_see(obj))
    {
	act_p("$c1 �� ����� �����.",keeper,0,ch,TO_VICT,POS_RESTING);
	return;
    }

    if ( ( cost = get_cost( keeper, obj, false, trader ) ) <= 0 )
    {
	act_p( "$c1 �� ������������ $o5.", keeper, obj, ch, TO_VICT,POS_RESTING );
	return;
    }

    if ( (cost / 100 + 1) > dreamland->getBalanceMerchantBank() )
    {
	act_p("$c1 ������� ���� '{g� ���� ��� �����, ���� ��������� ���� �� $o4.{x'",
	      keeper,obj,ch,TO_VICT,POS_RESTING);
	return;
    }

    /* haggle */
    roll = number_percent();

    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) && roll < gsn_haggle->getEffective( ch ))
    {
	roll = gsn_haggle->getEffective( ch ) + number_range(1, 20) - 10;
	ch->send_to("�� ���������� � ���������.\n\r");

	cost += obj->cost / 2 * roll / 100;
	cost = min(cost,95 * get_cost(keeper,obj,true, trader) / 100);
	//cost = min(cost, static_cast<int>( keeper->silver + 100 * keeper->gold ) );
	gsn_haggle->improve( ch, true );
	
	if ( (cost / 100 + 1) > dreamland->getBalanceMerchantBank() )
	{
	    act_p("$c1 ������� ���� '{g� ���� ��� �����, ���� ��������� ���� �� $o4.{x'",
	    keeper,obj,ch,TO_VICT,POS_RESTING);
	    return;
	}
    }

    act_p( "$c1 ������� $o4.", ch, obj, 0, TO_ROOM,POS_RESTING );
    silver = cost - (cost/100) * 100;
    gold   = cost/100;

    sprintf( buf2, "�� �������� $o4 �� %s%s%s.",
		    silver != 0 ? "%d �������":"",    
		    ( silver != 0 && gold != 0 ) ? " � ":"",	      
		    gold != 0 ? "%d ������":"");		      

    if (silver != 0 && gold != 0)
	sprintf( buf, buf2, silver, gold );
    else if (silver != 0 )
	sprintf( buf, buf2, silver );
    else
	sprintf( buf, buf2, gold );

    act_p( buf, ch, obj, 0, TO_CHAR,POS_RESTING );

    if ( cost <= keeper->silver )
	keeper->silver -= cost;
    else
    {
	cost -= keeper->silver;
	
	if ( !dreamland->getFromMerchantBank( cost / 100 + 1 ) )
	{
	    act_p("$c1 ������� ���� '{G� ���� ��� �����.{x'",
	    keeper,0,ch,TO_VICT,POS_RESTING);
	    return;
	}

	keeper->silver = ( cost / 100 + 1 ) * 100 - cost;
    }

    int gold_old = ch->gold;
    int silver_old = ch->silver;
    ch->gold     += gold;
    ch->silver   += silver;

    if ( ch->getCarryWeight( ) > ch->canCarryWeight( ) )
    {
	ch->gold = gold_old;
	ch->silver = silver_old;
	act_p( "$c1 �������� ���� ���� ������, �� �� �� ������ �� ��������.",
	keeper, 0, ch, TO_VICT,POS_RESTING );
	act_p( "$c1 ������ �� ��� ����� �����.", ch,0,0,TO_ROOM,POS_RESTING );
	obj_to_room( create_money( gold, silver ), ch->in_room );
    }

    if ( obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) )
    {
	extract_obj( obj );
    }
    else
    {
	obj_from_char( obj );
	
	if (obj->timer)
	    SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	else
	    obj->timer = number_range(50,100);
	
	obj_to_keeper( obj, keeper );
    }
}

/*----------------------------------------------------------------------------
 * 'list' command
 *---------------------------------------------------------------------------*/

CMDRUN( list )
{
    NPCharacter* keeper;
    ShopTrader::Pointer trader;
    DLString arg = constArguments;
    ostringstream buf;

    if ( !( trader = find_keeper( ch ) ) )
	return;
    
    keeper = trader->getChar( );
    ShopStock stock = get_stock_keeper( trader, ch, arg );

    if (stock.empty( )) {
	if (arg.empty( ))
	    tell_dim( ch, keeper, "��� ������� ������ ���� ����������.");
	else
	    tell_dim( ch, keeper, "� �� ������ '$t'.", arg.c_str( ) );
        return;
    }

    buf << "[ ���.| ��.  ���� ���-��] �����" << endl;

    for (int i = 0; i < stock.size( ); i++) {
        const StockInfo &si = stock.at( i );
        
	if (IS_OBJ_STAT( si.obj, ITEM_INVENTORY ))
	    buf << dlprintf( "[ {Y%3d{x |%3d %5d   --   ] ",
                    i+1, si.obj->level, si.cost );

        else 
	    buf << dlprintf( "[ {Y%3d{x |%3d %5d %6d ] ",
                    i+1, si.obj->level, si.cost, si.count );

        webManipManager->decorateShopItem( buf, si.obj->getShortDescr( '1' ), si.obj, ch );
        buf << endl;
    }

    ch->send_to( buf );
    tell_dim( ch, keeper, "����� ��� �������� ������, � � �������� �ӣ, ��� � ��� ����, �� 1%% �� ���������." );
}

/*----------------------------------------------------------------------------
 * 'value' command
 *---------------------------------------------------------------------------*/
CMDRUN( value )
{
    char buf[MAX_STRING_LENGTH];
    NPCharacter *keeper;
    ShopTrader::Pointer trader;
    Object *obj;
    int cost;
    DLString arg = constArguments;

    if (arg.empty( ))
    {
	ch->send_to("������ ���� ����?\n\r");
	return;
    }

    if ( !( trader = find_keeper( ch ) ) )
	return;
    
    keeper = trader->getChar( );

    if ( ( obj = get_obj_carry( ch, arg.c_str( ) ) ) == 0 )
    {
	act_p( "$c1 ������� ���� '{g� ���� ��� �����.{x'",
		keeper, 0, ch, TO_VICT,POS_RESTING );
	ch->reply = keeper;
	return;
    }

    if (!keeper->can_see(obj))
    {
	act_p("$c1 �� ����� �����.",keeper,0,ch,TO_VICT,POS_RESTING);
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	ch->send_to("�� �� ������ ���������� �� �����.\n\r");
	return;
    }

    if ( ( cost = get_cost( keeper, obj, false, trader ) ) <= 0 )
    {
	act_p("$c1 �� ������������ $o5.", keeper, obj, ch, TO_VICT,POS_RESTING );
	return;
    }

    if ( dreamland->getBalanceMerchantBank() < (cost / 100 + 1) )
    {
	sprintf( buf,
		"$c1 ������� ���� '{g� ��� �� ���� %d ������� � %d ������ �� $o4, �� � ���� ��� �����.{x'",
		cost - (cost/100) * 100, cost/100 );
    }
    else
    {	
	sprintf( buf,
		"$c1 ������� ���� '{g� ��� ���� %d ������� � %d ������ �� $o4.{x'",
		cost - (cost/100) * 100, cost/100 );
    }

    act_p( buf, keeper, obj, ch, TO_VICT,POS_RESTING );
    ch->reply = keeper;
}

/*----------------------------------------------------------------------------
 * 'properties' command
 *---------------------------------------------------------------------------*/
CMDRUN( properties )
{
    ShopTrader::Pointer trader;
    DLString arg = constArguments;

    if (arg.empty( ))
    {
	ch->send_to("������ �������������� ����?\n\r");
	return;
    }

    if ( !( trader = find_keeper( ch ) ) )
	return;
    
    trader->describeGoods( ch, arg, true );
}

/*
 * Local functions
 */
int get_cost( NPCharacter *keeper, Object *obj, bool fBuy, ShopTrader::Pointer trader ) 
{
    int cost;

    if(!obj)
	return 0;

    if( IS_OBJ_STAT( obj, ITEM_NOSELL ) ) 
	return 0;

    if( fBuy ) {
	cost = obj->cost * trader->profitBuy  / 100;
    } else {
	Object *obj2;

	cost = 0;
    
	if (trader->buys.isSetBitNumber( obj->item_type ))
	    cost = obj->cost * trader->profitSell / 100;

	if( !IS_OBJ_STAT( obj, ITEM_SELL_EXTRACT ) )
	    for( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content ) {
		if( obj->pIndexData == obj2->pIndexData &&
		    !str_cmp( obj->getShortDescr( ), obj2->getShortDescr( ) ) )
		{
		    cost /= 2;
		}
	    }
    }

    if( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND ) {
	if( !obj->value[1] ) 
	    cost /= 4;
	else 
	    cost = cost * obj->value[2] / obj->value[1];
    }

    return cost;
}

ShopTrader::Pointer find_keeper( Character *ch )
{
    NPCharacter *keeper = 0;
    ShopTrader::Pointer trader, null;
    
    trader = find_attracted_mob_behavior<ShopTrader>( ch, OCC_SHOPPER );
    if (!trader) {
	ch->send_to("����� ��� ���������.\n\r");
	return null;
    }

    keeper = trader->getChar( );

    if (!IS_AWAKE(keeper)) {
	interpret_raw( keeper, "snore" );
	return null;
    }
    
    if ( IS_SET(keeper->in_room->area->area_flag,AREA_HOMETOWN)
	 && !ch->is_npc() && IS_SET(ch->act,PLR_WANTED) )
    {
	do_say( keeper, "������������ �� ����� �����!" );
        DLString msg = fmt( 0, "%1$C1 - ���������%1$G�|�|��|��! �������� %1$P2!", ch );
	do_yell( keeper, msg.c_str( ) );
	return null;
    }

    /*
    * Shop hours.
    */
    if (time_info.hour > trader->closeHour) 
    {
	do_say( keeper, "������, ������� ��� ������. ������� ������." );
	return null;
    }

    /*
    * Invisible or hidden people.
    */
    if ( !keeper->can_see( ch ) && !ch->is_immortal() )
    {
	do_say( keeper, "� �� ������ � ���, ���� �� ����." );
	return null;
    }

    if (keeper->fighting) {
	do_say( keeper, "������� �������, ��� ������ �� �� ����." );
	return null;
    }
/*    
    if (ch->getCurrStat( STAT_CHA ) < 18) {
	switch (number_range( 1, 10 )) {
	case 1:
	    do_say( keeper, "���������� ��� ���� ������������, �� ���� � ���� �����������." );
	    return 0;
	case 2:
	    do_say();
	    return 0;
	}
    }
*/  
    return trader;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper( Object *obj, NPCharacter *ch )
{
    Object *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != 0; t_obj = t_obj_next)
    {
	t_obj_next = t_obj->next_content;

	if (obj->pIndexData == t_obj->pIndexData
	    &&  !str_cmp(obj->getShortDescr( ), t_obj->getShortDescr( )))
	{
	    if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY))
	    {
		extract_obj(obj);
		return;
	    }

	    obj->cost = t_obj->cost; /* keep it standard */
	    break;
	}
    }

    if (t_obj == 0)
    {
	obj->next_content = ch->carrying;
	ch->carrying = obj;
    }
    else
    {
	obj->next_content = t_obj->next_content;
	t_obj->next_content = obj;
    }

    obj->carried_by      = ch;
    obj->in_room         = 0;
    obj->in_obj          = 0;
    ch->carry_number    += obj->getNumber( );
    ch->carry_weight    += obj->getWeight( );
}


/* get an object from a shopkeeper's list */
Object *get_obj_keeper( Character *ch, ShopTrader::Pointer trader, const DLString &constArguments ) 
{
    char arg[MAX_INPUT_LENGTH];
    char argument[MAX_INPUT_LENGTH];
    Object *obj;
    int number;
    int count;
    int item, item_number;
    NPCharacter *keeper = trader->getChar( );

    strcpy( argument, constArguments.c_str( ) );
    long long id = get_arg_id( argument );

    if( is_number( argument ) && !id) {
	item_number = atoi( argument );
	
	for( obj = keeper->carrying, item = 1; obj;obj = obj->next_content ) {
	    if( obj->wear_loc == wear_none 
		&& get_cost( keeper, obj, true, trader ) > 0 
		&& ch->can_see( obj ) )  
	    {
		/* skip other objects of the same name */
		while( obj->next_content &&
			obj->pIndexData == obj->next_content->pIndexData &&
			!str_cmp( obj->getShortDescr( ), obj->next_content->getShortDescr( ) ) )
		    obj = obj->next_content;
		
		if( item == item_number )  
		    return obj;
		
		item++;
	    }
	}
	
	return 0;
	
    } else {
	number = number_argument( argument, arg );
	count = 0;
	
	for( obj = keeper->carrying; obj; obj = obj->next_content ) {
	    if( obj->wear_loc == wear_none &&
		keeper->can_see( obj ) &&
		ch->can_see( obj )  &&
		get_cost( keeper, obj, true, trader ) > 0 &&
                ((id && obj->getID( ) == id) || (!id && obj_has_name( obj, arg, ch ))))
	    {
                
                if (!id)
                    /* skip other objects of the same name */
                    while( obj->next_content &&
                            obj->pIndexData == obj->next_content->pIndexData &&
                            !str_cmp( obj->getShortDescr( ), obj->next_content->getShortDescr( ) ) )
                        obj = obj->next_content;
		
		if(id || ++count == number ) 
		    return obj;
	    }
	}
    }

    return 0;
}

/* deduct cost from a character */
void deduct_cost(Character *ch, int cost)
{
	int silver = 0, gold = 0;

	silver = min((int)ch->silver,cost);

	if (silver < cost)
	{
		gold = ((cost - silver + 99) / 100);
		silver = cost - 100 * gold;
	}

	ch->gold -= gold;
	ch->silver -= silver;

	if (ch->gold < 0)
	{
		bug("deduct costs: gold %d < 0",ch->gold.getValue( ));
		ch->gold = 0;
	}
	if (ch->silver < 0)
	{
		bug("deduct costs: silver %d < 0",ch->silver.getValue( ));
		ch->silver = 0;
	}
}

