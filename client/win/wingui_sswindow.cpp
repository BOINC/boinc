// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#include "wingui_sswindow.h"

/////////////////////////////////////////////////////////////////////////
// CMainWindow message map and member functions

BEGIN_MESSAGE_MAP(CSSWindow, CWnd)
    ON_WM_CLOSE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_PAINT()
    ON_WM_TIMER()
END_MESSAGE_MAP()

//////////
// CSSWindow::CSSWindow
// arguments:	void
// returns:		void
// function:	sets initial rect for window and refresh timer
CSSWindow::CSSWindow()
{
	int nX = rand() % 50;
	int nY = rand() % 50;
	m_Rect.SetRect(0+nX,0+nY,640+nX,480+nY);
}

// CMainWindow::ChangeMode
// arguments:	nMode: the new mode
// returns:		void
// function:	destroys the current window and creates a new window
//				in the new mode
void CSSWindow::SetMode(int nMode)
{
	RECT WindowRect = {0,0,0,0};
	m_nPrevMode = m_nMode;
	m_nMode = nMode;

	if(GetSafeHwnd()) {
		if(m_nPrevMode != MODE_FULLSCREEN) GetWindowRect(&m_Rect);
		DestroyWindow();
	}

	CString strWndClass = AfxRegisterWndClass(0);
	DWORD dwExStyle;
	DWORD dwStyle;

	if (nMode == MODE_FULLSCREEN) {
		HDC screenDC=::GetDC(NULL);
		WindowRect.left = WindowRect.top = 0;
		WindowRect.right=GetDeviceCaps(screenDC, HORZRES);
		WindowRect.bottom=GetDeviceCaps(screenDC, VERTRES);
		::ReleaseDC(NULL, screenDC);
		dwExStyle=WS_EX_TOPMOST;
		dwStyle=WS_POPUP;
		while(ShowCursor(false) >= 0);
	} else {
		if(m_Rect.IsRectEmpty()) m_Rect.SetRect(CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT);
		WindowRect = m_Rect;
		dwExStyle=WS_EX_APPWINDOW|WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
		while(ShowCursor(true) < 0);
	}

	CreateEx(dwExStyle, strWndClass, "BOINC Graphics",
		dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, WindowRect,
		NULL, 0, NULL);
	SetTimer(1, 100, NULL);

	if(nMode == MODE_FULLSCREEN || nMode == MODE_WINDOW) {
		ShowWindow(SW_SHOW);
		if(nMode == MODE_FULLSCREEN) SetForegroundWindow();
	} else {
		ShowWindow(SW_HIDE);
	}
	SetFocus();
}

// CMainWindow::GetMode
// arguments:	void
// returns:		the current mode of the window
// function:	gets the current mode of the window
int CSSWindow::GetMode()
{
	return m_nMode;
}

void CSSWindow::PaintDefault()
{
	PAINTSTRUCT ps;
	CDC* pdc;
	RECT winRect;
	CBrush cb;
	cb.CreateSolidBrush(RGB(0,0,0));

	pdc = BeginPaint(&ps);
	GetClientRect(&winRect);
	pdc->FillRect(&winRect, &cb);
	EndPaint(&ps);
}

// CMainWindow::DefWindowProc
// arguments:	message: message received
//				wParam: message's wparam
//				lParam: message's lparam
// returns:		dependent on message
// function:	handles messages as a screensaver would
LRESULT CSSWindow::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CPoint mousePos;
	switch(message) {
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			if(m_nMode == MODE_FULLSCREEN) SetMode(m_nPrevMode);
			return 0;
		case WM_MOUSEMOVE:
			if(m_nMode == MODE_FULLSCREEN) {
				GetCursorPos(&mousePos);
				if(mousePos != m_MousePos) SetMode(m_nPrevMode);
			}
			return 0;
	}
	return CWnd::DefWindowProc(message, wParam, lParam);
}

//////////
// CSSWindow::OnClose
// arguments:	void
// returns:		void
// function:	hides the window
void CSSWindow::OnClose()
{
	SetMode(MODE_NO_GRAPHICS);
}

//////////
// CSSWindow::OnCreate
// arguments:	lpcs: a pointer to the create structure
// returns:		0 if successful, otherwise -1
// function:	gets mouse position for screensaver and starts timer
int CSSWindow::OnCreate(LPCREATESTRUCT lpcs)
{
    if (CWnd::OnCreate(lpcs) == -1) {
		return -1;
	}

	GetCursorPos(&m_MousePos);
	SetTimer(1, 100, NULL);
    return 0;
}

//////////
// CSSWindow::OnDestroy
// arguments:	void
// returns:		void
// function:	kills timer
void CSSWindow::OnDestroy()
{
	KillTimer(1);
}

//////////
// CSSWindow::OnPaint
// arguments:	null
// returns:		null
// function:	for now, clears the window
void CSSWindow::OnPaint()
{
	PAINTSTRUCT ps;
	CDC* pdc;
	RECT winRect;
	CBrush cb;
	cb.CreateSolidBrush(RGB(0,0,0));

	pdc = BeginPaint(&ps);
	GetClientRect(&winRect);
	pdc->FillRect(&winRect, &cb);
	EndPaint(&ps);
}

//////////
// CSSWindow::OnTimer
// arguments:	null
// returns:		null
// function:	try to draw app graphics, otherwise draw default
void CSSWindow::OnTimer()
{
	if(m_nMode == MODE_NO_GRAPHICS) return;
	if(gstate.active_tasks.active_tasks.size() != 0) {
		UINT uPaintMsg = RegisterWindowMessage("BOINC_PAINT");
		CWnd* pAppWnd = GetWndFromProcId(gstate.active_tasks.active_tasks[0]->pid);
		if(pAppWnd) {
			if(!pAppWnd->SendMessage(uPaintMsg, 0, (LPARAM)GetSafeHwnd())) {
				PaintDefault();
			}
		} else {
			PaintDefault();
		}
	} else {
		PaintDefault();
	}
}
