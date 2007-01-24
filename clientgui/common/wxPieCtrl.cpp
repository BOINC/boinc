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


/* #### wxPieCtlrLegend */
BEGIN_EVENT_TABLE(wxPieCtrlLegend, wxWindow)
	EVT_PAINT(wxPieCtrlLegend::OnPaint)
	EVT_ERASE_BACKGROUND(wxPieCtrlLegend::OnEraseBackground)
END_EVENT_TABLE()

/* constructor */
wxPieCtrlLegend::wxPieCtrlLegend(wxPieCtrl * parent, wxString title,
		wxPoint pos, wxSize sz,
		long style)
		: wxWindow(parent, -1, pos, sz, style), m_IsTransparent(false),
		m_HorBorder(5), m_VerBorder(5)
{
	m_TitleColour = wxColour(0,0,127);
	m_LabelColour = *wxBLACK;
	m_BackColour = wxColour(255,255,0);
	m_TitleFont = *wxSWISS_FONT;
	m_TitleFont.SetWeight(wxBOLD);
	//remember the title, because GetLabel() doesn't seem to work under X
	m_szTitle = title;
}


void wxPieCtrlLegend::SetLabel(const wxString& label) {
	wxWindow::SetLabel(label);
	m_szTitle = label;
}

void wxPieCtrlLegend::SetTransparent(bool value)
{
	m_IsTransparent = value;
	Refresh();
}

void wxPieCtrlLegend::RecreateBackground(wxMemoryDC & parentdc)
{
	int w,h;
	GetSize(&w,&h);
	m_Background.Create(w,h);
	m_BackgroundDC.SelectObject(m_Background);
	if(IsTransparent())
	{
		m_BackgroundDC.Blit(0,0,w, h, &parentdc, GetPosition().x, GetPosition().y);
	}
	else
	{
		m_BackgroundDC.SetBackground(wxBrush(m_BackColour));
		m_BackgroundDC.Clear();
	}
	Refresh();
}

void wxPieCtrlLegend::SetHorBorder(unsigned int value)
{
	m_HorBorder = value;
	Refresh();
}

void wxPieCtrlLegend::SetVerBorder(unsigned int value)
{
	m_VerBorder = value;
	Refresh();
}

void wxPieCtrlLegend::SetLabelColour(wxColour colour)
{
	m_LabelColour = colour;
	Refresh();
}

void wxPieCtrlLegend::SetBackColour(wxColour colour)
{
	m_BackColour = colour;
	Refresh();
}

void wxPieCtrlLegend::OnPaint(wxPaintEvent & /*event*/)
{
	wxPaintDC pdc(this);

	int w,h;
	GetSize(&w,&h);
	wxBitmap bmp(w,h);
	wxMemoryDC mdc;
	mdc.SelectObject(bmp);
	if(IsTransparent())
	{
		wxClientDC parentdc(GetParent());
		mdc.Blit(0,0,w,h,&m_BackgroundDC, 0, 0);
	}
	else
	{
		mdc.SetBackground(wxBrush(m_BackColour));
		mdc.Clear();
	}
	wxPieCtrl * parent = (wxPieCtrl *)GetParent();
	unsigned int i;
	int dy(m_VerBorder),tw,th,titlew,titleh;
	//draw legend title
	mdc.SetFont(m_TitleFont);
	mdc.SetTextForeground(m_TitleColour);
	mdc.GetTextExtent(m_szTitle,&titlew,&titleh);
	mdc.DrawText(m_szTitle,m_HorBorder+2,m_VerBorder+2);
	dy += (titleh+5);
	//draw legend items
	mdc.SetFont(m_LabelFont);
	mdc.SetTextForeground(m_LabelColour);
	int maxwidth(titlew + 2*m_HorBorder + 15);
	for(i = 0; i < parent->m_Series.Count(); i++)
	{
		mdc.GetTextExtent(parent->m_Series[i].GetLabel(), &tw, &th);
		mdc.SetBrush(wxBrush(parent->m_Series[i].GetColour()));
		mdc.DrawCircle(m_HorBorder+5, dy+th/2, 5);
		mdc.DrawText(parent->m_Series[i].GetLabel(), m_HorBorder+15, dy);
		dy += (th+3);
		maxwidth = max(maxwidth, (int)(2*m_HorBorder+tw+15));
	}
	dy += m_VerBorder;
	if(w != maxwidth || h != dy) SetSize(maxwidth, dy);

    // SetWindowStyle borders distort the pie circle on Mac so we draw our own
    int x, y;
    wxPen savedPen = mdc.GetPen();
	GetSize(&x,&y);
    x--;
    y--;
    mdc.SetPen(*wxGREY_PEN);
	mdc.DrawLine(0,0,x,0);      // top
	mdc.DrawLine(0,y,0,0);      // left
    mdc.SetPen(*wxWHITE_PEN);
	mdc.DrawLine(0,y,x,y);      // bottom
	mdc.DrawLine(x,0,x,y);      // right
    mdc.SetPen(savedPen);

	pdc.Blit(0,0,w,h,&mdc,0,0);
}

void wxPieCtrlLegend::SetLabelFont(wxFont font)
{
	m_LabelFont = font;
	Refresh();
}

void wxPieCtrlLegend::OnEraseBackground(wxEraseEvent & /*event*/)
{
	//prevent flickering
}

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

	SetSizer(NULL);
	SetSize(sz);
	m_CanvasBitmap.Create(1,1);
	RecreateCanvas();
	m_Legend = new wxPieCtrlLegend(this, _("Pie Ctrl"), wxPoint(10,10), wxSize(100,75));
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
	m_Legend->RecreateBackground(m_CanvasDC);
}


void wxPieCtrl::Refresh(bool eraseBackground, const wxRect* rect)
{
	m_CanRepaint = true;
	wxWindow::Refresh(eraseBackground, rect);
}

