#include "stdafx.h"
#include "ui_conf.h"
#include "wsh_panel_window.h"
#include "ui_goto.h"
#include "ui_find.h"
#include "ui_replace.h"
#include "helpers.h"


using namespace pfc::stringcvt;

//config_dialog_manager g_config_dlg_mgr;

LRESULT CDialogConf::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	//g_config_dlg_mgr.add_window(m_hWnd);
	modeless_dialog_manager::g_add(m_hWnd);

	// Get caption text
	uGetWindowText(m_hWnd, m_caption);

	// Init resize
	DlgResize_Init();

	// Apply window placement
	if (m_parent->get_windowplacement().length == 0)
	{
		m_parent->get_windowplacement().length = sizeof(WINDOWPLACEMENT);

		if (!GetWindowPlacement(&m_parent->get_windowplacement()))
			memset(&m_parent->get_windowplacement(), 0, sizeof(WINDOWPLACEMENT));
	}
	else
	{
		SetWindowPlacement(&m_parent->get_windowplacement());
	}

	// Script Engine
	HWND combo_script_engine = GetDlgItem(IDC_SCRIPT_ENGINE);

	ComboBox_AddString(combo_script_engine, _T("JScript"));
    ComboBox_AddString(combo_script_engine, _T("JScript9"));
	ComboBox_AddString(combo_script_engine, _T("VBScript"));

	if (!uComboBox_SelectString(combo_script_engine, m_parent->get_script_engine()))
		ComboBox_SetCurSel(combo_script_engine, 0);

	// Edge Style
	HWND combo_edge_style = GetDlgItem(IDC_EDGE_STYLE);

	ComboBox_AddString(combo_edge_style, _T("None"));    // NO_EDGE
	ComboBox_AddString(combo_edge_style, _T("Sunken"));  // SUNKEN_EDGE
	ComboBox_AddString(combo_edge_style, _T("Grey"));    // GREY_EDGE
	ComboBox_SetCurSel(combo_edge_style, m_parent->get_edge_style());

	// Edit box
	pfc::string8_fast text;
	int cursel = ComboBox_GetCurSel(combo_script_engine);

	// Subclassing scintilla
	m_editorctrl.SubclassWindow(GetDlgItem(IDC_EDIT));

	if (uComboBox_GetText(combo_script_engine, cursel, text))
	{
		m_editorctrl.SetLanguage(text);
	}

	// Checkboxs
	uButton_SetCheck(m_hWnd, IDC_CHECK_GRABFOCUS, m_parent->get_grab_focus());
	uButton_SetCheck(m_hWnd, IDC_CHECK_PSEUDO_TRANSPARENT, m_parent->get_pseudo_transparent());
	uButton_SetCheck(m_hWnd, IDC_CHECK_DELAY_LOAD, m_parent->get_delay_load());

	// GUID Text
	pfc::string8 guid_text = "GUID: ";
	guid_text += pfc::print_guid(m_parent->get_config_guid());
	uSetWindowText(GetDlgItem(IDC_STATIC_GUID), guid_text);

	// Script
	m_editorctrl.SetContent(m_parent->get_script_code(), true);

	// Set save point
	m_editorctrl.SetSavePoint();

	return TRUE; // set focus to default control
}

LRESULT CDialogConf::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	switch (wID)
	{
	case IDOK:
		Apply();
		DestroyWindow();
		break;

	case IDAPPLY:
		Apply();
		break;

	case IDCANCEL:
		if (m_editorctrl.GetModify())
		{
			// Prompt?
			int ret = uMessageBox(m_hWnd, "Do you want to apply your changes?", m_caption, MB_ICONWARNING | MB_SETFOREGROUND | MB_YESNOCANCEL);

			switch (ret)
			{
			case IDYES:
				Apply();
				DestroyWindow();
				break;

			case IDCANCEL:
				return 0;
			}
		}

		DestroyWindow();
	}
	
	return 0;
}

LRESULT CDialogConf::OnScriptEngineCbnSelEndOk(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	pfc::string8_fast text;
	HWND edit = GetDlgItem(IDC_EDIT);
	HWND combo = GetDlgItem(IDC_SCRIPT_ENGINE);
	int cursel = ComboBox_GetCurSel(combo);

	if (uComboBox_GetText(combo, cursel, text))
	{
		m_editorctrl.SetLanguage(text);
	}

	return 0;
}

