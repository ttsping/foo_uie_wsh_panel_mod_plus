#pragma once
#include <string>

typedef TCHAR tchar;
#ifdef UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

class input_box
{
public:
	input_box(HWND hwnd, const tchar* title, const tchar* prompt, const tchar* defval, int num_only):
	  m_hwnd(hwnd)
	, m_title(title)
	, m_prompt(prompt)
	, m_defval(defval)
	{
		unsigned int s = WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL;
		if (num_only) s |= ES_NUMBER;
		m_editstyle = s;
		RtlZeroMemory((void*)m_buff,4096);
	}
    tchar* show();
	int CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
private:
	HWND create_window(HINSTANCE hInst);
	int run(HWND hwnd);
	static int CALLBACK g_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
private:
	HINSTANCE m_hinst;
	HWND m_hwnd;
	HWND m_hedit;
	HWND m_htext;
	HWND m_hbtnok, m_hbtncancel;
	UINT m_editstyle;
	tstring m_title;
	tstring m_prompt;
	tstring m_defval;
	tchar m_buff[2048];
};