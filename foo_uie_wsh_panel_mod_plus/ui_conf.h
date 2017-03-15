#pragma once

#include "splitter_window.h"
#include "editorctrl.h"
#include "resource.h"

// Forward declarations
class wsh_panel_window;
class CDialogFind;
class CDialogReplace;

//-----
class CDialogConf 
	: public CDialogImpl<CDialogConf>
	, public CDialogResize<CDialogConf>
{
public:
	class wsh_console_receiver_impl : public console_receiver
	{
	public:
		virtual void print(const char * p_message, unsigned p_message_length);
	};

	static HWND g_cfgdlg;//ensure that there is only one dlg instance.

private:
	wsh_splitter_window        m_split_wnd;
	CScriptEditorCtrl          m_editorctrl;
	CScriptEditorCtrl          m_consolectrl;
	CDialogFind *              m_dlgfind;
	CDialogReplace *           m_dlgreplace;
	wsh_panel_window *         m_parent;
	CDialogConf **             m_self;
	pfc::string8               m_caption;
	unsigned int               m_lastFlags;
	pfc::string8               m_lastSearchText;

	enum { IDM_ALWAYS_ON_TOP = 0x1002, };

public:
	CDialogConf(wsh_panel_window * p_parent , CDialogConf ** p_self) 
		: m_parent(p_parent)
		, m_dlgfind(NULL)
		, m_dlgreplace(NULL)
		, m_self(p_self)
        , m_lastSearchText("")
        , m_lastFlags(0)
	{
		//pfc::dynamic_assert(m_parent != NULL, "CDialogConf: m_parent invalid.");
	}

	virtual ~CDialogConf()
	{
		m_hWnd = NULL;
	}

public:

	void OnFinalMessage(HWND hWnd);

	bool MatchShortcuts(unsigned vk);
    void OpenFindDialog();
    void Apply();
	void OnResetDefault();
	void OnResetCurrent();
	void OnImport();
	void OnExport();
    
	//stolen from SciTE
	void OnMarginClick(int position, int modifiers);

public:
	enum { IDD = IDD_DIALOG_CONFIG };

	BEGIN_MSG_MAP(CDialogConf)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_NOTIFY(OnNotify)
		MSG_WM_INITMENUPOPUP(OnInitMenuPopup)
		MSG_WM_SYSCOMMAND(OnSysCommand)
		MESSAGE_HANDLER(UWM_KEYDOWN, OnUwmKeyDown)
        MESSAGE_HANDLER(UWM_FINDTEXTCHANGED, OnUwmFindTextChanged)
		MESSAGE_HANDLER(UWM_CONSOLE_PRINT,OnUwmConsolePrint)
		COMMAND_ID_HANDLER_EX(IDC_SHOW_CONSOLE_PANE,OnShowConsolePane)
		COMMAND_RANGE_HANDLER_EX(IDOK, IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER_EX(IDAPPLY, OnCloseCmd)
		COMMAND_HANDLER_EX(IDC_SCRIPT_ENGINE, CBN_SELENDOK, OnScriptEngineCbnSelEndOk)
		COMMAND_ID_HANDLER_EX(IDC_TOOLS, OnTools)
		CHAIN_MSG_MAP(CDialogResize<CDialogConf>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CDialogConf)
		DLGRESIZE_CONTROL(IDC_CHECK_PSEUDO_TRANSPARENT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CHECK_GRABFOCUS, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CHECK_DELAY_LOAD, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TOOLS, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_SHOW_CONSOLE_PANE, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC_HOLDER, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)  
		DLGRESIZE_CONTROL(IDAPPLY, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC_GUID, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

public:
	LRESULT OnInitDialog(HWND hwndFocus, LPARAM lParam);
	void OnShowConsolePane(UINT uNotifyCode, int nID, CWindow wndCtl);
	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnScriptEngineCbnSelEndOk(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnTools(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnNotify(int idCtrl, LPNMHDR pnmh);
	void OnInitMenuPopup(CMenuHandle menuPopup, UINT nIndex, BOOL bSysMenu);
	void OnSysCommand(UINT nID, CPoint point);
	LRESULT OnNCDestroy();
	LRESULT OnUwmKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnUwmFindTextChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnUwmConsolePrint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    static bool FindNext(HWND hWnd, HWND hWndEdit, unsigned flags, const char *which);
    static bool FindPrevious(HWND hWnd, HWND hWndEdit, unsigned flags, const char *which);
    static bool FindResult(HWND hWnd, HWND hWndEdit, int pos, const char *which);

private:
	BOOL IsMenuItemExist(HMENU hMenu, UINT_PTR uItem);
};