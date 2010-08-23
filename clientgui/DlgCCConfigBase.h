///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DlgCCConfigBase__
#define __DlgCCConfigBase__

#include <wx/sizer.h>
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/notebook.h>
#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DlgCCConfigBase
///////////////////////////////////////////////////////////////////////////////
class DlgCCConfigBase : public wxDialog 
{
	private:
	
	protected:
		wxNotebook* m_notebook;
		wxPanel* m_panelLogging;
		wxPanel* m_panelOptions;
		wxStdDialogButtonSizer* m_sdbSizerButtons;
		wxButton* m_sdbSizerButtonsOK;
		wxButton* m_sdbSizerButtonsCancel;
	
	public:
		DlgCCConfigBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("BOINC - Core Client Configuration"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 627,552 ), long style = wxDEFAULT_DIALOG_STYLE );
		~DlgCCConfigBase();
	
};

#endif //__DlgCCConfigBase__
