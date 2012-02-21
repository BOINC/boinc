///////////////////////////////////////////////////////////////////////////////
// Name:        wizard.h
// Purpose:     wxWizard class: a GUI control presenting the user with a
//              sequence of dialogs which allows to simply perform some task
// Author:      Vadim Zeitlin (partly based on work by Ron Kuris and Kevin B.
//              Smith)
// Modified by: Robert Cavanaugh
//              Added capability to use .WXR resource files in Wizard pages
//              Added wxWIZARD_HELP event
//              Robert Vazan (sizers)
// Created:     15.08.99
// RCS-ID:      $Id$
// Copyright:   (c) 1999 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WIZARDEX_H_
#define _WX_WIZARDEX_H_

// ----------------------------------------------------------------------------
// headers and other simple declarations
// ----------------------------------------------------------------------------

// forward declarations
class WXDLLIMPEXP_ADV wxWizardEx;
class WXDLLIMPEXP_ADV wxWizardExEvent;
class WXDLLIMPEXP_ADV wxWizardExSizer;

// ----------------------------------------------------------------------------
// wxWizardPage is one of the wizards screen: it must know what are the
// following and preceding pages (which may be NULL for the first/last page).
//
// Other than GetNext/Prev() functions, wxWizardPage is just a panel and may be
// used as such (i.e. controls may be placed directly on it &c).
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_ADV wxWizardPageEx : public wxPanel
{
public:
    wxWizardPageEx() { Init(); }

    // ctor accepts an optional bitmap which will be used for this page instead
    // of the default one for this wizard (should be of the same size). Notice
    // that no other parameters are needed because the wizard will resize and
    // reposition the page anyhow
    wxWizardPageEx(
                 wxWizardEx *parent,
                 int id = wxID_ANY,
                 const wxChar* resource = NULL
    );

    virtual bool Create(
                wxWizardEx *parent,
                int id = wxID_ANY,
                const wxChar* resource = NULL
    );

    // these functions are used by the wizard to show another page when the
    // user chooses "Back" or "Next" button
    virtual wxWizardPageEx *GetPrev() const = 0;
    virtual wxWizardPageEx *GetNext() const = 0;

#if wxUSE_VALIDATOR
    /// Override the base functions to allow a validator to be assigned to this page.
    bool TransferDataToWindow()
    {
        return GetValidator() ? GetValidator()->TransferToWindow() 
                              : wxPanel::TransferDataToWindow();
    }
    bool TransferDataFromWindow()
    {
        return GetValidator() ? GetValidator()->TransferFromWindow() 
                              : wxPanel::TransferDataFromWindow();
    }
    bool Validate()
    {
        return GetValidator() ? GetValidator()->Validate(this) 
                              : wxPanel::Validate();
    }
#endif // wxUSE_VALIDATOR

protected:
    // common part of ctors:
    void Init();

private:
    DECLARE_DYNAMIC_CLASS_NO_COPY(wxWizardPageEx)
};


