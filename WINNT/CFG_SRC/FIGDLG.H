// figDlg.h : header file
// CFigDlg dialog

class CFigDlg : public CDialog
{
public:
	CFigDlg(CWnd* pParent = NULL);	

// Dialog Data
	//{{AFX_DATA(CFigDlg)
	enum { IDD = IDD_HUI_DIALOG };
	int		m_baudrate;
	int		m_linkclk;
	int		m_workmode;
	CString	m_x121addr;
	//}}AFX_DATA
    char m_netcardname[100];
protected:
	//for registry operations
    BOOL   RegistryAdd(int ,char*,char*,CString);
	BOOL   GetValueFromRegistry(int& ,int&,int&,CString&);
	BOOL   CheckX121Addr(CString&  str);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CFigDlg)
	afx_msg void OnAdvanced();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
