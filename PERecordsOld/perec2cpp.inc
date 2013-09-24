?include "perecords.h"

/******************************************
typedef struct _RECFIELD {
	char * Name;
	int size;
	int type;
	RECFIELD * Ref;
} RECFIELD;

 Типы данных
00 - Неизвестный тип
10 - hex
01 - Символьный тип
02 - Знаковый целый
03 - Беззнаковый целый
04 - Плавающая запятая
05 - Флаги
06 - Структура

*******************************************/

// Создать описания структур
#define reserved(s, n) {0, 4 * n, 0},
#define reservedb(s, n) {0, n, 0},
#define char(s, n) {#s, n, DT_CHAR},
#define charh(s, n) {#s, n, DT_CHAR | DT_HEX},
#define word(s) {#s, 2, DT_HEX},
#define cardinal(s) {#s, 4, DT_HEX},
#define cardinal_d(s, d) {d, 4, DT_UNSIGNED},
#define pointer(s) {#s, 4, DT_HEX},
#define pchar_d(s, d) {d, 4, DT_PCHAR},
#define word_d(s, d) {d, 2, DT_HEX},
#define pointer_d(s, d) {d, 4, DT_HEX},
#define precord(s, r) {#s, 4, DT_PRECORD, RF_##r},
#define records(s, r) {#s, 8, DT_PRECORD, RF_##r},
#define PreDefRec(r)
#define StartRec(r) RECFIELD RF_##r[] = {
#define EndRec(r) {0, 0, 0}};
#define _

#include "PERecords.inc"

#undef StartRec
#undef EndRec
//#undef word
#undef cardinal
#undef cardinal_d
#undef pointer
#undef _
#undef reserved
#undef reservedb
#undef char
#undef pchar_d
#undef word_d
#undef pointer_d
#undef charh
#undef precord
