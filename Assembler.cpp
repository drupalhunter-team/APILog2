#define _CRT_SECURE_NO_DEPRECATE
#include "stdafx.h"
#include "Assembler.h"
#include "Binary.h"
#include <stdio.h>
#define _CRT_SECURE_NO_DEPRECATE

/******************************** TAsmGrid ********************************************/

char * AsmFields[] = {"Адрес", "Код", "Значение"};

#define FRowSize 4

TAsmGrid::TAsmGrid(TParentControl * Parent, RECT * Rect, int Layout, TDumpSource * Source):TCustomGrid(Parent, Rect, Layout)
{
	FSource = Source;
	FCaretPos = 0;
	FSubOffset = 0;
	FSelMode = SM_AROW;
	FFields = AsmFields;
	SetCols(3);
	FColOffsets[0] = 70;
//	FColOffsets[1] = 460;
	SetRows(Source ? DWORD(-1) / FRowSize : 0);
	CreateCaret(hWindow, (HBITMAP) 0, 2, FDefRowHeight);
}

void TAsmGrid::UpdateCaret(void)
{
	RECT Rect;
	if(!GetCellRect(FSelCol, FSelRow, &Rect))
	{
		HideCaret(hWindow);
		return;
	}
	SetCaretPos(Rect.left + FCharSize.cx * FCaretPos, Rect.top);
}

void TAsmGrid::OnFocus(bool Focused)
{
	if(Focused)
		ShowCaret(hWindow);
	else
		HideCaret(hWindow);
}

/// Метод расчёта нового адреса по имеющемуся и числу команд, на которое необходимо сместиться от него
void * TAsmGrid::GetNextAddress(void * Address, int Lines)
{
	if(!Lines) return Address;
	if(Lines > 0)
	{
		Codec.InitByDump(FSource, Address, Lines * 8);
		for( ; Lines; Lines--)
		{
			Codec.Decode();
			// Тут не мешало бы проверить - не закончился ли отведённый буфер?
		}
		return Codec.Address;
	} 
	else 
	{
		int Len = -Lines * 8; // Оценочное смещение
		void * Addr = (void *) (int(Address) - Len);
		Codec.InitByDump(FSource, Addr, Len + 10);
		void * * Addresses = (void * *) malloc(Len * sizeof(void *));
		int Counter;
		for(Counter = 0; Codec.Address < Address; Counter++) 
		{
			Codec.Decode();
			Addresses[Counter] = Codec.Address;
		}
		Addr = 0;
		if(Counter > -Lines) 
			Addr = Addresses[Counter + Lines - 1];
		free(Addresses);
		return Addr;
	}
}

/// Метод устанавливает FScrolls[Bar].nPos и возвращает смещение изображения в пикселах.
/// false означает отмену смещения. Если true, но смещение нулевое - Invalidate
bool TAsmGrid::ScrollQuery(int Bar, int Req, POINT * Offset)
{
	if(Bar != SB_VERT) return TCustomGrid::ScrollQuery(Bar, Req, Offset);
	Offset->y = FScrolls[Bar].nPos;
	Offset->x = 0;
	void * Address = (void *)(FScrolls[SB_VERT].nPos * FRowSize + FSubOffset);
	switch(Req)
	{
	case SB_BOTTOM:			// Scrolls to the lower right.
		FScrolls[Bar].nPos = FScrolls[Bar].nMax;
		break;
	case SB_LINEDOWN:		// Scrolls one line down.
		//if(FScrolls[Bar].nPos < FScrolls[Bar].nMax - int(FScrolls[Bar].nPage) + 1)
		//	FScrolls[Bar].nPos++;
		//Codec.InitByDump(FSource, Address, 32);
		//Codec.Decode();
		Address = GetNextAddress(Address, 1);

		FSubOffset = (DWORD)Address % FRowSize;
		FScrolls[SB_VERT].nPos = (DWORD)Address / FRowSize;

		Offset->y = -FScrollUnits[Bar];
		return true;
	case SB_LINEUP:			// Scrolls one line up.
		//if(FScrolls[Bar].nPos > FScrolls[Bar].nMin)
		//	FScrolls[Bar].nPos--;
		Address = GetNextAddress(Address, -1);

		FSubOffset = (DWORD)Address % FRowSize;
		FScrolls[SB_VERT].nPos = (DWORD)Address / FRowSize;

		Offset->y = FScrollUnits[Bar];
		return true;
	case SB_PAGEDOWN:		// Scrolls one page down.
		/*FScrolls[Bar].nPos += FScrolls[Bar].nPage;
		if(FScrolls[Bar].nPos > FScrolls[Bar].nMax - int(FScrolls[Bar].nPage))
			FScrolls[Bar].nPos = FScrolls[Bar].nMax - int(FScrolls[Bar].nPage) + 1;*/

		Address = GetNextAddress(Address, FScrolls[Bar].nPage);

		FSubOffset = (DWORD)Address % FRowSize;
		FScrolls[SB_VERT].nPos = (DWORD)Address / FRowSize;

		Offset->y = -FScrollUnits[Bar] * FScrolls[Bar].nPage;
		return true;
	case SB_PAGEUP:			// Scrolls one page up.
		FScrolls[Bar].nPos -= FScrolls[Bar].nPage;
		if(FScrolls[Bar].nPos < FScrolls[Bar].nMin)
			FScrolls[Bar].nPos = FScrolls[Bar].nMin;
		break;
	case SB_ENDSCROLL:		// Ends scroll.
	case SB_THUMBPOSITION:	// The user has dragged the scroll box (thumb) and released the
							//mouse button. The high-order word indicates the position of the
							//scroll box at the end of the drag operation.
		return false;

	case SB_THUMBTRACK:		// The user is dragging the scroll box. This message is sent
							//repeatedly until the user releases the mouse button. The
							//high-order word indicates the position that the scroll box has
							//been dragged to.
		FScrolls[Bar].fMask = SIF_TRACKPOS;
		GetScrollInfo(hWindow, Bar, FScrolls + Bar);
		int o; o =  FScrolls[Bar].nTrackPos - FScrolls[Bar].nPos;
		if(abs(o) < 1024)
		{
			Address = GetNextAddress(Address, o);
			FSubOffset = (DWORD)Address % FRowSize;
			FScrolls[SB_VERT].nPos = (DWORD)Address / FRowSize;
			Offset->y = -FScrollUnits[Bar] * o;
		}
		else
		{
			Offset->y = 0;
			FScrolls[Bar].nPos = FScrolls[Bar].nTrackPos;
			FSubOffset = 0;
		}
		return true;
		break;
	case SB_TOP:			// Scrolls to the upper left.
		FScrolls[Bar].nPos = 0;
		break;
	}
	if(!(Offset->y -= FScrolls[Bar].nPos)) return false;
	if(FScrollUnits[Bar] && (abs(Offset->y) > 16384)) Offset->y = 0;
	else Offset->y *= FScrollUnits[Bar];
	return true;
}

