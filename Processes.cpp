#include "stdafx.h"
#include "processes.h"
#include <windows.h>
#include <commctrl.h>
#include "APILog2.h"
#define _CRT_SECURE_NO_DEPRECATE


TCHAR szTitle[];								// The title bar text

REBARINFO RBInfo = {sizeof(RBInfo), 0, 0};
REBARBANDINFO BandInfo;

char FileNameBuf[MAX_PATH + 1];

OPENFILENAME PEOpenDlg = { 
	sizeof(OPENFILENAME),   // lStructSize; 
    0,						// hwndOwner; 
	0,						// hInstance; 
	"Файлы Portable Executable (*.exe, *.dll)\0*.exe;*.dll\0Все файлы (*.*)\0*.*\0",	// lpstrFilter; 
	0,	//LPTSTR        lpstrCustomFilter; 
	0,	//DWORD         nMaxCustFilter; 
	0,	//DWORD         nFilterIndex; 
	FileNameBuf, //LPTSTR        lpstrFile; 
	sizeof(FileNameBuf),//DWORD         nMaxFile; 
	0, //LPTSTR        lpstrFileTitle; 
	0,	//DWORD         nMaxFileTitle; 
	0,	//LPCTSTR       lpstrInitialDir; 
	"Загрузить модуль", //LPCTSTR       lpstrTitle; 
	OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST, //DWORD         Flags; 
	0,//WORD          nFileOffset; 
	0, //WORD          nFileExtension; 
	"exe",//LPCTSTR       lpstrDefExt; 
	0, //LPARAM        lCustData; 
	0, //LPOFNHOOKPROC lpfnHook; 
	0 //LPCTSTR       lpTemplateName; 
};

OPENFILENAME PESaveDlg = { 
	sizeof(OPENFILENAME),   // lStructSize; 
    0,						// hwndOwner; 
	0,						// hInstance; 
	"Файлы Portable Executable (*.exe, *.dll)\0*.exe;*.dll\0Все файлы (*.*)\0*.*\0",	// lpstrFilter; 
	0,	//LPTSTR        lpstrCustomFilter; 
	0,	//DWORD         nMaxCustFilter; 
	0,	//DWORD         nFilterIndex; 
	FileNameBuf, //LPTSTR        lpstrFile; 
	sizeof(FileNameBuf),//DWORD         nMaxFile; 
	0, //LPTSTR        lpstrFileTitle; 
	0,	//DWORD         nMaxFileTitle; 
	0,	//LPCTSTR       lpstrInitialDir; 
	"Сохранить модуль", //LPCTSTR       lpstrTitle; 
	OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST, //DWORD         Flags; 
	0,//WORD          nFileOffset; 
	0, //WORD          nFileExtension; 
	"exe",//LPCTSTR       lpstrDefExt; 
	0, //LPARAM        lCustData; 
	0, //LPOFNHOOKPROC lpfnHook; 
	0 //LPCTSTR       lpTemplateName; 
};

void AddBandToRebar(HWND hRebar, HWND hBar)
{
   REBARBANDINFO rbBand;
   //RECT          rc;

   // Initialize structure members that both bands will share.
   rbBand.cbSize = sizeof(REBARBANDINFO);  // Required
   rbBand.fMask  = /*RBBIM_COLORS | RBBIM_TEXT | RBBIM_BACKGROUND | */
                   RBBIM_STYLE | RBBIM_CHILD  | RBBIM_CHILDSIZE | 
                   RBBIM_SIZE/*| RBBIM_HEADERSIZE*/;
   rbBand.fStyle = RBBS_CHILDEDGE | RBBS_FIXEDBMP;// | RBBS_GRIPPERALWAYS;// | RBBS_BREAK;

   rbBand.hbmBack = 0;//LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_BITMAP1));   


	SIZE Size;
	SendMessage(hBar, TB_GETMAXSIZE, 0, (LPARAM)&Size);
   //GetWindowRect(hBar, &rc);
   rbBand.lpText     = "Tool Bar";
   rbBand.hwndChild  = hBar;
   int w = Size.cx;// rc.right - rc.left;
   rbBand.cxMinChild = w;//100;
   rbBand.cyMinChild = Size.cy;// rc.bottom - rc.top;
   rbBand.cx         = w;//150;
   rbBand.cxHeader = 2;
   // Add the band that has the toolbar.
   SendMessage(hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);

   //return hRebar;
}

// Кнопки
// BMP, Cmd, State, Style, {}, Data



