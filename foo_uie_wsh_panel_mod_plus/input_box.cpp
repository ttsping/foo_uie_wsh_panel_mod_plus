#include "stdafx.h"
#include "input_box.h"


HWND input_box::create_window( HINSTANCE hInst )
{
	WNDCLASSEX wc = {0};
	HWND hwnd;
	wc.cbSize        = sizeof(wc);
	wc.hInstance     = hInst;
	wc.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	//wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hIcon         = static_api_ptr_t<ui_control>()->get_main_icon();
	wc.hIconSm       = wc.hIcon;
	wc.lpfnWndProc   = (WNDPROC)&g_wnd_proc;
	wc.lpszClassName = _T("wsh_plus_inputbox_class");
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	::RegisterClassEx(&wc);
	RECT rc;
	::GetWindowRect(m_hwnd,&rc);
	int cx = (rc.right + rc.left - 365)/2;
	int cy = (rc.bottom + rc.top - 130)/2;
	hwnd = ::CreateWindowEx(0,
		_T("wsh_plus_inputbox_class"),
		m_title.c_str(),
		WS_DLGFRAME | WS_SYSMENU | WS_VISIBLE,
		cx,
		cy,
		365,
		130,
		m_hwnd,
		0,
		hInst,
		this);
	return hwnd;
}

int input_box::run( HWND hwnd )
{
	MSG msg;
	if (!hwnd)
		return 0;
	::ShowWindow(hwnd,SW_SHOW);
	::UpdateWindow(hwnd);
	while(::GetMessage(&msg,0,0,0))
	{
		if (msg.message == WM_KEYDOWN)
		{
			if (msg.wParam == VK_RETURN)
				::SendMessage(hwnd,msg.message,msg.wParam,msg.wParam);
		}
		::TranslateMessage (&msg) ;
		::DispatchMessage (&msg) ;
	}
	return msg.wParam;
}

int CALLBACK input_box::g_wnd_proc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	input_box * p_this;
	if (msg == WM_NCCREATE) {
		LPCREATESTRUCT cs = reinterpret_cast<LPCREATESTRUCT>(lp);
		p_this = reinterpret_cast<input_box *>(cs->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)p_this);
	} else {
		p_this = reinterpret_cast<input_box *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	int ret = FALSE;
	if (p_this) {
		ret = p_this->wnd_proc(hwnd, msg, wp, lp);
	}
	if (!ret) {
		ret = DefWindowProc(hwnd, msg, wp, lp);
	}

	return ret;
}

tchar* input_box::show()
{
	//m_hinst = ::GetModuleHandle(NULL);
	m_hinst = core_api::get_my_instance();
	run(create_window(m_hinst));
	return m_buff;
}

int CALLBACK input_box::wnd_proc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	WORD uid;
	int ret = TRUE;
	switch(msg)
	{
	case WM_DESTROY:
		{
			::PostQuitMessage(0);
		}
		break;
	case WM_CREATE:
		{
			m_hedit = ::CreateWindowEx(0,_T("Static"),m_prompt.c_str(),WS_CHILD | WS_VISIBLE,5,5,275,70,hwnd,(HMENU)1000,m_hinst,0);
			m_hbtnok = ::CreateWindowEx(0,_T("Button"),_T("OK(&O)"),WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,285,5,65,20,hwnd,(HMENU)IDOK,m_hinst,0);
			m_hbtncancel = ::CreateWindowEx(0,_T("Button"),_T("Cancel(&C)"),WS_CHILD | WS_VISIBLE,285,30,65,20,hwnd,(HMENU)IDCANCEL,m_hinst,0);
			m_hedit = ::CreateWindowEx(WS_EX_CLIENTEDGE,_T("Edit"),m_defval.c_str(),m_editstyle,5,80,350,20,hwnd,(HMENU)2000,m_hinst,0);
			
			HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			::DeleteObject(hfont);
			::SendDlgItemMessage(hwnd,1000,WM_SETFONT,(WPARAM)hfont,0);
			::SendDlgItemMessage(hwnd,2000,WM_SETFONT,(WPARAM)hfont,0);
			::SendDlgItemMessage(hwnd,IDOK,WM_SETFONT,(WPARAM)hfont,0);
			::SendDlgItemMessage(hwnd,IDCANCEL,WM_SETFONT,(WPARAM)hfont,0);
			::DeleteObject(hfont);
		}
		break;
	case WM_KEYDOWN:
		if (wp == VK_RETURN)
			::SendMessage(hwnd,WM_COMMAND,IDOK,0);
		break;
	case WM_SETFOCUS:
		::SetFocus(m_hedit);
		break;
	case WM_COMMAND:
		uid = LOWORD(wp);
		switch(uid)
		{
		case IDOK:
			::GetDlgItemText(hwnd,2000,m_buff,2048);
		case IDCANCEL:
			::DestroyWindow(hwnd);
			::UnregisterClass(_T("wsh_plus_inputbox_class"),m_hinst);
			break;
		};
	default:
		ret = FALSE;
		break;
	}
	return ret;
}
