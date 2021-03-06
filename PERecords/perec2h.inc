?ifndef _PERecords_
?define _PERecords_



//! �������� ������ ����
typedef struct _RECFIELD {
	char * Name;
	int Size;
	int Type;
	void * Desc;
} RECFIELD;

// ���� ������

?define DT_NULL 	0x000	// ����������� ���
?define DT_HEX  	0x001
?define DT_CHAR		0x002 	// ���������� ���
?define DT_SIGNED	0x004	// �������� �����
?define DT_UNSIGNED 	0x008	// ����������� �����
?define DT_FLOAT 	0x010	// ��������� �������
?define DT_FLAGS	0x020	// �����
?define DT_ENUM		0x040	// ������������
?define DT_PRECORD	0x080	// ���������
?define DT_PCHAR	0x200   // ������

#define DT_NULL 	0x000	// ����������� ���
#define DT_HEX  	0x001
#define DT_CHAR		0x002 	// ���������� ���
#define DT_SIGNED	0x004	// �������� �����
#define DT_UNSIGNED 	0x008	// ����������� �����
#define DT_FLOAT 	0x010	// ��������� �������
#define DT_FLAGS	0x020	// �����
#define DT_ENUM		0x040	// ������������
#define DT_PRECORD	0x080	// ���������
#define DT_PCHAR	0x200   // ������



// ������� ��������� ��������
#define reserved(s, n) unsigned int s[n];
#define reservedb(s, n) unsigned char s[n];
#define char(s, n) char s[n];
#define charh(s, n) char s[n];
#define word(s) unsigned short s;
#define word_d(s, d) unsigned short s;
#define cardinal(s) unsigned int s;
#define cardinal_d(s, d) unsigned int s;
#define pointer(s) char * s;
#define pointer_d(s, d) char * s;
#define pchar_d(s, d) char * s;
#define precord(s, r) _##r * s;
#define records(s, r) _##r * s; int s##_count;
#define PreDefRec(r) typedef struct _##r;
#define StartRec(r) RECFIELD RF_##r[]; typedef struct _##r {
#define EndRec(r) } r;
#define _

#include "PERecords.inc"

#undef _
#undef StartRec
#undef EndRec
#undef word
#undef word_d
#undef cardinal
#undef cardinal_d
#undef char
#undef pchar_d
#undef pointer_d
#undef charh
#undef reserved
#undef reservedb
#undef pointer
#undef precord
#undef records

?endif