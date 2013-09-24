//!< \BinControls.h Визуальные компоненты доступа к бинарным данным
#ifndef _BinControls_
#define _BinControls_
#include <Windows.h>
#include "..\units\WinControls.h"
#include "binary.h"
using namespace StdGUI;

/********************************* Классы визуальных компонентов доступа к данным *********************/

typedef struct _MEMBER {
	TFieldDesc * Field;
	int IndexCount;
	int Indexes[14];
	inline void * get_Pointer(void * Pointer)
	{
		Pointer = (void *)((DWORD)Pointer + Indexes[0]);
		for(int x = 1; x < IndexCount; x++)
			Pointer = (void *)(*(DWORD *)Pointer + Indexes[x]);
		return Pointer;
	}
} MEMBER;

/// Компонент отображения двоичных данных
class TDumpGrid: public TCustomGrid
{
private:
	static char * Fields[];
	TDumpSource * FSource;
	int FRowSize;
//	virtual char * OnDrawCell(int Col, int Row, RECT * Rect, HDC dc);
	char * FOutBuf;
	virtual void OnMouseEvent(int x, int y, int mk, UINT uMsg); // Процедура реакции на движение мыши
	virtual void OnSetText(char * Data);
	virtual void OnPaint(PAINTSTRUCT * PaintStruct); // Процедура отрисовки компонента
	virtual void OnFocus(bool Focused);
	void UpdateCaret(void);
	int FCaretPos, FSubOffset;
	virtual void OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	enum _DM {
		DM_BYTE		= 0x0001,
		DM_WORD		= 0x0002,
		DM_DWORD	= 0x0004,
		DM_SINGLE	= 0x0008,
		DM_DOUBLE	= 0x0010,
		DM_ASCII	= 0x0100,
		DM_UNICODE	= 0x0200
	} FDataMode;
	TDumpGrid(TParentControl * Parent, RECT * Rect, int Layout, TDumpSource * Source);
	virtual ~TDumpGrid();
};

/// Компонент отображения структуры в таблице
class TRecordGrid: public TCustomGrid
{
private:
	MEMBER * FMembers;
	int FMemberCount;
	virtual char * OnDrawCell(int Col, int Row, RECT * Rect, HDC dc);
public:
	void * FRecord;
	TRecordGrid(TParentControl * Parent, RECT * Rect, int Layout, TFieldDesc * Desc);
	virtual ~TRecordGrid();
};

/// Компонент отображения массива структур
class TArrayGrid: public TCustomGrid
{
private:
	virtual char * OnDrawCell(int Col, int Row, RECT * Rect, HDC dc);
	MEMBER * FMembers;
	int FMemberCount;
	int FRecCount;
	int FRecSize;
	void * FRecords;
public:
	void SetRecords(void * Records, int Count);
	TArrayGrid(TParentControl * Parent, RECT * Rect, int Layout, TFieldDesc * Desc);
	virtual ~TArrayGrid();
};



//! Вспомогательная структура для отображения записей данных в виде дерева.
/*! Для закрытых (tnClosed) или пустых (tnEmpty) случаев может не существовать.
**  В этих случаях, факт пропуска узла отражается в Skipped предыдущего узла.
*/
typedef struct _TREENODE {
	void * Data;		//!< Указатель на запись данных
	int OpenChildren;	//!< Число видимых детей, при открытии этого узла
	int Skipped;		//!< Число записей данных, следующих за Data, но не имеющих своего TREENODE, до Next
	enum State {tnOpen, tnClosed, tnEmpty}; //!< Состояние узла
	_TREENODE * Parent;
	// Прямой список
	_TREENODE * Next;	//!< Следующий узел
	_TREENODE * Child;	//!< Первый дочерний узел
	// Обратный список
	//_TREENODE * Prev;	//!< Предыдущий узел
	//_TREENODE * LastChild;	//!< Последний дочерний узел
} TREENODE;

typedef struct _TREETYPE TREETYPE;

typedef struct _TREEFIELD {
	TFieldDesc * Field;	//!< Описание типа поля
	long Index;			//!< Смещение поля в структуре
	int Column;			//!< Номер колонки в таблице
	_TREETYPE * Child;  //!< Связь с детьми
} TREEFIELD;

typedef struct _TREETYPE {
	_TREETYPE * Parent;
	TREEFIELD * Fields;
} TREETYPE;

//! Визуальный класс таблицы с деревом для отображения структурированнных данных.
class TTreeGrid: public TCustomGrid
{
private:
	inline TREENODE * GetNewNode(void);
	TREENODE * First;
public:
	virtual void OnPaint(PAINTSTRUCT * PaintStruct); //!< Метод отрисовки компонента
	TTreeGrid(TParentControl * Parent, RECT * Rect, int Layout);
	void SetRecords(void * Records, int Count);
};


#endif