
#include <wx/filedlg.h>
#include <wx/dc.h>
#include "sg_SkinClass.h"

class CDlgPreferences:public wxDialog
{
public:
 CDlgPreferences(wxWindow* parent, wxWindowID id = -1, const wxString& title = wxT(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("dialogBox"));
 //Skin Class
 SkinClass *appSkin;
 // Pointer control
 wxStaticText *st3c;
 wxStaticText *st4c;
 wxStaticText *st5c;
 wxStaticText *st6c;
 wxStaticText *st7c;
 wxComboBox *cmb8c;
 wxComboBox *cmb11c;
 wxComboBox *cmb12c;
 wxComboBox *cmb13c;
 wxStaticText *st14c;
 wxStaticText *st15c;
 wxTextCtrl *tx17c;
 wxStaticText *st18c;
 wxStaticText *st19c;
 wxStaticText *st22c;
 wxTextCtrl *tx23c;
 wxStaticText *st24c;
 wxComboBox *cmb25c;
 wxBitmapButton *btnSave;
 wxBitmapButton *btnCancel;
 wxStaticText *st28c;
 wxTextCtrl *tx30c;
 wxBitmapButton *btnOpen;
 wxBitmap *dlg10484fImg0;
 wxBitmap *bti26cImg1;
 wxBitmap *bti27cImg1;
 wxBitmap *btmpBtnAttProjL;
 wxBitmap fileImgBuf[4];
 virtual ~CDlgPreferences();
 void initBefore();
 void OnPreCreate();
 void InitDialog();
 void LoadSkinImages();
 wxString GetSkinPath() const { return m_SkinPath; }
 void SetSkinPath(const wxString& skinPath) { m_SkinPath = skinPath; }
 void initAfter();

 DECLARE_EVENT_TABLE()

protected:
  wxPoint m_tmppoint;
  wxSize  m_tmpsize;
  wxString m_SkinPath;
  wxPoint& SetwxPoint(long x,long y);
  wxSize& SetwxSize(long w,long h);
 void OnEraseBackground(wxEraseEvent& event);
 void OnBtnClick(wxCommandEvent& event);
 void VwXEvOnEraseBackground(wxEraseEvent& event);
 void VwXDrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap & bitMap,int opz);

//[win]end your code 
};
// end CDlgPreferences

//#endif