void CDialogConf::OnResetDefault()
{
	pfc::string8 code;
	HWND combo = GetDlgItem(IDC_SCRIPT_ENGINE);

	wsh_panel_vars::get_default_script_code(code);
	uComboBox_SelectString(combo, "JScript");
	m_editorctrl.SetContent(code);
}

void CDialogConf::OnResetCurrent()
{
	HWND combo = GetDlgItem(IDC_SCRIPT_ENGINE);

	uComboBox_SelectString(combo, m_parent->get_script_engine());			
	m_editorctrl.SetContent(m_parent->get_script_code());
}

void CDialogConf::OnImport()
{
	pfc::string8 filename;

	if (uGetOpenFileName(m_hWnd, "Text files|*.txt|JScript files|*.js|All files|*.*", 0, "txt", "Import from", NULL, filename, FALSE))
	{
		// Open file
		pfc::string8_fast text;

		helpers::read_file(filename, text);
		m_editorctrl.SetContent(text);
	}
}

void CDialogConf::OnExport()
{
	pfc::string8 filename;

	if (uGetOpenFileName(m_hWnd, "Text files|*.txt|All files|*.*", 0, "txt", "Save as", NULL, filename, TRUE))
	{
		int len = m_editorctrl.GetTextLength();
		pfc::string8_fast text;

		m_editorctrl.GetText(text.lock_buffer(len), len + 1);
		text.unlock_buffer();

		helpers::write_file(filename, text);
	}
}

LRESULT CDialogConf::OnTools(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	enum
	{
		kImport = 1,
		kExport,
		kResetDefault,
		kResetCurrent,
	};

	HMENU menu = CreatePopupMenu();
	int ret = 0;
	int flags = TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD;
	RECT rc = {0};

	AppendMenu(menu, MF_STRING, kImport, _T("&Import"));
	AppendMenu(menu, MF_STRING, kExport, _T("E&xport"));
	AppendMenu(menu, MF_SEPARATOR, 0, 0);
	AppendMenu(menu, MF_STRING, kResetDefault, _T("Reset &Default"));
	AppendMenu(menu, MF_STRING, kResetCurrent, _T("Reset &Current"));

	::GetWindowRect(::GetDlgItem(m_hWnd, IDC_TOOLS), &rc);

	ret = TrackPopupMenu(menu, flags, rc.left, rc.bottom, 0, m_hWnd, 0);

	switch (ret)
	{
	case kImport:
		OnImport();
		break;

	case kExport:
		OnExport();
		break;

	case kResetDefault:
		OnResetDefault();
		break;

	case kResetCurrent:
		OnResetCurrent();
		break;
	}

	DestroyMenu(menu);
	return 0;
}

void CDialogConf::Apply()
{
	pfc::string8 name;
	pfc::array_t<char> code;
	int len = 0;

	// Get engine name
	uGetWindowText(GetDlgItem(IDC_SCRIPT_ENGINE), name);
	// Get script text
	len = m_editorctrl.GetTextLength();
	code.set_size(len + 1);
	m_editorctrl.GetText(code.get_ptr(), len + 1);

	m_parent->get_edge_style() = static_cast<t_edge_style>(ComboBox_GetCurSel(GetDlgItem(IDC_EDGE_STYLE)));
	m_parent->get_disabled_before() = false;
	m_parent->get_grab_focus() = uButton_GetCheck(m_hWnd, IDC_CHECK_GRABFOCUS);
	m_parent->get_pseudo_transparent() = uButton_GetCheck(m_hWnd, IDC_CHECK_PSEUDO_TRANSPARENT);
	m_parent->get_delay_load() = uButton_GetCheck(m_hWnd, IDC_CHECK_DELAY_LOAD);
	m_parent->update_script(name, code.get_ptr());

	// Wndow position
	GetWindowPlacement(&m_parent->get_windowplacement());

	// Save point
	m_editorctrl.SetSavePoint();
}

LRESULT CDialogConf::OnNotify(int idCtrl, LPNMHDR pnmh)
{
	SCNotification * notification = (SCNotification *)pnmh;

	switch (pnmh->code)
	{
		// dirty
	case SCN_SAVEPOINTLEFT:
		{
			pfc::string8 caption = m_caption;

			caption += " *";
			uSetWindowText(m_hWnd, caption);
		}
		break;

		// not dirty
	case SCN_SAVEPOINTREACHED:
		uSetWindowText(m_hWnd, m_caption);
		break;

		// toggle fold
	case SCN_MARGINCLICK:
		{
			OnMarginClick(notification->position,notification->modifiers);
		}
		break;
	}

	SetMsgHandled(FALSE);
	return 0; 
}

