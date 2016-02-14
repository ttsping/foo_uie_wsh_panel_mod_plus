#include "stdafx.h"
#include "ui_conf.h"
#include "wsh_panel_window.h"
#include "ui_goto.h"
#include "ui_find.h"
#include "ui_replace.h"
#include "component_defines.h"
#include "helpers.h"
#include "panel_manager.h"

using namespace pfc::stringcvt;

namespace{
	static service_factory_single_t<CDialogConf::wsh_console_receiver_impl> wsh_console_receiver;
}

LRESULT CDialogConf::OnInitDialog(HWND hwndFocus, LPARAM lParam)
{
	
	modeless_dialog_manager::g_add(m_hWnd);
	panel_manager::instance().add_window(m_hWnd);
	// Set Icon
	HICON icon = static_api_ptr_t<ui_control>()->get_main_icon();
	SetIcon(icon, FALSE);

	// Save caption text
	pfc::string8 lang_text;
	m_caption << load_lang(IDS_WSHM_NAME, lang_text) << " " << load_lang(IDS_UI_CONF, lang_text);
	//Create Split Window
	const DWORD ctrl_style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	const DWORD ctrl_exstyle = WS_EX_STATICEDGE /*WS_EX_CLIENTEDGE*/;

	m_split_wnd.Create2(m_hWnd,IDC_STATIC_HOLDER,ctrl_style,ctrl_exstyle);
	m_split_wnd.m_cxySplitBar = 2;
	CWindow edit_pane,console_pane;
	edit_pane.Create(_T("WshScintilla"),m_split_wnd,NULL,NULL,ctrl_style);
	console_pane.Create(_T("WshScintilla"),m_split_wnd,NULL,NULL,ctrl_style,IDC_CONSOLE_PRINT);
	m_split_wnd.SetSplitterPanes(edit_pane,console_pane);

	CRect split_rect;
	m_split_wnd.GetWindowRect(split_rect);
	m_split_wnd.SetSplitterPos(split_rect.Height() * 4 / 5);
	m_split_wnd.SetSinglePaneMode(SPLIT_PANE_TOP);

	// Init resize
	DlgResize_Init();

	// Apply window placement
	if (m_parent->get_windowplacement().length == 0)
	{
		m_parent->get_windowplacement().length = sizeof(WINDOWPLACEMENT);
		CenterWindow(HWND_DESKTOP);
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

	pfc::string8 lang_style;
	uSendMessageText(combo_edge_style, CB_ADDSTRING, 0, load_lang(IDS_EDGE_STYLE_NONE, lang_style));
	uSendMessageText(combo_edge_style, CB_ADDSTRING, 0, load_lang(IDS_EDGE_STYLE_SUNKEN, lang_style));
	uSendMessageText(combo_edge_style, CB_ADDSTRING, 0, load_lang(IDS_EDGE_STYLE_GREY, lang_style));
	ComboBox_SetCurSel(combo_edge_style, m_parent->get_edge_style());

	

	// Edit box
	pfc::string8_fast text;
	int cursel = ComboBox_GetCurSel(combo_script_engine);
	
	// Subclassing scintilla
	m_editorctrl.SubclassWindow(edit_pane);

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

	// Subclass console ctrl
	m_consolectrl.SubclassWindow(console_pane);
	
	m_consolectrl.SetLanguage("");

	m_consolectrl.SetReadOnly(true);

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
			pfc::string8 lang_prompt;
			int ret = uMessageBox(m_hWnd, load_lang(IDS_SCRIPT_CHANGE_TIP, lang_prompt), m_caption, MB_ICONWARNING | MB_SETFOREGROUND | MB_YESNOCANCEL);

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
	//HWND edit = GetDlgItem(IDC_EDIT);
	HWND combo = GetDlgItem(IDC_SCRIPT_ENGINE);
	int cursel = ComboBox_GetCurSel(combo);

	if (uComboBox_GetText(combo, cursel, text))
	{
		m_editorctrl.SetLanguage(text);
		m_editorctrl.SendMessage(OCM_COMMAND,MAKEWPARAM(m_editorctrl.GetDlgCtrlID(),SCEN_CHANGE));
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
	pfc::string8 filename, lang_filetype, lang_import;

	if (uGetOpenFileName(m_hWnd, load_lang(IDS_IMPORT_FILE_TYPE, lang_filetype), 0, "txt", load_lang(IDS_IMPORT_IMPORT, lang_import), NULL, filename, FALSE))
	{
		// Open file
		pfc::string8_fast text;

		helpers::read_file(filename, text);
		m_editorctrl.SetContent(text);
	}
}

void CDialogConf::OnExport()
{
	pfc::string8 filename, lang_filetype, lang_saveas;

	if (uGetOpenFileName(m_hWnd, load_lang(IDS_IMPORT_FILE_TYPE, lang_filetype), 0, "txt", load_lang(IDS_IMPORT_SAVE_AS, lang_saveas), NULL, filename, TRUE))
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

	if (uButton_GetCheck(m_hWnd,IDC_SHOW_CONSOLE_PANE)){
		m_consolectrl.SetReadOnly(false);
		m_consolectrl.ClearAll();
		m_consolectrl.SetReadOnly(true);
		return 0;
	}

	HMENU menu = CreatePopupMenu();
	int ret = 0;
	int flags = TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD;
	RECT rc = {0};
	pfc::string8 lang_menu;
	uAppendMenu(menu, MF_STRING, kImport, load_lang(IDS_TOOL_MENU_IMPORT, lang_menu));
	uAppendMenu(menu, MF_STRING, kExport, load_lang(IDS_TOOL_MENU_EXPORT, lang_menu));
	uAppendMenu(menu, MF_SEPARATOR, 0, 0);
	uAppendMenu(menu, MF_STRING, kResetDefault, load_lang(IDS_TOOL_MENU_RESET_DEFAULT, lang_menu));
	uAppendMenu(menu, MF_STRING, kResetCurrent, load_lang(IDS_TOOL_MENU_RESET_CURRENT, lang_menu));

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

	if (pnmh->hwndFrom == m_editorctrl.m_hWnd){

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
					m_dlgreplace = new CDialogReplace(m_editorctrl/*GetDlgItem(IDC_EDIT)*/);
					
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
				CDialogGoto dlg(m_editorctrl);
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

LRESULT CDialogConf::OnUwmConsolePrint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	const char * p_msg = reinterpret_cast<const char*>(wParam);
	unsigned p_len = static_cast<unsigned>(lParam);
	if(p_msg && p_len){
		pfc::string8_fast temp;
		int len = m_consolectrl.GetTextLength();
		if (len)temp << "\n";
		temp << p_msg;
		m_consolectrl.SetReadOnly(false);
		m_consolectrl.AppendText(temp,temp.get_length());
		m_consolectrl.SetReadOnly(true);
		m_consolectrl.ScrollToLine(m_consolectrl.GetLineCount() - 1);
	} 

	return TRUE;
}

bool CDialogConf::FindNext(HWND hWnd, HWND hWndEdit, unsigned flags, const char *which)
{
    ::SendMessage(::GetAncestor(hWndEdit, GA_ROOT), UWM_FINDTEXTCHANGED, flags, reinterpret_cast<LPARAM>(which));

    SendMessage(hWndEdit, SCI_CHARRIGHT, 0, 0);
    SendMessage(hWndEdit, SCI_SEARCHANCHOR, 0, 0);
    int pos = ::SendMessage(hWndEdit, SCI_SEARCHNEXT, flags, reinterpret_cast<LPARAM>(which));
    return FindResult(hWnd, hWndEdit, pos, which);
}

bool CDialogConf::FindPrevious(HWND hWnd, HWND hWndEdit, unsigned flags, const char *which)
{
    ::SendMessage(::GetAncestor(hWndEdit, GA_ROOT), UWM_FINDTEXTCHANGED, flags, reinterpret_cast<LPARAM>(which));

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

    pfc::string8 buff;
	load_lang(IDS_MSG_CANNT_FIND, buff);
	buff += " \"";
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
        m_dlgfind = new CDialogFind(m_editorctrl/*GetDlgItem(IDC_EDIT)*/);
        m_dlgfind->Create(m_hWnd);
    }

    m_dlgfind->ShowWindow(SW_SHOW);
    m_dlgfind->SetFocus();
}

void CDialogConf::OnFinalMessage( HWND hWnd )
{
	modeless_dialog_manager::g_remove(m_hWnd);
	panel_manager::instance().remove_window(m_hWnd);
	(*m_self) = NULL;
	delete this;
}

void CDialogConf::OnMarginClick( int position, int modifiers )
{
	int lineClick = m_editorctrl.LineFromPosition(position);
	m_editorctrl.ToggleFold(lineClick);
}

void CDialogConf::OnShowConsolePane( UINT uNotifyCode, int nID, CWindow wndCtl )
{
	bool show_pane = uButton_GetCheck(m_hWnd,IDC_SHOW_CONSOLE_PANE);
	m_split_wnd.SetSinglePaneMode( show_pane ? SPLIT_PANE_NONE : SPLIT_PANE_TOP);
	pfc::string8 lang_caption;
	uSetWindowText(GetDlgItem(IDC_TOOLS), show_pane ? load_lang(IDS_CAPTION_CLEAR, lang_caption) : load_lang(IDS_CAPTION_TOOL, lang_caption));
}




void CDialogConf::wsh_console_receiver_impl::print( const char * p_message, unsigned p_message_length )
{
	panel_manager::instance().send_msg_to_all(UWM_CONSOLE_PRINT,(WPARAM)p_message,(LPARAM)p_message_length);
}
