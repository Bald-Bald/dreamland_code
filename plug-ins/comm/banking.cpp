/* $Id$
 *
 * ruffina, 2004
 */
#include "banking.h"
#include "arg_utils.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "act.h"
#include "descriptor.h"

/*------------------------------------------------------------------------
 * BankAction
 *-----------------------------------------------------------------------*/
bool BankAction::handleCommand( Character *ch, const DLString &cmdName, const DLString &cmdArgs )
{
    DLString args = cmdArgs;
    
    if (ch->is_npc( ))
	return false;
    
    if (cmdName == "balance") {
	doBalance( ch->getPC( ) );
	return true;
    }
    
    if (cmdName == "deposit") {
	doDeposit( ch->getPC( ), args );
	return true;
    }

    if (cmdName == "withdraw") {
	doWithdraw( ch->getPC( ), args );
	return true;
    }

    return false;
}

/*
 * 'balance' command
 */
void BankAction::doBalance( PCharacter *ch )
{
    long bank_g, bank_s;

    if ( ch->bank_s + ch->bank_g == 0 )  {
	ch->send_to("� ���� ��� ������� ����� � �����.\n\r");
	return;
    }

    bank_g = ch->getPC( )->bank_g;
    bank_s = ch->getPC( )->bank_s;

    if  (bank_g!=0 && bank_s!=0)
	ch->printf( "� ���� %ld �����%s � %ld ��������%s � �����.\n\r",
	 bank_g,GET_COUNT(bank_g,"��","��","��"),
	 bank_s,GET_COUNT(bank_s,"�� ������","�� ������","�� �����"));

    if  (bank_g!=0 && bank_s == 0)
	ch->printf( "� ���� %ld �����%s � �����.\n\r",
	 bank_g,GET_COUNT(bank_g,"�� ������","�� ������","�� �����"));

    if  (bank_g == 0 && bank_s!= 0)
	ch->printf( "� ���� %ld ��������%s � �����.\n\r",
	 bank_s,GET_COUNT(bank_s,"�� ������","�� ������","�� �����"));
}

/*
 * 'withdraw' command
 */
void BankAction::doWithdraw( PCharacter *ch, DLString &arguments )
{
    ostringstream buf;
    int amount_s, amount_g, amount;
    DLString argOne, argTwo;
    
    argOne = arguments.getOneArgument( );
    argTwo = arguments.getOneArgument( );
    amount_s = amount_g = amount = 0;
    
    if (argOne.empty( ) 
	|| argTwo.empty( )
	|| !argOne.isNumber( ) 
	|| (!arg_is_gold( argTwo) && !arg_is_silver( argTwo )))
    {
	ch->println( "����� ����� � �������� �������. ��������: '{lR������� 77 ������{lEwithdraw 77 gold{x' ��� '{lR������� 9000 �������{lEwithdraw 9000 silver{lx'.");
	return;
    }
    
    try {
	amount = argOne.toInt( );
    } catch (const ExceptionBadType &) {
	ch->println( "����� ������� �������." );
	return;
    }

    if (amount <= 0) {
	ch->println( "����� �����." );
	return;
    }

    if (arg_is_silver( argTwo ))
	amount_s = amount;
    else
	amount_g = amount;
    
    if (amount_g > ch->bank_g || amount_s > ch->bank_s) {
	ch->send_to("������, �� �� ���� ������.\n\r");
	return;
    }

    if (amount_s > 0 )
    {
	if (amount_s < 10)
	{
	    if (amount_s == 1)
		buf << "����";
	    else
		buf << amount_s;

	    buf << " ����" << GET_COUNT(amount_s, "��", "��", "�")
		<< "?! �� ����?!" << endl;
	}
	else
	{
	    int fee = max( 1, amount_s / 10 );
	    ch->bank_s -= amount_s;
	    ch->silver += amount_s - fee;

	    buf << "�� �������� �� ����� " << amount_s 
	        << " ��������" << GET_COUNT(amount_s,"�� ������","�� ������","�� �����")
		<< "." << endl << "������ ����� ��������� " << fee 
		<< " ��������" << GET_COUNT(fee,"�� ������","�� ������","�� �����") << endl;
	}
    }

    if (amount_g > 0)
    {
	if (amount_g == 1)
	{
	    buf << "���� ������� ����" << GET_COUNT(amount_g, "��", "��", "�")
		<< "?! �� ����?!" << endl;
	}
	else
	{
	    int fee = max( 1, amount_g / 50 );
	    ch->bank_g -= amount_g;
	    ch->gold += amount_g - fee;

	    buf << "�� �������� �� ����� " << amount_g 
	        << " �����" << GET_COUNT(amount_g,"�� ������","�� ������","�� �����")
		<< "." << endl << "������ ����� ��������� " << fee 
		<< " �����" << GET_COUNT(fee,"�� ������","�� ������","�� �����") << endl;
	}
    }

    ch->send_to(buf);
    act("$c1 ���������� ���������� ��������.",ch,0,0,TO_ROOM);
}

/*
 * 'deposit' command
 */
