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
#define BOX_RIGHT	100
#define WIN_NLINES	40
#define WIN_NCOLS	102

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

// a TEXT_TABLE is a labeled rectangle containing fixed rows and columns,
// with a title at the head of each column.
// The create() args say how big the array is,
// and where the rectangle is positioned on the window
// NOTE: this assumes the window is organized as text lines
//
class TEXT_TABLE {
public:
    CButton group_box;
	TEXT_LINE lines[MAX_LINES];
	int ncols;
    int nrows;
	int nlines;     // == nrows+1 (includes title line)

	TEXT_TABLE() {}
	~TEXT_TABLE() {}
	void create(
        char* title,        // label on the rectangle
        int ncols,          // # fields per line
        int start_line,     // starting line in window
        int nrows,          // # of rows (not counting column title line)
        char** col_titles,  // column titles
        int* widths         // column widths in characters
    );
	void set_field(int row, int col, char*);     // put text in cell
	void blank_row(int row);                       // set line to blank
};
