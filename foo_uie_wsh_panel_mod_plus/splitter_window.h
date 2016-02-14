#pragma once

class wsh_splitter_window : public CHorSplitterWindow
{
public:
	DECLARE_WND_CLASS(_T("WSH_SplitterWindow"));

public:
	//create from a holder window,and destroy it .
	HWND Create2(HWND hWnd, UINT uHolderID, DWORD dwStyle, DWORD dwExStyle)
	{
		ATLASSERT(::IsWindow(hWnd));
		
		RECT rcHolder;
		HWND hHolderWnd = ::GetDlgItem(hWnd,uHolderID);

		ATLASSERT(::IsWindow(hHolderWnd));

		::GetWindowRect(hHolderWnd,&rcHolder);
		::ScreenToClient(hWnd,(LPPOINT)&rcHolder);
		::ScreenToClient(hWnd,(LPPOINT)&rcHolder + 1);

		HWND Wnd = NULL;

		Wnd = Create(hWnd,rcHolder,NULL,dwStyle,dwExStyle,uHolderID);

		if(Wnd)::DestroyWindow(hHolderWnd);

		return Wnd;
	}
};