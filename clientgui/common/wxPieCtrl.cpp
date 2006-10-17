/////////////////////////////////////////////////////////////////////////////
// Name:        wxPieCtrl.cpp
// Purpose:     wxPieCtrl (v0.1.2)
// Author:      Volodymir (T-Rex) Tryapichko
// Modified by:	Frank Weiler, 10.10.2006
// Created:     June-07-2005
// RCS-ID:      $Id:
// Copyright:   (c) Volodymir (T-Rex) Tryapichko
// Licence:     wxWidgets license
/////////////////////////////////////////////////////////////////////////////
#include "wxPieCtrl.h"
#include <wx/arrimpl.cpp>
#include <algorithm>

using namespace std;

WX_DEFINE_OBJARRAY(wxArrayDouble);
WX_DEFINE_OBJARRAY(wxPieSeries);

BEGIN_EVENT_TABLE(wxPieCtrlLegend, wxWindow)
EVT_PAINT(wxPieCtrlLegend::OnPaint)
EVT_ERASE_BACKGROUND(wxPieCtrlLegend::OnEraseBackground)
END_EVENT_TABLE()

wxPieCtrlLegend::wxPieCtrlLegend(wxPieCtrl * parent, wxString title, 
		wxPoint pos, wxSize sz,
		long style)
		: wxWindow(parent, -1, pos, sz, style), m_IsTransparent(false),
		m_HorBorder(5), m_VerBorder(5)
{
	m_TitleColour = wxColour(0,0,127);
	m_LabelColour = *wxBLACK;
	m_BackColour = wxColour(255,255,0);	
#ifdef __WXMAC__
	m_TitleFont = *wxNORMAL_FONT;   // Prevent wxDebug assert
#endif
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

void wxPieCtrlLegend::OnPaint(wxPaintEvent & event)
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
	mdc.GetTextExtent(this->GetTitle(),&titlew,&titleh);
	mdc.DrawText(this->GetTitle(),m_HorBorder+2,m_VerBorder+2);
	dy += (titleh+5);
	//draw legend items
	mdc.SetFont(m_LabelFont);
	mdc.SetTextForeground(m_LabelColour);
	int maxwidth(titlew);
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

#ifdef __WXMAC__
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
#endif
	pdc.Blit(0,0,w,h,&mdc,0,0);
}

void wxPieCtrlLegend::SetLabelFont(wxFont font)
{
	m_LabelFont = font;
	Refresh();
}

void wxPieCtrlLegend::OnEraseBackground(wxEraseEvent & event)
{
}

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

BEGIN_EVENT_TABLE(wxPieCtrl, wxWindow)
EVT_PAINT(wxPieCtrl::OnPaint)
EVT_ERASE_BACKGROUND(wxPieCtrl::OnEraseBackground)
EVT_SIZE(wxPieCtrl::OnSize)
END_EVENT_TABLE()

wxPieCtrl::wxPieCtrl(wxWindow * parent, wxWindowID id, wxPoint pos,
		wxSize sz, long style, wxString name)
		:wxWindow(parent, id, pos, sz, style, name), m_Angle(M_PI/12), m_RotationAngle(0), m_Height(10),
		m_Background(wxNullBitmap), m_BackColour(wxColour(255,255,255)), m_ShowEdges(true),
		m_CanRepaint(true),m_bPaint3D(true),m_bDrawCircle(false)
{	
	SetSizer(NULL);
	SetSize(sz);
	m_CanvasBitmap.Create(1,1);
	RecreateCanvas();
	m_Legend = new wxPieCtrlLegend(this, _("Pie Ctrl"), wxPoint(10,10), wxSize(100,75));			
}

void wxPieCtrl::SetBackground(wxBitmap bmp)
{
	m_Background = bmp;	
	Refresh();
}

void wxPieCtrl::OnSize(wxSizeEvent & event)
{
	RecreateCanvas();	
	Refresh();
}

