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
// function:	sets initial rect for window and other members
CSSWindow::CSSWindow()
{
	int nX = rand() % 50;
	int nY = rand() % 50;
	m_Rect.SetRect(0+nX,0+nY,640+nX,480+nY);
	SetMode(MODE_NO_GRAPHICS);

	m_hBOINCIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON));
	m_uScreenSaverMsg = RegisterWindowMessage(START_SS_MSG);
	m_uSetMsg = RegisterWindowMessage(APP_SET_MSG);
	m_uGetMsg = RegisterWindowMessage(APP_GET_MSG);
	m_uAppModeMsg = RegisterWindowMessage("BOINC_APP_MODE");
	m_dwAppId = 0;
	m_uStartTime = 0;
	m_bCleared = false;
}

// CMainWindow::SetMode
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
		if(m_nPrevMode != MODE_FULLSCREEN)
			GetWindowRect(&m_Rect);
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
		m_uStartTime = 0;
		if(m_Rect.IsRectEmpty()) m_Rect.SetRect(CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT);
		WindowRect = m_Rect;
		dwExStyle=WS_EX_APPWINDOW|WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
		while(ShowCursor(true) < 0);
	}

	CreateEx(dwExStyle, strWndClass, "DEFAULT BOINC Graphics",
		dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, WindowRect,
		NULL, 0, NULL);

	if(nMode == MODE_FULLSCREEN || nMode == MODE_WINDOW) {
		ShowWindow(SW_SHOW);
		SetFocus();
	} else {
		ShowWindow(SW_HIDE);
	}
}

//////////
// CSSWindow::CheckAppWnd
// arguments:	void
// returns:		void
// function:	polls application windows to see if the currently shown
//				window needs to be switched, or if an app has closed
void CSSWindow::CheckAppWnd()
{
	if(m_nMode != MODE_FULLSCREEN) return;

	CWnd* pAppWnd;

	if (m_dwAppId == 0) {
		if(BlankScreen()) return;
		if(gstate.active_tasks.active_tasks.size() == 0) return;
		m_dwAppId = gstate.active_tasks.active_tasks[0]->pid;
		pAppWnd = GetWndFromProcId(m_dwAppId);
		if(pAppWnd) {
			pAppWnd->PostMessage(m_uSetMsg, MODE_FULLSCREEN, 0);
		} else {
			m_dwAppId = 0;
		}
	} else {
		if(BlankScreen()) {
			pAppWnd = GetWndFromProcId(m_dwAppId);
			if(pAppWnd && IsWindow(pAppWnd->m_hWnd)) {
				SetMode(MODE_FULLSCREEN);
				pAppWnd->PostMessage(m_uSetMsg, MODE_NO_GRAPHICS, 0);
			}
			m_dwAppId = 0;
		} else {
			pAppWnd = GetWndFromProcId(m_dwAppId);
			if(!pAppWnd || !IsWindow(pAppWnd->m_hWnd)) {
				SetMode(MODE_FULLSCREEN);
				m_dwAppId = 0;
			}
		}
		return;
	}
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
			if(m_nMode == MODE_FULLSCREEN)
				SetMode(MODE_NO_GRAPHICS);
			return 0;
		case WM_MOUSEMOVE:
			if(m_nMode == MODE_FULLSCREEN) {
				GetCursorPos(&mousePos);
				if(mousePos != m_MousePos) SetMode(MODE_NO_GRAPHICS);
			}
			return 0;
	}

	if(m_uScreenSaverMsg == message) {
		if(gstate.active_tasks.active_tasks.size() == 0) m_dwAppId = 0;
		else m_dwAppId = gstate.active_tasks.active_tasks[0]->pid;
		m_uStartTime = (double)time(0);
		m_bCleared = false;
		SetMode(MODE_FULLSCREEN);
		CWnd* pAppWnd = GetWndFromProcId(m_dwAppId);
		if(pAppWnd && IsWindow(pAppWnd->m_hWnd)) {
			pAppWnd->PostMessage(m_uSetMsg, MODE_FULLSCREEN, 0);
		}
		return 0;
	} else if(m_uAppModeMsg == message) {
		if(lParam == m_dwAppId && wParam != MODE_FULLSCREEN) {
			SetMode(MODE_NO_GRAPHICS);
			m_dwAppId = 0;
			m_uStartTime = 0;
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
	m_uPaintTimerID = SetTimer(PAINT_TIMER, PAINT_WAIT, (TIMERPROC) NULL);
	m_uAppTimerID = SetTimer(APP_TIMER, APP_WAIT, (TIMERPROC) NULL);

	m_nPosX = m_nPosY = 0;
	m_nDX = m_nDY = 5;

	UtilGetRegKey(REG_BLANK_NAME, m_bBlankScreen);
	UtilGetRegKey(REG_BLANK_TIME, m_uBlankTime);
	m_uBlankTime *= 60;

    return 0;
}

//////////
// CSSWindow::OnDestroy
// arguments:	void
// returns:		void
// function:	kills timer
void CSSWindow::OnDestroy()
{
	KillTimer(m_uPaintTimerID);
	KillTimer(m_uAppTimerID);
}

//////////
// CSSWindow::OnPaint
// arguments:	null
// returns:		null
// function:	clears the window and draws the bouncing icon if necessary
void CSSWindow::OnPaint()
{
	PAINTSTRUCT ps;
	CDC* pdc;
	RECT winRect;

	pdc = BeginPaint(&ps);
	GetClientRect(&winRect);
	pdc->FillSolidRect(&winRect, RGB(0,0,0));

	if ((m_nMode == MODE_FULLSCREEN || m_nMode == MODE_WINDOW) && !BlankScreen()) {
		pdc->DrawIcon(m_nPosX, m_nPosY, m_hBOINCIcon);
		m_nPosX += m_nDX;
		m_nPosY += m_nDY;
		if (m_nPosX <= winRect.left || (m_nPosX+32) >= winRect.right) m_nDX *= -1;
		if (m_nPosY <= winRect.top || (m_nPosY+32) >= winRect.bottom) m_nDY *= -1;
		if (m_nPosX < winRect.left) m_nPosX = winRect.left;
		if ((m_nPosX+32) > winRect.right) m_nPosX = winRect.right-32;
		if (m_nPosY < winRect.top) m_nPosY = winRect.top;
		if ((m_nPosY+32) > winRect.bottom) m_nPosY = winRect.bottom-32;
	}

	EndPaint(&ps);
}

//////////
// CSSWindow::BlankScreen
// arguments:	null
// returns:		null
// function:	returns true if we should go into blank screen mode
bool CSSWindow::BlankScreen()
{
	return	(m_bBlankScreen) &&
			(m_nMode == MODE_FULLSCREEN) &&
			(time(0) >= m_uBlankTime + m_uStartTime);
}

//////////
// CSSWindow::OnTimer
// arguments:	uEventID: id of timer signaling event
// returns:		null
// function:	redraw the window if needed
void CSSWindow::OnTimer(UINT uEventID)
{
	if(uEventID == m_uAppTimerID) {
		CheckAppWnd();
	}

	if(uEventID == m_uPaintTimerID) {
		if(!BlankScreen() || !m_bCleared) {
			if(BlankScreen()) m_bCleared = true;
			Invalidate();
			OnPaint();
		}
	}
}