void BankAction::doDeposit( PCharacter *ch, DLString &arguments )
{
    std::ostringstream mbuf, cbuf, vbuf;
    PCharacter *victim;
    long amount, amount_s, amount_g;
    DLString arg;

    if (arguments.empty( )) {
	ch->send_to("�������� �� ���� �������?\n\r");
	return;
    }
    
    amount_s = amount_g = 0;
    victim = ch->getPC( );

    while (!( arg = arguments.getOneArgument( ) ).empty( )) {
	if (arg.isNumber( )) {
	    try {
		if (( amount = arg.toInt( ) ) <= 0)
		    throw Exception( );
	    }
	    catch (const Exception &) {
		ch->send_to( "����� ������� �������.\r\n" );
		return;
	    }

	    if (( arg = arguments.getOneArgument( ) ).empty( )) {
		ch->send_to( "����� �������� �������: {lR������ ��� �������{lEgold ��� silver{lx.\r\n" );
		return;
	    }

	    if (arg_is_silver( arg ) && amount_s == 0)
		amount_s = amount;
	    else if (arg_is_gold( arg ) && amount_g == 0)
		amount_g = amount;
	    else {
		ch->send_to("�� ������ �������� �� ���� ������ ������� ��� ���������� ������.\r\n"
		            "������: {lEdeposit 3 silver 18 gold{lR������ 3 ������� 18 ������{lx\r\n" );
		return;
	    }
	}
	else if (victim != ch) {
	    ch->send_to( "����� ����� ������ ������ �� �����. ��������� ���..\r\n" );
	    return;
	}
	else {
	    PCMemoryInterface *pci = PCharacterManager::find( arg );

	    if (!pci) {
		ch->send_to( "������� �������� �� ������. ����� ��� ��������� � ���������.\r\n" );
		return;
	    }

	    if (!( victim = dynamic_cast<PCharacter *>( pci ) ) || !ch->can_see( victim )) {
		ch->send_to( "��������� ��� ������������� � ���� ������� �� ������ ����� ��������.\r\n" );
		return;
	    }
	}
    }


    if (amount_g > ch->gold || amount_s > ch->silver) {
	ch->send_to("��� ������, ��� ���� � ���� � ��������.\n\r");
	return;
    }

    if ( (amount_g + victim->getPC( )->bank_g) > 100000 ) {
	ch->send_to("������ ����������� ����� �� ����� ��������� 100.000 �������.\n\r");
	return;
    }

    victim->getPC( )->bank_s += amount_s;
    victim->getPC( )->bank_g += amount_g;
    ch->gold -= amount_g;
    ch->silver -= amount_s;
    
    if (victim == ch)
	cbuf << "�� ���� ���������� ���� ";
    else {
	cbuf << "�� ���������� ���� {W" << victim->getName( ) << "{x ";
	vbuf << "{W" << ch->getName( ) << "{x ��������� �� ���� ���������� ���� ";
    }
    
    cbuf << "����������: ";
    
    if (amount_g > 0) 
	mbuf << "{Y" << amount_g << "{x �����" << GET_COUNT(amount_g,"�� ������","�� ������","�� �����");
    
    if (amount_s > 0) {
	if (amount_g > 0)
	    mbuf << " � ";

	mbuf << "{W" << amount_s << "{x ��������" << GET_COUNT(amount_s,"�� ������","�� ������","�� �����");
    }
    
    cbuf << mbuf.str( ) << "." << endl;
    vbuf << mbuf.str( ) << "." << endl;
    ch->send_to( cbuf );
    
    if (victim != ch) 
	victim->send_to( vbuf );

    act( "$c1 ���������� ���������� ��������.", ch, 0, 0, TO_ROOM );
}

/*------------------------------------------------------------------------
 * BankRoom
 *-----------------------------------------------------------------------*/
bool BankRoom::command( Character *ch, const DLString &cmdName, const DLString &cmdArgs )
{
    return handleCommand( ch, cmdName, cmdArgs );
}

/*------------------------------------------------------------------------
 * CreditCard
 *-----------------------------------------------------------------------*/
bool CreditCard::command( Character *ch, const DLString &cmdName, const DLString &cmdArgs )
{
    return handleCommand( ch, cmdName, cmdArgs );
}

bool CreditCard::hasTrigger( const DLString &t )
{
    return (t == "withdraw" || t == "deposit");
}

/*------------------------------------------------------------------------
 * TaxesListener
 *-----------------------------------------------------------------------*/
void TaxesListener::run( int oldState, int newState, Descriptor *d )
{
    PCharacter *ch;

    if (!d->character || !( ch = d->character->getPC( ) ))
	return;

    if (oldState != CON_READ_MOTD || newState != CON_PLAYING) 
	return;

    if (ch->is_immortal( ))
	return;
    
    if (ch->gold > 6000) {
	ch->printf("� ���� �������� %d ������� ������ �� ������ ��������� �����.\n\r",
	           (ch->gold - 6000) / 2);
	ch->gold -= (ch->gold - 6000) / 2;
    }
    

    if (ch->bank_g + ch->bank_s / 100 > 80000) {
	long silver;
	
	silver = ch->bank_g + ch->bank_s / 100 - 20000;
	
	ch->printf( "� ���� �������� %ld ������� ������ �� ������ ������� �������� �������.\n\r", silver );
    
	if( silver < ch->bank_s / 100 ) {
	    ch->bank_s -= ( silver * 100 );
	} else {
	    silver -= ( ch->bank_s / 100 );
	    ch->bank_s %= 100;
	    ch->bank_g -= silver;
	}
    }
}

