#include <afxcmn.h>
#include <afxtempl.h>
#include <afxcoll.h>

#define SORT_ASCEND		0
#define SORT_DESCEND	1

class CProgressBarCtrl : public CProgressCtrl
{
public:
							CProgressBarCtrl();

protected:
	afx_msg void			OnLButtonDown(UINT, CPoint);
	afx_msg void			OnLButtonUp(UINT, CPoint);
    DECLARE_MESSAGE_MAP()
};

class CProgressHeaderCtrl : public CHeaderCtrl
{
public:
							CProgressHeaderCtrl();

protected:
	afx_msg void			OnRButtonDown(UINT, CPoint);
    DECLARE_MESSAGE_MAP()
};

class CProgressListCtrl : public CListCtrl
{
public:
							CProgressListCtrl();
	void					SetItemProgress(int, int, int);
	void					RepositionProgress();
	int						InsertColumn(int, LPCTSTR, int, int, int);
	BOOL					SetColumnWidth(int, int);

protected:
	CMenu					m_PopupMenu;
	CMenu					m_Menu;
	CMapStringToOb			m_Progs;
	CProgressHeaderCtrl		m_Header;
	CArray<int,int>			m_ColWidths;
	int						m_iSort;

	void					SwapItems(int, int);
	void					Sort(int, int);
	void					SwapColumnVisibility(int);
	int						ColumnFromPopup(int);
	int						PopupFromColumn(int);

	afx_msg BOOL			OnCommand(WPARAM, LPARAM);
    afx_msg int				OnCreate(LPCREATESTRUCT);
    afx_msg void			OnDestroy();
	afx_msg BOOL			OnNotify(WPARAM, LPARAM, LRESULT*);
	afx_msg void			OnPaint();
	afx_msg void			OnRButtonDown(UINT, CPoint);
    DECLARE_MESSAGE_MAP()
};

class CMyApp : public CWinApp 
{
public:
    virtual BOOL			InitInstance();
};

class CMainWindow : public CWnd 
{
public:
    CMainWindow ();
	static void CALLBACK	TimerProc(HWND, UINT, UINT, DWORD);
	void					MessageUser(char*);

protected:
	CMenu					m_MainMenu;
	CBitmap					m_Logo;
	CProgressListCtrl		m_CtrlProjects;
	CProgressListCtrl		m_CtrlXfers;
	CProgressListCtrl		m_CtrlWorkunits;
	CEdit					m_CtrlMessages;
	CTabCtrl				m_Tabs;
	bool					m_bCreated;

    virtual void			PostNcDestroy ();

	afx_msg void			OnCommandAccountLogin();
	afx_msg void			OnCommandHelpAbout();
	afx_msg void			OnCommandFileClose();
    afx_msg int				OnCreate(LPCREATESTRUCT);
	afx_msg BOOL			OnNotify(WPARAM, LPARAM, LRESULT*);
    afx_msg void			OnPaint();
    afx_msg void			OnSize(UINT, int, int);
    DECLARE_MESSAGE_MAP()
};

class CLoginDialog : public CDialog 
{
public:
							CLoginDialog(UINT);
	afx_msg BOOL			OnInitDialog();
	CString					m_url;
	CString					m_auth;

protected:
	afx_msg void			OnOK();
	DECLARE_MESSAGE_MAP()
};