bool CDialogConf::MatchShortcuts(unsigned vk)
{
	int modifiers = 
		(IsKeyPressed(VK_SHIFT) ? SCMOD_SHIFT : 0) |
		(IsKeyPressed(VK_CONTROL) ? SCMOD_CTRL : 0) |
		(IsKeyPressed(VK_MENU) ? SCMOD_ALT : 0);

    // Hotkeys
	if (modifiers == SCMOD_CTRL)
	{
		switch (vk)
		{
		case 'F':
            OpenFindDialog();
			return true;

		case 'H':
			{
				if (!m_dlgreplace)
				{
					m_dlgreplace = new CDialogReplace(GetDlgItem(IDC_EDIT));
					
					if (!m_dlgreplace || !m_dlgreplace->Create(m_hWnd))
						break;
				}

				m_dlgreplace->ShowWindow(SW_SHOW);
				m_dlgreplace->SetFocus();
			}
			return true;

		case 'G':
			{
				modal_dialog_scope scope(m_hWnd);
				CDialogGoto dlg(GetDlgItem(IDC_EDIT));
				dlg.DoModal(m_hWnd);
			}
			return true;

		case 'S':
			Apply();
			return true;
		}
	} 
    else if (modifiers == 0) 
    {
        if (vk == VK_F3)
        {
            // Find next one
            if (!m_lastSearchText.is_empty()) 
            {
                FindNext(m_hWnd, m_editorctrl.m_hWnd, m_lastFlags, m_lastSearchText);
            }
            else
            {
                OpenFindDialog();
            }
        }
    }
    else if (modifiers == SCMOD_SHIFT)
    {
        if (vk == VK_F3)
        {
            // Find previous one
            if (!m_lastSearchText.is_empty()) 
            {
                FindPrevious(m_hWnd, m_editorctrl.m_hWnd, m_lastFlags, m_lastSearchText);
            }
            else
            {
                OpenFindDialog();
            }
        }
    }

	return false;
}

LRESULT CDialogConf::OnUwmKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return MatchShortcuts(wParam);
}

LRESULT CDialogConf::OnUwmFindTextChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_lastFlags = wParam;
    m_lastSearchText = reinterpret_cast<const char*>(lParam);
    return 0;
}

bool CDialogConf::FindNext(HWND hWnd, HWND hWndEdit, unsigned flags, const char *which)
{
    ::SendMessage(::GetAncestor(hWndEdit, GA_PARENT), UWM_FINDTEXTCHANGED, flags, reinterpret_cast<LPARAM>(which));

    SendMessage(hWndEdit, SCI_CHARRIGHT, 0, 0);
    SendMessage(hWndEdit, SCI_SEARCHANCHOR, 0, 0);
    int pos = ::SendMessage(hWndEdit, SCI_SEARCHNEXT, flags, reinterpret_cast<LPARAM>(which));
    return FindResult(hWnd, hWndEdit, pos, which);
}

bool CDialogConf::FindPrevious(HWND hWnd, HWND hWndEdit, unsigned flags, const char *which)
{
    ::SendMessage(::GetAncestor(hWndEdit, GA_PARENT), UWM_FINDTEXTCHANGED, flags, reinterpret_cast<LPARAM>(which));

    SendMessage(hWndEdit, SCI_SEARCHANCHOR, 0, 0);
    int pos = ::SendMessage(hWndEdit, SCI_SEARCHPREV, flags, reinterpret_cast<LPARAM>(which));
    return FindResult(hWnd, hWndEdit, pos, which);
}

bool CDialogConf::FindResult(HWND hWnd, HWND hWndEdit, int pos, const char *which)
{
    if (pos != -1)
    {
        // Scroll to view
		int line = ::SendMessage(hWndEdit,SCI_LINEFROMPOSITION, pos, 0);
		::SendMessage(hWndEdit, SCI_ENSUREVISIBLEENFORCEPOLICY, line, 0);
        ::SendMessage(hWndEdit, SCI_SCROLLCARET, 0, 0);
        return true;
    }

    pfc::string8 buff = "Cannot find \"";
    buff += which;
    buff += "\"";
    uMessageBox(hWnd, buff.get_ptr(), WSPM_NAME, MB_ICONINFORMATION | MB_SETFOREGROUND);
    return false;
}

