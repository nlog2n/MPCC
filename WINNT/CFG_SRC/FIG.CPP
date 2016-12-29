// fig.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "fig.h"
#include <string.h>
#include "figDlg.h"
#include "paradef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFigApp

BEGIN_MESSAGE_MAP(CFigApp, CWinApp)
	//{{AFX_MSG_MAP(CFigApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFigApp construction

CFigApp::CFigApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CFigApp object

CFigApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CFigApp initialization


BOOL CFigApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CFigDlg dlg;
	m_pMainWnd = &dlg;

    strcpy(dlg.m_netcardname,"");
	if(m_lpCmdLine[0]=='\0')
	{ 
	sprintf(dlg.m_netcardname,"%s1",NET_CARD_PATH);
	//strcat(dlg.m_netcardname,NET_CARD_PATH);
	}
	else
	{
		//char tt[100];
		//AfxMessageBox(m_lpCmdLine,MB_OK);
sprintf(dlg.m_netcardname,"%s%s",NET_CARD_PATH,m_lpCmdLine);
		//strcat(dlg.m_netcardname,NET_CARD_PATH);
		//strcat(dlg.m_netcardname,m_lpCmdLine);//digit
		//strcat(dlg.m_netcardname,"1");
	}


	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
    
	
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
