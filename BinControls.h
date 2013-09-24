//!< \BinControls.h ���������� ���������� ������� � �������� ������
#ifndef _BinControls_
#define _BinControls_
#include <Windows.h>
#include "..\units\WinControls.h"
#include "binary.h"
using namespace StdGUI;

/********************************* ������ ���������� ����������� ������� � ������ *********************/

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

/// ��������� ����������� �������� ������
class TDumpGrid: public TCustomGrid
{
private:
	static char * Fields[];
	TDumpSource * FSource;
	int FRowSize;
//	virtual char * OnDrawCell(int Col, int Row, RECT * Rect, HDC dc);
	char * FOutBuf;
	virtual void OnMouseEvent(int x, int y, int mk, UINT uMsg); // ��������� ������� �� �������� ����
	virtual void OnSetText(char * Data);
	virtual void OnPaint(PAINTSTRUCT * PaintStruct); // ��������� ��������� ����������
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

/// ��������� ����������� ��������� � �������
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

/// ��������� ����������� ������� ��������
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



//! ��������������� ��������� ��� ����������� ������� ������ � ���� ������.
/*! ��� �������� (tnClosed) ��� ������ (tnEmpty) ������� ����� �� ������������.
**  � ���� �������, ���� �������� ���� ���������� � Skipped ����������� ����.
*/
typedef struct _TREENODE {
	void * Data;		//!< ��������� �� ������ ������
	int OpenChildren;	//!< ����� ������� �����, ��� �������� ����� ����
	int Skipped;		//!< ����� ������� ������, ��������� �� Data, �� �� ������� ������ TREENODE, �� Next
	enum State {tnOpen, tnClosed, tnEmpty}; //!< ��������� ����
	_TREENODE * Parent;
	// ������ ������
	_TREENODE * Next;	//!< ��������� ����
	_TREENODE * Child;	//!< ������ �������� ����
	// �������� ������
	//_TREENODE * Prev;	//!< ���������� ����
	//_TREENODE * LastChild;	//!< ��������� �������� ����
} TREENODE;

typedef struct _TREETYPE TREETYPE;

typedef struct _TREEFIELD {
	TFieldDesc * Field;	//!< �������� ���� ����
	long Index;			//!< �������� ���� � ���������
	int Column;			//!< ����� ������� � �������
	_TREETYPE * Child;  //!< ����� � ������
} TREEFIELD;

typedef struct _TREETYPE {
	_TREETYPE * Parent;
	TREEFIELD * Fields;
} TREETYPE;

//! ���������� ����� ������� � ������� ��� ����������� ������������������ ������.
class TTreeGrid: public TCustomGrid
{
private:
	inline TREENODE * GetNewNode(void);
	TREENODE * First;
public:
	virtual void OnPaint(PAINTSTRUCT * PaintStruct); //!< ����� ��������� ����������
	TTreeGrid(TParentControl * Parent, RECT * Rect, int Layout);
	void SetRecords(void * Records, int Count);
};


#endif