/// Метод установки выделения
//
void TAsmGrid::SetSelection(DWORD Col, DWORD Row)
{
	if(Row >= FRows) Row = FRows - 1;
	if(Col >= FCols) Col = FCols - 1;
	if(FSelMode & SM_AROW) FSelMode = _SM(FSelMode | SM_SROW);
	if(FSelMode & SM_ACOL) FSelMode = _SM(FSelMode | SM_SCOL);
	if(((FSelMode & SM_AROW) && (Row != FSelRow)) || ((FSelMode & SM_ACOL) && (Col != FSelCol)))
	{
		RECT Rect;
		if(GetCellRect(FSelCol, FSelRow, &Rect, FSelMode)) InvalidateRect(hWindow, &Rect, false);
		if(GetCellRect(Col, Row, &Rect, FSelMode)) InvalidateRect(hWindow, &Rect, false);

		FSelRow = Row;
		FSelCol = Col;
	}
	if(FSelRow < (DWORD)FScrolls[SB_VERT].nPos) SetScroll(SB_VERT, FSelRow);
	else
	if(FSelRow - FScrolls[SB_VERT].nPos >= (int)FScrolls[SB_VERT].nPage) SetScroll(SB_VERT, FSelRow - FScrolls[SB_VERT].nPage + 1);
}


void TAsmGrid::OnKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_CHAR)
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
		FSubOffset = Addr % FRowSize;
		FScrolls[SB_VERT].nPos = Addr / FRowSize;
		SetSelection(FSelCol, Addr / FRowSize);
		InvalidateRect(hWindow, &FClient, false);

		if(FCaretPos < 8) FCaretPos++;
		UpdateCaret();
		return;
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


void TAsmGrid::OnMouseEvent(int x, int y, int mk, UINT uMsg)
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

void TAsmGrid::OnPaint(PAINTSTRUCT * PaintStruct) // Процедура отрисовки компонента
{
	TCustomGrid::OnPaint(PaintStruct);
	RECT Rect, Range;
	if(!RectToRange(&PaintStruct->rcPaint, &Range)) return;
	HGDIOBJ OldFont;
	if(FFont) OldFont = SelectObject(PaintStruct->hdc, FFont);
	SetTextColor(PaintStruct->hdc, FTxtCol);

	Codec.Address = (void *)(FScrolls[SB_VERT].nPos * FRowSize + FSubOffset);
	DWORD Type, Size;
	Size = (Range.bottom - FScrolls[SB_VERT].nPos) * 4;
	Codec.Position = FSource->GetData((void *)Codec.Address, &Size, &Type, 0);

	Codec.FSize = Size;
	//Addr += Codec.FSize;
	for(DWORD y = FScrolls[SB_VERT].nPos; y <= (DWORD)Range.bottom; y++)
	{
		char Buf[100];
		if(GetCellRect(0, y, &Rect))
		{
			sprintf(Buf, "%.8X", Codec.Address);
			DrawText(PaintStruct->hdc, Buf, strlen(Buf), &Rect, DT_LEFT | DT_NOCLIP);
		}
		void * Pos = Codec.Position;
		void * Addr = Codec.Address;
		char * Text = 0;
		if(Pos)
		{
			COMMAND * Coms = Codec.Decode();
			if(Coms && Coms->Name)
				Text = Codec.CommandsToText(Coms);
		}
		else
		{
			Codec.FSize--;
			Codec.Address = (void *)((int)Codec.Address + 1);
		}
		if(GetCellRect(1, y, &Rect))
		{
			char * b = Buf;
			*b = 0;
			for(void * a = Addr; a < Codec.Address; a = (void *) ((int) a + 1))
			{
				if(Pos) 
				{
					sprintf(b, "%.2X ", *(unsigned char *)Pos);
					Pos = (void *) ((int) Pos + 1);
				} else strcpy(b, "?? ");
				b += 3;
			}
			DrawText(PaintStruct->hdc, Buf, strlen(Buf), &Rect, DT_LEFT | DT_NOCLIP);
		}
		if(GetCellRect(2, y, &Rect) && Text)
			DrawText(PaintStruct->hdc, Text, strlen(Text), &Rect, DT_LEFT | DT_NOCLIP);
		free(Text);
	}
	if(FFont) SelectObject(PaintStruct->hdc, OldFont);
}

