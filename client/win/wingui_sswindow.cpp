// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "boinc_win.h"

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
CSSWindow::CSSWindow()
{
	// Use a 640x480 window randomly positioned at (0-49,0-49)
	int nX = rand() % 50;
	int nY = rand() % 50;
	// Start off in no graphics mode
	ShowSSWindow(false);

	m_hBOINCIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON));
}

// CMainWindow::SetMode
// arguments:	nMode: the new mode
// returns:		void
// function:	destroys the current window and creates a new window
//				in the new mode
void CSSWindow::ShowSSWindow(bool show_win)
{
	RECT WindowRect = {0,0,0,0};
	PAINTSTRUCT ps;
	CDC* pdc;
	RECT winRect;

	if (!show_win) {
		// If there's a window, destroy it
		if (GetSafeHwnd()) DestroyWindow();
		// Make sure the cursor is visible again
		while(ShowCursor(true) < 0);
		return;
	}

	CString strWndClass = AfxRegisterWndClass(0);
	DWORD dwExStyle;
	DWORD dwStyle;

	// Get the dimensions of the screen
	HDC screenDC=::GetDC(NULL);
	WindowRect.left = WindowRect.top = 0;
	WindowRect.right=GetDeviceCaps(screenDC, HORZRES);
	WindowRect.bottom=GetDeviceCaps(screenDC, VERTRES);
	::ReleaseDC(NULL, screenDC);
	dwExStyle=WS_EX_TOPMOST;
	dwStyle=WS_POPUP;
	// Hide the cursor
	while(ShowCursor(false) >= 0);

	CreateEx(dwExStyle, strWndClass, "DEFAULT BOINC Graphics",
		dwStyle|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, WindowRect,
		NULL, 0, NULL);

	ShowWindow(SW_SHOW);
	SetFocus();

	// Fill the window with black
	pdc = BeginPaint(&ps);
	pdc->SetBkColor(RGB(0,0,0));
	GetClientRect(&winRect);
	pdc->FillSolidRect(&winRect, RGB(0,0,0));
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
			gstate.ss_logic.stop_ss();
			return 0;
		case WM_MOUSEMOVE:
			GetCursorPos(&mousePos);
			if(mousePos.x != m_MousePos.x && mousePos.y != m_MousePos.y)
				gstate.ss_logic.stop_ss();
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
	ShowSSWindow(false);
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

	// Initial position is (0,0)
	m_nPosX = m_nPosY = 0;
	m_nTextPosX = -10000;
	m_nTextPosY = 0;
	// Initial velocity is (1-5,1-5)
	m_nDX = (rand() % 5)+1;
	m_nDY = (rand() % 5)+1;
	SetRect(&oldWinRect, 0, 0, 0, 0);
	SetRect(&oldTextRect, 0, 0, 0, 0);

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
	RECT winRect, textRect;

	// Fill the window with black
	pdc = BeginPaint(&ps);
	pdc->SetBkColor(RGB(0,0,0));
	GetClientRect(&winRect);

	if (gstate.ss_logic.do_blank) {
		pdc->FillSolidRect(&winRect, RGB(0,0,0));
	} else 
	// Draw the bouncing BOINC icon if we're not in blank screen mode
	if (gstate.ss_logic.do_boinc_logo_ss) {
		pdc->FillSolidRect(&oldTextRect, RGB(0,0,0));
		pdc->SetTextColor(RGB(255,255,255));
		// Draw status text
		SetRect(&textRect, m_nTextPosX, m_nTextPosY, m_nTextPosX, m_nTextPosY);
		pdc->DrawText(gstate.ss_logic.ss_msg, &textRect, DT_CALCRECT);
		m_nTextPosX += 2;
		if (m_nTextPosX + (textRect.right-textRect.left) < 0)
			m_nTextPosX = textRect.left - textRect.right;
		pdc->DrawText(gstate.ss_logic.ss_msg, &textRect, DT_LEFT);
		if (m_nTextPosX > winRect.right) {
			m_nTextPosX = -10000;
			m_nTextPosY = rand() % (winRect.bottom-winRect.top);
		}
		CopyRect(&oldTextRect, &textRect);

		// Draw the bouncing icon
		m_nPosX += m_nDX;
		m_nPosY += m_nDY;
		if (m_nPosX <= winRect.left || (m_nPosX+32) >= winRect.right) m_nDX *= -1;
		if (m_nPosY <= winRect.top || (m_nPosY+32) >= winRect.bottom) m_nDY *= -1;
		if (m_nPosX < winRect.left) m_nPosX = winRect.left;
		if ((m_nPosX+32) > winRect.right) m_nPosX = winRect.right-32;
		if (m_nPosY < winRect.top) m_nPosY = winRect.top;
		if ((m_nPosY+32) > winRect.bottom) m_nPosY = winRect.bottom-32;
		pdc->FillSolidRect(&oldWinRect, RGB(0,0,0));
		pdc->DrawIcon(m_nPosX, m_nPosY, m_hBOINCIcon);
		SetRect(&oldWinRect, m_nPosX, m_nPosY, m_nPosX+32, m_nPosY+32);
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
	// Paint our own window
	if(uEventID == m_uPaintTimerID &&
			(gstate.ss_logic.do_boinc_logo_ss || gstate.ss_logic.do_blank)) {
		Invalidate();
		OnPaint();
	}
}

const char *BOINC_RCSID_e394ca1c58 = "$Id$";
