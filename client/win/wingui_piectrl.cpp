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

#include "stdafx.h"

#include "wingui_piectrl.h"

/////////////////////////////////////////////////////////////////////////
// CPieChartCtrl member functions

BEGIN_MESSAGE_MAP(CPieChartCtrl, CWnd)
    ON_WM_PAINT()
END_MESSAGE_MAP()

//////////
// CPieChartCtrl::CPieChartCtrl
// arguments:	void
// returns:		void
// function:	initializes members
CPieChartCtrl::CPieChartCtrl()
{
	m_xTotal = 0;
	m_pFont = NULL;
}

//////////
// CPieChartCtrl::AddPiece
// arguments:	szLabel: label for the piece
//				clr: color of the piece
//				xValue: the initial value for the piece
// returns:		void
// function:	adds a piece to the pie
void CPieChartCtrl::AddPiece(LPTSTR szLabel, COLORREF clr, double xValue)
{
	if(xValue < 0) xValue = 0;
	m_xValues.Add(xValue);
	m_colors.Add(clr);
	CString strLabel;
	strLabel.Format("%s", szLabel);
	m_strLabels.Add(strLabel);
	m_dwData.Add(0);
}

void CPieChartCtrl::RemovePiece(int nItem)
{
	if(nItem >= 0 && nItem < GetItemCount()) {
		m_xValues.RemoveAt(nItem);
		m_colors.RemoveAt(nItem);
		m_strLabels.RemoveAt(nItem);
		m_dwData.RemoveAt(nItem);
	}
}

//////////
// CPieChartCtrl::SetPiece
// arguments:	nIndex: index of piece to change
//				xValue: the new value for the piece
// returns:		void
// function:	changes the piece's value
void CPieChartCtrl::SetPiece(int nIndex, double xValue)
{
	if(nIndex < 0 || nIndex >= GetItemCount()) return;
	if(xValue < 0) xValue = 0;
	m_xValues.SetAt(nIndex, xValue);
}

//////////
// CPieChartCtrl::SetPieceLabel
// arguments:	nIndex: index of label to set
//				szLabel: the new label
// returns:		void
// function:	gives the pie piece at the given index the given label
void CPieChartCtrl::SetPieceLabel(int nIndex, LPTSTR szLabel)
{
	if(nIndex < 0 || nIndex >= GetItemCount()) return;
	CString strLabel;
	strLabel.Format("%s", szLabel);
	m_strLabels.SetAt(nIndex, strLabel);
}

//////////
// CPieChartCtrl::GetItemCount
// arguments:	void
// returns:		the number of pieces in this pie
// function:	finds number of pieces in the pie
int CPieChartCtrl::GetItemCount()
{
	return m_xValues.GetSize();
}

//////////
// CPieChartCtrl::Create
// arguments:	dwStyle: the style of control to create
//				rect: size and position
//				pParentWnd: control's parent window
//				nID: control's id
// returns:		true if successful, otherwise false
// function:	creates this control
BOOL CPieChartCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    CString strWndClass = AfxRegisterWndClass (0, NULL, (HBRUSH)GetStockObject(WHITE_BRUSH), NULL);
	return CWnd::Create(strWndClass, NULL, dwStyle, rect, pParentWnd, nID);
}

