#ifndef _ASSEMBLER_
#define _ASSEMBLER_
#include "..\units\stdgui.h"
#include "Binary.h"

// ������������ ���� ����������, ������������ � ���������� ������
#define X86_SegReg	0x00000007	///< DS ������������ ���������� �������
#define X86_o32		0x00000008	///< 1 ����������� ���������
#define X86_a32		0x00000010	///< 1 ����������� �������
#define X86_ModRM	0x00000020	///< 0 ����������� ModRM
#define X86_RO		0x00000040	///< 0 ������������ RO ��� ��� ��������
#define X86_Cond	0x00000080	///< 0 ������������ �������
#define X86_Prefix	0x00000100
#define X86_r		0x00000200	///< ������� � ��������� �����
#define X86_w		0x00000400	///< ������� � ��������� �����
#define X86_s		0x00000800	///< ������ ����������������� �������� ������� � ��������� �����
#define X86_d		0x00001000	///< ����������� � ��������� �����
//#define X86_i8      0x00002000  ///< ���������������� �������
#define X86_ac		0x00002000	///< ����������� - �������
#define X86_im		0x00004000	///< �������� � �������
#define X86_i8		0x00008000
#define X86_i16		0x00010000
#define X86_i32		0x00020000
#define X86_i		0x00040000
#define X86_jadd	0x00080000  ///< ������� �������� ������
#define X86_d1		0x00100000	///< ���������� d � 1
#define X86_w2	    0x00200000  ///< w �������� ������ ������� ��������
#define X86_1CL		0x00400000
#define X86_O32D	0x00800000	///< ��������� D ���� X86_o32

typedef struct _REGISTER
{
	char * Name;
	int Size;
	DWORD Offset;
} REGISTER;

REGISTER Registers[];

typedef enum TAsmType {otInt, otSigned, otUnsigned, otFloat}; ///< ���� ������
typedef enum TStoreType {ptNone, ptRegister, ptStack, ptConst}; // ��� ��������: ��������, ��������� ���������, ��������� (���� ��������� �� ������ � Pos, �� Pos - ���������)
typedef enum TCommandType {cPrimary, cEA, cStack, cUnknown}; ///< ��� �������: ��������������� �������, ����� ������� ������������ ������ ��� ������ �� ������

typedef struct _INSTRUCTION
{
	long Code;		///< ��� ����������
	long CodeMask;		///< ����� ��� ��������� ����
	long CodeLen;   ///< ����� ����
	char * Name;
	long StateMask;	///< �������� � ����������: 0 - XOR, 1 - MOV
	long StateSet;	///< �������� �������� � ����������
} INSTRUCTION;


typedef struct _OPERAND {
	TAsmType AsmType;
	TStoreType StoreType; // ��� ��������: ��������, ��������� ���������, ��������� (���� ��������� �� ������ � Pos, �� Pos - ���������)
	int Size;
	int Pos; // �������� Pos: 
	inline void SetRegister(REGISTER * Register) 
	{ 
		StoreType = ptRegister; 
		Size = Register->Size;
		Pos = Register->Offset;
	}
	inline void SetStackEA(int Size)
	{
		AsmType = otInt;
		Pos = 0;
		this->Size = Size;
		StoreType = ptStack;
	}
	inline void SetConst(long Const, int Size)
	{
		AsmType = otInt;
		Pos = Const;
		this->Size = Size;
		StoreType = ptConst;
	}
} OPERAND;


typedef struct _COMMAND
{
	char * Name;
	TCommandType Type;
	OPERAND Operands[4]; ///< ������� ������� - ���������.
} COMMAND;



INSTRUCTION x86InstructionSet[];

//#pragma pack(push, 1)

/// �������� ����� ModR/M
typedef /*__declspec(align(1))*/ struct _MODRM
{
	unsigned RM  : 3; ///< ������� ��� ����� ���������
	unsigned RO  : 3; ///< ������� ��� ����������� ���� �������
	unsigned MOD : 2; ///< ����� ��������� 
	// 00 - �������� �����������
	// 01 - ������������ 8-������ ��������
	// 10 - ������������ 16-32-������ ��������
	// 11 - ������������ ������� RM.
} MODRM;

/// �������� ����� SIB. ����������� ����� = I << S + B
typedef struct _SIB
{
	unsigned B : 3; ///< ������� ���� 
	unsigned I : 3; ///< ��������� ������� (���� ESP - ������� ���)
	unsigned S : 2; ///< ���������������
} SIB;

class TX86Codec
{
private:
	/// ��������� ���� ���������� ����� ���� �������
	typedef struct _LASTBYTE
	{
		union 
		{
			unsigned char Byte; ///< ��������� ����
			unsigned c : 4;		///< �������
			unsigned r : 3; ///< �������
			struct
			{
				unsigned w : 1; ///< ����������� ������� 8-16/32
				unsigned sd : 1;/*!< ���������������� ������� s 8 - 16/32 ��� 
								     ����������� d R/O > R/M - R/M > R/O */
			};
		};
	} LASTBYTE;
	unsigned int State;	///< ������� ��������� ��������
	DWORD FOSize, FASize; ///< ������ �������� � ������ � ����� 
	COMMAND FCommands[8]; ///< ����� ����������
	COMMAND * FCommand;
	MODRM ModRM;
	DWORD DecodeModRM(OPERAND * Result, int OpSize);
	inline REGISTER * DecodeSIB(DWORD MOD, bool * RegUsed);
	void AddMemRef(REGISTER * R, int OpSize);
	void AddConstMemRef(long Const, int OpSize);
	COMMAND * AddEACommand(REGISTER * A, REGISTER * B, char * Command, int Size);
	COMMAND * AddEACommand(REGISTER * A, long Const, char * Command, int Size);
public:
	char * CommandsToText(COMMAND * Commands);
	void * Position; ///< �������� ���������� ������������ ������
	void * Address;  ///< ������ ���������� ������������ ������
	int FSize; ///< ���������� �� ����� ������
	COMMAND * Decode(void);
	void * InitByDump(TDumpSource * Source, void * VA, DWORD Size);
};

//#pragma pack(pop)


class TAsmGrid: public TCustomGrid
{
private:
	TX86Codec Codec;
	TDumpSource * FSource;
	virtual void OnPaint(PAINTSTRUCT * PaintStruct); //!< ��������� ��������� ����������
	void UpdateCaret(void);
	int FCaretPos, FSubOffset;
	virtual void OnMouseEvent(int x, int y, int mk, UINT uMsg);
	virtual void OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnFocus(bool Focused);
	bool ScrollQuery(int Bar, int Req, POINT * Offset);
	virtual void SetSelection(DWORD Col, DWORD Row);
	void * GetNextAddress(void * Address, int Lines);
public:
	void SetRecords(void * Records, int Count);
	TAsmGrid(TParentControl * Parent, RECT * Rect, int Layout, TDumpSource * Source);
};

#endif