void CDialogConf::OpenFindDialog()
{
    if (!m_dlgfind)
    {
        // Create it on request.
        m_dlgfind = new CDialogFind(GetDlgItem(IDC_EDIT));
        m_dlgfind->Create(m_hWnd);
    }

    m_dlgfind->ShowWindow(SW_SHOW);
    m_dlgfind->SetFocus();
}

void CDialogConf::OnFinalMessage( HWND hWnd )
{
	modeless_dialog_manager::g_remove(m_hWnd);
	(*m_self) = NULL;
	delete this;
}

void CDialogConf::OnMarginClick( int position, int modifiers )
{
	int lineClick = m_editorctrl.LineFromPosition(position);
	m_editorctrl.ToggleFold(lineClick);
}

VOID CDialogConf::OnContextMenu( CWindow wnd, CPoint point )
{
	enum
	{
		kCmdUndo = 1 ,
		kCmdRedo ,
		kCmdCut ,
		kCmdCopy ,
		kCmdPaste ,
		kCmdDelete ,
		kCmdSelectAll ,
		kCmdOutliningToggle ,
		kCmdOutliningToggleAll ,

	};

	int cmd = 0;
	const int flag_normal = MF_STRING;
	const int flag_separator = MF_SEPARATOR;
	const int flag_disable = flag_normal | MF_GRAYED | MF_DISABLED;
	const int flag_submenu = flag_normal | MF_POPUP;

	CMenu menu;
	menu.CreatePopupMenu();

	bool writable = !m_editorctrl.GetReadOnly();
	AppendMenu(menu,(writable && m_editorctrl.CanUndo() ? flag_normal : flag_disable), kCmdUndo, _T("Undo"));
	AppendMenu(menu,(writable && m_editorctrl.CanRedo() ? flag_normal : flag_disable), kCmdRedo, _T("Redo"));
	AppendMenu(menu,flag_separator,NULL,NULL);
	AppendMenu(menu,(writable && !m_editorctrl.GetSelectionEmpty() ? flag_normal : flag_disable),kCmdCut,_T("Cut"));
	AppendMenu(menu,(!m_editorctrl.GetSelectionEmpty() ? flag_normal : flag_disable),kCmdCopy,_T("Copy"));
	AppendMenu(menu,(writable && m_editorctrl.CanPaste() ? flag_normal : flag_disable),kCmdPaste,_T("Paste"));
	AppendMenu(menu,(writable && !m_editorctrl.GetSelectionEmpty() ? flag_normal : flag_disable),kCmdDelete,_T("Delete"));
	AppendMenu(menu,flag_separator,NULL,NULL);
	AppendMenu(menu,flag_normal,kCmdSelectAll,_T("Select All"));
	AppendMenu(menu,flag_separator,NULL,NULL);
	CMenu outline_menu;
	outline_menu.CreateMenu();
	AppendMenu(outline_menu,flag_normal,kCmdOutliningToggle,_T("Toggle Outlining Expansion"));
	AppendMenu(outline_menu,flag_normal,kCmdOutliningToggleAll,_T("Toggle All Outlining"));
	AppendMenu(menu,flag_submenu,(UINT_PTR)outline_menu.m_hMenu,_T("Outlining"));



	cmd = menu.TrackPopupMenu(TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,point.x,point.y,m_hWnd);

	switch(cmd){
	case kCmdUndo:
		m_editorctrl.Undo();
		break;
	case kCmdRedo:
		m_editorctrl.Redo();
		break;
	case kCmdCut:
		m_editorctrl.Cut();
		break;
	case kCmdCopy:
		m_editorctrl.Copy();
		break;
	case kCmdPaste:
		m_editorctrl.Paste();
		break;
	case kCmdDelete:
		m_editorctrl.Clear();
		break;
	case kCmdSelectAll:
		m_editorctrl.SelectAll();
		break;
	case kCmdOutliningToggle:
		{
			CPoint temp(point);
			::ScreenToClient(m_editorctrl,&temp);
			OnMarginClick(m_editorctrl.PositionFromPoint(temp.x,temp.y),0);
		}
		break;
	case kCmdOutliningToggleAll:
		m_editorctrl.FoldAll(SC_FOLDACTION_TOGGLE);
		break;
	}

}

