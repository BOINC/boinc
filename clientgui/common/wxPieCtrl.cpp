/////////////////////////////////////////////////////////////////////////////
// Name:        wxPieCtrl.cpp
// Purpose:     wxPieCtrl (v0.1.2)
// Author:      Volodymir (T-Rex) Tryapichko
// Modified by:	Frank Weiler, 24.02.2007
// Created:     June-07-2005
// RCS-ID:      $Id:
// Copyright:   (c) Volodymir (T-Rex) Tryapichko
// Licence:     wxWidgets license
/////////////////////////////////////////////////////////////////////////////
#include "wxPieCtrl.h"
#include <wx/arrimpl.cpp>

using namespace std;

#if ! wxCHECK_VERSION(2,8,0)
WX_DEFINE_OBJARRAY(wxArrayDouble);
#endif
WX_DEFINE_OBJARRAY(wxPieSeries);

/* ####### wxPiePart */
wxPiePart::wxPiePart()
: m_Value(0)
{
}

wxPiePart::wxPiePart(double value, wxColour colour, wxString label)
: m_Value(value), m_Colour(colour), m_Label(label)
{
}

wxPiePart::wxPiePart(const wxPiePart & part)
: m_Value(part.m_Value), m_Colour(part.m_Colour), m_Label(part.m_Label)
{
}


/* ############ wxPieCtrl */
BEGIN_EVENT_TABLE(wxPieCtrl, wxWindow)
	EVT_PAINT(wxPieCtrl::OnPaint)
	EVT_ERASE_BACKGROUND(wxPieCtrl::OnEraseBackground)
	EVT_SIZE(wxPieCtrl::OnSize)
	EVT_MOTION(wxPieCtrl::OnMouseMove)
END_EVENT_TABLE()

/* constructor */
wxPieCtrl::wxPieCtrl(wxWindow * parent, wxWindowID id, wxPoint pos,
		wxSize sz, long style, wxString name)
		:wxWindow(parent, id, pos, sz, style, name)
{
	m_ShowEdges=true;
	m_CanRepaint=true;
	m_BackColour=*wxWHITE;
	m_padding=10;

	m_TitleColour = wxColour(0,0,0);
	m_LabelColour = *wxBLACK;
	m_LegendBackColour = wxColour(255,255,0);
	m_TitleFont = *wxSWISS_FONT;
	m_TitleFont.SetWeight(wxBOLD);
	m_LabelFont = *wxSWISS_FONT;
	m_legendHorBorder = 10;
 	m_LegendVerBorder = 10;
	m_szTitle = _("Pie Ctrl");

	SetSizer(NULL);
	SetSize(sz);
	m_CanvasBitmap.Create(1,1);
	RecreateCanvas();
}

/* getter and setter */

void wxPieCtrl::SetPadding(int pad) {
	m_padding = pad;
	Refresh();
}

int wxPieCtrl::GetPadding() {
	return m_padding;
}

void wxPieCtrl::SetShowEdges(bool value)
{
	m_ShowEdges = value;
	Refresh();
}

bool wxPieCtrl::GetShowEdges() {
	return m_ShowEdges;
}

void wxPieCtrl::SetBackColour(wxColour colour)
{
	m_BackColour = colour;
	Refresh();
}

wxColour wxPieCtrl::GetBackColour() {
	return m_BackColour;
}

/* event handlers */
/* handles mouse motion events to show tooltips 
   for some (unkown) reason under X no tooltips are displayed
*/
void wxPieCtrl::OnMouseMove(wxMouseEvent& ev) {
	//get the pie part, over which the mouse pointer is
	int piepart = GetCoveredPiePart(ev.GetX(),ev.GetY());
	//part identified
	if(piepart >=0) {		
		//prevent tooltip flicker
		if(piepart != m_lastCoveredPart) {
			wxString tooltip = this->m_Series[piepart].GetLabel();
			this->SetToolTip(tooltip);
			m_lastCoveredPart=piepart;
		}
	}
	else {
		this->SetToolTip(wxEmptyString);
		m_lastCoveredPart=-1;
	}	
	ev.Skip();	
}

void wxPieCtrl::OnSize(wxSizeEvent & /*event*/)
{
	RecreateCanvas();
	Refresh();
}

void wxPieCtrl::OnEraseBackground(wxEraseEvent & /*event*/)
{
	//prevent flicker
}