/************************************ Список мнемоник **************************************/
#define mnem(n) char _##n[] = #n;

char * Conditions[] = {
"O",
"NO",
"B",//C/NAE
"AE",//NC/NB
"E",//Z
"NE",//NZ
"BE",//NA
"A",//NBE
"S",
"NS",
"P",//PE
"NP",//PO
"L",//NGE
"GE",//NL
"LE",//NG
"G"//LNE
};

char O32Post[] = "D";
char * OPosts[] = {"B", "W", O32Post};


mnem(AAA)mnem(AAD)mnem(AAM)mnem(AAS)
mnem(ADC)mnem(ADD)mnem(AND)
mnem(ARPL)mnem(BOUND)
mnem(BSF)mnem(BSR)mnem(BSWAP)mnem(BT)mnem(BTC)mnem(BTR)mnem(BTS)
mnem(CALL)
mnem(CBW)mnem(CDQ)
mnem(CLC)mnem(CLD)mnem(CLI)
mnem(CLTS)
mnem(CMC)
mnem(CMOV)mnem(CMP)mnem(CMPS)mnem(CMPXCHG)mnem(CMPXCHG8)
mnem(CPUID)
mnem(CWD)mnem(CWDE)
mnem(DAA)mnem(DAS)
mnem(DEC)
mnem(DIV)
mnem(EMMS)mnem(ENTER)

mnem(F2XM1)mnem(FABS)mnem(FADD)mnem(FADDP)mnem(FBLD)mnem(FBSTP)
mnem(FCHS)mnem(FCLEX)

mnem(HLT)
mnem(IDIV)mnem(IMUL)
mnem(IN)
mnem(INC)
mnem(INS)
mnem(INT)mnem(INT3)mnem(INTO)
mnem(INVD)
mnem(INVLPG)
mnem(IRET)mnem(IRETD)
mnem(J)mnem(JECXZ)mnem(JCXZ)mnem(JMP)
mnem(LAHF)
mnem(LAR)
mnem(LDS)
mnem(LEA)
mnem(LEAVE)
mnem(LES)mnem(LFS)
mnem(LGDT)mnem(LGS)mnem(LIDT)mnem(LLDT)
mnem(LMSW)
mnem(LOCK)
mnem(LODS)
mnem(LOOP)mnem(LOOPE)mnem(LOOPNE)
mnem(LSL)mnem(LSS)mnem(LTR)
mnem(MOV)mnem(MOVD)mnem(MOVQ)mnem(MOVS)
mnem(MOVSX)mnem(MOVZX)
mnem(MUL)mnem(NEG)mnem(NOP)mnem(NOT)mnem(OR)
mnem(OUT)mnem(OUTS)
mnem(PACKSSWB)
mnem(SAL)mnem(SAR)mnem(SHR)mnem(SHL)mnem(ROL)mnem(ROR)mnem(TEST)
mnem(POP)mnem(POPA)mnem(POPAD)mnem(POPF)mnem(POPFD)
mnem(POR)mnem(PSLLW)mnem(PSLLD)mnem(PSLLQ)

mnem(PUSH)mnem(PUSHA)mnem(PUSHAD)mnem(PUSHF)mnem(PUSHFD)mnem(PXOR)
mnem(RCL)mnem(RCR)mnem(RDMSR)mnem(RDPMC)mnem(RDTSC)mnem(SUB)mnem(SBB)mnem(RET)mnem(RETF)
mnem(XOR)mnem(XCHG)

/***************************** TX86Codec ********************************************/

#define Regs16 (Registers + 8)

COMMAND * TX86Codec::AddEACommand(REGISTER * A, REGISTER * B, char * Command, int Size)
{
	FCommand->Name = Command;
	FCommand->Type = cEA;
	FCommand->Operands[0].SetStackEA(Size);
	if(A) FCommand->Operands[1].SetRegister(A);
	else FCommand->Operands[1].SetStackEA(Size);
	if(B) FCommand->Operands[2].SetRegister(B);
	else FCommand->Operands[2].SetStackEA(Size);
	return FCommand++;
}

COMMAND * TX86Codec::AddEACommand(REGISTER * A, long Const, char * Command, int Size)
{
	FCommand->Name = Command;
	FCommand->Type = cEA;
	FCommand->Operands[0].SetStackEA(Size);
	if(A) FCommand->Operands[1].SetRegister(A);
	else FCommand->Operands[1].SetStackEA(Size);
	FCommand->Operands[2].SetConst(Const, Size);
	return FCommand++;
}

