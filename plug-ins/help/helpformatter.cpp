/* $Id$
 *
 * ruffina, 2004
 */
#include "helpformatter.h"
#include "character.h"

HelpFormatter::HelpFormatter( )
                : fParse( true ), text( 0 )
{
}

HelpFormatter::~HelpFormatter( )
{
}

void HelpFormatter::reset( )
{
    fParse = true;
}

void HelpFormatter::setup( Character * )
{
}

// *...*     ->  {y...{w                      (bold text)
// _..._     ->  {D<{w...{D>{w                (italic with <>)
// =...=     ->  {c...{w                      (header text)
// (eng,���) ->  {lEeng{lR���{lx              (language choice)
// [...]     ->  {W...{w                      (reference)
// %KEYWORD%
void HelpFormatter::run( Character *ch, ostringstream &out )
{
    bool t_asterix = false;
    bool t_underline = false;
    bool t_equals = false;
    bool t_ref = false;
    bool t_bracket = false;
    
    reset( );

    if (!text)
	return;

    setup( ch );

    for (const char *p = text; *p; ++p) {
	if (*p == '%') {
	    DLString kw;

	    while (*++p && *p >= 'A' && *p <= 'Z')
		kw << *p;
	    
	    if (*p == '%') {
		if (handleKeyword( kw, out ))
		    continue;
	    }

	    out << "%" << kw << (*p ? *p : '\0');
	    continue;
	}

	if (!fParse) {
	    out << *p;
	    continue;
	}
	
	switch (*p) {    
	case '*':
	    t_asterix = !t_asterix;
	    t_bracket = false;
	    out << (t_asterix ? "{y" : "{w");
	    break;

	case '_':
	    t_underline = !t_underline;
	    t_bracket = false;
	    out << (t_underline ? "{D<{w" : "{D>{w");
	    break;

	case '=':
	    t_equals = !t_equals;
	    t_bracket = false;
	    out << (t_equals ? "{c" : "{w");
	    break;

	case '[':
	    if (!t_ref) {
		t_ref = true;
		t_bracket = false;
		out << "{W";
	    } else
		out << *p;
	    break;
	    
	case ']':
	    if (t_ref) {
		t_ref = false;
		t_bracket = false;
		out << "{w";
	    } else
		out << *p;
	    break;
	
	case '(':
	    if (t_ref || t_equals || t_underline || t_asterix) {
		out << "{lE";
		t_bracket = true;
	    } else
		out << *p;
	    break;

	case ',':
	    if (t_bracket) 
		out << "{lR";
	    else
		out << *p;
	    break;

	case ')':
	    if (t_bracket) {
		t_bracket = false;
		out << "{lx";
	    } else
		out << *p;
	    break;
	
	default:
	    out << *p;
	    break;
	}
    }
    
}

// %KEYWORD%:
// H, HELP     ->  {W{lEhelp{lR�������{lx{w
// SA, SEEALSO ->  ��. �����
// U, USAGE    ->  �������������
// FMT         ->  {w������:{w
// FFF         ->  {w       {w
// PAUSE       ->  stops help tag parsing
// RESUME      ->  resumes help tag parsing
// A           ->  * 
// CAST        ->  {lEcast{lR���������{lx
// OBJ         ->  �������
// CHAR        ->  ��������
// PLR         ->  �����
// MOB         ->  ������
// VICT        ->  ������
// DIR         ->  �����������
// YES, NO     ->  {lR��{lEyes{lx, {lR���{lEno{lx
// ALL         ->  {lR���{lEall{lx
// SHOW        ->  {lR�����{lEshow{lx
// ON          ->  {lR���{lEon{lx
// OFF         ->  {lR����{lEoff{lx
bool HelpFormatter::handleKeyword( const DLString &kw, ostringstream &out )
{
    if (kw == "H" || kw == "HELP") {
	out << "{W{lEhelp{lR?{lx{w";
	return true;
    }

    if (kw == "A") {
	out << "*";
	return true;
    }

    if (kw == "FMT") {
	out << "{w������:{w";
	return true;
    }

    if (kw == "FFF") {
	out << "{w       {w";
	return true;
    }

    if (kw == "U" || kw == "USAGE") {
	out << "�������������";
	return true;
    }

    if (kw == "SA" || kw == "SEEALSO") {
	out << "��. �����";
	return true;
    }

    if (kw == "CAST") {
	out << "{lEcast{lR���������{lx";
	return true;
    }

    if (kw == "OBJ") {
	out << "�������";
	return true;
    }

    if (kw == "CHAR") {
	out << "��������";
	return true;
    }

    if (kw == "PLR") {
	out << "�����";
	return true;
    }

    if (kw == "MOB") {
	out << "������";
	return true;
    }


    if (kw == "VICT") {
	out << "������";
	return true;
    }

    if (kw == "DIR") {
	out << "�����������";
	return true;
    }

    if (kw == "YES") {
	out << "{lR��{lEyes{lx";
	return true;
    }

    if (kw == "NO") {
	out << "{lR���{lEno{lx";
	return true;
    }

    if (kw == "ALL") {
	out << "{lR���{lEall{lx";
	return true;
    }

    if (kw == "SHOW") {
	out << "{lR�����{lEshow{lx";
	return true;
    }

    if (kw == "ON") {
	out << "{lR���{lEon{lx";
	return true;
    }

    if (kw == "OFF") {
	out << "{lR����{lEoff{lx";
	return true;
    }

    if (kw == "PAUSE") {
	fParse = false;
	return true;
    }
    
    if (kw == "RESUME") {
	fParse = true;
	return true;
    }

    return false;
}

DefaultHelpFormatter::DefaultHelpFormatter( const char *text )
{
    this->text = text;
}

DefaultHelpFormatter::~DefaultHelpFormatter( )
{
}