void wxPieCtrl::OnPaint(wxPaintEvent & /*event*/)
{
	wxPaintDC pdc(this);
	Draw(pdc);
}

/* internal methods */
/* gets the pie part on coords using pixel colour */
int wxPieCtrl::GetCoveredPiePart(int x,int y) {
	wxColour col;
	wxClientDC dc(this);
	if(dc.GetPixel(x,y,&col)) {
		for(unsigned int i=0; i <m_Series.Count();i++) {
			if(m_Series[i].GetColour() == col) {
				return i;
			}
		}
	}
	return -1;
}

void wxPieCtrl::RecreateCanvas()
{
	int x = GetSize().GetWidth();
	int y = GetSize().GetHeight();
//#ifdef __WXMAC__
	if ((x < 1) || (y < 1))
		return;
//#endif
 	m_CanvasBitmap.Create(x, y);
	m_CanvasDC.SelectObject(m_CanvasBitmap);
}

void wxPieCtrl::GetPartAngles(wxArrayDouble & angles)
{
	angles.Clear();
	double total(0);
	unsigned int i;
	for(i = 0; i < m_Series.Count(); i++)
	{
		total += m_Series[i].GetValue();
	}
	double current(0);
	angles.Add(current);
	for(i = 0; i < m_Series.Count(); i++)
	{
		current += m_Series[i].GetValue();
		angles.Add(360 * (double)current / (double)total);
	}	
}


void wxPieCtrl::DrawParts(wxRect& pieRect)
{
	wxArrayDouble angles;
	wxPen oldpen = m_CanvasDC.GetPen();
	if(m_ShowEdges) {
		m_CanvasDC.SetPen(*wxBLACK_PEN);
	}
	if(m_Series.Count() == 1)
	{
		m_CanvasDC.SetBrush(wxBrush(m_Series[0].GetColour()));
		m_CanvasDC.DrawEllipticArc(pieRect.GetLeft(),
											   pieRect.GetTop(),
											   pieRect.GetRight()-pieRect.GetLeft(),
											   pieRect.GetBottom()-pieRect.GetTop(),
											   0,360);
	}
	else {
		GetPartAngles(angles);
		for(unsigned int i = 0; i < angles.Count(); i++)
		{
			if(i > 0)
			{
				if(!m_ShowEdges)
				{
					m_CanvasDC.SetPen(wxPen(m_Series[i-1].GetColour()));
				}
				m_CanvasDC.SetBrush(wxBrush(m_Series[i-1].GetColour()));
				double t1,t2;
#ifndef __WXMSW__
				// Convert angles to ints and back to doubles to avoid roundoff error which causes gaps between parts
				t1 = (double)(int)angles[i-1];
				t2 = (double)(int)angles[i];
				// !!! very little parts (angel diff < 1) are not shown
				// because t1=t2 after type conversion
#else
				t1 = angles[i-1];
				t2 = angles[i];
#endif 
				if(t1 != t2) {
					m_CanvasDC.DrawEllipticArc(pieRect.GetLeft(),
											   pieRect.GetTop(),
											   pieRect.GetRight()-pieRect.GetLeft(),
											   pieRect.GetBottom()-pieRect.GetTop(),
											   t1,t2);
				}
			}
		}
	}
	m_CanvasDC.SetPen(oldpen);
}

