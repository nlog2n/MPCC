// figDlg.cpp : implementation file
//

#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include "fig.h"
#include "paradef.h"
#include "figDlg.h"
#include "figdlg2.h"
#include <winreg.h>

/////////////////////////////////////////////////////////////////////////////
// CFigDlg dialog

CFigDlg::CFigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFigDlg)
	m_baudrate = 0;
	m_linkclk = 0;
	m_workmode = 0;
	m_x121addr = "0";
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32

    GetValueFromRegistry(m_baudrate,
		                 m_workmode,
						 m_linkclk,
						 m_x121addr
						 );

}

void CFigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFigDlg)
	DDX_Radio(pDX, IDC_BAUDRATE, m_baudrate);
	DDX_Radio(pDX, IDC_LINKCLK, m_linkclk);
	DDX_Radio(pDX, IDC_WORKMODE, m_workmode);
	DDX_Text(pDX, IDC_X121ADDR, m_x121addr);
	DDV_MaxChars(pDX, m_x121addr, 16);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFigDlg, CDialog)
	//{{AFX_MSG_MAP(CFigDlg)
	ON_BN_CLICKED(ID_ADVANCED, OnAdvanced)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFigDlg message handlers

void CFigDlg::OnAdvanced() 
{
	CFigDlg2  dlg;
	//strcpy(dlg.m_netcardname,"");
	strcpy(dlg.m_netcardname,m_netcardname);
	//dlg.m_netcardname=m_netcardname;
	dlg.DoModal();
	// TODO: Add your control notification handler code here
	
}

void CFigDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CFigDlg::OnOK() 
{
	// TODO: Add extra validation here
	CWaitCursor wait;
	UpdateData(TRUE);//enbedded the variable a value
	                 //they are:m_baudrate,
	                 //m_workmode,m_linkclk,
	                 //m_x121addr.

	int baudrate;
	switch(m_baudrate){
	  case 0:
		   baudrate=9600;
		   break;
      case 1:
           baudrate=19200;
		   break;
      case 2:
		   baudrate=38400;
		   break;
	  case 3:
		   baudrate=64000;
		   break;
	  default:
		   baudrate=9600;
		   break;}
	
	char* workmode;
	switch(m_workmode){
	case 0:
           workmode="DEV_DTE";
		   break;
	case 1:
		   workmode="DEV_DCE";
		   break;
	default:workmode="DEV_DTE";
		    break;}
	
	char* linkclk;
	switch(m_linkclk){
	case 0:linkclk="CLK_INT";break;
	case 1:linkclk="CLK_EXT";break;
	default:linkclk="CLK_INT";break;}

    //DWORD x121addr=0;
	//if(m_x121addr!="") x121addr=atol(m_x121addr); 
 	//check the tel-code
	if(!CheckX121Addr(m_x121addr)) return;

    int res=RegistryAdd(baudrate,
		                workmode,
				        linkclk,
						m_x121addr
                        );
    if(res==(int)FALSE) return ;

	CDialog::OnOK();
}

BOOL CFigDlg::CheckX121Addr(CString&  str)
{
	CString eg="";
	for (int i=0;i<str.GetLength();i++)
	{
		char ch=str.GetAt(i);
		if( ch<='9'&&ch>='0' )
		{
			eg+=ch;
		}
	}
	/*
	while(eg!="" &&
		  eg.GetAt(0)=='0')eg=eg.Right(eg.GetLength()-1 );
    */
	if(eg=="")
	{
	  eg="0";	
	}
	if(eg.GetLength()>16)
	{
		AfxMessageBox("The x121 address length should NOT overpass 16",MB_OK);
		return FALSE;
	}
    
	str=CString(eg);
	return TRUE;
}


BOOL CFigDlg::RegistryAdd(int baudrate,
						  char* workmode,
						  char* linkclk,
						  CString x121addr
						  )
{

    HKEY  hKey,hKey2;
    char* subpath=m_netcardname;
	DWORD  Disposition;
	
    DWORD  retcode=RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                      subpath,
					  0,
					  "",
					  REG_OPTION_NON_VOLATILE,
					  KEY_ALL_ACCESS,
					  NULL,
					  &hKey,
					  &Disposition
                       );
	
      if(retcode!=ERROR_SUCCESS){
	    char teext[80];
		if(retcode==ERROR_ACCESS_DENIED)
	      wsprintf(teext,"exist a error,may be security reasons");
        else wsprintf(teext,"failed for unknowned reasons");
		
		AfxMessageBox(teext,MB_OK);
        return FALSE;
       }
	
