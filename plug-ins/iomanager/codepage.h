/* $Id$
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

#ifndef _CHARSET_H_
#define _CHARSET_H_

#define	NCODEPAGES		7

struct codepage_t {
  const char *		name;
  unsigned char *	from;
  unsigned char *	to;
};

extern unsigned char koi8_koi8[256];
extern unsigned char alt_koi8[256];
extern unsigned char koi8_alt[256];
extern unsigned char win_koi8[256];
extern unsigned char koi8_win[256];
extern unsigned char iso_koi8[256];
extern unsigned char koi8_iso[256];
extern unsigned char mac_koi8[256];
extern unsigned char koi8_mac[256];
extern unsigned char tran_koi8[256];
extern unsigned char koi8_tran[256];

extern codepage_t russian_codepages[];

#endif
