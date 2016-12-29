// FigDlg2.cpp : implementation file

#include "stdafx.h"
#include "fig.h"
#include "paradef.h"
#include "FigDlg2.h"
#include <stdlib.h>
#include <winreg.h>
#include <string.h>

/////////////////////////////////////////////////////////////////////////////
// CFigDlg2 dialog


CFigDlg2::CFigDlg2(CWnd* pParent /*=NULL*/)
	: CDialog(CFigDlg2::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFigDlg2)
	m_x121addr ="0";
	m_ipaddr1 = 0;
	m_ipaddr2 = 0;
	m_ipaddr3 = 0;
	m_ipaddr4 = 0;
	
	//}}AFX_DATA_INIT

	m_list=CString(' ');
	GetRegistryText(m_list);
	//AfxMessageBox(m_list,MB_OK);
	m_bChanged=FALSE;
}



void CFigDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFigDlg2)
	DDX_Text(pDX, ID_X121ADDR, m_x121addr);
	DDV_MaxChars(pDX, m_x121addr, 16);
	DDX_Text(pDX, IDC_IPADDR_1, m_ipaddr1);
	DDV_MinMaxInt(pDX, m_ipaddr1, 0, 255);
	DDX_Text(pDX, IDC_IPADDR_2, m_ipaddr2);
	DDV_MinMaxInt(pDX, m_ipaddr2, 0, 255);
	DDX_Text(pDX, IDC_IPADDR_3, m_ipaddr3);
	DDV_MinMaxInt(pDX, m_ipaddr3, 0, 255);
	DDX_Text(pDX, IDC_IPADDR_4, m_ipaddr4);
	DDV_MinMaxInt(pDX, m_ipaddr4, 0, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFigDlg2, CDialog)
	//{{AFX_MSG_MAP(CFigDlg2)
	ON_BN_CLICKED(IDC_BTN_ADD, OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DELETE, OnBtnDelete)
	ON_BN_CLICKED(ID_MYOK, OnMyok)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFigDlg2 message handlers
BOOL CFigDlg2::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	DisplayEntryInListBox();
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CFigDlg2::OnBtnAdd() 
{
	if( !UpdateData(TRUE) )return;
 
 if( !CheckX121Addr(m_x121addr) )
 {
	// AfxMessageBox("Please input the  correct x121 address.",MB_OK);
	 return;
 }
  //modify the string to have no chars
 CString entry;
 GetIPAddress(entry);
   
 CListBox* pLB=(CListBox*)GetDlgItem(IDC_LIST);
 if(pLB->FindStringExact(-1,entry)!=LB_ERR)
 {//has been existing in the list
	 AfxMessageBox("this pair of IP-X121 adress has been existing",MB_OK);
	 return ;
 }
 pLB->InsertString(-1,entry);
 m_bChanged=TRUE;

	// TODO: Add your control notification handler code here
	
}

void CFigDlg2::OnBtnDelete() 
{
	
  CListBox* pBox=(CListBox*)GetDlgItem(IDC_LIST);
  pBox->DeleteString(pBox->GetCurSel() ); 
  m_bChanged=TRUE;
	// TODO: Add your control notification handler code here
	
}

void CFigDlg2::OnMyok() 
{
    CWaitCursor wait;
	if(m_bChanged)
	{
        UpdateData(TRUE);
		RegistryAdd();
		//ModifyRegistry();//according to listbox
	}
	CDialog::OnOK();
	// TODO: Add your control notification handler code here
	
}

void CFigDlg2::OnOK() 
{}

void CFigDlg2::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

///////////////////////////
BOOL CFigDlg2::CheckX121Addr(CString&  str)
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
		AfxMessageBox("The x121 address is empty",MB_OK);
		return FALSE;
	}
	if(eg.GetLength()>16)
	{
		AfxMessageBox("The x121 address length should NOT overpass 16",MB_OK);
		return FALSE;
	}
    if(eg=="0")
	{
		AfxMessageBox("The x121 address cannot be 0",MB_OK);
		return FALSE;
	}

	str=eg;
	return TRUE;
}

BOOL CFigDlg2::GetIPAddress(CString& str)
{
 CString eg;         
 char ip1[3],ip2[3],ip3[3],ip4[3];
 itoa(m_ipaddr1,ip1,10);
 itoa(m_ipaddr2,ip2,10);
 itoa(m_ipaddr3,ip3,10);
 itoa(m_ipaddr4,ip4,10);
   
 eg=ip1;eg+=".";eg+=ip2;eg+=".";eg+=ip3;eg+=".";eg+=ip4;
 //eg is ip address
 char buf[100];
 wsprintf(buf,"%-20s           %-25s",eg,m_x121addr);
 str=CString(buf);
  
 return TRUE;
}

