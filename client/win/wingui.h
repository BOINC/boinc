class CMyApp : public CWinApp {
public:
    virtual BOOL InitInstance ();
};

class CMainWindow : public CWnd {
private:

public:
    CMainWindow ();
	static void CALLBACK TimerProc(HWND h, UINT n, UINT id, DWORD time);
protected:
    virtual void PostNcDestroy ();
    afx_msg int OnCreate (LPCREATESTRUCT);
	afx_msg void OnCloseMenu();
	afx_msg void OnLoginMenu();
    DECLARE_MESSAGE_MAP ()
};

#define MAX_FIELDS	10
#define MAX_LINES	10
#define BOX_LEFT	2
#define TEXT_LEFT	4
#define BOX_RIGHT	80
#define WIN_NLINES	40
#define WIN_NCOLS	82

class CLoginDialog: public CDialog {
public:
	OnInitDialog();
	CLoginDialog(UINT);
	CString url, auth;
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};

struct TEXT_LINE {
	CStatic fields[MAX_FIELDS];
	void set_field(int field, char*);
};

class TEXT_TABLE {
public:
    CButton group_box;
	TEXT_LINE lines[MAX_LINES];
	int nfields;
	int nlines;

	TEXT_TABLE() {}
	~TEXT_TABLE() {}
	void create(char* title, int nfields, int start_line, int nl, char** titles, int* widths);
	void set_field(int line, int field, char*);
	void blank_line(int line);
};