//////////
// CPieChartCtrl::DrawPiece
// arguments:	pDC: pointer to dc to draw in
//				xStartAngle: starting angle of piece
//				xEndAngle: ending angle of piece
// returns:		void
// function:	draws a pie piece in the dc
void CPieChartCtrl::DrawPiePiece(CDC* pDC, double xStartAngle, double xEndAngle)
{
	if(xEndAngle - xStartAngle <= 1.00) return;

	// gdi objects needed
	CRect rt, rt2;
	CRgn rellipsehi, rellipselow, rrect, rdepthcurve, rdepth;
	CPoint pt1, pt2, pt3, pt4;
	CRgn rpie, rangle, rellipse;
	CPoint poly[8];

	// set up coordinates
	GetWindowRect(&rt);
	ScreenToClient(&rt);
	CPoint cp;
	int major = (int)((rt.Width() - 2 * PIE_BUFFER) * 0.5);
	int minor = (int)((rt.Height() - 2 * PIE_BUFFER) * 0.25);
	int maxmajor = (int)(pDC->GetDeviceCaps(HORZRES) * PIE_MAJOR_MAX);
	int maxminor = (int)(pDC->GetDeviceCaps(VERTRES) * PIE_MINOR_MAX);
	if(major > maxmajor) major = maxmajor;
	if(minor > maxminor) minor = maxminor;

	int depth = (int)(minor * PIE_DEPTH);
	cp.x = (int)(rt.Width() * 0.5);
	cp.y = rt.Height() - minor - PIE_BUFFER - depth;
	rt.SetRect(cp.x - major, cp.y - minor, cp.x + major, cp.y + minor);
	rt2.SetRect(cp.x - major, cp.y - minor + depth, cp.x + major, cp.y + minor + depth);

	// draw elliptical part of piece

	// set up coordinates
	poly[0].x = cp.x; poly[0].y = cp.y;
	EllipsePoint(&rt, xStartAngle + (xEndAngle - xStartAngle) * 0.00f, &poly[1]);
	CirclePoint(&cp, major * 5, xStartAngle + (xEndAngle - xStartAngle) * 0.00f, &poly[2]);
	CirclePoint(&cp, major * 5, xStartAngle + (xEndAngle - xStartAngle) * 0.25f, &poly[3]);
	CirclePoint(&cp, major * 5, xStartAngle + (xEndAngle - xStartAngle) * 0.50f, &poly[4]);
	CirclePoint(&cp, major * 5, xStartAngle + (xEndAngle - xStartAngle) * 0.75f, &poly[5]);
	CirclePoint(&cp, major * 5, xStartAngle + (xEndAngle - xStartAngle) * 1.00f, &poly[6]);
	EllipsePoint(&rt, xStartAngle + (xEndAngle - xStartAngle) * 1.00f, &poly[7]);
	
	// filled part
	rellipse.CreateEllipticRgnIndirect(&rt);
	rangle.CreatePolygonRgn(poly, 8, ALTERNATE);
	rpie.CreateRectRgnIndirect(&rt);
	rpie.CombineRgn(&rellipse, &rangle, RGN_AND);
	pDC->FillRgn(&rpie, pDC->GetCurrentBrush());

	// outline
	pDC->MoveTo(rt.CenterPoint());
	pDC->LineTo(poly[1]);
	pDC->Arc(&rt, poly[1], poly[7]);
	pDC->MoveTo(poly[7]);
	pDC->LineTo(rt.CenterPoint());

	// clean up
	rellipse.DeleteObject();
	rangle.DeleteObject();
	rpie.DeleteObject();

	// draw depth part of pie piece if needed
	if(xStartAngle >= 180 || xEndAngle >= 180) {

		// set up coordinates
		double xLowerAngle = 180;
		if(xStartAngle > 180) xLowerAngle = xStartAngle;
		double xHigherAngle = 360;
		if(xEndAngle < 360) xHigherAngle = xEndAngle;
		EllipsePoint(&rt, xLowerAngle, &pt1);
		EllipsePoint(&rt, xHigherAngle, &pt2);
		EllipsePoint(&rt2, xLowerAngle, &pt3);
		EllipsePoint(&rt2, xHigherAngle, &pt4);
		if(xStartAngle > 180) {
			pt1.x = poly[1].x;
			pt3.x = poly[1].x;
		}
		if(xEndAngle < 360) {
			pt2.x = poly[7].x;
			pt4.x = poly[7].x;
		}

		// filled part
		rellipsehi.CreateEllipticRgnIndirect(&rt);
		rellipselow.CreateEllipticRgnIndirect(&rt2);
		rrect.CreateRectRgn(pt1.x, rt.top, pt4.x, rt2.bottom);
		rdepthcurve.CreateRectRgnIndirect(&rt2);
		rdepthcurve.CombineRgn(&rellipselow, &rellipsehi, RGN_DIFF);
		rdepth.CreateRectRgnIndirect(&rt2);
		rdepth.CombineRgn(&rdepthcurve, &rrect, RGN_AND);
		pDC->FillRgn(&rdepth, pDC->GetCurrentBrush());
		
		// ouline
		pDC->Arc(&rt, pt1, pt2);
		pDC->Arc(&rt2, pt3, pt4);
		pDC->MoveTo(pt1);
		pDC->LineTo(pt3);
		pDC->MoveTo(pt4);
		pDC->LineTo(pt2);

		// clean up
		rellipsehi.DeleteObject();
		rellipselow.DeleteObject();
		rdepthcurve.DeleteObject();
		rdepth.DeleteObject();
	}
}

//////////
// CPieChartCtrl::CirclePoint
// arguments:	center: center point of circle
//				nRadius: radius of circle
//				xAngle: angle of radius
//				pResult: pointer to CPoint to put result in
// returns:		void
// function:	calculates the point on the circle at the given angle
void CPieChartCtrl::CirclePoint(CPoint* center, int nRadius, double xAngle, CPoint* pResult)
{
	pResult->x = (long)(center->x + nRadius * cos(xAngle * (PI / 180)));
	pResult->y = (long)(center->y - nRadius * sin(xAngle * (PI / 180)));
}