////////////
BOOL CFigDlg2::RegistryAdd(  )
{

	HKEY  hKey,hKey2;
    CString subpath=CString(m_netcardname);
    	 
	DWORD Disposition;
	DWORD  retcode;

	retcode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		                  (LPCTSTR)subpath,
						  0,
						  KEY_ALL_ACCESS,
						  &hKey
						  );
	if(retcode!=ERROR_SUCCESS)
	{
	
	  RegCloseKey(hKey);
	  retcode=RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                      (LPCTSTR)subpath,
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
	}


////////////////////////////////
//////MPCC95\PAREMETERS ARGUMENT
    subpath=CString(PARAMETERS);//"Tcpip";
	retcode = RegOpenKeyEx(hKey,
		                  (LPCTSTR)subpath,
						  0,
						  KEY_ALL_ACCESS,
						  &hKey2
						  );

	if(retcode!=ERROR_SUCCESS)
	{    
	  RegCloseKey(hKey2);
	  retcode=RegCreateKeyEx(hKey,
		                   (LPCTSTR)subpath,
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
    }

    CString tt;
	CombineListBoxText(tt);
	//AfxMessageBox(tt,MB_OK);
	retcode=RegSetValueEx(hKey2,
		          ADDR_PAIR,
				  0,
				  REG_SZ,
				  (BYTE*) (LPCTSTR)tt,
				  tt.GetLength()+1
				  );
    if(retcode!=ERROR_SUCCESS){
		AfxMessageBox("create error linkclk",MB_OK);
		return FALSE;}


    
///////MPCC95\PARAMETERS  END
////////////////////
    
	//RegCloseKey(hKey2);
	//RegCloseKey(hKey);
	RegCloseKey(HKEY_LOCAL_MACHINE);
	return TRUE;
}

BOOL  CFigDlg2::CombineListBoxText(CString& tt)
{
    CListBox* pBox=(CListBox*)GetDlgItem(IDC_LIST);
	int ItemCount=pBox->GetCount();
	
	
	CString entry;
	tt="";
	//char aa[100];
		
	for (int i=0;i<ItemCount;i++)
	{
		pBox->GetText(i,entry);
	 
		CString ip,x121;
		ip=entry.Left(entry.Find(' ') );
		ip.TrimRight();ip.TrimLeft();
		x121=entry.Right(entry.GetLength()-entry.Find(' ') );
		x121.TrimLeft();x121.TrimRight();
	 
	    CString bb(' ',2),cc('+');
		tt+=cc+ip+bb+x121;//strcat(tt,ent);
		          //this has resolve the bug
		          //"cc95".remind!

		//AfxMessageBox(ip,MB_OK);
	}
 
	return TRUE;
}

BOOL  CFigDlg2::DisplayEntryInListBox( )
{
	CString text=m_list;
	
	CListBox* pList=(CListBox*)GetDlgItem(IDC_LIST);
	CString ip_addr,x121_addr;
    while(!text.IsEmpty())
	{
        int i;
		if( (i=text.Find('+') )==-1){
			break;
		}
		
		text=text.Right(text.GetLength()-i-1);
		text.TrimLeft();
        i=text.Find(' ');
		ip_addr=text.Left(i);
		ip_addr.TrimRight();

		text=text.Right(text.GetLength()-i);
		text.TrimLeft();

		i=text.Find('+');
		if(i==-1)  
		{
			x121_addr=text;
			//end
		}
		else  
		{
			  x121_addr=text.Left(i);
			  x121_addr.TrimRight();

			  text.Right(text.GetLength()-i+1);
			  
		}
        //get the tcp/ip information from registry,and insert
	    // to Listbox
	    ////////
	    CString bb(' ',20),entry;
		entry=ip_addr+bb+x121_addr;
		
		pList->InsertString(-1,entry);
	}
	
	return TRUE;
}

BOOL    CFigDlg2::GetRegistryText(CString& text)
{

	HKEY hkey,hkey2;
	DWORD retcode;
	char* subpath=m_netcardname;
		
	text=CString(' ');
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

	char tt[1024];
	iDataSize=1024;
    dwType=REG_SZ;
	retcode=RegQueryValueEx(hkey2,
		            ADDR_PAIR,
					0,
					&dwType,
					(BYTE*)tt,
					&iDataSize
					);
	if(retcode!=ERROR_SUCCESS) return FALSE;//"CLK_INT"
	
	text=CString( tt);
	//AfxMessageBox(text,MB_OK);
    //text=CString(tt);
	
    //AfxMessageBox(tt,MB_OK);
	RegCloseKey(hkey);
	RegCloseKey(hkey2);
	RegCloseKey(HKEY_LOCAL_MACHINE);					
    return TRUE;
}