void TX86Codec::AddMemRef(REGISTER * R, int OpSize)
{
	int aSize = State & X86_a32 ? 32 : 16;
#define _CHECK_DS_
#ifdef _CHECK_DS_
	if((State & X86_SegReg) != 3)
#endif
	{
		FCommand->Name = _LDS;
		FCommand->Type = cEA;
		REGISTER * SegReg = Registers + 3 * 8 + (State & X86_SegReg);
		if(R) FCommand->Operands[2].SetRegister(R);
		else FCommand->Operands[2].SetStackEA(aSize);
		FCommand->Operands[0].SetStackEA(aSize += SegReg->Size);
		FCommand->Operands[1].SetRegister(SegReg);
		R = 0;
		FCommand++;
	}

	FCommand->Name = _LODS;
	FCommand->Type = cEA;
	FCommand->Operands[0].SetStackEA(OpSize);
	if(R) FCommand->Operands[1].SetRegister(R);
	else FCommand->Operands[1].SetStackEA(aSize);
	FCommand->Operands[2].StoreType = ptNone;
	FCommand++;
}

void TX86Codec::AddConstMemRef(long Const, int OpSize)
{
	int aSize = State & X86_a32 ? 32 : 16;
#ifdef _CHECK_DS_
	bool nDS = ((State & X86_SegReg) != 3);
	if(nDS)
#endif
	{
		FCommand->Name = _LDS;
		FCommand->Type = cEA;
		REGISTER * SegReg = Registers + 3 * 8 + (State & X86_SegReg);
		FCommand->Operands[1].SetRegister(SegReg);
		FCommand->Operands[2].SetConst(Const, aSize);
		FCommand->Operands[0].SetStackEA(aSize += SegReg->Size);
		FCommand++;
	}

	FCommand->Name = _LODS;
	FCommand->Type = cEA;
	FCommand->Operands[0].SetStackEA(OpSize);
#ifdef _CHECK_DS_
	if(nDS)
		FCommand->Operands[1].SetStackEA(aSize);
	else
		FCommand->Operands[1].SetConst(Const, aSize);
#else
	FCommand->Operands[1].SetStackEA(aSize);
#endif
	FCommand->Operands[2].StoreType = ptNone;
	FCommand++;
}

inline REGISTER * TX86Codec::DecodeSIB(DWORD MOD, bool * RegUsed)
{
	SIB sib;
	if(FSize-- <= 0) return 0;
	*(BYTE *)&sib = *(BYTE *) Position;
	Position = (void *)(int(Position) + 1);
	REGISTER * I = 0, * B = 0;
	if(sib.I != 4) I = Registers + sib.I;
	if((sib.B != 5) || MOD) B = Registers + sib.B;
	if(I && sib.S) 
	{
		COMMAND * c = AddEACommand(I, 2, _MUL, 32);
		c->Operands[2].Size = 8;
		c->Operands[2].AsmType = otUnsigned;
		if(B)AddEACommand(0, B, _ADD, 32);
		return 0;
	}
	if(I && B)
	{
		AddEACommand(I, B, _ADD, 32);
		return 0;
	}
	if(I) return I;
	if(B) return B;
	// Пустой SIB!!!
	*RegUsed = false;
	return 0;
}

inline REGISTER * RegsBySize(int Size)
{
	switch(Size)
	{
	case 8: return Registers + 16;
	case 16: return Registers + 8;
	case 32: return Registers;
	}
	return 0;
}

/// Метод для обработки ModR/M, SIB и смещений с ними. 
/*!Увеличивает Position и FCommand по надобности.
Возвращает регистр из RO и заполняет операнд, извлечённый из R/M
*/
DWORD TX86Codec::DecodeModRM(OPERAND * Result, int OpSize)
{
	// Выбираем размер смещения
	int OffSize = 0;
	switch(ModRM.MOD) 
	{
	case 1:// 01 - используется 8-битное смещение
		OffSize = 1;
		break;
	case 2:// 10 - используется 16-32-битное смещение
		OffSize = State & X86_a32 ? 4 : 2;
		break;
	case 3:
		Result->SetRegister(RegsBySize(OpSize) + ModRM.RM);
		return ModRM.RO;
	}
	// Обработка обращения к памяти
	bool RegUsed = true;
	REGISTER * Reg = 0; // Регистр для косвенного обращения к памяти
	if(State & X86_a32) switch(ModRM.RM)
	{
	case 4: 
	// Загружаем SIB
		Reg = DecodeSIB(ModRM.MOD, &RegUsed);
		if(FSize < 0) return 0;
		if(!(OffSize || RegUsed)) OffSize = 4; // Недокументировано!
		break;
	case 5:
		if(!ModRM.MOD) 
		{
			RegUsed = false;
			OffSize = 4;
			break;
		}
	default:
		Reg = Registers + ModRM.RM;
	}
	else switch(ModRM.RM)
	{
	case 0: AddEACommand(Regs16 + 3, Regs16 + 6, _ADD, 16); break;
	case 1: AddEACommand(Regs16 + 3, Regs16 + 7, _ADD, 16); break;
	case 2: AddEACommand(Regs16 + 5, Regs16 + 6, _ADD, 16); break;
	case 3: AddEACommand(Regs16 + 5, Regs16 + 7, _ADD, 16); break;
	case 4: Reg = Regs16 + 6; break;
	case 5: Reg = Regs16 + 7; break;
	case 6: if(ModRM.MOD) Reg = Regs16 + 5;
			else 
			{
				OffSize = 2;	
				RegUsed = false;
			}; break;
	case 7: Reg = Regs16 + 3; break;
	}
	// Загружаем смещение
	long Offset = 0;
	FSize -= OffSize;
	if(FSize < 0) return 0;
	switch(OffSize)
	{
	case 1: Offset = *(char *)Position; break;
	case 2: Offset = *(short *)Position; break;
	case 4: Offset = *(long *)Position; break;
	}
	Position = (void *)(int(Position) + OffSize);
	// Генерируем результат
	Result->SetStackEA(OpSize);
	if(RegUsed && Offset)
	{
		if((Offset < 0) && (Offset >= -128))
			AddEACommand(Reg, -Offset, _SUB, FASize);
		else
			AddEACommand(Reg, Offset, _ADD, FASize);
		AddMemRef(0, OpSize);
		return ModRM.RO;
	}
	if(RegUsed)
	{
		AddMemRef(Reg, OpSize);
		return ModRM.RO;
	}
	AddConstMemRef(Offset, OpSize);
	return ModRM.RO;
}

