#include <windows.h>
#include "PEFiles.h"
#include "..\units\StdGUI.h"
#include "BinControls.h"
#include "Assembler.h"
using namespace PE;

class THeadersForm: public TMDIChildWindow
{
private:
	TGroupBox GBSections;
	TSplitter BSplitter;
	TGroupBox GBMZ;
	TSplitter VSplitter;
	TGroupBox GBPE;
	TRecordGrid PEGR;
	TRecordGrid MZGR;
	TArrayGrid AGSX;
public:
	THeadersForm(TMDIClient * MDIClient, TPESource * PE);
};

class TExportForm: public TMDIChildWindow
{
private:
	TGroupBox GroupBox;
	TArrayGrid ExportGrid;
public:
	TExportForm(TMDIClient * MDIClient, TPESource * PE);
};

class TImportForm: public TMDIChildWindow
{
private:
	TGroupBox GroupBox;
	TArrayGrid ImportGrid;
public:
	TImportForm(TMDIClient * MDIClient, TPESource * PE);
};

class TDumpForm: public TMDIChildWindow
{
private:
	TGroupBox GroupBox;
	TDumpGrid DumpGrid;
public:
	TDumpForm(TMDIClient * MDIClient, TDumpSource * PE);
};

class TDisasmForm: public TMDIChildWindow
{
private:
	TGroupBox GroupBox;
	TAsmGrid AsmGrid;
public:
	TDisasmForm(TMDIClient * MDIClient, TDumpSource * PE);
};

class TProcess: public TAppWindow
{
private:
	enum {sEmpty, sDLL, sRunable, sStop, sRun} FState;

	// Статические визуальные компоненты
	TStatusBar FStatusBar;
	TReBar FReBar;
	TMDIClient FMDIClient;

	// Дочерние MDI-формы
	TExportForm * FExportForm;
	TImportForm * FImportForm;
	THeadersForm * FHeadersForm;
	TDumpForm * FDumpForm;
	TDisasmForm * FDisasmForm;
//	char * FullModuleName;
	TPESource * FPE;
	void __delcall OnChildDestroy(TControl * Child);
	void __delcall OnChildShow(TProcess * Sender, UINT Identifier);
public:
	void __delcall OnNew(TProcess * Sender);
	void __delcall OnRecent(TProcess * Sender);
	void __delcall OnOpen(TProcess * Sender);
	void __delcall OnSave(TProcess * Sender);
	void __delcall OnSaveAs(TProcess * Sender);
	void __delcall OnExit(TProcess * Sender);
	void __delcall OnAbout(TProcess * Sender);
	TProcess(HINSTANCE hInstance, char * ModuleName, int nCmdShow, HFONT hFont = 0);
	virtual ~TProcess();
};
