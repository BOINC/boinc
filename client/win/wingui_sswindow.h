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

#ifndef __WIN_SSWINDOW_H_
#define __WIN_SSWINDOW_H_

// includes

#include "wingui.h"
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library

// constants

#define APP_TIMER			104			// app check
#define APP_WAIT			100			// timeout sleep time (in milliseconds)
#define PAINT_TIMER			105			// paint timer
#define PAINT_WAIT			100			// timeout sleep time (in milliseconds)

//////////
// class:		CSSWindow
// parent:		CWnd
// description:	window for the default boinc screen saver
class CSSWindow : public CWnd
{
public:
							CSSWindow();
	void					ShowSSWindow(bool);

private:
	CPoint					m_MousePos;

	UINT					m_uPaintTimerID;		// ID of current app timer	

	CRect					m_AppRect;
	int						m_AppMode;

	HICON					m_hBOINCIcon;
	int						m_nPosX, m_nPosY;
	int						m_nTextPosX, m_nTextPosY;
	int						m_nDX, m_nDY;

	LRESULT					DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	RECT					oldWinRect, oldTextRect;

    afx_msg int				OnCreate(LPCREATESTRUCT);
    afx_msg void			OnDestroy();
    afx_msg void			OnClose();
    afx_msg void			OnPaint();
    afx_msg void			OnTimer(UINT);

    DECLARE_MESSAGE_MAP()
};

#endif