void * TX86Codec::InitByDump(TDumpSource * Source, void * VA, DWORD Size)
{
	Address = VA;
	DWORD Type = 0;
	Position = Source->GetData(VA, &Size, &Type, 0);
	FSize = Size;
	return Position;
}


/// Декодирует одну команду вместе с префиксами
COMMAND * TX86Codec::Decode(void)
{
	if(FSize <= 0) return 0;
	State = X86_a32 | X86_o32 | 3; // DS:
	void * StartPos = Position;
	INSTRUCTION * x = 0;
	do
	{
		int Code = 0;
		if(FSize < 4)
			memcpy(&Code, Position, FSize);
		else
			Code = *(int *) Position;
		for(x = x86InstructionSet; x->CodeMask; x++)
			if((Code & x->CodeMask) == x->Code)
			{
				if((x->StateSet & (X86_o32|X86_a32)) && !(x->StateSet & X86_Prefix))
					if((x->StateSet ^ State) & (X86_o32|X86_a32) & x->StateMask) continue;// не совпадает размерность операнда
				State ^= x->StateSet ^ (State & x->StateMask);
				break;
			}
		if(!x->CodeMask) 
		{ 
			Position = (void *)(int(Position) + 1);
			FSize--;
			Address = (void *)(int(Address) + int(Position) - int(StartPos));
			return 0;
		}
		Position = (void *)(int(Position) + x->CodeLen);
		if(x->CodeLen > FSize) return 0;
		FSize -= x->CodeLen;
	} while(/*x->StateSet*/ State & X86_Prefix);
	// Загружаем последний байт
	LASTBYTE b = *(LASTBYTE *)((int)Position - 1);
	// Определяем направление
	int d = State & X86_d ? b.sd : (State & X86_d1) ? 1 : 0;
	// Создаём команду
	FCommand = FCommands;
	COMMAND Command = {x->Name, cPrimary};
	Command.Operands[0].StoreType = ptNone;
	Command.Operands[0].Pos = 0;
	if(State & X86_Cond) Command.Operands[0].Pos = (int)Conditions[b.c];
	if((State & X86_O32D) && (State & X86_o32)) Command.Operands[0].Pos = (int) O32Post;

	OPERAND * Operand = Command.Operands + 1;
	// Определяем размерность команды
	int w = State & X86_w ? b.w : 1;
	int Size = w ? (State & X86_o32 ? 4 : 2) : 1;
	FOSize = Size * 8;
	FASize = State & X86_a32 ? 32 : 16;
	int Sizes[2] = {Size, Size};
	if(State & X86_w2)
	{
		Sizes[0] = State & X86_o32 ? 4 : 2;
		Sizes[1] = w ? 2 : 1;
	}
	int * pSize = Sizes;

	// Загружаем ModR/M и SIB
	if(State & X86_ModRM)
	{
		if(!Size--) return 0;
		*(BYTE *)&ModRM = *(BYTE *) Position;
		Position = (void *)(int(Position) + 1);

		if(!d) 	
		{
			DecodeModRM(Operand++, *(pSize++) * 8);
			if(FSize < 0) return 0;
		}
		if(State & X86_RO) 
			(Operand++)->SetRegister(RegsBySize(*(pSize++) * 8) + ModRM.RO);
		if(d) 
		{
			DecodeModRM(Operand++, *(pSize++) * 8);
			if(FSize < 0) return 0;
		}
	}
	// Загружаем аккумулятор
	if(State & X86_ac) (d + Operand++)->SetRegister(RegsBySize(Size * 8));

	// Загружаем непосредственное смещение
	if(State & X86_im)
	{
		int oSize = State & X86_a32 ? 4 : 2;
		long Offset = State & X86_a32 ? *(DWORD *)Position : *(WORD *)Position;
		if(oSize > FSize) return 0;
		Position = (void *)(int(Position) + oSize);
		FSize -= oSize;
		AddConstMemRef(Offset, Size * 8);
		(Operand++ - d)->SetStackEA(Size * 8);
	}
	if(State & X86_r)
	{
		Operand->SetRegister(RegsBySize(Size * 8) + b.r);
		Operand++;
	}
	// Второй операнд - 1 или CL
	if(State & X86_1CL)
	{
		if(b.sd) Operand->SetRegister(Registers + 2 * 8 + 1);
		else Operand->SetConst(1, 8);
		Operand++;
	}
	// Загружаем непосредственный операнд в последнюю очередь
	if(State & (X86_s | X86_i8 | X86_i16 | X86_i32 | X86_i))
	{
		int iSize = Size;
		if(State & X86_s) 
		{
			iSize = b.sd ? 1 : ((State & X86_o32) ? 4 : 2);
			if(iSize > Size) iSize = Size;
		}
		if(State & X86_i8) iSize = 1;
		if(State & X86_i16) iSize = 2;
		if(State & X86_i32) iSize = 4;
		Operand->StoreType = ptConst;
		Operand->AsmType = otInt;
		if(FSize < iSize) return 0;
		switch(iSize)
		{
		case 1: Operand->Pos = *(char *)Position; break;
		case 2: Operand->Pos = *(short *)Position; break;
		case 4: Operand->Pos = *(long *)Position; break;
		}
		Position = (void *)(int(Position) + iSize);
		FSize -= iSize;
		if(State & X86_jadd) Operand->Pos += int(Address) + int(Position) - int(StartPos);
		Operand->Size = FOSize;
		Operand++;
	}

	// Выводим результат
	*FCommand++ = Command;
	FCommand->Name = 0;
	if(FCommand - FCommands >= 8) MessageBox(0, "!!!!", "!!!", 0);
	Address = (void *)(int(Address) + int(Position) - int(StartPos));
	return FCommands;
}

