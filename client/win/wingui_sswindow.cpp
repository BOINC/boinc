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
#include "parse.h"

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
CSSWindow::CSSWindow(char *ss_shm)
{
	// Use a 640x480 window randomly positioned at (0-49,0-49)
	int nX = rand() % 50;
	int nY = rand() % 50;
	m_Rect.SetRect(0+nX,0+nY,640+nX,480+nY);
	// Start off in no graphics mode
	SetMode(MODE_HIDE_GRAPHICS);

	m_hBOINCIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON));
	m_bCleared = false;

	boinc_screensaver_shm = new APP_CLIENT_SHM;
	boinc_screensaver_shm->shm = ss_shm;
}

// CMainWindow::SetMode
// arguments:	nMode: the new mode
// returns:		void
// function:	destroys the current window and creates a new window
//				in the new mode
void CSSWindow::SetMode(int nMode)
{
	RECT WindowRect = {0,0,0,0};

	if(GetSafeHwnd()) {
		if(m_nMode != MODE_FULLSCREEN)
			GetWindowRect(&m_Rect);
		DestroyWindow();
	}
	m_nMode = nMode;

	CString strWndClass = AfxRegisterWndClass(0);
	DWORD dwExStyle;
	DWORD dwStyle;

	if (nMode == MODE_FULLSCREEN || nMode == MODE_BLANKSCREEN) {
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

	CreateEx(dwExStyle, strWndClass, "DEFAULT BOINC Graphics",
		dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, WindowRect,
		NULL, 0, NULL);

	if(nMode == MODE_HIDE_GRAPHICS) {
		ShowWindow(SW_HIDE);
	} else {
		ShowWindow(SW_SHOW);
		SetFocus();
	}
	//boinc_screensaver_shm->send_msg(xml_graphics_modes[nMode], APP_CORE_GFX_SEG);
}

//////////
// CSSWindow::CheckAppMsg
// arguments:	void
// returns:		void
// function:	polls application message queues to see if the currently shown
//				window needs to be switched, or if an app has closed
void CSSWindow::CheckMsgQueue()
{
	char msg_buf[SHM_SEG_SIZE];

	if (boinc_screensaver_shm->get_msg(msg_buf,CORE_APP_GFX_SEG)) {
		if (match_tag(msg_buf, xml_graphics_modes[MODE_HIDE_GRAPHICS])) {
			SetMode(MODE_HIDE_GRAPHICS);
		} else if (match_tag(msg_buf, xml_graphics_modes[MODE_WINDOW])) {
			SetMode(MODE_WINDOW);
		} else if (match_tag(msg_buf, xml_graphics_modes[MODE_FULLSCREEN])) {
			SetMode(MODE_FULLSCREEN);
		} else if (match_tag(msg_buf, xml_graphics_modes[MODE_BLANKSCREEN])) {
			SetMode(MODE_BLANKSCREEN);
		}
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
			if(m_nMode == MODE_FULLSCREEN || m_nMode == MODE_BLANKSCREEN)
				SetMode(MODE_HIDE_GRAPHICS);
			return 0;
		case WM_MOUSEMOVE:
			if(m_nMode == MODE_FULLSCREEN || m_nMode == MODE_BLANKSCREEN) {
				GetCursorPos(&mousePos);
				if(mousePos.x != m_MousePos.x && mousePos.y != m_MousePos.y)
					SetMode(MODE_HIDE_GRAPHICS);
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
	SetMode(MODE_HIDE_GRAPHICS);
}

//////////
// CSSWindow::OnCreate
// arguments:	lpcs: a pointer to the create structure
// returns:		0 if successful, otherwise -1
// function:	gets mouse position for screensaver and starts timer
int CSSWindow::OnCreate(LPCREATESTRUCT lpcs)
{
    if (CWnd::OnCreate(lpcs)) {
		return -1;
	}

	GetCursorPos(&m_MousePos);
	// Set up the timer for painting this window
	m_uPaintTimerID = SetTimer(PAINT_TIMER, PAINT_WAIT, (TIMERPROC) NULL);
	// Set up the timer for checking the status of app graphics
	m_uAppTimerID = SetTimer(APP_TIMER, APP_WAIT, (TIMERPROC) NULL);

	// Initial position is (0,0)
	m_nPosX = m_nPosY = 0;
	// Initial velocity is (1-5,1-5)
	m_nDX = (rand() % 5)+1;
	m_nDY = (rand() % 5)+1;

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

	// Fill the window with black
	pdc = BeginPaint(&ps);
	GetClientRect(&winRect);
	pdc->FillSolidRect(&winRect, RGB(0,0,0));

	// Draw the bouncing BOINC icon if we're not in blank screen mode
	if ((m_nMode != MODE_HIDE_GRAPHICS) && (m_nMode != MODE_BLANKSCREEN)) {
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
// CSSWindow::OnTimer
// arguments:	uEventID: id of timer signaling event
// returns:		null
// function:	redraw the window if needed
void CSSWindow::OnTimer(UINT uEventID)
{
	// Check the apps for any new messages
	if(uEventID == m_uAppTimerID) {
		CheckMsgQueue();
	}

	// Paint our own window
	if(uEventID == m_uPaintTimerID) {
		if((m_nMode != MODE_BLANKSCREEN) || !m_bCleared) {
			if(m_nMode == MODE_BLANKSCREEN) m_bCleared = true;
			Invalidate();
			OnPaint();
		}
	}
}