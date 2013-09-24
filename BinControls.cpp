#include "BinControls.h"

/********************************** Набор сервисных функций и определений **************************/



MEMBER * CreateDescription(TFieldDesc * Desc, int * Count, int * Size)
{
	int FMemberCount = 0;
	for(int x = 0; Desc[x].Size; x++)
		if(Desc[x].Type)
		{
			if(Desc[x].Type & DT_PRECORD)
			{
				for(int y = 0; Desc[x].Record[y].Size; y++)
				{
					int Type = Desc[x].Record[y].Type;
					if(Type && !(Type & DT_COUNTER))
						FMemberCount++;
				}
			} else
			FMemberCount++;
		}
	MEMBER * FMembers = (MEMBER *) malloc(FMemberCount * sizeof(MEMBER));
	int y = 0;
	int Index = 0;
	for(int x = 0; Desc[x].Size; x++) 
	{
		if(Desc[x].Type)
		{
			if(Desc[x].Type & DT_PRECORD)
			{
				int Index1 = 0;
				for(int z = 0; Desc[x].Record[z].Size; z++)
				{
					int Type = Desc[x].Record[z].Type;
					if(Type && !(Type & DT_COUNTER))
					{
						FMembers[y].Field = Desc[x].Record + z;
						FMembers[y].Indexes[0] = Index;
						FMembers[y].Indexes[1] = Index1;
						FMembers[y].IndexCount = 2;
						y++;
					}
					Index1 += Desc[x].Record[z].Size;
				}
			}
			else
			{
				FMembers[y].Field = Desc + x;
				FMembers[y].Indexes[0] = Index;
				FMembers[y].IndexCount = 1;
				y++;
			}
		}
		Index += Desc[x].Size;
	}
	if(Count) *Count = FMemberCount;
	if(Size) *Size = Index;
	return FMembers;
}

/************************************ TRecordGrid ****************************************/

char * RGFields[] = {"Поле", "Значение"};

TRecordGrid::TRecordGrid(TParentControl * Parent, RECT * Rect, int Layout, TFieldDesc * Desc):TCustomGrid(Parent, Rect, Layout)
{
	FUseOnDrawCell = true;
	SetCols(2);
	FSelMode = _SM(SM_AROW | SM_SHOW);
	FMemberCount = 0;
	FMembers = CreateDescription(Desc, &FMemberCount, 0);
	SetRows(FMemberCount);
	FFields = RGFields;
	FRecord = 0;
};

char * TRecordGrid::OnDrawCell(int Col, int Row, RECT * Rect, HDC dc) 
{
	switch(Col)
	{
	case 0: return FMembers[Row].Field->Name;
	case 1: 
		if(FRecord)
			FMembers[Row].Field->Draw(dc, Rect, FMembers[Row].get_Pointer(FRecord));
	default:
		return 0;
	}
};


TRecordGrid::~TRecordGrid()
{
	free(FMembers);
}


/************************************ TArrayGrid ****************************************/

TArrayGrid::TArrayGrid(TParentControl * Parent, RECT * Rect, int Layout, TFieldDesc * Desc):TCustomGrid(Parent, Rect, Layout)
{
	FUseOnDrawCell = true;
	FSelMode = _SM(SM_AROW | SM_SHOW);
	FMemberCount = 0;
	FMembers = CreateDescription(Desc, &FMemberCount, &FRecSize);
	FFields = (char **) malloc(FMemberCount * sizeof(char *));
	for(int x = 0; x < FMemberCount; x++)
		FFields[x] = FMembers[x].Field->Name;
	SetCols(FMemberCount);
	SetRows(0);
	FRecords = 0;
	FRecCount = 0;
};

TArrayGrid::~TArrayGrid(void)
{
	free(FMembers);
	free(FFields);
}

void TArrayGrid::SetRecords(void * Records, int Count)
{
	FRecords = Records;
	SetRows(Count);
}

char * TArrayGrid::OnDrawCell(int Col, int Row, RECT * Rect, HDC dc) 
{
	void * Record = (void *)(int(FRecords) + Row * FRecSize);
	FMembers[Col].Field->Draw(dc, Rect, FMembers[Col].get_Pointer(Record));
	return 0;
};