REGISTER * FindRegister(int Size, DWORD Offset)
{
	for(REGISTER * r = Registers; r->Name; r++)
		if((r->Size == Size) && (r->Offset == Offset)) return r;
	return 0;
}

inline void StoreOperand(char * &r, OPERAND * Operand, char * * &Top, char * * Stack)
{
#define _SHOWTYPE_
#ifdef _SHOWTYPE_
	switch(Operand->AsmType)
	{
	case otInt: *(r++) = 'i'; break;
	case otUnsigned: *(r++) = 'u'; break;
	case otSigned: *(r++) = 's'; break;
	case otFloat: *(r++) = 'f'; break;
	}
	for(int x = Operand->Size; x; x /= 10) r++;
	char * t = r;
	for(int x = Operand->Size; x; x /= 10) *(--t) = x %10 + '0';
	*(r++) = '(';
#endif

	REGISTER * reg;
	switch(Operand->StoreType)
	{
	case ptRegister:
		if(reg = FindRegister(Operand->Size, Operand->Pos))
		{
			strcpy(r, reg->Name);
			r += strlen(reg->Name);
		} 
		else *(r++) = '?';
		break;
	case ptStack:
		if(Top <= Stack)
		{
			*(r++) = '?';
			Top = Stack;
			break;
		}
		strcpy(r, *(--Top));
		r += strlen(*Top);
		break;
	case ptConst:
		int Size; Size = Operand->Size / 8;
		IntToHex(Size > 4 ? *(void **)Operand->Pos : &Operand->Pos, Size * 2, r);
		r += Size * 2;
		break;
	}
#ifdef _SHOWTYPE_
	*(r++) = ')';
#endif
}

char * TX86Codec::CommandsToText(COMMAND * Commands)
{
	char Buf[256];
	char * r = Buf;
	char * Stack[20];
	char * * Top = Stack;
	for(COMMAND * x = Commands; x->Name; x++)
	{
		if(x->Type == cEA)
		{
			char * buf = r;
			if(x->Name == _LODS)
			{
				*(r++) = '[';
				StoreOperand(r, x->Operands + 1, Top, Stack);
				*(r++) = ']';
			}
			else
			{
				StoreOperand(r, x->Operands + 1, Top, Stack);
				*(r++) = ' ';
				if(x->Name == _ADD) *(r++) = '+';
				else
				if(x->Name == _MUL) *(r++) = '*';
				else
				if(x->Name == _SUB) *(r++) = '-';
				else
				if(x->Name == _LDS) *(r++) = ':';
				else
				{
					strcpy(r, x->Name);
					r += strlen(x->Name);
				}
				*(r++) = ' ';
				StoreOperand(r, x->Operands + 2, Top, Stack);
			}
			*(r++) = 0;
			*(Top++) = buf;
		}
		else
		{
			char * buf = r;
			strcpy(r, x->Name);
			r += strlen(x->Name);
			if(x->Operands[0].Pos && !x->Operands[0].StoreType)
			{
				char * s = (char *)x->Operands[0].Pos;
				strcpy(r, s);
				r += strlen(s);
			}

			while(r - buf < 5) *(r++) = ' ';

			bool cl = false;
			for(int o = 0; o < sizeof(x->Operands) / sizeof(*x->Operands); o++)
			{
				if(!x->Operands[o].StoreType) continue;
				if(cl) *(r++) = ',';
					else cl = true;
				*(r++) = ' ';
				StoreOperand(r, x->Operands + o, Top, Stack);
			}
			*(r++) = 0;
			*(Top++) = buf;
		}
		
	}
	int l = strlen(*(--Top)) + 1;
	char * b = (char *) malloc(l);
	memcpy(b, *Top, l);
	return b;
}

/**************************************************************************************/