// ----------------------------------------------------------------------------
// wxWizard
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_ADV wxWizardBaseEx : public wxDialog
{
public:
    /*
       The derived class (i.e. the real wxWizard) has a ctor and Create()
       function taking the following arguments:

        wxWizard(wxWindow *parent,
                 int id = wxID_ANY,
                 const wxString& title = wxEmptyString,
                 const wxBitmap& bitmap = wxNullBitmap,
                 const wxPoint& pos = wxDefaultPosition,
                 long style = wxDEFAULT_DIALOG_STYLE);
    */
    wxWizardBaseEx() { }

    // executes the wizard starting from the given page, returns true if it was
    // successfully finished, false if user cancelled it
    virtual bool RunWizard(wxWizardPageEx *firstPage) = 0;

    // get the current page (NULL if RunWizard() isn't running)
    virtual wxWizardPageEx *GetCurrentPage() const = 0;

    // set the min size which should be available for the pages: a
    // wizard will take into account the size of the bitmap (if any)
    // itself and will never be less than some predefined fixed size
    virtual void SetPageSize(const wxSize& size) = 0;

    // get the size available for the page
    virtual wxSize GetPageSize() const = 0;

    // set the best size for the wizard, i.e. make it big enough to contain all
    // of the pages starting from the given one
    //
    // this function may be called several times and possible with different
    // pages in which case it will only increase the page size if needed (this
    // may be useful if not all pages are accessible from the first one by
    // default)
    virtual void FitToPage(const wxWizardPageEx *firstPage) = 0;

    // Adding pages to page area sizer enlarges wizard
    virtual wxSizer *GetPageAreaSizer() const = 0;

    // Set border around page area. Default is 0 if you add at least one
    // page to GetPageAreaSizer and 5 if you don't.
    virtual void SetBorder(int border) = 0;

    // the methods below may be overridden by the derived classes to provide
    // custom logic for determining the pages order

    virtual bool HasNextPage(wxWizardPageEx *page)
        { return page->GetNext() != NULL; }

    virtual bool HasPrevPage(wxWizardPageEx *page)
        { return page->GetPrev() != NULL; }

    /// Override these functions to stop InitDialog from calling TransferDataToWindow
    /// for _all_ pages when the wizard starts. Instead 'ShowPage' will call
    /// TransferDataToWindow for the first page only.
    bool TransferDataToWindow() { return true; }
    bool TransferDataFromWindow() { return true; }
    bool Validate() { return true; }

private:
    DECLARE_NO_COPY_CLASS(wxWizardBaseEx)
};


class WXDLLIMPEXP_ADV wxWizardEx : public wxWizardBaseEx
{
public:
    // ctor
    wxWizardEx() { Init(); }
    wxWizardEx(wxWindow *parent,
             int id = wxID_ANY,
             const wxString& title = wxEmptyString,
             const wxPoint& pos = wxDefaultPosition,
             long style = wxDEFAULT_DIALOG_STYLE)
    {
        Init();
        Create(parent, id, title, pos, style);
    }

    bool Create(wxWindow *parent,
             int id = wxID_ANY,
             const wxString& title = wxEmptyString,
             const wxPoint& pos = wxDefaultPosition,
             long style = wxDEFAULT_DIALOG_STYLE);
    void Init();

    // implement base class pure virtuals
    virtual bool RunWizard(wxWizardPageEx *firstPage);
    virtual wxWizardPageEx *GetCurrentPage() const;
    virtual void SetPageSize(const wxSize& size);
    virtual wxSize GetPageSize() const;
    virtual void FitToPage(const wxWizardPageEx *firstPage);
    virtual wxSizer *GetPageAreaSizer() const;
    virtual void SetBorder(int border);

    // implementation only from now on
    // -------------------------------

    // is the wizard running?
    bool IsRunning() const { return m_page != NULL; }

    // show the prev/next page, but call TransferDataFromWindow on the current
    // page first and return false without changing the page if
    // TransferDataFromWindow() returns false - otherwise, returns true
    bool ShowPage(wxWizardPageEx *page, bool goingForward = true);

    // do fill the dialog with controls
    // this is app-overridable to, for example, set help and tooltip text
    virtual void DoCreateControls();

private:
    // was the dialog really created?
    bool WasCreated() const { return m_btnPrev != NULL; }

    // event handlers
    void OnCancel(wxCommandEvent& event);
    void OnBackOrNext(wxCommandEvent& event);
    void OnHelp(wxCommandEvent& event);

    void OnWizEvent(wxWizardExEvent& event);

    void AddBitmapRow(wxBoxSizer *mainColumn);
    void AddStaticLine(wxBoxSizer *mainColumn);
    void AddBackNextPair(wxBoxSizer *buttonRow);
    void AddButtonRow(wxBoxSizer *mainColumn);

    // the page size requested by user
    wxSize m_sizePage;

    // the dialog position from the ctor
    wxPoint m_posWizard;

    // wizard state
    wxWizardPageEx *m_page;       // the current page or NULL