//////////
// CPieChartCtrl::EllipsePoint
// arguments:	rt: pointer to rect of ellipse
//				xAngle: angle of radius
//				pResult: pointer to CPoint to put result in
// returns:		void
// function:	calculates the point on the ellipse in the given rect at
//				the given angle
void CPieChartCtrl::EllipsePoint(CRect* rt, double xAngle, CPoint* pResult)
{
	pResult->x = (int)(rt->CenterPoint().x + (rt->Width() / 2) * cos(xAngle * (PI / 180)));
	pResult->y = (int)(rt->CenterPoint().y - (rt->Height() / 2) * sin(xAngle * (PI / 180)));
}

//////////
// CPieChartCtrl::SetFont
// arguments:	pFont: pointer to font to set
// returns:		void
// function:	sets this control's font
void CPieChartCtrl::SetFont(CFont* pFont)
{
	m_pFont = pFont;
}

//////////
// CPieChartCtrl::SetTotal
// arguments:	xTotal: the total amount of the pie chart
// returns:		void
// function:	sets the tag for data, which is a string that will be displayed
//				after the numbers in the label
void CPieChartCtrl::SetTotal(double xTotal)
{
	if(xTotal >= 0) m_xTotal = xTotal;
}

//////////
// CPieChartCtrl::OnPaint
// arguments:	void
// returns:		void
// function:	draws the control by drawing the labels and pie pieces for
//				each piece of the pie
void CPieChartCtrl::OnPaint()
{
	CWnd::OnPaint();

	// no pieces, so dont do anything
	if(m_xValues.GetSize() == 0) return;

	// gdi objects needed
	CClientDC cdc(this);
	CRect rt;
	CDC MemDC;
	CBitmap MemBmp;
	CBrush MemBrush;
	CPen MemPen;
	CBitmap* pOldBmp = NULL;
	CBrush* pOldBrush = NULL;
	CPen* pOldPen = NULL;
	CFont* pOldFont = NULL;

	// create offscreen buffer
	GetClientRect(&rt);
	MemDC.CreateCompatibleDC(&cdc);
	MemBmp.CreateCompatibleBitmap(&cdc, rt.Width(), rt.Height());
	MemPen.CreatePen(PS_SOLID, 0, RGB(0, 0, 0));

	// select gdi objects
	pOldBmp = MemDC.SelectObject(&MemBmp);
	pOldPen = MemDC.SelectObject(&MemPen);
	pOldFont = MemDC.SelectObject(m_pFont);
	MemDC.FillSolidRect(&rt, RGB(255, 255, 255));

	// go through each xPercent and draw its label and pie
	double xSoFar = 0;
	CRect wndrect;
	CRect textrect;
	GetWindowRect(&wndrect);
	for(int i = 0; i < m_xValues.GetSize(); i ++) {
		MemBrush.CreateSolidBrush(m_colors.GetAt(i));

		pOldBrush = MemDC.SelectObject(&MemBrush);

		// display color box and label
		if(PIE_BUFFER + 20 + i * 20 < wndrect.Height() / 2) {
			textrect.SetRect(PIE_BUFFER + 0, PIE_BUFFER + i * 20 + 4, PIE_BUFFER + 11, PIE_BUFFER + 20 + i * 20 - 4);
			MemDC.FillRect(&textrect, &MemBrush);
			MemDC.MoveTo(textrect.left, textrect.top);
			MemDC.LineTo(textrect.right, textrect.top);
			MemDC.LineTo(textrect.right, textrect.bottom);
			MemDC.LineTo(textrect.left, textrect.bottom);
			MemDC.LineTo(textrect.left, textrect.top);
			textrect.SetRect(PIE_BUFFER + 16, PIE_BUFFER + i * 20, wndrect.Width() - PIE_BUFFER, PIE_BUFFER + 20 + i * 20);
			char size_buf[256];
			nbytes_to_string(m_xValues.GetAt(i), 0, size_buf, 256);
			CString strBuf;
			strBuf.Format("%s (%s)", m_strLabels.GetAt(i).GetBuffer(0), size_buf);
			MemDC.DrawText(strBuf, textrect, DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_END_ELLIPSIS);
		}

		// display pie piece
		double xPercent = 0;
		if(m_xTotal > 0) xPercent = m_xValues.GetAt(i) / m_xTotal;
		DrawPiePiece(&MemDC, xSoFar * 360, (xSoFar + xPercent) * 360);
		xSoFar += xPercent;

		MemDC.SelectObject(pOldBrush);
		MemBrush.DeleteObject();
	}

	// copy offscreen buffer to screen
	cdc.BitBlt(0, 0, rt.Width(), rt.Height(), &MemDC, 0, 0, SRCCOPY);

	// clean up
	MemDC.SelectObject(pOldBmp);
	MemDC.SelectObject(pOldPen);
	MemDC.SelectObject(pOldFont);
	MemPen.DeleteObject();
	MemBmp.DeleteObject();
	MemBrush.DeleteObject();
	MemDC.DeleteDC();
}



const char *BOINC_RCSID_1c43879c85 = "$Id$";