/************************************ TDumpGrid ****************************************/

char * TDumpGrid::Fields[] = {"Адрес", "Данные", "Текст"};

TDumpGrid::TDumpGrid(TParentControl * Parent, RECT * Rect, int Layout, TDumpSource * Source):TCustomGrid(Parent, Rect, Layout)
{
	FDataMode = _DM(DM_BYTE | DM_ASCII);
	if(FDataMode & DM_BYTE)
	FSource = Source;
	//FUseOnDrawCell = true;
	FSelMode = _SM(SM_AROW | SM_ACOL);
	FSelRow = 0;
	SetCols(3);
	FColOffsets[0] = 70;
	FColOffsets[1] = 460;
	FFields = Fields;
	FSubOffset = 0;
	FRowSize = 16;
	SetRows(Source ? DWORD(-1) / FRowSize : 0);
	FOutBuf = (char *)malloc(FRowSize * 3);
	int FCaretPos = 0;
	CreateCaret(hWindow, (HBITMAP) 0, FCharSize.cx, FDefRowHeight);
}

void TDumpGrid::OnPaint(PAINTSTRUCT * PaintStruct) // Процедура отрисовки компонента
{
	TCustomGrid::OnPaint(PaintStruct);
	RECT Range, Rect;
	if(!RectToRange(&PaintStruct->rcPaint, &Range)) return;
	HGDIOBJ OldFont;
	if(FFont) OldFont = SelectObject(PaintStruct->hdc, FFont);
	SetTextColor(PaintStruct->hdc, FTxtCol);
	if(Range.left <= 0) // Отрисовка адресов
	{
		char Buf[9];
		DWORD Addr = Range.top * FRowSize + FSubOffset;
		for(DWORD x = Range.top; x <= (DWORD)Range.bottom; x++)
		{
			IntToHex(&Addr, 8, Buf);
			if(GetCellRect(0, x, &Rect))
				DrawText(PaintStruct->hdc, Buf, 8, &Rect, DT_LEFT | DT_NOCLIP);
			Addr += FRowSize;
		}
	}
	if((Range.left <= 2) && (Range.right >= 1))
	{
		DWORD Addr = Range.top * FRowSize + FSubOffset;
		DWORD DataSize = (Range.bottom - Range.top + 1) * FRowSize;
		DWORD Type = 0;
		void * Data = FSource->GetData((void *)Addr, &DataSize, &Type, 0);
		Addr += DataSize;
		for(DWORD x = Range.top; x <= (DWORD)Range.bottom; x++)
		{
			if(GetCellRect(1, x, &Rect))
			{
				char TextBuf[128];
				char * t = TextBuf;
				char * b = FOutBuf;
				for(int y = 0; y < FRowSize; y++)
				{
					if(!DataSize--)
					{
						DataSize = (Range.bottom - x + 2) * FRowSize - y + 1;
						Data = FSource->GetData((void *)Addr, &DataSize, &Type, 0);
						Addr += DataSize;
					}
					if(Data)
					{
						unsigned char c = *(unsigned char *)Data;
						*t = c > ' ' ? c : ' ';
						IntToHex(Data, 2, b);
						Data = (void *)((DWORD)Data + 1);
					}
					else
					{
						b[0] = '?';
						b[1] = '?';
						*t = '?';
					}
					b += 2;
					t++;
					*(b++) = ' ';
				}
				b[-1] = 0;
				DrawText(PaintStruct->hdc, FOutBuf, b - FOutBuf - 1, &Rect, DT_LEFT | DT_NOCLIP);
				if((Range.right >= 2) && (FDataMode & DM_ASCII) && GetCellRect(2, x, &Rect))
					DrawText(PaintStruct->hdc, TextBuf, t - TextBuf, &Rect, DT_LEFT | DT_NOCLIP);
			}
		}

	}
	if(FFont) SelectObject(PaintStruct->hdc, OldFont);
}

