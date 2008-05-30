///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DlgItemPropertiesBase__
#define __DlgItemPropertiesBase__

#include <wx/intl.h>

#include <wx/html/htmlwin.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

#define ID_DEFAULT wxID_ANY // Default

///////////////////////////////////////////////////////////////////////////////
/// Class CDlgItemPropertiesBase
///////////////////////////////////////////////////////////////////////////////
class CDlgItemPropertiesBase : public wxDialog 
{
	private:
	
	protected:
		wxHtmlWindow* m_html;
		wxButton* m_btnClose;
	
	public:
		CDlgItemPropertiesBase( wxWindow* parent, wxWindowID id = ID_DEFAULT, const wxString& title = _("BOINC Manager - Item Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 503,480 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		~CDlgItemPropertiesBase();
	
};

#endif //__DlgItemPropertiesBase__