typedef struct _X86REGISTERS
{
	union{ DWORD _EAX; WORD _AX; struct{ BYTE _AL, _AH;};};
	union{ DWORD _ECX; WORD _CX; struct{ BYTE _CL, _CH;};};
	union{ DWORD _EDX; WORD _DX; struct{ BYTE _DL, _DH;};};
	union{ DWORD _EBX; WORD _BX; struct{ BYTE _BL, _BH;};};
	union{ DWORD _ESP; WORD _SP;};
	union{ DWORD _EBP; WORD _BP;};
	union{ DWORD _ESI; WORD _SI;};
	union{ DWORD _EDI; WORD _DI;};



} X86REGISTERS;

REGISTER Registers[] = {
	// 0
	{"EAX",		32,		0x00},
	{"ECX",		32,		0x20},
	{"EDX",		32,		0x40},
	{"EBX",		32,		0x60},
	{"ESP",		32,		0x80},
	{"EBP",		32,		0xA0},
	{"ESI",		32,		0xC0},
	{"EDI",		32,		0xE0},
	// 1
	{"AX",		16,		0x00},
	{"CX",		16,		0x20},
	{"DX",		16,		0x40},
	{"BX",		16,		0x60},
	{"SP",		16,		0x80},
	{"BP",		16,		0xA0},
	{"SI",		16,		0xC0},
	{"DI",		16,		0xE0},
	// 2
	{"AL",		 8,		0x00},
	{"CL",		 8,		0x20},
	{"DL",		 8,		0x40},
	{"BL",		 8,		0x60},
	{"AH",		 8,		0x08},
	{"CH",		 8,		0x28},
	{"DH",		 8,		0x48},
	{"BH",		 8,		0x68},
	// 3
#define SRO	0x100
	{"ES",		16,	SRO+0x00},
	{"CS",		16,	SRO+0x10},
	{"SS",		16,	SRO+0x20},
	{"DS",		16,	SRO+0x30},
	{"FS",		16,	SRO+0x40},
	{"GS",		16,	SRO+0x50},
	{"?S",		16,	SRO+0x60},
	{"?S",		16,	SRO+0x70},

{0, 0, 0}};



/// Список команд

/*typedef struct _INSTRUCTION
{
	long Code;		///< Код инструкции
	long CodeMask;		///< Маска для сравнения кода
	long CodeLen;   ///< Длина кода
	char * Name;
	long StateMask;	///< Операция с состоянием: 0 - XOR, 1 - MOV
	long StateSet;	///< Аргумент операции с состоянием
} INSTRUCTION;*/
#define am(Flags)(	(((Flags) & X86_Cond)        ? 0xFFFFFFF0 : -1) &\
					(((Flags) & X86_r)           ? 0xFFFFFFF8 : -1) &\
					(((Flags) & (X86_s | X86_d | X86_1CL)) ? 0xFFFFFFFC : -1) &\
					(((Flags) & X86_w)           ? 0xFFFFFFFE : -1) )

#define b1c(Code, Name, Flags)      {               Code,	              0xFF & am(Flags), 1, _##Name, X86_Prefix |((Flags)&(X86_o32|X86_a32)),             (Flags)},
#define b2c(c1, c2, Name, Flags)    {     c1 + (c2 << 8), 0xFF | ((0xFF & am(Flags)) << 8), 2, _##Name, X86_Prefix |((Flags)&(X86_o32|X86_a32)),             (Flags)},
#define b1co(Code, RO, Name, Flags) {  Code + (RO << 11),    	        0x38FF & am(Flags), 1, _##Name, X86_Prefix |((Flags)&(X86_o32|X86_a32)),   (Flags)|X86_ModRM},
#define segpfx(Code, Name, SegReg)  {               Code,                             0xFF, 1,    Name, X86_Prefix |                 X86_SegReg, X86_Prefix | SegReg},

