#ifndef _ASSEMBLER_
#define _ASSEMBLER_
#include "..\units\stdgui.h"
#include "Binary.h"

// Модификаторы кода инструкции, используются с последними битами
#define X86_SegReg	0x00000007	///< DS Используемый сегментный регистр
#define X86_o32		0x00000008	///< 1 Разрядность операндов
#define X86_a32		0x00000010	///< 1 Разрядность адресов
#define X86_ModRM	0x00000020	///< 0 Использвать ModRM
#define X86_RO		0x00000040	///< 0 Использовать RO как код регистра
#define X86_Cond	0x00000080	///< 0 Присутствует условие
#define X86_Prefix	0x00000100
#define X86_r		0x00000200	///< Регистр в последних битах
#define X86_w		0x00000400	///< Регистр в последних битах
#define X86_s		0x00000800	///< Размер непосредственного операнда Регистр в последних битах
#define X86_d		0x00001000	///< Направление в последнем байте
//#define X86_i8      0x00002000  ///< Непосредственный операнд
#define X86_ac		0x00002000	///< Аккумулятор - операнд
#define X86_im		0x00004000	///< Смещение в команде
#define X86_i8		0x00008000
#define X86_i16		0x00010000
#define X86_i32		0x00020000
#define X86_i		0x00040000
#define X86_jadd	0x00080000  ///< Добавка текущего адреса
#define X86_d1		0x00100000	///< Установить d в 1
#define X86_w2	    0x00200000  ///< w касается только второго операнда
#define X86_1CL		0x00400000
#define X86_O32D	0x00800000	///< Добавлять D если X86_o32

typedef struct _REGISTER
{
	char * Name;
	int Size;
	DWORD Offset;
} REGISTER;

REGISTER Registers[];

typedef enum TAsmType {otInt, otSigned, otUnsigned, otFloat}; ///< Типы данных
typedef enum TStoreType {ptNone, ptRegister, ptStack, ptConst}; // Тип хранения: регистры, временное хранилище, константа (если константа не влазит в Pos, то Pos - указатель)
typedef enum TCommandType {cPrimary, cEA, cStack, cUnknown}; ///< Тип команды: самостоятельная команда, часть расчёта эффективного адреса или работа со стеком

typedef struct _INSTRUCTION
{
	long Code;		///< Код инструкции
	long CodeMask;		///< Маска для сравнения кода
	long CodeLen;   ///< Длина кода
	char * Name;
	long StateMask;	///< Операция с состоянием: 0 - XOR, 1 - MOV
	long StateSet;	///< Аргумент операции с состоянием
} INSTRUCTION;


typedef struct _OPERAND {
	TAsmType AsmType;
	TStoreType StoreType; // Тип хранения: регистры, временное хранилище, константа (если константа не влазит в Pos, то Pos - указатель)
	int Size;
	int Pos; // Значение Pos: 
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
	OPERAND Operands[4]; ///< Нулевой операнд - результат.
} COMMAND;



INSTRUCTION x86InstructionSet[];

//#pragma pack(push, 1)

/// Описание байта ModR/M
typedef /*__declspec(align(1))*/ struct _MODRM
{
	unsigned RM  : 3; ///< Регистр или режим адресации
	unsigned RO  : 3; ///< Регистр или продолжение кода команды
	unsigned MOD : 2; ///< Режим адресации 
	// 00 - смещение остутствует
	// 01 - используется 8-битное смещение
	// 10 - используется 16-32-битное смещение
	// 11 - используется регистр RM.
} MODRM;

/// Описание байта SIB. Эффективный адрес = I << S + B
typedef struct _SIB
{
	unsigned B : 3; ///< Регистр базы 
	unsigned I : 3; ///< Индексный регистр (Если ESP - индекса нет)
	unsigned S : 2; ///< Масштабирование
} SIB;

class TX86Codec
{
private:
	/// Возможные поля последнего байта кода команды
	typedef struct _LASTBYTE
	{
		union 
		{
			unsigned char Byte; ///< Последний байт
			unsigned c : 4;		///< Условие
			unsigned r : 3; ///< Регистр
			struct
			{
				unsigned w : 1; ///< Размерность команды 8-16/32
				unsigned sd : 1;/*!< Непосредственный операнд s 8 - 16/32 или 
								     направление d R/O > R/M - R/M > R/O */
			};
		};
	} LASTBYTE;
	unsigned int State;	///< Текущее состояние декодера
	DWORD FOSize, FASize; ///< Размер операнда и адреса в битах 
	COMMAND FCommands[8]; ///< Буфер результата
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
	void * Position; ///< Реальное нахождение декодируемых команд
	void * Address;  ///< Мнимое нахождение декодируемых команд
	int FSize; ///< Расстояние до конца секции
	COMMAND * Decode(void);
	void * InitByDump(TDumpSource * Source, void * VA, DWORD Size);
};

//#pragma pack(pop)


class TAsmGrid: public TCustomGrid
{
private:
	TX86Codec Codec;
	TDumpSource * FSource;
	virtual void OnPaint(PAINTSTRUCT * PaintStruct); //!< Процедура отрисовки компонента
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