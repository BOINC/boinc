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

#ifndef __WIN_SSWINDOW_H_
#define __WIN_SSWINDOW_H_

// includes

#include "wingui.h"
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library

//////////
// class:		CSSWindow
// parent:		CWnd
// description:	window for the default boinc screen saver
class CSSWindow : public CWnd
{
public:
							CSSWindow();
	void					SetMode(int, int);
	int						GetMode();
	int						GetPrevMode();
	bool					BlankScreen();

private:
	int						m_nMode;
	int						m_nPrevMode;
	CPoint					m_MousePos;
	CRect					m_Rect;
	UINT					m_uSetMsg;
	UINT					m_uGetMsg;
	unsigned long			m_bBlankScreen;
	unsigned long			m_uBlankTime;

	HICON					m_hBOINCIcon;
	int						m_nPosX, m_nPosY;
	int						m_nDX, m_nDY;

	HDC hdc;
	HGLRC hrc;


	void					PaintDefault();
	LRESULT					DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    afx_msg int				OnCreate(LPCREATESTRUCT);
    afx_msg void			OnDestroy();
    afx_msg void			OnClose();
    afx_msg void			OnPaint();
    afx_msg void			OnTimer();

    DECLARE_MESSAGE_MAP()
};

#endif