#define X86_dw_RO (X86_d | X86_w | X86_ModRM | X86_RO)	// XXX r/m, r/m
#define X86_sw_RM (X86_s | X86_w | X86_ModRM)			// XXX r/m, i
#define X86_ac_i (X86_w | X86_i | X86_ac)				// XXX ac, i
INSTRUCTION x86InstructionSet[] = {
// Двоичная арифметика b1c(8 * n, n, dw_RO) b1c(4 + 8 * n, n, ac_i) b1co(0x80, n, n, sw_RM)
	b1c(	0x00,	 ADD, X86_dw_RO)
	b1c(	0x04,	 ADD, X86_ac_i)
	b1co(	0x80, 0, ADD, X86_sw_RM)

	b1c(	0x08,	  OR, X86_dw_RO)
	b1c(	0x0C,	  OR, X86_ac_i)
	b1co(	0x80, 1,  OR, X86_sw_RM)

	b1c(	0x10,	 ADC, X86_dw_RO)
	b1c(	0x14,	 ADC, X86_ac_i)
	b1co(	0x80, 2, ADC, X86_sw_RM)

	b1c(	0x18,	 SBB, X86_dw_RO)
	b1c(	0x1C,	 SBB, X86_ac_i)
	b1co(	0x80, 3, SBB, X86_sw_RM)

	b1c(	0x20,	 AND, X86_dw_RO)
	b1c(	0x24,	 AND, X86_ac_i)
	b1co(	0x80, 4, AND, X86_sw_RM)

	b1c(	0x28,	 SUB, X86_dw_RO)
	b1c(	0x2C,	 SUB, X86_ac_i)
	b1co(	0x80, 5, SUB, X86_sw_RM)

	b1c(	0x30,	 XOR, X86_dw_RO)
	b1c(	0x34,	 XOR, X86_ac_i)
	b1co(   0x80, 6, XOR, X86_sw_RM)

	b1c(	0x38,	 CMP, X86_dw_RO)
	b1c(	0x3C,	 CMP, X86_ac_i)
	b1co(   0x80, 7, CMP, X86_sw_RM)
	// INC/DEC
	b1c(	0x40,	 INC, X86_r)
	b1co(	0xFE, 0, INC, X86_w | X86_ModRM)
	b1c(	0x48,	 DEC, X86_r)
	b1co(	0xFE, 1, DEC, X86_w | X86_ModRM)

// Сдвиги
	b1co(	0xC0, 0, ROL, X86_w | X86_i8)
	b1co(   0xD0, 0, ROL, X86_w | X86_1CL)

	b1co(	0xC0, 1, ROR, X86_w | X86_i8)
	b1co(   0xD0, 1, ROR, X86_w | X86_1CL)

	b1co(	0xC0, 2, RCL, X86_w | X86_i8)
	b1co(   0xD0, 2, RCL, X86_w | X86_1CL)

	b1co(	0xC0, 3, RCR, X86_w | X86_i8)
	b1co(   0xD0, 3, RCR, X86_w | X86_1CL)

	b1co(	0xC0, 4, SHL, X86_w | X86_i8)// Он же SAL
	b1co(   0xD0, 4, SHL, X86_w | X86_1CL)

	b1co(	0xC0, 5, SHR, X86_w | X86_i8)
	b1co(   0xD0, 5, SHR, X86_w | X86_1CL)

	b1co(	0xC0, 7, SAR, X86_w | X86_i8 )
	b1co(   0xD0, 7, SAR, X86_w | X86_1CL)

// Передача данных
	b1c(	0x58,	 POP, X86_r) // Нет сегментных регистров!!!
	b1co(	0x8F, 0, POP, X86_ModRM)
	b1c(	0x50,	PUSH, X86_r)
	b1c(	0x68,	PUSH, X86_s)
	b1co(	0xFF, 6,PUSH, X86_ModRM)

	b1c(	0x60,  PUSHA, X86_O32D)
	b1c(	0x61,   POPA, X86_O32D)

	b1c(	0x9C,  PUSHF, X86_O32D)
	b1c(	0x9D,   POPF, X86_O32D)

	b1c(	0x90,	 NOP, 0)
	b1c(	0x86,   XCHG, X86_ModRM | X86_RO | X86_w)
	b1c(	0x90,   XCHG, X86_r | X86_ac)

// Переходы
	b1c(	0xE8,	CALL, X86_i | X86_jadd)
	b1co(	0xFF, 2,CALL, X86_ModRM)
	b1c(	0x70,	   J, X86_Cond | X86_i8 | X86_jadd)
	b1c(	0xEB,	 JMP,            X86_i8 | X86_jadd)
	b1c(	0xE9,	 JMP,            X86_i  | X86_jadd)
	b1co(	0xFF, 4, JMP, X86_ModRM)
	b1c(	0xC2,	 RET, X86_i16)
	b1c(    0xC3,    RET, 0)
	b1c(	0xCA,	RETF, X86_i16)
	b1c(    0xCB,   RETF, 0)
	b1c(	0xE0, LOOPNE, X86_i8 | X86_jadd)
	b1c(	0xE1,  LOOPE, X86_i8 | X86_jadd)
	b1c(	0xE2,   LOOP, X86_i8 | X86_jadd)
	b1c(	0xE3,  JECXZ, X86_i8 | X86_jadd | X86_a32)
	b1c(	0xE3,   JCXZ, X86_i8 | X86_jadd)

//

	b1c(	0x37,	 AAA, 0)
	b1c(	0xC9,  LEAVE, 0)
	b1c(	0x8D,	 LEA, X86_ModRM | X86_RO | X86_d1) // d = 1


	b1c(	0x88,	 MOV, X86_dw_RO)
	b1c(	0xA0,	 MOV, X86_d | X86_w | X86_ac    | X86_im) // d = 0 : ac, im
	b1c(	0xB8,	 MOV, X86_r | X86_i)
	b1c(	0xB0,	 MOV, X86_r | X86_i8)
	b1co(	0xC6, 0, MOV, X86_ModRM | X86_i | X86_w)

	b1co(	0xF6, 0,TEST,         X86_w | X86_ModRM | X86_i)
	b1c(	0x84,	TEST,		  X86_w | X86_ModRM | X86_RO) // d = 0
	b2c(0x0F, 0xB6, MOVZX, X86_w | X86_w2 | X86_ModRM | X86_RO | X86_d1) // Размер результата определяется o32, источника - w
// Префиксы
	segpfx(	0x2E,  "CS:", 1)
	segpfx(	0x36,  "SS:", 2)
	segpfx(	0x3E,  "DS:", 3)
	segpfx(	0x26,  "ES:", 0)
	segpfx(	0x64,  "FS:", 4)
	segpfx(	0x65,  "GS:", 6)
	{0x66, 0xFF, 1, "_o32", X86_Prefix | X86_o32, X86_Prefix},
	{0x67, 0xFF, 1, "_a32", X86_Prefix | X86_a32, X86_Prefix},
{0, 0, 0, 0}};
/*
d = 0 :  ac, im
d = 0 : R/M, R/O
d = 1 :  im, ac
d = 1 : R/O, R/M
*/