HWND CreateToolBar(HWND hOwner, HINSTANCE hInstance, UINT BMP, TBBUTTON * Buttons, int ButtonCount)
{
#define ToolbarStyle WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TBSTYLE_FLAT | TBSTYLE_CUSTOMERASE | TBSTYLE_ALTDRAG | TBSTYLE_WRAPABLE | TBSTYLE_TRANSPARENT

	TBADDBITMAP TBAddBitmap = {hInstance, BMP}; // IDB_BITMAP1

	HWND hToolbar = CreateWindowEx( 0, "ToolbarWindow32", 0, 
		WS_CHILD|TBSTYLE_LIST|TBSTYLE_FLAT|TBSTYLE_TOOLTIPS|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|CCS_NODIVIDER|WS_VISIBLE|CCS_NORESIZE,
		0,0, 0, 24, hOwner, 0, hInstance, 0);

	SendMessage(hToolbar, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
	SendMessage(hToolbar, TB_ADDBITMAP, ButtonCount, (long) &TBAddBitmap);
	SendMessage(hToolbar, TB_SETBUTTONWIDTH, 0, MAKELONG(24, 24));
	SendMessage(hToolbar, TB_SETBUTTONSIZE, 0, MAKELONG(24, 24));

	SendMessage(hToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	SendMessage(hToolbar, TB_ADDBUTTONS, ButtonCount, (long) Buttons);

	return hToolbar;
}

THeadersForm::THeadersForm(TMDIClient * MDIClient, TPESource * PE):TMDIChildWindow(MDIClient, "Заголовки и секции"), 
GBSections(this, (RECT*)((GetHeight() * 2) / 5), LT_SALBOTTOM | LT_SCALE, "Секции"),BSplitter(this, 8, LT_ALBOTTOM),
GBMZ(this, (RECT*) (GetWidth() / 2), LT_SALLEFT | LT_SCALE, "Заголовок MZ"), VSplitter(this, 8, LT_ALLEFT),
GBPE(this, 0, LT_ALCLIENT, "Заголовок PE"),
PEGR(&GBPE, 0, LT_ALCLIENT, FD_PEHEADER), MZGR(&GBMZ, 0, LT_ALCLIENT, FD_MZHEADER),
AGSX(&GBSections, 0, LT_ALCLIENT, FD_OBJECTTABLE)
{
	if(!PE) return;
	if(PE->PEHeader)
	{
		PEGR.FRecord = PE->PEHeader;
		AGSX.SetRecords(PE->FSections, PE->FSectionCount);
	}
	if(PE->MZHeader) 
		MZGR.FRecord = PE->MZHeader;
}

TExportForm::TExportForm(TMDIClient * MDIClient, TPESource * PE):TMDIChildWindow(MDIClient, "Экспорт"),
GroupBox(this, 0, LT_ALCLIENT, "Список экспортируемых функций"), ExportGrid(&GroupBox, 0, LT_ALCLIENT, FD_EXPORTEDENTRY)
{
	if(PE && PE->PEHeader && PE->FExported) ExportGrid.SetRecords(PE->FExported, PE->FExportedCount);
}

TImportForm::TImportForm(TMDIClient * MDIClient, TPESource * PE):TMDIChildWindow(MDIClient, "Импорт"),
GroupBox(this, 0, LT_ALCLIENT, "Список импортируемых функций"), ImportGrid(&GroupBox, 0, LT_ALCLIENT, FD_IMPORTEDENTRY/*LIB*/)
{
	if(PE && PE->PEHeader && PE->FImportedLibCount) ImportGrid.SetRecords(PE->FImported/*Libs*/, PE->FImportedCount);
}

TDisasmForm::TDisasmForm(TMDIClient * MDIClient, TDumpSource * PE):TMDIChildWindow(MDIClient, "Дизассемблер"),
GroupBox(this, 0, LT_ALCLIENT, "Дизассемблер"), AsmGrid(&GroupBox, 0, LT_ALCLIENT, PE)
{
	//AsmGrid = new TAsmGrid(&GroupBox, 0, LT_ALCLIENT);
	//TAsmGrid AsmGrid(&GroupBox, 0, LT_ALCLIENT);
	//if(PE && PE->FPELoaded) AsmGrid.SetPosition(PE->RVA(PE->PETitle.Entrypoint_RVA), (char *)PE->PETitle.Entrypoint_RVA + PE->PETitle.Image_base);
}

TDumpForm::TDumpForm(TMDIClient * MDIClient, TDumpSource * PE):TMDIChildWindow(MDIClient, "Дамп"),
GroupBox(this, 0, LT_ALCLIENT, "Дамп"), DumpGrid(&GroupBox, 0, LT_ALCLIENT, PE)
{
	//AsmGrid = new TAsmGrid(&GroupBox, 0, LT_ALCLIENT);
	//TAsmGrid AsmGrid(&GroupBox, 0, LT_ALCLIENT);
	//if(PE && PE->FPELoaded) AsmGrid.SetPosition(PE->RVA(PE->PETitle.Entrypoint_RVA), (char *)PE->PETitle.Entrypoint_RVA + PE->PETitle.Image_base);
}

TBBUTTON FileButtons[] = {
        {0, IDM_NEW, TBSTATE_ENABLED, 0,                        {0, 0}, 0},
        {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0, 0}, 0},
        {1, IDM_OPEN, TBSTATE_ENABLED, 0,           {0, 0}, 0}
};

TBBUTTON WinButtons[] = {
        {0, IDM_HEADERS, TBSTATE_ENABLED, 0,                    {0, 0}, 0},
        {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0, 0}, 0},
        {7, IDM_EXPORT, TBSTATE_ENABLED, 0,                             {0, 0}, 0},
        {6, IDM_IMPORT, TBSTATE_ENABLED, 0,                             {0, 0}, 0},
        {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0, 0}, 0},
        {1, IDM_DISASM, TBSTATE_ENABLED, 0,                             {0, 0}, 0}
};

