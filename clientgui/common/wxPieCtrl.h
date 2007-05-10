/////////////////////////////////////////////////////////////////////////////
// Name:        wxPieCtrl.h
// Purpose:     wxPieCtrl (v0.1.2)
// Author:      Volodymir (T-Rex) Tryapichko
// Modified by: Frank Weiler, 10.10.2006
// Created:     June-07-2005
// RCS-ID:      $Id:
// Copyright:   (c) Volodymir (T-Rex) Tryapichko
// Licence:     wxWidgets license
/////////////////////////////////////////////////////////////////////////////
#ifndef _WX_PIE_CTRL
#define _WX_PIE_CTRL


#include <wx/wx.h>
#include <wx/image.h>
#include <wx/dynarray.h>

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

#if ! wxCHECK_VERSION(2,8,0)
WX_DECLARE_OBJARRAY(double, wxArrayDouble);
#endif

// ========================================================================
//  wxPiePart
// ------------------------------------------------------------------------
///	This class is used for storing data in wxPieCtrl.
class wxPiePart
{
	double m_Value;
	wxColour m_Colour;
	wxString m_Label;
public:
	/// Default constructor
	wxPiePart();
	/// Parametrized constructor
	/*!
		\param value used for drawing the pie. Specifies the size of the sector.
		\param colour specifies the colour of the sector
		\param label specifies the text to be shown in diagram legend
	*/
	wxPiePart(double value, wxColour colour, wxString label = wxEmptyString);
	/// Copy constructor
	wxPiePart(const wxPiePart & part);
	/// Returns the colour of sector
	wxColour GetColour() {return m_Colour;}
	/// Sets the colour of sector
	void SetColour(wxColour colour) {m_Colour = colour;}
	/// Returns the size of sector
	double GetValue() {return m_Value;}
	/// Sets the size of sector
	void SetValue(double value) {m_Value = fabs(value);}
	/// Returns the label of sector
	wxString GetLabel() {return m_Label;}
	/// Sets the label of sector
	void SetLabel(wxString value) {m_Label = value;}
	//
};

WX_DECLARE_OBJARRAY(wxPiePart, wxPieSeries);

class wxPieCtrl;

// ========================================================================
//  wxPieCtrlLegend
// ------------------------------------------------------------------------
///	The component draws a rectangle and outputs text information for
/// each sector of pie diagram
class wxPieCtrlLegend : public wxWindow
{
	wxFont m_TitleFont;
	wxFont m_LabelFont;
	bool m_IsTransparent;
	wxColour m_TitleColour;
	wxColour m_LabelColour;
	wxColour m_BackColour;
	wxBitmap m_Background;
	wxMemoryDC m_BackgroundDC;
	unsigned int m_HorBorder;
	unsigned int m_VerBorder;
	wxString m_szTitle;

protected:
        void MeasureText();
	void RecreateBackground(wxMemoryDC & parentdc);
public:
	/// Constructor
	/*!
		\param parent Pointer to a parent window
		\param title Legend window title
		\param pos Legend window position
		\param sz Legend window size
		\param style Window style
	*/
	wxPieCtrlLegend(wxPieCtrl * parent, wxString title = wxEmptyString,
		wxPoint pos = wxPoint(10,10), wxSize sz = wxDefaultSize,
		long style = 0);
	/// Returns transparency flag of legend window
	bool IsTransparent() {return m_IsTransparent;}
	/// Sets transparency flag of legend window
	void SetTransparent(bool value);
	/// Returns the font used for displaying the labels of sectors
	wxFont GetLabelFont() {return m_LabelFont;}
	/// Sets the font used for displaying the labels of sectors
	void SetLabelFont(wxFont font);
	/// Returns the size of horizontal border of legend window
	unsigned int GetHorBorder() {return m_HorBorder;}
	/// Returns the size of vertical border of legend window
	unsigned int GetVerBorder() {return m_VerBorder;}
	/// Sets the size of horizontal border of legend window
	void SetHorBorder(unsigned int value);
	/// Sets the size of vertical border of legend window
	void SetVerBorder(unsigned int value);
	/// Returns the colour used for displaying the labels of sectors
	wxColour GetLabelColour() {return m_LabelColour;}
	/// Sets the colour used for displaying the labels of sectors
	void SetLabelColour(wxColour colour);
	/// Returns the colour used for displaying lagend window background
	wxColour GetBackColour() {return m_BackColour;}
	/// Sets the colour used for displaying legend window background
	void SetBackColour(wxColour colour);

	void SetLabel(const wxString& label);

	DECLARE_EVENT_TABLE()
	void OnPaint(wxPaintEvent & event);
	void OnEraseBackground(wxEraseEvent & event);
	friend class wxPieCtrl;
};
// ========================================================================
//  wxPieCtrl
// ------------------------------------------------------------------------
///	The component for drawing pie diagrams
class wxPieCtrl : public wxWindow
{
protected:
	int m_padding;
	wxBitmap m_CanvasBitmap;
	wxMemoryDC m_CanvasDC;
	wxColour m_BackColour;
	wxPieCtrlLegend * m_Legend;
	bool m_CanRepaint;
	bool m_ShowEdges;
	int m_lastCoveredPart;
	//internal methods
	void GetPartAngles(wxArrayDouble & angles);	
	void RecreateCanvas();
	int GetCoveredPiePart(int x,int y);
	void DrawParts(wxRect& pieRect);
	void Draw(wxPaintDC & pdc);
public:
	/// An array of wxPiePart objects for storing information about sectors
	wxPieSeries m_Series;

	wxPieCtrl(wxWindow * parent, wxWindowID id = wxID_ANY, wxPoint pos = wxDefaultPosition,
		wxSize sz = wxDefaultSize, long style = 0, wxString name = wxT("wxPieCtrl"));

	wxColour GetBackColour();
	void SetBackColour(wxColour colour);

	bool GetShowEdges();
	void SetShowEdges(bool value);

	void SetPadding(int pad);
	int GetPadding();
	/// Returns the pointer of diagram legend
	wxPieCtrlLegend * GetLegend() {return m_Legend;}
	virtual void Refresh(bool eraseBackground = true, const wxRect* rect = NULL);
	

	DECLARE_EVENT_TABLE()
	void OnPaint(wxPaintEvent & event);
	void OnSize(wxSizeEvent & event);
	void OnMouseMove(wxMouseEvent& ev);
	void OnEraseBackground(wxEraseEvent & /*event*/);
};

#endif
