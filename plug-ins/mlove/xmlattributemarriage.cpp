/* $Id: xmlattributemarriage.cpp,v 1.1.2.4.22.1 2007/09/11 00:34:17 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "xmlattributemarriage.h"

#include "pcharactermanager.h"
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

XMLAttributeMarriage::XMLAttributeMarriage( )
{
}

bool XMLAttributeMarriage::handle( const WhoisArguments &args )
{
    DLString buf;

    if (!spouse.empty( )) {
	PCMemoryInterface *spousePCM = PCharacterManager::find( spouse );

	if (spousePCM) {
	    if (wife)
		buf << "������� �� ";
	    else 
		buf << "�����" << GET_SEX( args.pch, "", "�", "�" ) << " �� ";
	} else {
	    if (wife)
		buf << "����� ";
	    else
		buf << "������ ";
	}

	buf << "{W" << spouse << "{w";
    } 

    if (!history.empty( )) {
	if (!spouse.empty( ))
	    buf << ", ";

	buf << "���" << GET_SEX( args.pch, " �����(�������)", "�", "� �������(������)" ) 
	    << " {W" << history.size( ) 
	    << "{w ���" << GET_COUNT( history.size( ), "", "�", "" );
    }

    if (!buf.empty( )) {
	args.lines.push_back( buf );
	return true;
    }

    return false;
}