TBBUTTON RunButtons[] = {
	{8, IDM_STOP, TBSTATE_ENABLED|TBSTATE_PRESSED, 0,			{0, 0}, 0},
	{9, IDM_PAUSE, TBSTATE_ENABLED, 0,				{0, 0}, 0},
	{10, IDM_START, TBSTATE_ENABLED, 0,			{0, 0}, 0},
};

TProcess::TProcess(HINSTANCE hInstance, char * ModuleName, int nCmdShow, HFONT hFont):
TAppWindow(hInstance, szTitle, LoadMenu(hInstance, (char *) IDC_APILOG2)), FState(sEmpty),
FStatusBar(this, "Text"), FReBar(this), FMDIClient(this, 0, LT_ALCLIENT)
{
	set_Font(hFont);
	FMDIClient.set_Font(hFont);
	FExportForm = 0;
	FImportForm = 0;
	FDisasmForm = 0;
	FHeadersForm = 0;
	FDumpForm = 0;

	FPE = 0;
	Commands(IDM_NEW)->Assign(&TProcess::OnNew, this);
	Commands(IDM_OPEN)->Assign(&TProcess::OnOpen, this);
	Commands(IDM_SAVE)->Assign(&TProcess::OnSave, this);
	Commands(IDM_SAVEAS)->Assign(&TProcess::OnSaveAs, this);
	Commands(IDM_RECENT)->Assign(&TProcess::OnRecent, this);
	Commands(IDM_EXIT)->Assign(&TProcess::OnExit, this);
	Commands(IDM_ABOUT)->Assign(&TProcess::OnAbout, this);
	Commands(IDM_HEADERS)->Assign(&TProcess::OnChildShow, this);
	Commands(IDM_EXPORT)->Assign(&TProcess::OnChildShow, this);
	Commands(IDM_IMPORT)->Assign(&TProcess::OnChildShow, this);
	Commands(IDM_DISASM)->Assign(&TProcess::OnChildShow, this);
	Commands(IDM_DUMP)->Assign(&TProcess::OnChildShow, this);

	PEOpenDlg.hInstance = hInstance;
	HWND hToolbar = 0;

	if(ModuleName)
	{
		int l = strlen(szTitle);
		char * p = szTitle + l;
		strcpy(p, " - ");
		strcpy(p + 3, ModuleName);
		set_Text(szTitle);
		*p = 0;
	}

	// Создать панель инструментов
	hToolbar = CreateToolBar(FReBar.GetWindowHandle(), hInstance, IDB_BITMAP1, FileButtons, 3);
	HWND hToolbar1 = CreateToolBar(FReBar.GetWindowHandle(), hInstance, IDB_BITMAP1, WinButtons, 6);
	HWND hToolbar2 = CreateToolBar(FReBar.GetWindowHandle(), hInstance, IDB_BITMAP1, RunButtons, 3);

	AddBandToRebar(FReBar.GetWindowHandle(), hToolbar);
	AddBandToRebar(FReBar.GetWindowHandle(), hToolbar1);
	AddBandToRebar(FReBar.GetWindowHandle(), hToolbar2);

	if(ModuleName) 
	{
		FPE = new TPEFile(ModuleName);
		FHeadersForm = new THeadersForm(&FMDIClient, FPE);
		FHeadersForm->OnDestroy.Assign(&TProcess::OnChildDestroy, this);
	}

	ShowWindow(hWindow, nCmdShow);
	UpdateWindow(hWindow);
}