void wxPieCtrl::RecreateCanvas()
{
    int x = GetSize().GetWidth();
    int y = GetSize().GetHeight();
#ifdef __WXMAC__
    if ((x < 1) || (y < 1))
        return;
#endif
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

void wxPieCtrl::SetAngle(double angle)
{
	if(angle < 0) angle = 0;
	if(angle > M_PI/2) angle = M_PI/2;
	m_Angle = angle;	
	Refresh();
}

void wxPieCtrl::SetRotationAngle(double angle)
{
	if(angle < 0) angle = 0;
	if(angle > 2 * M_PI) angle = 2 * M_PI;
	m_RotationAngle = angle;	
	Refresh();
}

void wxPieCtrl::SetShowEdges(bool value)
{
	m_ShowEdges = value;	
	Refresh();
}

void wxPieCtrl::SetBackColour(wxColour colour)
{
	m_BackColour = colour;	
	Refresh();
}

#if defined(__WXMSW__) || defined(__WXMAC__)
void wxPieCtrl::DrawParts(wxMemoryDC & dc, int cx, int cy, int w, int h)
{
	if(m_bDrawCircle) {
		//no angle
		if(cy==0) {
			cy = (int)(h/2 - (min(w,h)/2));
		}
		w = min(w,h);
		h = w;
		
	}

	wxArrayDouble angles;	
	GetPartAngles(angles);	
	wxPen oldpen = dc.GetPen();	
	if(m_ShowEdges) dc.SetPen(*wxBLACK_PEN);
	unsigned int i;
	for(i = 0; i < angles.Count(); i++)
	{
		if(i > 0)
		{				
			if(!m_ShowEdges) dc.SetPen(wxPen(m_Series[i-1].GetColour()));
			dc.SetBrush(wxBrush(m_Series[i-1].GetColour()));
			if(angles[i-1] != angles[i]) {
#ifdef __WXMAC__                // Convert angles to ints and back to doubles to avoid roundoff error which causes gaps between parts
				dc.DrawEllipticArc(0, (int)((1-sin(m_Angle))*(h/2)+cy), w, (int)(h * sin(m_Angle)), (double)((int)angles[i-1]+m_RotationAngle/M_PI*180), (double)((int)angles[i]+m_RotationAngle/M_PI*180));						
#else
				dc.DrawEllipticArc(0, (int)((1-sin(m_Angle))*(h/2)+cy), w, (int)(h * sin(m_Angle)), angles[i-1]+m_RotationAngle/M_PI*180, angles[i]+m_RotationAngle/M_PI*180);						
#endif
                        }
		}
	}
	if(m_Series.Count() == 1)
	{
		dc.SetBrush(wxBrush(m_Series[0].GetColour()));
		dc.DrawEllipticArc(0, (int)((1-sin(m_Angle))*(h/2)+cy), w, (int)(h * sin(m_Angle)), 0, 360);
	}
	dc.SetPen(oldpen);
}
#endif

void wxPieCtrl::SetDrawCircle(bool bCircle) {
	m_bDrawCircle = bCircle;
}

bool wxPieCtrl::GetDrawCircle() {
	return m_bDrawCircle;
}

void wxPieCtrl::Draw(wxPaintDC & pdc)
{
	int w,h,i,j;
	int px, py;
	GetSize(&w,&h);	
	if(m_CanRepaint)
	{
		m_CanvasDC.BeginDrawing();	
		m_CanvasDC.SetBackground(*wxWHITE_BRUSH);
		m_CanvasDC.Clear();
		if(m_Background != wxNullBitmap)
		{
			for(i = 0; i < w; i+= m_Background.GetWidth())
			{
				for(j = 0; j < h; j+= m_Background.GetHeight())
				{
					m_CanvasDC.DrawBitmap(m_Background,i,j);
				}
			}
		} 
		else
		{
			m_CanvasDC.SetBackground(wxBrush(m_BackColour));
			m_CanvasDC.Clear();
		}	
		if(m_Series.Count())
		{
#if defined(__WXMSW__) || defined(__WXMAC__)
			if(m_Angle <= M_PI/2)
			{	
				DrawParts(m_CanvasDC, 0, (int)(m_Height*cos(m_Angle)), w,h);				
			} else DrawParts(m_CanvasDC, 0, 0, w, h);
#endif
			wxPoint points[4];	
			m_CanvasDC.SetPen(wxPen(*wxBLACK));	
			wxArrayDouble angles;
			GetPartAngles(angles);
			unsigned angleindex(0);
			m_CanvasDC.SetBrush(wxBrush(wxColour(m_Series[angleindex].GetColour().Red(),
				m_Series[angleindex].GetColour().Green(),
				m_Series[angleindex].GetColour().Blue())));		
			double x;
			bool changeangle(false);
			wxColour curColour;
			wxPen oldPen;
#if ! (defined(__WXMSW__) || defined(__WXMAC__))
			wxPoint triangle[3];	
			for(x = 0; x <= 2 * M_PI; x += 0.05)
			{
				changeangle = false;
				if(angleindex < angles.Count())
				{
					if((double)x/(double)M_PI*(double)180 >= angles[angleindex+1]) 
					{				
						changeangle = true;				
						x = angles[angleindex+1]*M_PI/180;						
					}
				}			
				points[0].x = points[1].x;
				points[0].y = points[1].y;				
				px = (int)(w/2 * (1+cos(x+m_RotationAngle)));
				py = (int)(h/2-sin(m_Angle)*h/2*sin(x+m_RotationAngle)-1);
				points[1].x = px;
				points[1].y = py;				
				triangle[0].x = w / 2;
				triangle[0].y = h / 2;
				triangle[1].x = points[0].x;
				triangle[1].y = points[0].y;
				triangle[2].x = points[1].x;
				triangle[2].y = points[1].y;
				if(x > 0) 
				{
					m_CanvasDC.SetBrush(wxBrush(m_Series[angleindex].GetColour()));
					oldPen = m_CanvasDC.GetPen();
					m_CanvasDC.SetPen(wxPen(m_Series[angleindex].GetColour()));					
					m_CanvasDC.DrawPolygon(3, triangle);
					m_CanvasDC.SetPen(oldPen);
				}
				if(changeangle) 			
				{			
					angleindex += 1;			
				}
			}	
			x = 2 * M_PI;					
			points[0].x = points[1].x;
			points[0].y = points[1].y;			
			px = (int)(w/2 * (1+cos(x+m_RotationAngle)));
			py = (int)(h/2-sin(m_Angle)*h/2*sin(x+m_RotationAngle)-1);
			points[1].x = px;
			points[1].y = py;			
			triangle[0].x = w / 2;
			triangle[0].y = h / 2;
			triangle[1].x = points[0].x;
			triangle[1].y = points[0].y;
			triangle[2].x = points[1].x;
			triangle[2].y = points[1].y;
			m_CanvasDC.SetBrush(wxBrush(m_Series[angleindex].GetColour()));
			oldPen = m_CanvasDC.GetPen();
			m_CanvasDC.SetPen(wxPen(m_Series[angleindex].GetColour()));
			m_CanvasDC.DrawPolygon(3, triangle);
			m_CanvasDC.SetPen(oldPen);
			angleindex = 0;
#endif
//-----------------------------------------------------------------------
		if(m_bPaint3D) {
			for(x = 0; x <= 2 * M_PI; x += 0.05)
			{
				changeangle = false;
				if(angleindex < angles.Count())
				{
					if((double)x/(double)M_PI*(double)180 >= angles[angleindex+1]) 
					{				
						changeangle = true;				
						x = angles[angleindex+1]*M_PI/180;						
					}
				}			
				points[0].x = points[1].x;
				points[0].y = points[1].y;
				points[3].x = points[2].x;
				points[3].y = points[2].y;		
				px = (int)(w/2 * (1+cos(x+m_RotationAngle)));
				py = (int)(h/2-sin(m_Angle)*h/2*sin(x+m_RotationAngle)-1);
				points[1].x = px;
				points[1].y = py;
				points[2].x = px;
				points[2].y = (int)(py+m_Height*cos(m_Angle));
				if(w > 0) 
				{
					curColour = wxColour((unsigned char)(m_Series[angleindex].GetColour().Red()*((double)1-(double)px/(double)w)),
							(unsigned char)(m_Series[angleindex].GetColour().Green()*((double)1-(double)px/(double)w)),
							(unsigned char)(m_Series[angleindex].GetColour().Blue()*((double)1-(double)px/(double)w)));
					if(!m_ShowEdges) {
						m_CanvasDC.SetPen(wxPen(curColour));	
					}
					m_CanvasDC.SetBrush(wxBrush(curColour));			
				}		
				if(sin(x+m_RotationAngle)<0 && sin(x-0.05+m_RotationAngle)<=0 && x > 0)
				{
					m_CanvasDC.DrawPolygon(4, points);
				}
				if(changeangle) 			
				{			
					angleindex += 1;			
				}
			}//for(x = 0; x <= 2 * M_PI; x += 0.05)
			
			x = 2 * M_PI;		
			points[0].x = points[1].x;
			points[0].y = points[1].y;
			points[3].x = points[2].x;
			points[3].y = points[2].y;
			px = (int)(w/2 * (1+cos(x+m_RotationAngle)));
			py = (int)(h/2-sin(m_Angle)*h/2*sin(x+m_RotationAngle)-1);
			points[1].x = px;
			points[1].y = py;
			points[2].x = px;
			points[2].y = (int)(py+m_Height*cos(m_Angle));
			if(w > 0) 
			{
				curColour = wxColour((unsigned char)(m_Series[angleindex].GetColour().Red()*((double)1-(double)px/(double)w)),
						(unsigned char)(m_Series[angleindex].GetColour().Green()*((double)1-(double)px/(double)w)),
						(unsigned char)(m_Series[angleindex].GetColour().Blue()*((double)1-(double)px/(double)w)));
				if(!m_ShowEdges) 
				{
					m_CanvasDC.SetPen(wxPen(curColour));
				}
				m_CanvasDC.SetBrush(wxBrush(curColour));			
			}	
			if(sin(x+m_RotationAngle)<0 && sin(x-0.05+m_RotationAngle)<0) {
				m_CanvasDC.DrawPolygon(4, points);			
			}
//-----------------------------------------------------------------------
#if defined(__WXMSW__) || defined(__WXMAC__)
			if(m_Angle <= M_PI/2)
			{		
				DrawParts(m_CanvasDC, 0, 0, w, h);
			} else DrawParts(m_CanvasDC, 0, (int)(m_Height*cos(m_Angle)), w,h);
#endif
		}//if(m_bPaint3D)			

		}
		
		m_CanvasDC.EndDrawing();
		m_CanRepaint = false;
	}
	pdc.Blit(0,0,w,h,&m_CanvasDC,0,0);	
	m_Legend->RecreateBackground(m_CanvasDC);			
}

void wxPieCtrl::OnPaint(wxPaintEvent & event)
{
	wxPaintDC pdc(this);	
	Draw(pdc);	
}

void wxPieCtrl::OnEraseBackground(wxEraseEvent & event)
{
}

void wxPieCtrl::Refresh(bool eraseBackground, const wxRect* rect)
{
	m_CanRepaint = true;
	wxWindow::Refresh(eraseBackground, rect);	
}

void wxPieCtrl::SetPaint3D(bool b3D) {
	m_bPaint3D = b3D;
}

bool wxPieCtrl::GetPaint3D() {
	return m_bPaint3D;
}

wxProgressPie::wxProgressPie(wxWindow * parent, wxWindowID id, double maxvalue, double value,
		wxPoint pos, wxSize sz, long style)
		:wxPieCtrl(parent, id, pos, sz, style), m_MaxValue(maxvalue), m_Value(value)
{
	GetLegend()->Hide();
	m_FilledColour = wxColour(0,0,127);
	m_UnfilledColour = *wxWHITE;
	wxPiePart part;
	part.SetColour(m_FilledColour);
	double a = min(value, m_MaxValue);
	part.SetValue(max(a, (double)0.0));
	m_Series.Add(part);
	part.SetColour(m_UnfilledColour);
	part.SetValue(max((double)0.0, (double)(m_MaxValue-part.GetValue())));
	m_Series.Add(part);
}

void wxProgressPie::SetValue(double value)
{
	m_Value = min(value, m_MaxValue);
	m_Series[0].SetValue(max(m_Value, (double)0.0));
	m_Series[1].SetValue(max((double)(m_MaxValue-m_Value), (double)0.0));
	Refresh();
}

void wxProgressPie::SetMaxValue(double value)
{
	m_MaxValue = value;
	m_Value = min(m_Value, m_MaxValue);
	m_Series[0].SetValue(max(m_Value, (double)0.0));
	m_Series[1].SetValue(max((double)(m_MaxValue-m_Value), (double)0.0));
	Refresh();
}

void wxProgressPie::SetFilledColour(wxColour colour)
{
	m_FilledColour = colour;
	m_Series[0].SetColour(m_FilledColour);
	Refresh();
}

void wxProgressPie::SetUnfilledColour(wxColour colour)
{
	m_UnfilledColour = colour;
	m_Series[1].SetColour(m_UnfilledColour);
	Refresh();
}

wxColour wxProgressPie::GetFilledColour()
{
	return m_FilledColour;
}

wxColour wxProgressPie::GetUnfilledColour()
{
	return m_UnfilledColour;
}
