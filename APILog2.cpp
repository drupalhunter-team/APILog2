// APILog2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <commctrl.h>
#include <crtdbg.h>
#include "StdGUI.h"
#include "Processes.h"

extern TCHAR szTitle[MAX_PATH + 1] = "";								// The title bar text


// Foward declarations of functions included in this code module:
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

INITCOMMONCONTROLSEX ICC = {sizeof(ICC), ICC_BAR_CLASSES|ICC_DATE_CLASSES|ICC_USEREX_CLASSES|ICC_COOL_CLASSES};

LOGFONT LogFont = {
12, //      lfHeight;
0, //      lfWidth;
0, //      lfEscapement;
0, //      lfOrientation;
400, //    lfWeight;
false, //  lfItalic;
false, //  lfUnderline;
false, //  lfStrikeOut;
DEFAULT_CHARSET,//lfCharSet;
OUT_DEFAULT_PRECIS, //lfOutPrecision;
CLIP_DEFAULT_PRECIS, // lfClipPrecision;
ANTIALIASED_QUALITY, // lfQuality;
FIXED_PITCH, // lfPitchAndFamily;
"Courier"//"MS Sans Serif"
};

COLORREF mCol = 0x15E276;
COLORREF hCol = 0x06512A;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	InitCommonControlsEx(&ICC);
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_PATH);

	HACCEL hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_APILOG2);
#define MATRIX
#ifdef MATRIX
	TControl::FDefaultBackBrush = CreateSolidBrush(0);
	TControl::FDefaultPen = CreatePen(PS_SOLID, 1, hCol);
	TAppWindow::FWindowClass.hIconSm = LoadIcon(hInstance, (LPCTSTR)IDI_SMALL);
	TAppWindow::FWindowClass.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_APILOG2);
	TControl::FDefaultTextColor = 0x00FF79;
	TGroupBox::FDefaultStyle = 0;

	TCustomGrid::FDefaultTxtColor = mCol;
	TCustomGrid::FDefaultSelTxtColor = 0x00FF79;//0x0FF079;

	TCustomGrid::FDefaultCellsBrush = TControl::FDefaultBackBrush;
	TCustomGrid::FDefaultSelBrush = CreateSolidBrush(hCol);
	TCustomGrid::FDefaultBlurBrush = CreateSolidBrush(0x032C17);
#endif
	HFONT hFont = CreateFontIndirect(&LogFont);
	TProcess * Process = new TProcess(hInstance, 0, nCmdShow, hFont);

    int Result = DefMessageLoop(hAccelTable);
	//DestroyIcon(TAppWindow::FWindowClass.hIconSm);
	//DestroyIcon(TAppWindow::FWindowClass.hIcon);
	DeleteObject(hFont);
#ifdef MATRIX
	DeleteObject(TControl::FDefaultBackBrush);
	DeleteObject(TCustomGrid::FDefaultSelBrush);
	DeleteObject(TControl::FDefaultPen);
#endif

	DestroyAcceleratorTable(hAccelTable);

	_CrtMemDumpAllObjectsSince(0);
    return Result;	
}

//! Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
