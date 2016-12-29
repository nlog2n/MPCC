// FigDlg2.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CFigDlg2 dialog
#ifndef NETCARDNAME
#define NETCARDNAME "mpcc95"
#endif

class CFigDlg2 : public CDialog
{

public:
	CFigDlg2(CWnd* pParent = NULL); 

// Dialog Data
	//{{AFX_DATA(CFigDlg2)
	enum { IDD = IDD_IPREAD_DIALOG };
	CString	m_x121addr;
	int		m_ipaddr1;
	int		m_ipaddr2;
	int		m_ipaddr3;
	int		m_ipaddr4;
	//}}AFX_DATA
	char    m_netcardname[100];
private:
	BOOL   m_bChanged;
	CString  m_list;
private:
    
	BOOL  CheckX121Addr(CString& );
	BOOL  GetIPAddress(CString&);

	BOOL  RegistryAdd();
	BOOL  DisplayEntryInListBox();
	BOOL  CombineListBoxText(CString&);
	BOOL  GetRegistryText(CString&);
	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFigDlg2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFigDlg2)
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnDelete();
	afx_msg void OnMyok();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