void wxPieCtrl::DrawLegend(int left, int top)
{
	unsigned int i;
	int dy(m_LegendVerBorder),tw,th,titlew,titleh;

	// First determine the size of the legend box
	m_CanvasDC.SetFont(m_TitleFont);
	m_CanvasDC.GetTextExtent(m_szTitle,&titlew,&titleh);

	m_CanvasDC.DrawText(m_szTitle,left+m_legendHorBorder+2,top+m_LegendVerBorder+2);
	dy += (titleh+5);
	m_CanvasDC.SetFont(m_LabelFont);

	int maxwidth(titlew + 2*m_legendHorBorder + 15);
	for(i = 0; i < m_Series.Count(); i++)
	{
		m_CanvasDC.GetTextExtent(m_Series[i].GetLabel(), &tw, &th);
		dy += (th+3);
		maxwidth = max(maxwidth, (int)(2*m_legendHorBorder+tw+15));
	}
	dy += m_LegendVerBorder;

	int right(left+maxwidth-1), bottom(top+dy-1);

	if(! IsTransparent())
	{
		m_CanvasDC.SetBrush(wxBrush(m_LegendBackColour));
		m_CanvasDC.DrawRectangle(left, top, maxwidth, dy);
	}

	// Now draw the legend title
	dy = m_LegendVerBorder+titleh+5;
	m_CanvasDC.SetFont(m_TitleFont);
	m_CanvasDC.SetTextForeground(m_TitleColour);
	m_CanvasDC.DrawText(m_szTitle,left+m_legendHorBorder+2,top+m_LegendVerBorder+2);

 	// Draw the legend items
	m_CanvasDC.SetFont(m_LabelFont);
	m_CanvasDC.SetTextForeground(m_LabelColour);
	for(i = 0; i < m_Series.Count(); i++)
	{
		m_CanvasDC.SetBrush(wxBrush(m_Series[i].GetColour()));
		m_CanvasDC.DrawCircle(left+m_legendHorBorder+5, top+dy+th/2, 5);
		m_CanvasDC.DrawText(m_Series[i].GetLabel(), left+m_legendHorBorder+15, top+dy);
		dy += (th+3);
	}

	// Draw the legend frame
	wxPen savedPen = m_CanvasDC.GetPen();
	m_CanvasDC.SetPen(*wxGREY_PEN);
	m_CanvasDC.DrawLine(left,top,right,top);		// top
	m_CanvasDC.DrawLine(left,bottom,left,top);		// left
	m_CanvasDC.SetPen(*wxWHITE_PEN);
	m_CanvasDC.DrawLine(left,bottom,right,bottom);		// bottom
	m_CanvasDC.DrawLine(right,top,right,bottom);		// right
	m_CanvasDC.SetPen(savedPen);
}

/* paint the pie */
void wxPieCtrl::Draw(wxPaintDC & pdc)
{
	int bgW,bgH;
	wxRect pieRect;
	//size for background ops
	GetSize(&bgW,&bgH);
	//pie rect respects padding and is centered
	int maxL = min(bgW,bgH) - 2* m_padding;

	if(m_CanRepaint)
	{
		//calc the pie rect coords
		pieRect.SetLeft(bgW/2 - maxL/2);
		pieRect.SetTop(bgH/2 - maxL/2);
		pieRect.SetBottom(pieRect.GetTop() + maxL);
		pieRect.SetRight(pieRect.GetLeft() + maxL);

#if ! wxCHECK_VERSION(2,8,0)
		m_CanvasDC.BeginDrawing();
#endif
		m_CanvasDC.SetBackground(wxBrush(m_BackColour));
		m_CanvasDC.Clear();
		if(m_Series.Count())
		{
			DrawParts(pieRect);
			DrawLegend(10, 10);
		}
		else {
			//no data, draw an black circle
			m_CanvasDC.SetBrush(*wxBLACK_BRUSH);
			m_CanvasDC.DrawEllipticArc(pieRect.GetLeft(),
											   pieRect.GetTop(),
											   pieRect.GetRight()-pieRect.GetLeft(),
											   pieRect.GetBottom()-pieRect.GetTop(),
											   0,360);
		}
#if ! wxCHECK_VERSION(2,8,0)
		m_CanvasDC.EndDrawing();		
#endif
		m_CanRepaint = false;
	}
	pdc.Blit(0,0,bgW,bgH,&m_CanvasDC,0,0);
}


void wxPieCtrl::Refresh(bool eraseBackground, const wxRect* rect)
{
	m_CanRepaint = true;
	wxWindow::Refresh(eraseBackground, rect);
}



void wxPieCtrl::SetLabel(const wxString& label) {
	wxWindow::SetLabel(label);
	m_szTitle = label;
}

void wxPieCtrl::SetTransparent(bool value)
{
	m_LegendIsTransparent = value;
	Refresh();
}

void wxPieCtrl::SetLabelFont(wxFont font)
{
	m_LabelFont = font;
	Refresh();
}

void wxPieCtrl::SetHorLegendBorder(unsigned int value)
{
	m_legendHorBorder = value;
	Refresh();
}

void wxPieCtrl::SetVerLegendBorder(unsigned int value)
{
	m_LegendVerBorder = value;
	Refresh();
}

void wxPieCtrl::SetLabelColour(wxColour colour)
{
	m_LabelColour = colour;
	Refresh();
}

void wxPieCtrl::SetLegendBackColour(wxColour colour)
{
	m_LegendBackColour = colour;
	Refresh();
}
