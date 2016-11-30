// NxTWAINLaunchPickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NxTWAINLaunchPickerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CNxTWAINLaunchPickerDlg dialog


CNxTWAINLaunchPickerDlg::CNxTWAINLaunchPickerDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNxTWAINLaunchPickerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNxTWAINLaunchPickerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_dwSelectedPID = 0;
}


void CNxTWAINLaunchPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNxTWAINLaunchPickerDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNxTWAINLaunchPickerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNxTWAINLaunchPickerDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxTWAINLaunchPickerDlg message handlers

BOOL CALLBACK NxTWAINLaunchGetWindowText( HWND hwnd, LPARAM lParam )
{
	CNxTWAINLaunchPickerDlg* pDlg = (CNxTWAINLaunchPickerDlg*)lParam;
	DWORD dwID;

	GetWindowThreadProcessId(hwnd, &dwID);

	if(dwID == (DWORD)pDlg->GetSelectedPID())
	{
		char sz[512];
		GetWindowText(hwnd, sz, 512);
		if (strstr(sz, "Nextech Practice"))
			pDlg->SetSelectedPIDWindowText(sz);
	}
	return TRUE;
}

BOOL CNxTWAINLaunchPickerDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_list = GetDlgItem(IDC_LIST_PRACAPP)->GetControlUnknown();

	for (long i=0; i < m_adwPIDs.GetSize(); i++)
	{
		long nPID = (long)m_adwPIDs[i];
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, nPID);
		if (hProcess)
		{
			IRowSettingsPtr pRow = m_list->GetRow(-1);
			FILETIME ftCreate, ftDummy;
			m_dwSelectedPID = nPID;
			GetProcessTimes(hProcess, &ftCreate, &ftDummy, &ftDummy, &ftDummy);
			COleDateTime dt = ftCreate;
			EnumWindows((WNDENUMPROC)NxTWAINLaunchGetWindowText, (LPARAM)this);
			pRow->Value[0] = nPID;
			pRow->Value[1] = _bstr_t(m_strPIDWindowText);
			pRow->Value[2] = COleVariant(dt);
			m_list->AddRow(pRow);
			CloseHandle(hProcess);
		}
	}
	m_dwSelectedPID = 0;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNxTWAINLaunchPickerDlg::SetPIDs(const CDWordArray &adwPIDs)
{
	for (long i=0; i < adwPIDs.GetSize(); i++)
	{
		m_adwPIDs.Add( adwPIDs[i] );
	}
}

DWORD CNxTWAINLaunchPickerDlg::GetSelectedPID()
{
	return m_dwSelectedPID;
}

BEGIN_EVENTSINK_MAP(CNxTWAINLaunchPickerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNxTWAINLaunchPickerDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNxTWAINLaunchPickerDlg::OnOK()
{
	if (m_list->CurSel == -1)
	{
		MsgBox("Please select a process before clicking OK");
		return;
	}
	else m_dwSelectedPID = m_list->GetValue( m_list->CurSel, 0 ).lVal;
	CDialog::OnOK();
}

void CNxTWAINLaunchPickerDlg::SetSelectedPIDWindowText(const CString& str)
{
	m_strPIDWindowText = str;
}