/*char * TDumpGrid::OnDrawCell(int Col, int Row, RECT * Rect, HDC dc) 
{
	DWORD Addr = Row * FRowSize;
	switch(Col)
	{
	case 1:
		if(Row == FMinRow)
		{
			DWORD Type = 0;
			DWORD FDataSize = (FMaxRow - FMinRow + 1) * FRowSize;
			FData = FSource->GetData((void *)Addr, &FDataSize, &Type, 0);
		}
		{
			char * b = FOutBuf;
			for(int x = 0; x < FRowSize; x++)
			{
				if(!--FDataSize)
				{
					DWORD Type = 0;
					DWORD FDataSize = (FMaxRow - FMinRow + 1) * FRowSize;
					FData = FSource->GetData((void *)Addr, &FDataSize, &Type, 0);
				}
				if(FData) 
				{
					IntToHex(FData, 2, b);
					FData = (void *)((DWORD)FData + 1);
				}
				else
				{
					b[0] = '?';
					b[1] = '?';
				}
				b += 2;
				*(b++) = ' ';
			}
			b[-1] = 0;

		}
		return FOutBuf;
	case 0:
		char Buf[9];
		IntToHex(&Addr, 8, Buf);
		DrawText(dc, Buf, 8, Rect, DT_LEFT | DT_NOCLIP);
	default:
		return 0;
	}
};*/

void TDumpGrid::UpdateCaret(void)
{
	RECT Rect;
	if(!GetCellRect(FSelCol, FSelRow, &Rect))
	{
		HideCaret(hWindow);
		return;
	}
	SetCaretPos(Rect.left + FCharSize.cx * FCaretPos + 2, Rect.top);
}

void TDumpGrid::OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_CHAR)
	{
		switch(FSelCol)
		{
		case 0:
			{
				unsigned int d;
				if((d = wParam - '0') > 9)
					if((d = wParam - 'A') < 6) d += 10;
					else
					if((d = wParam - 'a') < 6) d += 10;
					else return;
				DWORD Addr = FSelRow * FRowSize + FSubOffset;
				int Shift = 28 - FCaretPos * 4;
				Addr &= ~(0xF << Shift);
				Addr |= d << Shift;
				int SubOffset = Addr % FRowSize;
				if(FSubOffset != SubOffset)
				{
					InvalidateRect(hWindow, &FClient, false);
					FSubOffset = SubOffset;
				}
				SetSelection(FSelCol, Addr / FRowSize);
				if(FCaretPos < 8) FCaretPos++;
				UpdateCaret();
				return;
			}
		case 1:
			{
				unsigned int d;
				if((d = wParam - '0') > 9)
					if((d = wParam - 'A') < 6) d += 10;
					else
					if((d = wParam - 'a') < 6) d += 10;
					else return;
				DWORD Addr = FSelRow * FRowSize + FSubOffset;
				int t = FCaretPos % 3;
				if(t < 2)
				{
					Addr += FCaretPos / 3;
					DWORD l = 1, Type = 0;
					char * Data = (char *)FSource->GetData((void *)Addr, &l, &Type, 0);
					if(!Data || !Type) return;
					char dt = *Data;
					if(t) dt = dt & 0xF0 | d;
					else dt = dt & 0x0F | (d << 4);
					InvalidateRect(hWindow, &FClient, false);
					*Data = dt;
				}
				if(FCaretPos < FRowSize * 3 - 2) 
				{
					FCaretPos++;
					if(FCaretPos % 3 == 2) FCaretPos++;
				}
				UpdateCaret();
				return;
			}
		}
	} else
	if(uMsg == WM_KEYDOWN) switch(wParam)
	{
		//int t;
		case 0x24:/*Home*/FCaretPos = 0; UpdateCaret(); return;
		case 0x23:/*End*/ FCaretPos = 8; UpdateCaret(); return;
		//case 0x21:/*PageUp*/t = FSelRow - FScrolls[SB_VERT].nPage; if(FSelRow >= 0)SetSelection(FSelCol, t >= 0 ? t : 0); return;
		//case 0x22:/*PageDn*/if(FSelRow >= 0)SetSelection(FSelCol, FSelRow + FScrolls[SB_VERT].nPage); return;
		//case 0x26: if(FSelRow > 0) SetSelection(FSelCol, FSelRow - 1); return;
		//case 0x28: if(FSelRow >= 0) SetSelection(FSelCol, FSelRow + 1); return;
		case 0x25: if(FCaretPos) FCaretPos--; UpdateCaret(); return;
		case 0x27: if(FCaretPos < 8) FCaretPos++; UpdateCaret(); return;
	}
	TCustomGrid::OnKeyEvent(uMsg, wParam, lParam);
	if(uMsg == WM_KEYDOWN) switch(wParam)
	{
		//int t;
		//case 0x24:/*Home*/FCaretPos = 0; UpdateCaret(); return;
		//case 0x23:/*End*/ FCaretPos = 8; UpdateCaret(); return;
		//case 0x21:/*PageUp*/t = FSelRow - FScrolls[SB_VERT].nPage; if(FSelRow >= 0)SetSelection(FSelCol, t >= 0 ? t : 0); return;
		//case 0x22:/*PageDn*/if(FSelRow >= 0)SetSelection(FSelCol, FSelRow + FScrolls[SB_VERT].nPage); return;
		case 0x26: 
		case 0x28: UpdateCaret();
		//case 0x25: if(FCaretPos) FCaretPos--; UpdateCaret(); return;
		//case 0x27: if(FCaretPos < 8) FCaretPos++; UpdateCaret(); return;
	}

}