////////////////////////////////
//////MPCC95\PAREMETERS ARGUMENT
    char *subpath2=PARAMETERS;//"Parameters";
	retcode=RegCreateKeyEx(hKey,
		                   subpath2,
						   0,
						   "",
						   REG_OPTION_NON_VOLATILE,
						   KEY_ALL_ACCESS,
						   NULL,
						   &hKey2,
						   &Disposition
						   );
	  if(retcode!=ERROR_SUCCESS){
		AfxMessageBox("create error2",MB_OK);
		return FALSE;}
	

    
	retcode=RegSetValueEx(hKey2,
		          BAUDRATE,//for Baud rate
				  0,
                  REG_DWORD,
				  (BYTE*)(&baudrate),
				  sizeof(int)
                  );
    if(retcode!=ERROR_SUCCESS){
		AfxMessageBox("create error baudrate",MB_OK);
		return FALSE;}
    int k=2;//nonsense 
	retcode=RegSetValueEx(hKey2,
		          LINKCLK,
				  0,
				  REG_SZ,
				  (BYTE*) linkclk,
				  strlen(linkclk)+1
				  );
    if(retcode!=ERROR_SUCCESS){
		AfxMessageBox("create error linkclk",MB_OK);
		return FALSE;}


    retcode=RegSetValueEx(hKey2,
		          LINKDEV,
				  0,
				  REG_SZ,
				  (BYTE*)workmode,
				  strlen(workmode)+1
				  );
	if(retcode!=ERROR_SUCCESS){
		AfxMessageBox("create error linkdev",MB_OK);
		return FALSE;}


	retcode=RegSetValueEx(hKey2,
		          X121ADDR,
				  0,
				  REG_SZ,
				  (BYTE*)(LPCTSTR)x121addr,
				  x121addr.GetLength()+1
				  );
	if(retcode!=ERROR_SUCCESS){
		AfxMessageBox("create error x121 addr",MB_OK);
		return FALSE;}



///////MPCC95\PARAMETERS  END
////////////////////

	
	RegCloseKey(HKEY_LOCAL_MACHINE);
	return TRUE;
}


BOOL CFigDlg::GetValueFromRegistry(int& baudrate_id,
						           int& workmode_id,
						           int& linkclk_id,
						           CString& x121addr
								   )
{

	HKEY hkey,hkey2;
	DWORD retcode;
	char* subpath=m_netcardname;
	retcode=RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		              subpath,
			          0,
			          KEY_ALL_ACCESS,
			          &hkey
			         );
	if(retcode!=ERROR_SUCCESS)return FALSE;

	retcode=RegOpenKeyEx(hkey,
		              PARAMETERS,
					  0,
					  KEY_ALL_ACCESS,
                      &hkey2
					  );
	if(retcode!=ERROR_SUCCESS)return FALSE;

	DWORD dwType;
	unsigned long iDataSize;

	int baudrate;//default
	iDataSize=sizeof(int);
	dwType=REG_DWORD;
	retcode=RegQueryValueEx(hkey2,
		            BAUDRATE,
					0,
					&dwType,
                    (BYTE*)&baudrate,
					&iDataSize
					);
	if(retcode!=ERROR_SUCCESS) baudrate_id=0;
	else
    	switch(baudrate){
         case 9600:baudrate_id=0;break;
         case 19200:baudrate_id=1;break;
         case 38400:baudrate_id=2;break;
         case 64000:baudrate_id=3;break;
		 default:break;}

    char linkclk[20];
	iDataSize=20;
    dwType=REG_SZ;
	retcode=RegQueryValueEx(hkey2,
		            LINKCLK,
					0,
					&dwType,
					(BYTE*)linkclk,
					&iDataSize
					);
	if(retcode!=ERROR_SUCCESS) linkclk_id=0;//"CLK_INT"
	else{
		if(strcmp(linkclk,"CLK_INT")==0)
		     linkclk_id=0;
        else if(strcmp(linkclk,"CLK_EXT")==0)linkclk_id=1;
     	}
	

	char workmode[20];
	iDataSize=20;
	dwType=REG_SZ;
	retcode=RegQueryValueEx(hkey2,
		                    LINKDEV,
							0,
							&dwType,
							(BYTE*)workmode,
							&iDataSize
							);
	if(retcode!=ERROR_SUCCESS)workmode_id=0;
	else {
		if(strcmp(workmode, "DEV_DTE")==0)workmode_id=0;
		else if(strcmp(workmode,"DEV_DCE")==0)workmode_id=1;
	}
	/////////////////
    
	char addr[20];
	iDataSize=20;
	dwType=REG_SZ;
	//DWORD dwData;
	retcode=RegQueryValueEx(hkey2,
		                    X121ADDR,
							0,
							&dwType,
                            (BYTE*)addr,
							&iDataSize
							);
	if(retcode!=ERROR_SUCCESS) x121addr=CString('0');
	else{
		 x121addr=CString( addr);}

//	RegCloseKey(hkey);
//	RegCloseKey(hkey2);
    RegCloseKey(HKEY_LOCAL_MACHINE);					
    return TRUE;
}