    // wizard controls
protected:
    wxButton    *m_btnPrev,     // the "<Back" button
                *m_btnNext,     // the "Next>" or "Finish" button
                *m_btnCancel;   // the "Cancel" button
private:
    wxStaticBitmap *m_statbmp;  // the control for the bitmap

    // Whether user called SetBorder()
    bool m_calledSetBorder;
    // Border around page area sizer requested using SetBorder()
    int m_border;

    // Whether RunWizard() was called
    bool m_started;

    // Whether was modal (modeless has to be destroyed on finish or cancel)
    bool m_wasModal;

    // True if pages are laid out using the sizer
    bool m_usingSizer;

    // Page area sizer will be inserted here with padding
    wxBoxSizer *m_sizerBmpAndPage;

    // Actual position and size of pages
    wxWizardExSizer *m_sizerPage;

    friend class wxWizardExSizer;

    DECLARE_DYNAMIC_CLASS(wxWizardEx)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxWizardEx)
};

// ----------------------------------------------------------------------------
// wxWizardEvent class represents an event generated by the wizard: this event
// is first sent to the page itself and, if not processed there, goes up the
// window hierarchy as usual
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_ADV wxWizardExEvent : public wxNotifyEvent
{
public:
    wxWizardExEvent(wxEventType type = wxEVT_NULL,
                  int id = wxID_ANY,
                  bool direction = true,
                  wxWizardPageEx* page = NULL);

    // for EVT_WIZARD_PAGE_CHANGING, return true if we're going forward or
    // false otherwise and for EVT_WIZARD_PAGE_CHANGED return true if we came
    // from the previous page and false if we returned from the next one
    // (this function doesn't make sense for CANCEL events)
    bool GetDirection() const { return m_direction; }

    wxWizardPageEx*   GetPage() const { return m_page; }

private:
    bool m_direction;
    wxWizardPageEx*    m_page;

    DECLARE_DYNAMIC_CLASS(wxWizardExEvent)
    DECLARE_NO_COPY_CLASS(wxWizardExEvent)
};

// ----------------------------------------------------------------------------
// macros for handling wxWizardExEvents
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_ADV, wxEVT_WIZARDEX_PAGE_CHANGED, 50000)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_ADV, wxEVT_WIZARDEX_PAGE_CHANGING, 50001)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_ADV, wxEVT_WIZARDEX_CANCEL, 50002)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_ADV, wxEVT_WIZARDEX_HELP, 50003)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_ADV, wxEVT_WIZARDEX_FINISHED, 50004)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*wxWizardExEventFunction)(wxWizardExEvent&);

#define wxWizardExEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxWizardExEventFunction, &func)

#define wx__DECLARE_WIZARDEXEVT(evt, id, fn) \
    wx__DECLARE_EVT1(wxEVT_WIZARDEX_ ## evt, id, wxWizardExEventHandler(fn))

// notifies that the page has just been changed (can't be vetoed)
#define EVT_WIZARDEX_PAGE_CHANGED(id, fn) wx__DECLARE_WIZARDEXEVT(PAGE_CHANGED, id, fn)

// the user pressed "<Back" or "Next>" button and the page is going to be
// changed - unless the event handler vetoes the event
#define EVT_WIZARDEX_PAGE_CHANGING(id, fn) wx__DECLARE_WIZARDEXEVT(PAGE_CHANGING, id, fn)

// the user pressed "Cancel" button and the wizard is going to be dismissed -
// unless the event handler vetoes the event
#define EVT_WIZARDEX_CANCEL(id, fn) wx__DECLARE_WIZARDEXEVT(CANCEL, id, fn)

// the user pressed "Finish" button and the wizard is going to be dismissed -
#define EVT_WIZARDEX_FINISHED(id, fn) wx__DECLARE_WIZARDEXEVT(FINISHED, id, fn)

// the user pressed "Help" button
#define EVT_WIZARDEX_HELP(id, fn) wx__DECLARE_WIZARDEXEVT(HELP, id, fn)


#endif // _WX_WIZARDEX_H_