void TDumpGrid::OnMouseEvent(int x, int y, int mk, UINT uMsg)
{
	/*if((uMsg == WM_LBUTTONDBLCLK))// && (FSelCol == 0))
	{
		char Buf[9];
		DWORD Addr = FSelRow * FRowSize;
		IntToHex(&Addr, 8, Buf);
		ShowEditor(0, FSelRow, Buf);
		return;
	}*/
	if(uMsg == WM_LBUTTONDOWN)
	{
		DWORD cx, cy;
		if(CoordsToCell(x, y, &cx, &cy))
		{
			RECT Rect;
			GetCellRect(cx, cy, &Rect);
			FCaretPos = ((x - Rect.left) + (FCharSize.cx >> 1)) / FCharSize.cx;
			if(!cx && FCaretPos > 9) FCaretPos = 9;
			SetCaretPos(Rect.left + FCharSize.cx * FCaretPos, Rect.top);
			SetSelection(cx, cy);
			return;
		}
	}
	TCustomGrid::OnMouseEvent(x, y, mk, uMsg);
}

void TDumpGrid::OnFocus(bool Focused)
{
	if(Focused)
	{
		ShowCaret(hWindow);
	}
	else
		HideCaret(hWindow);
}


void TDumpGrid::OnSetText(char * Data)
{
	DWORD Address = 0;
	for(unsigned char * x = (unsigned char *)Data; *x; x++)
	{
		Address <<= 4;
		unsigned char k;
		if((k = *x - '0') < 10) Address += k;
		else
		if((k = *x - 'a') < 6) Address += k + 10;
		else
		if((k = *x - 'A') < 6) Address += k + 10;
		else return;
	}
	free(Data);
	SetSelection(FSelCol, Address / FRowSize);
};

TDumpGrid::~TDumpGrid(void)
{
	free(FOutBuf);
}

/************************************ TTreeGrid ****************************************/


//! Метод отрисовки компонента

/*!	  Описание алгоритма метода:

1. Ищем запись, находящуюся на PaintStruct->rcPaint.top
	1) Определяем номер строки, на которой находится PaintStruct->rcPaint.top
	2) Начинаем с First и идём далее по Next. (Ускоренный последовательный проход)
	Счётчик строк накапливает количество обработанных узлов, отображаемых детей 
	 (TTREENODE::OpenChildren) и мнимых (TTREENODE::Skipped), до тех пор, пока не превысит искомый номер.
		* Счётчик точно равен номеру - данная запись (Data) и есть искомая.
		* Счётчик вместе с OpenChildren превышает номер - необходимо продолжить поиск среди потомков.
		? Счётчик вместе с OpenChildren равен номеру - данная запись и есть искомая (закрывающая строка)
		* Счётчик вместе с OpenChildren + Skipped превышает номер - искомая запись находится на 
		определённом смещении от данной, среди мнимых узлов.
2. Последовательно отрисовываем записи, пока не дойдём до PaintStruct->rcPaint.bottom
	(Полный последовательный проход)

*/
void TTreeGrid::OnPaint(PAINTSTRUCT * PaintStruct)
{
	TCustomGrid::OnPaint(PaintStruct);
}