TProcess::~TProcess()
{
	if(FPE) delete(FPE);
	RemoveChild(&FStatusBar, false);
	RemoveChild(&FReBar, false);
	RemoveChild(&FMDIClient, false);
}


void __delcall TProcess::OnNew(TProcess * Sender)
{
	new TProcess(hInstance, 0, SW_SHOWNORMAL, FFont);
}

void __delcall TProcess::OnRecent(TProcess * Sender)
{
	/*FPE = new TPELocalModule((void *)0x400000);
	FPE->ReadExports();
	FPE->ReadImports();*/
	//new TProcess(hInstance, 0, SW_SHOWNORMAL, FFont);
}

void __delcall TProcess::OnSave(TProcess * Sender)
{
	if(FPE)
		((TPEFile *)FPE)->SaveToFile(FileNameBuf);
}

void __delcall TProcess::OnSaveAs(TProcess * Sender)
{
	PESaveDlg.hwndOwner = hWindow;
	if(FPE && GetSaveFileName(&PESaveDlg))
	{
		((TPEFile *)FPE)->SaveToFile(FileNameBuf);
	}

}

void __delcall TProcess::OnOpen(TProcess * Sender)
{
	PEOpenDlg.hwndOwner = hWindow;
	if(GetOpenFileName(&PEOpenDlg))
	{
		if(FPE)	new TProcess(hInstance, FileNameBuf, SW_SHOWNORMAL, FFont);
		else 
		{
			FPE = new TPEFile(FileNameBuf);
			if(FHeadersForm) delete FHeadersForm;
			FHeadersForm = new THeadersForm(&FMDIClient, FPE);
			FHeadersForm->OnDestroy.Assign(&TProcess::OnChildDestroy, this);

			int l = strlen(szTitle);
			char * p = szTitle + l;
			strcpy(p, " - ");
			strcpy(p + 3, FileNameBuf);
			set_Text(szTitle);
			*p = 0;
		}
	}
}

void __delcall TProcess::OnExit(TProcess * Sender)
{
	Sender->SendMsg(WM_CLOSE);
}

void __delcall TProcess::OnAbout(TProcess * Sender)
{
	DialogBox(hInstance, (LPCTSTR)IDD_ABOUTBOX, hWindow, (DLGPROC)About);
}

void __delcall TProcess::OnChildShow(TProcess * Sender, UINT Identifier)
{
	TControl * * Control = 0;
	switch(Identifier)
	{
	case IDM_EXPORT:
		if(!FExportForm) 
		{
			FExportForm = new TExportForm(&FMDIClient, FPE);
			FExportForm->OnDestroy.Assign(&TProcess::OnChildDestroy, this);
			return;
		}
		Control = (TControl * *)&FExportForm;
		break;
	case IDM_IMPORT:
		if(!FImportForm) 
		{
			FImportForm = new TImportForm(&FMDIClient, FPE);
			FImportForm->OnDestroy.Assign(&TProcess::OnChildDestroy, this);
			return;
		}
		Control = (TControl * *)&FImportForm;
		break;
	case IDM_DISASM:
		if(!FDisasmForm)
		{
			FDisasmForm = new TDisasmForm(&FMDIClient, FPE);
			FDisasmForm->OnDestroy.Assign(&TProcess::OnChildDestroy, this);
			return;
		}
		Control = (TControl * *)&FDisasmForm;
		break;
	case IDM_DUMP:
		if(!FDumpForm)
		{
			FDumpForm = new TDumpForm(&FMDIClient, FPE);
			FDumpForm->OnDestroy.Assign(&TProcess::OnChildDestroy, this);
			return;
		}
		Control = (TControl * *)&FDumpForm;
		break;
	case IDM_HEADERS:
		if(!FHeadersForm) 
		{
			FHeadersForm = new THeadersForm(&FMDIClient, FPE);
			FHeadersForm->OnDestroy.Assign(&TProcess::OnChildDestroy, this);
			return;
		}
		Control = (TControl * *)&FHeadersForm;
		break;
	}
	if(Control && *Control)
	{
		delete *Control; 
		*Control = 0;
	}
}

void __delcall TProcess::OnChildDestroy(TControl * Child)
{
	if(Child == FImportForm) FImportForm = 0;
	else
	if(Child == FExportForm) FExportForm = 0;
	else
	if(Child == FHeadersForm) FHeadersForm = 0;
	else
	if(Child == FDisasmForm) FDisasmForm = 0;
	else
	if(Child == FDumpForm) FDumpForm = 0;
}